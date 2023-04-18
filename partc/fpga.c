#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <pthread.h>

#define FPGA_PATH "/dev/spidev1.0"
#define TRUE 1
#define FALSE 0

#define ENABLE_FPGA_FALSH 0xFF
#define DISABLE_FPGA_FALSH 0xFE

#define msleep(x) usleep((x)*1000)
//#define PRINTF dvb_printf


static int fpga_fd = -1;

//extern int fft_printf(const char *fmt, ...);
#define fft_printf printf
static pthread_mutex_t fpga_mutex;

// int read_fpga_register (unsigned char chl_no,unsigned char reg_address,unsigned short * value_from_read);
// int write_fpga_register (unsigned char chl_no,unsigned char reg_address,unsigned short value_to_write);
// int write_fpga_register_upgrade(unsigned char reg_address,unsigned short value_to_write);

int init_fpga()
{
	// unsigned short ver = 0,cnt = 0;
	fpga_fd = open(FPGA_PATH,O_RDWR);
	if (fpga_fd < 0)
	{
		fft_printf("open fpga fail\n");
		return -1;
	}
	pthread_mutex_init(&fpga_mutex,NULL);
	//mutex_init(&fpga_mutex);

	// //read_fpga_register(0,0x1F,&ver);
	// read_fpga_register(0,0X1F,&ver);
	// while (ver == 0 && cnt < 30)
	// {
	// 	msleep(1000);
	// 	read_fpga_register(0,0,&ver);
	// 	cnt++;
	// }
	
	// if (ver_str)
	// 	*ver_str = ver;
	//PRINTF("\n====FPGA sw ver:%x====\n",ver);
	return 0;
}

int write_fpga_register (unsigned char chl_no,unsigned char reg_address,unsigned short value_to_write)
{
	//unsigned char addr = 0;
	//unsigned short val = 0x1234;
	unsigned int data = 0;
	int retval;

	if (fpga_fd <= 0)
	{
		fft_printf(" invalid fd \n");
		return -1;
	}

	//addr = atoi(buf);
	data  |= ((reg_address<<8)|chl_no);
	data |= (value_to_write<<16);
	pthread_mutex_lock(&fpga_mutex);
	retval = write(fpga_fd,&data ,4);
	pthread_mutex_unlock(&fpga_mutex);

	if (retval <0)
		fft_printf("write err: retval =%d  ,val =0x%x\n",retval,data );
	
	return retval < 0 ? retval : 0;
}


int read_fpga_register (unsigned char chl_no,unsigned char reg_address,unsigned short * value_from_read)
{
	//unsigned char addr = 0;
	//unsigned short val = 0x1234;
	unsigned int data = 0;
	int retval;

	if (fpga_fd <= 0 || value_from_read == NULL)
	{
		fft_printf(" invalid param\n");
		return -1;
	}

	//addr = atoi(buf);
	data  |= ((reg_address<<8) | chl_no);

	// data  |= ((0x80<<8) + reg_address);
	// data  = 0xFFFFFFFF;

	pthread_mutex_lock(&fpga_mutex);
	retval = read(fpga_fd,&data ,4);
	pthread_mutex_unlock(&fpga_mutex);

	if (retval <0)
	{
		fft_printf(" read err: chl_no%d, addr %d\n",chl_no,reg_address);
	}
	if (retval >=0 )
		*value_from_read = (unsigned short)(data >>16);
	else
		*value_from_read = 0;
	
	// printf("read retval =%d  ,val =0x%x\n",retval,*value_from_read );
	printf("read data %x\n",data);
	return retval < 0 ? retval : 0;
}

void release_fpga (void)
{
	if (fpga_fd > 0)
		close(fpga_fd);
	fpga_fd = 0;

	pthread_mutex_destroy(&fpga_mutex);
}

int spi_test()
{
	int ret =0;
	ret = init_fpga(FPGA_PATH);
	if	(ret < 0 )
	{
		return -1;
	}

	// for (int i = 0; i < 3; i++)
	// {
		
	// }

	unsigned char addr = 1;
	unsigned short data = 0;
	unsigned short write_data = 0xBFFB;

	while (1)
	{
		write_data++;

		// printf("write fpga : %x\n",write_data);
		// ret=write_fpga_register(0,addr,write_data);
		// if (ret < 0)
		// {
		// 	printf("write failed\n");
		// }
		// msleep(1000);

		ret = read_fpga_register(0,addr,&data);
		if (ret < 0)
		{
			printf("read failed\n");
		}
		printf("read fpga : %x\n",data);
		msleep(1000);
		// break;
	}
	


	return 0;
}

