/**
 * @brief   DMA
 * @author  xuewang
 * @date    2022/08/30
*/

#ifndef _FLY_DMA_H_
#define _FLY_DMA_H_

struct dma_addr
{
    void * addr;
    unsigned int len;
};


struct dma_dev
{
    int fd;
    char status;
    char inited;
    char array_num;
    struct dma_addr *array_list;
    struct CacheBuf *cache;
};


int init_dma_1(void);

int start_dma(int fd);

int release_dma_1(void);

#endif