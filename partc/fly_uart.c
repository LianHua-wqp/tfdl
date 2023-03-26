#include "fly_uart.h"

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
// #include "fly_uart.h"
#include <sys/select.h> //fd_set
// #include <termios.h>   //CRTSCTS

/*
sscanf以及snprintf用法详见string文件
*/
#define TRUE 1
#define FALSE 0
#define DEBUG 1

int uart_fd;
// struct timeval timeout; /* 超时结构体 */
fd_set readfds; /* 读操作文件描述符集 */
struct pollfd fds[1];

/*******************************************************************
*名称：             UART1_Open
*功能：             打开串口并返回串口设备文件描述
*入口参数：
                    port    串口号(ttyS0,ttyS1,ttyS2)
*出口参数：正确返回为1，错误返回为0
*******************************************************************/
int UART1_Open(char *port)
{
    int fd = 0;
    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        perror("Can't Open Serial 1 Port");
        return -1;
    }
    // 恢复串口为阻塞状态
    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed!\n");
        return -1;
    }
    //   else
    //  {
    // printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
    //    }
    // 测试是否为终端设备
#if 0
    if(0 == isatty(STDIN_FILENO))    
    {    
        printf("standard input is not a terminal device\n");    
        return -1;    
    }    
    else    
    {    
        printf("isatty success!\n");    
    }                  
    printf("fd->open=%d\n",fd);
#endif
    return fd;
}

int close_uart(int uart_fd)
{
    if (uart_fd > 0)
        close(uart_fd);
    return 0;
}

/*******************************************************************
 *名称：             UART1_Set
 *功能：             设置串口数据位，停止位和效验位
 *入口参数：         fd          串口文件描述符
 *                   speed       串口速度
 *                   flow_ctrl   数据流控制
 *                   databits    数据位   取值为 7 或者8
 *                   stopbits    停止位   取值为 1 或者2
 *                   parity      效验类型 取值为N,E,O,,S
 *出口参数：正确返回为1，错误返回为0
 *******************************************************************/
int UART1_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{

    int i;
    int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300};

    struct termios options;

    /*  tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，
        该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.  */
    if (tcgetattr(fd, &options) != 0)
    {
        perror("SetupSerial 1");
        return (FALSE);
    }

    // 设置串口输入波特率和输出波特率
    for (i = 0; i < (int)(sizeof(speed_arr) / sizeof(int)); i++)
    {
        if (speed == name_arr[i])
        {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
        }
    }

    // 修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    // 修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;

    // 设置数据流控制
    switch (flow_ctrl)
    {

    case 0: // 不使用流控制
        options.c_cflag &= ~CRTSCTS;
        break;

    case 1: // 使用硬件流控制
        options.c_cflag |= CRTSCTS;
        break;
    case 2: // 使用软件流控制
        options.c_cflag |= IXON | IXOFF | IXANY;
        break;
    }
    // 设置数据位
    // 屏蔽其他标志位
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
    case 5:
        options.c_cflag |= CS5;
        break;
    case 6:
        options.c_cflag |= CS6;
        break;
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr, "Unsupported data size\n");
        return (FALSE);
    }
    // 设置校验位
    switch (parity)
    {
    case 'n':
    case 'N': // 无奇偶校验位。
        options.c_cflag &= ~PARENB;
        options.c_iflag &= ~INPCK;
        break;
    case 'o':
    case 'O': // 设置为奇校验
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;
        break;
    case 'e':
    case 'E': // 设置为偶校验
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;
        break;
    case 's':
    case 'S': // 设置为空格
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported parity\n");
        return (FALSE);
    }
    // 设置停止位
    switch (stopbits)
    {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported stop bits\n");
        return (FALSE);
    }

    // 修改输出模式，原始数据输出
    options.c_oflag &= ~OPOST;

    // 关闭0x0A 0X0D 字符映射
    options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~(ONLCR | OCRNL | ONLRET);
    // options.c_lflag &= ~(ISIG | ICANON);

    // 设置等待时间和最小接收字符
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = 1;  /* 读取字符的最少个数为1 */

    // 如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd, TCIFLUSH);

    // 激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("com set error!\n");
        return (FALSE);
    }
    return (TRUE);
}

/********************************************************************
 * 名称：            UART1_Send
 * 功能：            发送数据
 * 入口参数：        fd           文件描述符
 *                   send_buf     存放串口发送数据
 *                   data_len     一帧数据的个数
 * 出口参数：        正确返回为1，错误返回为0
 *******************************************************************/
int uart_send(int fd, char *send_buf, int data_len)
{
    int len = 0;

    len = write(fd, send_buf, data_len);
    if (len == data_len)
    {
        // printf("send data is %s\n",send_buf);
        return 1;
    }
    else
    {
        printf("send data fail\n");
        tcflush(fd, TCOFLUSH);
        return 0;
    }
}

/********************************************************************
 * 名称：            UART1_Read
 * 功能：            接收数据
 * 入口参数：        fd           文件描述符
 *                   recv_buf     接收串口数据的缓存
 *                   data_len     一帧数据的个数
 * 出口参数：        正确返回为1，错误返回为0
 *******************************************************************/
int uart_read(int fd, char *recv_buf, int data_len) // 无超时机制
{
    int len = 0;
    len = read(fd, recv_buf, data_len);
    return len;
}

void init_select()
{
    ///////////////////////////////////////////////////////////////////
    /* 构造超时时间 */
    // timeout.tv_sec = 0;
    // timeout.tv_usec = 0; /* 100ms */
    fds[0].fd = uart_fd;
    fds[0].events = POLLPRI;
}
/********************************************************************
 * 名称：            uart_recv
 * 功能：
 * 入口参数：          fd       文件描述符
 *                   buff     接收串口数据的缓存
 *                   number   一帧数据的个数
 * 出口参数：        正确返回为接收长度，超时返回为0，传输错误返回-1
 *******************************************************************/
int uart_recv(int fd, char buff[], int number) // fd 为串口句柄 buff为用来接收的buff，number为接收的数据的最大长度，不定长
{
    int recv_number;
    int ret; /* 要监视的文件描述符 */
    // int need_number = 0;
    // int flag = 1;

    FD_ZERO(&readfds);    /* 清除readfds */
    FD_SET(fd, &readfds); /* 将fd 添加到readfds 里面 */

    ret = select(fd + 1, &readfds, NULL, NULL, NULL);
    switch (ret)
    {
    case 1:
        if (FD_ISSET(fd, &readfds))
        {
            recv_number = uart_read(fd, buff, number);
            // if(flag == 1)
            // {
            //     need_number =  (data[12] - 0x30) * 1000 + (data[13] - 0x30) * 100 + (data[14] - 0x30) * 10 + (data[15] - 0x30) * 1;
            //     flag = 2;
            // }
            // strncpy(buff+number-need_number,uart_rbuf,recv_number);
            // if(recv_number != need_number)
            // {
            //     need_number = need_number - recv_number;
            //     continue;
            // }
            // else
            // {
            //     return need_number;
            // }
            return recv_number;
        }
        break;
    case 0: /* 超时 */
        return 0;
        break;
    case -1: /* 错误 */
        return -1;
        break;
    default: /* 可以读取数据 */
        printf("uart_recv1 select type error\n");
        return -3;
    }

    return 1;
}

int init_rs422_uart(char * devName)
{
    if (devName == NULL)
    {
        printf("init_rs422_uart devName is null");
        return -1;
    }

    uart_fd = UART1_Open(devName);
    // uart_fd = UART1_Open("/dev/ttyPS0");
    if (uart_fd < 0)
    {
        printf("open tty fail\n");
        return -1;
    }

    printf("open tty success\n");

    if (UART1_Set(uart_fd, 115200, 0, 8, 1, 'n') == FALSE)
    {
        close(uart_fd);
        return -1;
    }
    init_select();

    pthread_t uart_data_id;
    pthread_create(&uart_data_id, NULL, (void *)uart_send_state, NULL);
    pthread_join(uart_data_id, NULL);

    return 0;
}

void uart_send_state()
{
    char uart_sbuf[50] = "test uart\r\n";
    int count = 0;

    for (size_t i = 0; i < 10; i++)
    {
        uart_send(uart_fd, uart_sbuf, strlen(uart_sbuf));
        usleep(10000);
    }

    // while (1)
    // {
    //     uart_send(uart_fd, uart_sbuf, strlen(uart_sbuf));
    // 	sleep(500);
    // }
}