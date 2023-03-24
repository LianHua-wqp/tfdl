package main

//
// 引用的C头文件需要在注释中声明，紧接着注释需要有import "C"，且这一行和注释之间不能有空格
//

/*
#cgo CFLAGS: -I .
#cgo LDFLAGS: -L . -lfly
#include "fly_uart.h"
#include "hello.h"
#include "fly_dma.h"
int callOnMeGo_cgo(int in); // 声明,cmiddle.go封装的中间件
*/
import "C"
import (
	"fmt"
	"tfdl/settings"
	"unsafe"
)

//export callOnMeGo
func callOnMeGo(in int) int {
	return in + 1
}

// CFLAGS: -I .	//在头文件的搜索路径列表中添加dir目录
// LDFLAGS: -L . -lfly	//LDFLAGS ：链接库使用的选项 –L -l (大写L指定动态库的路径，小写L指定动态库的名称),lfly
func main() {
	//使用C.CString创建的字符串需要手动释放。

	err := settings.Init("./conf/config.yaml")
	if err != nil {
		fmt.Println("read config.yaml failed", err)
		return
	}

	//使用unsafe.Pointer转换
	//(C.callback_fcn)	强转为typedef int (*callback_fcn)(int);
	//C.callOnMeGo_cgo 调用cmiddle.go封装的中间件（callOnMeGo_cgo调用的export callOnMeGo）
	C.some_c_func((C.callback_fcn)(unsafe.Pointer(C.callOnMeGo_cgo)))
	cs := C.CString(settings.Conf.Hardware.Uart)
	defer C.free(unsafe.Pointer(cs))
	C.init_rs422_uart(cs) //
	return

	// cs := C.CString("Hello World\n")
	// C.myprint(cs)

	C.init_dma_1()
	// C.init_rs422_uart() //
	// C.hello()
}
