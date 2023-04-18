#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>             // Strlen function

#include <fcntl.h>              // Flags for open()
#include <sys/stat.h>           // Open() system call
#include <sys/types.h>          // Types for open()
#include <sys/mman.h>           // Mmap system call
#include <sys/ioctl.h>          // IOCTL system call
#include <unistd.h>             // Close() system call
#include <sys/time.h>           // Timing functions and definitions
#include <getopt.h>             // Option parsing
#include <errno.h>              // Error codes

#include "libaxidma.h"          // Interface to the AXI DMA
#include "fly_dma.h"


// axidma_init(); //初始化 axi dma 设备，获取fd
// axidma_malloc(axidma_dev, tx_size);//这里会做内存映射，映射cma 内存到用户空间
// axidma_oneway_transfer(dev, rx_channel, rx_buf, rx_size, true);//单向传输，可以发送、接收
// axidma_free(axidma_dev, tx_buf, tx_size);//释放内存映射，也就是cma 内存
// axidma_destroy(axidma_dev);//关闭设备

// #define SINGLE_TRANS_LEN    (4102*8*4)  //yici tu de zijie changdu 
// #define BUFF_SIZE (512<<10)

axidma_dev_t axidma_dev;
char *tx_buf;
char *rx_buf;
int buf_size;
int read_size;

// void callbackAfterRecive(int channelid,void* data)
// {
//     // printf("INFO: callback func triggerd,channelid: %d\n",channelid);
//     for(int i = 0;i < 100;i++)
//     {
//         printf("%x ",*((unsigned char*)(rx_buf)+i));
//     }
//     printf("\n");
//     fly_dma_recv_data();
// }

axidma_cb_t gcallback;
void callbackdma(int channelid,void* data)
{
    for(int i = 0;i < 10;i++)
    {
        printf("%x ",*((unsigned char*)(rx_buf)+i));
    }
    printf("\n");
    if (gcallback != NULL)
    {
        gcallback(channelid,data);
    }
}

int fly_dma_init(int bufSize,int readSize)
{
    buf_size = bufSize;
    read_size = readSize;

    // printf("buf_size %d,read_size %d\n",buf_size,read_size);

    const array_t *rx_chans;
    axidma_dev = axidma_init();
    if (axidma_dev == NULL) {
        fprintf(stderr, "Failed to initialize the AXI DMA device.\n");
        return -1;
    }

    // printf("send chl len %d\n",axidma_get_dma_tx(axidma_dev)->len);
    // printf("read chl len %d\n",axidma_get_dma_rx(axidma_dev)->len);
    rx_chans = axidma_get_dma_rx(axidma_dev);
    if (rx_chans->len < 1) {
        fprintf(stderr, "Error: No receive channels were found.\n");
        return -1;
    }

    rx_buf = axidma_malloc(axidma_dev, buf_size);
    if (rx_buf == NULL) {
        perror("Unable to allocate receive buffer from the AXI DMA device");
        return -1;
    }
    axidma_stop_transfer(axidma_dev,0);

    return 0;
}

int fly_dma_free()
{
    axidma_free(axidma_dev, rx_buf, buf_size);
    axidma_destroy(axidma_dev);
    return 0;
}


int fly_dma_setCallBackAndStart(axidma_cb_t callback)
{
    if (callback == NULL)
    {
       return -1;
    }
    
    // callback(1,NULL);
    // return 0;

    gcallback = callback;

    //todo 可以尝试使用异步传输
    // axidma_set_callback(axidma_dev,0,callback,NULL);
    // axidma_set_callback(axidma_dev,0,callbackdma,NULL);
    fly_dma_recv_data();
    return 0;
}

char * fly_dma_getRxBuf()
{
    return rx_buf;
}

void fly_dma_recv_data()
{
    // axidma_oneway_transfer(axidma_dev,0,rx_buf,read_size,false);

    int ret= axidma_oneway_transfer(axidma_dev,0,rx_buf,read_size,true);
    if (ret == 0)
    {
        gcallback(0,NULL);
        // printf("recv data\n");
        // for (int i = 0; i < read_size; i++)
        // {
        //     printf("%x",rx_buf[i]);
        // }
    }
}