package partcgo

/*
#include <stdlib.h>		//C.free
#include "../partc/fly_dma.h"

void RecvDmaDataCallBack_CGO(int channel, void * data);	//dma_cmidlle.go
*/
import "C"
import (
	"errors"
	"fmt"
	"tfdl/logic"
	"unsafe"

	"go.uber.org/zap"
)

const (
	DMA_BUF_SIZE  = 512 << 10    //通过AXI_DMA开辟的缓存rx_buf的大小
	DMA_READ_SIZE = 4102 * 8 * 4 //一次传输的数据长度，单位字节
)

var arrayPtr *[DMA_READ_SIZE]uint8
var recvData []byte

//typedef void (*axidma_cb_t)(int channel_id, void *data);

//export RecvDmaDataCallBack
func RecvDmaDataCallBack(channel int, data unsafe.Pointer) {
	// zap.L().Debug("dma call back func")
	// fmt.Println("channel ", channel)

	fmt.Printf("%X\n", recvData[0:100])
	// fmt.Println(recvData[0:10])

	if logic.ConnNum > 0 && len(logic.DmaDataChl) < cap(logic.DmaDataChl) {
		logic.DmaDataChl <- recvData
	}

	C.fly_dma_recv_data()
}

func InitDma() error {
	ret := C.fly_dma_init(C.int(DMA_BUF_SIZE), C.int(DMA_READ_SIZE))
	if ret < 0 {
		return errors.New("init dma error")
	}
	zap.L().Debug("init dma success")

	//将C地址转换为可用的地址
	unsafePtr := unsafe.Pointer(C.fly_dma_getRxBuf())
	arrayPtr = (*[DMA_READ_SIZE]uint8)(unsafePtr)
	recvData = arrayPtr[0:DMA_READ_SIZE]

	ret = C.fly_dma_setCallBackAndStart((C.axidma_cb_t)(unsafe.Pointer(C.RecvDmaDataCallBack_CGO)))
	if ret < 0 {
		return errors.New("dma set callback error")
	}
	zap.L().Debug("dma set callback success")

	return nil
}
