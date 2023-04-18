package logic

import (
	"fmt"
	"net"
	"sync/atomic"
	"tfdl/settings"

	"go.uber.org/zap"
)

var DmaDataChl chan []byte
var ConnNum uint32

func TcpServerInit() error {
	zap.L().Debug(fmt.Sprintf("tcp port %d", settings.Conf.Port))
	listen, err := net.Listen("tcp", fmt.Sprintf("0.0.0.0:%d", settings.Conf.Port))
	if err != nil {
		return err
	}

	DmaDataChl = make(chan []byte, 10)
	atomic.StoreUint32(&ConnNum, 0)

	go func() {
		for {
			conn, err := listen.Accept() // 建立连接
			if err != nil {
				fmt.Println("accept failed, err:", err)
				continue
			}
			atomic.AddUint32(&ConnNum, 1)
			go ProcessDMAData(conn) // 启动一个goroutine处理连接
		}
	}()

	return nil
}

func ProcessDMAData(conn net.Conn) {
	defer conn.Close()

	for {
		data := <-DmaDataChl
		_, err := conn.Write(data)
		if err != nil {
			break
		}
	}

	atomic.AddUint32(&ConnNum, ^uint32(0))
}

// func process(conn net.Conn) {
// 	defer conn.Close() // 关闭连接
// 	for {
// 		reader := bufio.NewReader(conn)
// 		var buf [128]byte
// 		n, err := reader.Read(buf[:]) // 读取数据
// 		if err != nil {
// 			fmt.Println("read from client failed, err:", err)
// 			break
// 		}
// 		recvStr := string(buf[:n])
// 		fmt.Println("收到client端发来的数据：", recvStr)
// 		conn.Write([]byte(recvStr)) // 发送数据
// 	}
// }
