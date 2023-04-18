#ifndef _FLY_DMA_H_
#define _FLY_DMA_H_
typedef void (*axidma_cb_t)(int channel_id, void *data);

int fly_dma_init(int bufSize,int readSize);
int fly_dma_free();
int fly_dma_setCallBackAndStart(axidma_cb_t callback);
char * fly_dma_getRxBuf();
void fly_dma_recv_data();
#endif