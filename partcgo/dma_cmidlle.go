package partcgo

/*
#include "../partc/fly_dma.h"


extern void RecvDmaDataCallBack(int channel, void * data);	//dma.go
void RecvDmaDataCallBack_CGO(int channel, void * data)
{
	RecvDmaDataCallBack(channel,data);
	//return RecvDmaDataCallBack(channel,data);
}
*/
import "C"
