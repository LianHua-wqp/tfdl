#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdbool.h>
#include <assert.h>           
#include <fcntl.h>              // Flags for open()
#include <sys/stat.h>           // Open() system call
#include <sys/mman.h>           // Mmap system call
#include <sys/ioctl.h>          // IOCTL system call
#include <signal.h>             // Signal handling functions
#include <net/if.h>

//  #include <bits/sigaction.h>    //sigaction  //qwe wewqe

#include "fly_dma.h"

////////////////////////////////////////////////////////////////DMA_TRANS
#define AXIDMA_IOCTL_MAGIC              'W'
struct copy_done
{
    char id;    
};

struct dma_status
{
    char status;    
};

#define SET_DMA_SIGNAL           _IO(AXIDMA_IOCTL_MAGIC, 2)
#define SET_DMA_START           _IO(AXIDMA_IOCTL_MAGIC, 3)
#define SET_DMA_STOP           _IO(AXIDMA_IOCTL_MAGIC, 4)
#define SET_DMA_RESET           _IO(AXIDMA_IOCTL_MAGIC, 5)
#define SET_DMA_COPY_DONE       _IOR(AXIDMA_IOCTL_MAGIC, 6 ,struct copy_done)
#define GET_DMA_STATUS       _IOR(AXIDMA_IOCTL_MAGIC, 7 ,struct dma_status)

#define DMA_ADDR_MAX_NUM 10
#define DMA_1_MAX_LEN 524800
#define DMA1_DEV_PATH     ("/dev/fly_dma1")

///////////////////////////////////////////////////////DMA
typedef void (*SIG_HANDLER)(int signal, siginfo_t *siginfo, void *context);
static struct dma_addr dma1_addr_array[DMA_ADDR_MAX_NUM];

struct dma_dev fly_dma1;

////////////////////////////////////////////////////////////

static int mmap_array_init_1 (int fd)
{
    void *addr;
    int i = 0, num = sizeof(dma1_addr_array)/sizeof(dma1_addr_array[0]);

    memset(dma1_addr_array,0,sizeof(dma1_addr_array));
    for (i = 0; i < num; i++)
    {
        
        addr = mmap(NULL, DMA_1_MAX_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
         if (addr == MAP_FAILED) {
            break;
         }
         
        dma1_addr_array[i].addr =  addr;
        dma1_addr_array[i].len = DMA_1_MAX_LEN;
    }

    printf("mmap num %d\n",i);
    
        return i;
}

///////////////////////////////////////DMA1 irq handle function to write cache by malloc from dma1 cache
static void signal_handler_1(int signal, siginfo_t *siginfo, void *context)
{
    //char * p;
    //int trans_number_c = 0;
    int len;
    int ret;
    unsigned char err_no,id;
    struct copy_done param;
    //int i;
    // Silence the compiler
    (void)signal;
    (void)context;

    err_no =  (siginfo->si_int >> 29)&0x07;
    id   = (siginfo->si_int >> 24)&0x1F;
    len = (siginfo->si_int&0xFFFFFF);
    //printf("dma1:%d\n",len);            
    if (err_no)
    {
        printf("dma1 err no %d\n",err_no);
        //return;
    }
    else //copy data  
    {
        if (!fly_dma1.cache || (id >= DMA_ADDR_MAX_NUM) || (0 == dma1_addr_array[id].addr))
        {
           printf("signal param error\n");
        }
        else
        {
            // ret = cache_write_data(fly_dma1.cache,dma1_addr_array[id].addr,len);

            if (ret == 0)
            {
                printf("cache1 is full\n");
            }
        }
        //printf("len is %d\n",len);
    }
    param.id = id;
    // p = (char *)dma1_addr_array[id].addr;
    // printf("dma1 to cache:%x %x %x %x\n",p[4],p[5],p[6],p[7]);
    ret = ioctl(fly_dma1.fd,SET_DMA_COPY_DONE,&param);
    if(ret < 0)
        printf("SET_DMA1_COPY_DONE error\n");
}

static int setup_dma1_signal_handler(int fd,SIG_HANDLER handler)//,int dma_number)
{
    
    int rc;
    struct sigaction sigact;

    if ( fd < 0 || handler == NULL)
        return -1;
    // Register a signal handler for the real-time signal
    sigact.sa_sigaction = handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_RESTART | SA_SIGINFO;
    rc = sigaction(SIGRTMIN+6, &sigact, NULL);
    if (rc < 0) {
        perror("Failed to create DMA signal");
        return rc;
    }

    // Tell the driver to deliver us SIGRTMIN upon DMA completion
    rc = ioctl(fd, SET_DMA_SIGNAL, SIGRTMIN+6);//+dma_number);
    if (rc < 0) {
        perror("Failed to register the DMA  signal");
        return rc;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////

int start_dma(int fd)
{
    int rc;
    
    if ( fd < 0 )
        return -1;
    
    rc = ioctl(fd, SET_DMA_START, 0);
    if (rc < 0) {
        printf("START dma fail %d\n",rc);
        return rc;
    }

    return 0;
}

static int reset_dma(int fd)
{
    int rc;

    if ( fd < 0 )
        return -1;
        
    rc = ioctl(fd, SET_DMA_RESET, 0);
    if (rc < 0) {
        printf("RESET dma fail %d\n",rc);
        return rc;
    }

    return 0;
}

int init_dma_1(void)
{
    int ret1 = 0;
    
    if ( fly_dma1.inited )
        return -1;

    fly_dma1.fd = open(DMA1_DEV_PATH, O_RDWR|O_EXCL);

    printf("dma1 fd is %d\n",fly_dma1.fd);

    if ( fly_dma1.fd < 0)
    {
        printf("open %s fail\n",DMA1_DEV_PATH);
        return fly_dma1.fd;
    }

    reset_dma(fly_dma1.fd);
    
    ret1 = mmap_array_init_1(fly_dma1.fd);

    if (ret1 < 0)
    {
        printf("mmap fail  %d\n",ret1);
        goto FAIL;
    }

    fly_dma1.array_num = ret1;

    ret1 = setup_dma1_signal_handler(fly_dma1.fd,signal_handler_1);//,1);
    if (ret1 < 0)
    {
        printf("setup_dma1_signal_handler fail %d\n",ret1);
        goto FAIL;
    }

    fly_dma1.inited = true;
    fly_dma1.status = 1;
    
    //
    start_dma(fly_dma1.fd);
    // pthread_t dma_data_id;
    // pthread_create(&dma_data_id, NULL, (void *)uart_send_state, NULL);
    // pthread_join(dma_data_id, NULL);

    return 0;

FAIL:
    if (fly_dma1.array_num)
    {
        //munmap ?
    }

    if ( fly_dma1.fd )
    {
        close(fly_dma1.fd);
        fly_dma1.fd = -1;
    }

    return -1;
}

////////////////////////////////////////////////////////

////////////////////////////////////////////////////////

int release_dma_1(void)
{
    if ( !fly_dma1.inited )
        return 0;

    if (fly_dma1.array_num)
    {
        //munmap ?
    }

    if (fly_dma1.cache)
        free(fly_dma1.cache);
    fly_dma1.cache = NULL;
    
    close(fly_dma1.fd);
    fly_dma1.fd = -1;
    fly_dma1.inited = false;
    return 0;
}


//////////////////////////////////////////////////////////////////////
    