package partcgo

//
// 引用的C头文件需要在注释中声明，紧接着注释需要有import "C"，且这一行和注释之间不能有空格
//

/*
#cgo CFLAGS: -I ../partc
#cgo LDFLAGS: -L ../ -lfly

#include "../partc/fly_bd.h"
#include "../partc/fly_gpio.h"
#include "../partc/fpga.h"
*/
import "C"
import "errors"

// CFLAGS: -I .	//在头文件的搜索路径列表中添加dir目录
// LDFLAGS: -L . -lfly	//LDFLAGS ：链接库使用的选项 –L -l (大写L指定动态库的路径，小写L指定动态库的名称),lfly
func Init() (err error) {

	//拉高GPIO读取信号，方便后续读取DMA数据
	ret := C.PullHighGpio()
	if ret < 0 {
		return errors.New("pull high gpio failed")
	}

	//初始化GPIO的接口
	ret = C.init_fpga()
	if ret < 0 {
		return errors.New("init spi failed")
	}

	ret = C.bd_open_uart()
	if ret < 0 {
		return errors.New("init bd failed")
	}

	//读取北斗串口的时间以及定位，通过SPI设置到FPGA
	go C.bd_start_task()

	err = InitDma()
	if err != nil {
		return err
	}
	return
}
