#ifndef _FLY_UART_H_
#define _FLY_UART_H_
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>        //read write
//#include <stdbool.h>   //use true false
#include <sys/poll.h>           //poll

// extern int uart_fd;

//////////////////////////////////////////////////////
//struct timeval timeout; /* 超时结构体 */

///////////////////////////////////////////////////////
int close_uart(int uart_fd);
int uart_send(int fd, char *send_buf,int data_len);
int uart_recv(int fd, char buff[], int number);//fd 为串口句柄 buff为用来接收的buff，number为接收的数据的最大长度，不定长

int uart_open(char* path,int speed);
#endif