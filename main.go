package main

//
// 引用的C头文件需要在注释中声明，紧接着注释需要有import "C"，且这一行和注释之间不能有空格
//

/*
#cgo CFLAGS: -I .
#cgo LDFLAGS: -L . -lfly
#include "fly_uart.h"
#include "hello.h"
*/
import "C"
import "fmt"

// CFLAGS: -I .	//在头文件的搜索路径列表中添加dir目录
// LDFLAGS: -L . -lfly_uart	//LDFLAGS ：链接库使用的选项 –L -l (大写L指定动态库的路径，小写L指定动态库的名称)
func main() {
	//使用C.CString创建的字符串需要手动释放。
	fmt.Println("cgo test")
	C.init_rs422_uart()
	C.hello()
}
