package partcgo

import (
	"errors"
	"fmt"
	"log"
	"strings"
	"tfdl/logic"
	"time"
	"unsafe"
)

/*
#include <stdlib.h>		//C.free
#include "../partc/fly_uart.h"
*/
import "C"

type Uart4G struct {
	RecvBuff [2024]byte
	RecvCont int

	tcpip   string //连接到4G模块的ip
	tcpport uint16 //

	uart_fd int
}

func NewUart4G() *Uart4G {
	return &Uart4G{RecvCont: 0} //, channel: make(chan []byte, 10)
}

func (uart *Uart4G) OpenUartPort() error {
	cs := C.CString("/dev/ttyPS0")
	defer C.free(unsafe.Pointer(cs))
	ret := C.uart_open(cs, C.int(9600))
	if ret < 0 {
		return errors.New("open uart 4g failed")
	}
	return nil
}

func (uart *Uart4G) Send(data []byte) (int, error) {
	ret := C.uart_send(C.int(uart.uart_fd), (*C.char)(unsafe.Pointer(&data)), C.int(len(data)))
	if ret > 0 {
		return int(ret), nil
	}
	return int(ret), errors.New("send data failed")
}

// RecvData 单独接收数据的线程
func (uart *Uart4G) RecvData() {
	for {
		buf := uart.RecvBuff[uart.RecvCont:]

		ret := C.uart_recv(C.int(uart.uart_fd), (*C.char)(unsafe.Pointer(&buf)), C.int(255)) //TODO
		// ret := C.uart_recv(C.int(uart.uart_fd), unsafe.Pointer(&buf), C.int(255))
		if ret > 0 {
			uart.RecvCont += int(ret)
		}
		// ret, err := uart.Port.Read(buf)
		// if err != nil {
		// 	log.Fatal(err, ret)
		// }
		// uart.RecvCont += ret
		// //fmt.Println("Recv :", ret, string(buf[0:ret-2]))
		// time.Sleep(time.Millisecond * 50)
	}
}

// func (uart *Uart4G) Recv(data []byte) (int, error) {
// 	ret := C.uart_recv(C.int(uart.uart_fd), unsafe.Pointer(&uart.RecvBuff))
// 	if ret > 0 {
// 		return ret, nil
// 	}
// 	return ret, errors.New("send data failed")
// }

func (uart *Uart4G) clearBuffer() {
	uart.RecvCont = 0
}

// SendDataAndCheck 发送数据
// interval time.Duration 等待的间隔时间
// success string 成功的返回值
// failed ...string 失败时的返回值
func (uart *Uart4G) SendDataAndCheck(data []byte, interval time.Duration, success string, failed ...string) error {
	uart.clearBuffer() // 发送前清空缓存
	_, err := uart.Send(data)
	if err != nil {
		return err
	}

	lasttime := time.Now()
FIND_ERROR:
	for {
		if uart.RecvCont > 0 {
			s := logic.Bytes2String(uart.RecvBuff[0:uart.RecvCont])
			//s := string(uart.RecvBuff[0:uart.RecvCont]) //TODO 后续优化
			if index := strings.Index(s, success); index >= 0 {
				//fmt.Println("find success")
				break
			} else {
				for _, code := range failed {
					if index = strings.Index(s, code); index >= 0 {
						err = errors.New(fmt.Sprintf("send data failed: %s", s))
						break FIND_ERROR
					}
				}
			}
		}

		if time.Now().Sub(lasttime).Seconds() >= interval.Seconds() {
			err = errors.New("send data time out")
			break
		}
		time.Sleep(time.Millisecond * 50)
	}

	uart.clearBuffer() // 发送结束清空缓存
	return err
}

// EC20_InitConfig AT测试指令
func (uart *Uart4G) EC20_InitConfig() (err error) {

	// 测试AT指令功能是否正常
	if err = uart.EC20_SendCmd(GetVerifyStruct(AT_TEST), time.Second*5, 10); err != nil {
		return
	}
	// 查询SIM卡是否正常，返回ready则表示SIM卡正常
	if err = uart.EC20_SendCmd(GetVerifyStruct(AT_CPIN), time.Second*10, 2); err != nil {
		return
	}
	// 查询模组是否注册上GSM网络
	if err = uart.EC20_SendCmd(GetVerifyStruct(AT_CREG), time.Second*10, 2); err != nil {
		return
	}
	// 查询模组是否注册上GPRS网络
	if err = uart.EC20_SendCmd(GetVerifyStruct(AT_CGREG), time.Second*10, 2); err != nil {
		return
	}
	return
}

// EC200S_InitNet 初始化网络配置，会连接到TCP服务器
func (uart *Uart4G) EC200S_InitNet(ip string, port uint16) (err error) {
	uart.tcpip = ip
	uart.tcpport = port

	//配置电信网络
	err = uart.EC20_SendCmd(GetVerifyStruct(AT_QICSGP_CTNET), time.Second*10, 2)
	if err != nil {
		return
	}

	//场景激活
	err = uart.EC20_SendCmd(GetVerifyStruct(AT_ACTIVATE_SCENE), time.Second*10, 1)
	if err != nil {
		return
	}

	//连接到网络
	err = uart.EC20_Connect()
	//fmt.Println("uart.EC20_Connect()", err)
	return
}

// EC20_SendCmd 发送EC20指令
// verify *VerifyStruct 获取CMD 成功以及失败的返回值
// interval time.Duration 等待的间隔时间
// sendNum int 失败时重发次数
func (uart *Uart4G) EC20_SendCmd(verify *VerifyStruct, interval time.Duration, sendNum int) error {
	cmd := logic.String2Bytes(verify.CMD)

	var err error
	for i := 0; i < sendNum; i++ {
		fmt.Println("send cmd:", verify.CMD)

		err = uart.SendDataAndCheck(cmd, interval, verify.Success, verify.Failed...)
		if err == nil || verify.TurnOffError {
			//无错误返回 或者关闭了错误开关直接返回
			//fmt.Println("success:", verify.CMD)
			return nil
		}
	}
	return errors.New(fmt.Sprintf("error %s %s", verify.CMD, err))
}

// EC20_Connect 连接到TCP服务器
func (uart *Uart4G) EC20_Connect() (err error) {
	verify := GetVerifyStruct(AT_OPEN_SOCKET)
	verify.CMD = fmt.Sprintf("AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,1\r\n", uart.tcpip, uart.tcpport)
	//连接到服务器
	if err = uart.EC20_SendCmd(verify, time.Second*10, 1); err == nil {
		// uart.isconnet = true
	} else {
		//断开重新进行连接 错误码563 Socket标识被占用
		uart.EC20_Close()
		err = uart.EC20_SendCmd(verify, time.Second*10, 1)
		//fmt.Println("EC20_Connect ", err)
	}

	return
}

// EC20_Send_TCP_Data EC20模块发送数据
// data []byte 待发送的数据
// interval time.Duration 等待的时间
// 发送数据失败时，会自动进行重连
func (uart *Uart4G) EC20_Send_TCP_Data(data []byte, interval time.Duration) (err error) {
	verify := GetVerifyStruct(AT_SEND_LENTH)
	verify.CMD = fmt.Sprintf("AT+QISEND=0,%d\r\n", len(data)) //拼接待发送数据的长度

	err = uart.EC20_SendCmd(verify, interval, 1)
	if err != nil {
		goto RE_CONNET
	}
	err = uart.SendDataAndCheck(data, interval, REPLY_OK, REPLY_FAILED, "closed")

RE_CONNET:
	if err != nil {
		//发送数据失败，尝试重新连接
		uart.EC20_Close()
		err = uart.EC20_Connect()
		if err != nil {
			fmt.Println("reconnect failed", err)
			return
		}
		err = uart.SendDataAndCheck(data, interval, REPLY_OK, REPLY_FAILED, "closed")
	}

	return
}

// EC20_Close 关闭当前的TCP连接
func (uart *Uart4G) EC20_Close() {
	uart.EC20_SendCmd(GetVerifyStruct(AT_CLOSE_SOCKET), time.Second*10, 1)
}

func SimpleTest(ip string, port uint16) {
	uartcfg := NewUart4G()
	err := uartcfg.OpenUartPort()
	if err != nil {
		log.Fatal("uart init failed:", err)
		return
	}
	go uartcfg.RecvData() //先开启接收线程

	err = uartcfg.EC20_InitConfig()
	if err != nil {
		fmt.Println("EC20_init failed ", err)
		return
	}

	err = uartcfg.EC200S_InitNet(ip, uint16(port))
	if err != nil {
		fmt.Println("EC200S_InitNet failed ", err)
		return
	} else {
		fmt.Println("connect success")
	}

	fmt.Println("开始发送数据，记得要加回车啊")
	go func() {
		num := 0
		var err2 error
		for {
			//uartcfg.Port.Write([]byte("AT+QISENDEX=0,\"31323334\"\r\n"))	//该命令注意加回车换行
			num++
			err2 = uartcfg.EC20_Send_TCP_Data([]byte(fmt.Sprintf("test %d", num)), time.Second*2) //该命令可不加回车换行
			if err2 != nil {
				fmt.Println("failed EC20_Send_TCP_Data ", err2)
			}
			time.Sleep(time.Millisecond * 500)
		}
	}()
}
