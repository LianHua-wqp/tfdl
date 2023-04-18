#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "fly_gpio.h"

//#define GPIO_ACTIVE_VAL "1"
//#define GPIO_INACTIVE_VAL "0"

//char GPIO_NUM_STR[4] = "1023"; 
int export;
int direction_fd;
int value_fd;
char direction_path[100] = {0};
char value_path[100] = {0};



int write_gpio(char *GPIO_NUM_STR,char *value)
{

    sprintf(direction_path,"/sys/class/gpio/gpio%d/direction",atoi(GPIO_NUM_STR));
    sprintf(value_path,"/sys/class/gpio/gpio%d/value",atoi(GPIO_NUM_STR));
    direction_fd = open(direction_path,O_RDWR);
    if(direction_fd < 0)
    {
        printf("can not open gpio controller direction\n");
        return -1;
    }
    write(direction_fd,"out",3);
    close(direction_fd);
    //printf("change gpio direction successfully\n");
    value_fd = open(value_path,O_RDWR);
    if(value_fd < 0)
    {
        printf("can not open gpio controller value\n");
        return -1;
    }
    //printf("open gpio value successfully\n");
    write(value_fd,value,strlen(value));
    close(value_fd);

/*********************************************************/
    return 0;
}


int read_gpio(char *GPIO_NUM_STR)
{    
    char value_str[3];
    sprintf(direction_path,"/sys/class/gpio/gpio%d/direction",atoi(GPIO_NUM_STR));
    sprintf(value_path,"/sys/class/gpio/gpio%d/value",atoi(GPIO_NUM_STR));
/*********************************************************/
    direction_fd = open(direction_path,O_RDWR);
    if(direction_fd < 0)
    {
        printf("can not open gpio controller direction\n");
        return -1;
    }
    write(direction_fd,"in",2);
    close(direction_fd);
    //printf("change gpio direction successfully\n");

/*********************************************************/
    value_fd = open(value_path,O_RDWR);
    if(value_fd < 0)
    {
        printf("can not open gpio controller value\n");
        return -1;
    }
    //printf("open gpio value successfully\n");
    if(read(value_fd,value_str,3) < 0)
    {
        perror("Failed to read value!\n");
        return -1;
    }
    close(value_fd);

/*********************************************************/
    return (atoi(value_str));
}


int init_gpio(char *GPIO_NUM_STR)
{
    /*********************************************************/

    export = open("/sys/class/gpio/export",O_WRONLY);
    if(export < 0)
    {
        printf("can not open gpio controller export\n");
        return -1;
    }
    write(export,GPIO_NUM_STR,4);
    close(export);
    printf("gpio controller export successfully\n");
    
    return 0;
/*********************************************************/
}


int PullHighGpio(){
    char gpioNum[4]="1023";
    int ret =0;
    if(0 != init_gpio(gpioNum)){
        return -1;
    }

    ret=read_gpio(gpioNum);
    printf("read gpio %d\n",ret);
    if (0==ret)
    {
        write_gpio(gpioNum,"1");
        printf("read gpio %d\n",read_gpio(gpioNum));
    }
}