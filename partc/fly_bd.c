// #include <time.h>
#include "fly_bd.h"
#include "fly_uart.h"
#include <unistd.h>  //usleep
#include <time.h>
#include <string.h>

#include "fpga.h"

int gBDUartFd =0;

#define BD_GPS_TIME	//命令参考链接: https://blog.csdn.net/Mr__FOUR/article/details/127325258

#ifdef BD_GPS_TIME
#define GPS_SAVE_CONFIG "$PCAS00*01"					//设置保存配置指令
#define GPS_SET_BAUDRATE_115200 "$PCAS01,5" 			//设置串口波特率
#define GPS_SET_REFRESH_TIME "$PCAS02,1000*2E\r\n"		//设置定位数据输出的频率,1000ms一次
// #define GPS_SET_PRINT "$PCAS03,0,0,0,0,0,0,1,1" //
#define GPS_SET_PRINT "$PCAS03,0,0,0,0,1,0,0,0*03\r\n" 	//设置输出数据NMEA语句过滤,只接收RMC
#define GPS_GET_SN "$PCAS06,1*1A\r\n"					//查询模块信息
#define GPS_SET_REBOOT "$PCAS10,1"						//设置重启
#define GPS_SET_MODE "$PCAS11,1*1C\r\n"					//设置工作模式,静止模式

#define GPS_FIND_HEAD '$'
// #define GPS_FIND_TAIL '\r'
#define GPS_FIND_TAIL '\n'

#define UART_CACHE_SIZE 1024*2		//用来存放接收数据的缓存长度
#define UART_BUF_SIZE 256			//用于接收数据以及存放一帧命令的长度

struct uart_msg_buffer
{
	unsigned int size;
	unsigned int read_pos;
	unsigned int write_pos;
	unsigned char *pdata;

	unsigned int cmd_head;
	// unsigned int cmd_tail;
};
#endif //(BD_GPS_TIME)

struct uart_msg_buffer gUartMsgStruct;
unsigned char gDataBuff[UART_BUF_SIZE];
unsigned int gDataBuffIndex=0;

void UartMsgStruct_Init(){
	gUartMsgStruct.size = UART_CACHE_SIZE;
	gUartMsgStruct.pdata = (unsigned char *)malloc(gUartMsgStruct.size);
	gUartMsgStruct.read_pos = 0;
	gUartMsgStruct.write_pos = 0;

	gUartMsgStruct.cmd_head = 0;
	// gUartMsgStruct.cmd_tail = 0;
}

//往gUartMsgStruct.pdata写入数据，同时调整gUartMsgStruct.write_pos
void UartMsgStruct_Push(unsigned char* data,int len){
	if(gUartMsgStruct.write_pos + len <= gUartMsgStruct.size)
	{
		memcpy(gUartMsgStruct.pdata+gUartMsgStruct.write_pos,data,len);
		gUartMsgStruct.write_pos+=len;
		if(gUartMsgStruct.write_pos == gUartMsgStruct.size)
		{
			gUartMsgStruct.write_pos=0;
		}
	}
	else{
		int remain = gUartMsgStruct.size-gUartMsgStruct.write_pos;
		memcpy(gUartMsgStruct.pdata+gUartMsgStruct.write_pos,data,remain);
		memcpy(gUartMsgStruct.pdata,data+remain,len - remain);
		gUartMsgStruct.write_pos=len - remain;
	}
}

//获取当前可读的连续内存 的长度以及数据
unsigned char* UartMsgStruct_GetData(int *len)
{
	if(gUartMsgStruct.read_pos == 0 && gUartMsgStruct.write_pos == 0)
		return NULL;

	if(gUartMsgStruct.read_pos <= gUartMsgStruct.write_pos)
	{
		*len= gUartMsgStruct.write_pos - gUartMsgStruct.read_pos;
	}
	else{
		*len= gUartMsgStruct.size - gUartMsgStruct.read_pos;
	}

	return gUartMsgStruct.pdata+gUartMsgStruct.read_pos;
}

//提取一个命令帧，包含GPS_FIND_HEAD，不包含GPS_FIND_TAIL
int UartMsgStruct_FindCmd()
{
	int len =0;
	int retVal = 0;
	int tmpLen =0;
	char * pdata=NULL;
	char * findhead=NULL;
	char * findtail=NULL;
	if(gUartMsgStruct.read_pos == 0 && gUartMsgStruct.write_pos == 0)
		return 0;

	//获取一截数据
	pdata = (char *)UartMsgStruct_GetData(&len);
	if(len > 0 && pdata != NULL)
	{
		// printf("get data len %d\n",len);
		if(gUartMsgStruct.cmd_head == 0)
		{
			findhead = strchr(pdata,GPS_FIND_HEAD);
			if (findhead != NULL && (findhead - pdata) < len)
			{
				findtail = strchr(pdata,GPS_FIND_TAIL);
				if (findtail != NULL && (findtail - pdata) < len)
				{
					//找到完整的一帧
					retVal  = findtail-findhead;
					memcpy(gDataBuff,findhead,retVal);
					gDataBuffIndex+=retVal;
					gUartMsgStruct.read_pos+=findtail- pdata +1;
				}
				else
				{
					//找到头未找到尾，填充头部往后的命令
					tmpLen = len - (findhead - pdata);
					memcpy(gDataBuff,findhead,tmpLen);
					gDataBuffIndex+=tmpLen;
					gUartMsgStruct.read_pos+=len;
					gUartMsgStruct.cmd_head = 1;
				}
			}
			else
			{
				//没找到头
				gUartMsgStruct.read_pos+=len;
			}
		}
		else
		{
			//查找尾部
			findtail = strchr(pdata,GPS_FIND_TAIL);
			if (findtail != NULL && (findtail - pdata) < len)
			{
				retVal = findtail-pdata;
				memcpy(gDataBuff+gDataBuffIndex,pdata,retVal);
				gDataBuffIndex+=retVal;
				gUartMsgStruct.read_pos+=retVal+1;
				gUartMsgStruct.cmd_head = 0;
			}
			else
			{
				//没找到尾，直接复制数据
				memcpy(gDataBuff+gDataBuffIndex,pdata,len);
				gDataBuffIndex+=len;
				gUartMsgStruct.read_pos+=len;
			}
		}
	}
	
	if (gUartMsgStruct.read_pos >= gUartMsgStruct.size)
	{
		// printf("gUartMsgStruct.read_pos %d\n",gUartMsgStruct.read_pos);
		gUartMsgStruct.read_pos -= gUartMsgStruct.size;
	}
	

	return retVal;
}

//校验数据从'$'到'*'中间的数据
unsigned char NMEA_check_sum(unsigned char *str,int len)
{
	unsigned char sum = 0;
	for (int i=0;i <len;i++)
		sum ^= str[i];
	return sum;
}

int bd_open_uart()
{
	gBDUartFd = uart_open("/dev/ttyPS1",9600);
	if (gBDUartFd < 0)
		return -1;
}

int bd_start_task(){
	UartMsgStruct_Init();
	int recv=0;

	char recvBuff[UART_BUF_SIZE] = {0};
	int uart_fd = gBDUartFd;

	uart_send(uart_fd,GPS_SET_PRINT,strlen(GPS_SET_PRINT)); 
	usleep(1000*200);
	
	uart_send(uart_fd,GPS_SET_REFRESH_TIME,strlen(GPS_SET_REFRESH_TIME));   
	usleep(1000*200);
	
	uart_send(uart_fd,GPS_SET_MODE,strlen(GPS_SET_MODE));
	usleep(1000*1000);

	// char *data = GPS_SET_REBOOT;
	// data+=1;
    // unsigned char sum;
	// sum = NMEA_check_sum(data,strlen(data));
	// snprintf(recvBuff,UART_BUF_SIZE,"%s*%X%X\r\n",GPS_SET_REBOOT,(sum>>4),sum&0x0F);
	// UART0_Send(uart_fd,recvBuff,strlen(recvBuff));

/*RMC
字段 0：$GPRMC，语句ID，表明该语句为Recommended Minimum Specific GPS/TRANSIT Data(RMC)推荐最小定位信息
字段 1：UTC时间，hhmmss.sss格式
字段 2：状态，A=定位，V=未定位
字段 3：纬度ddmm.mmmm，度分格式(前导位数不足则补0)
字段 4：纬度N(北纬)或S(南纬)
字段 5：经度dddmm.mmmm，度分格式(前导位数不足则补0)
字段 6：经度E(东经)或W(西经)
字段 7：速度，节，Knots(一节也是1.852千米／小时)
字段 8：方位角，度(二维方向指向，相当于二维罗盘)
字段 9：UTC日期，DDMMYY格式
字段10：磁偏角，(000 - 180)度(前导位数不足则补0)
字段11：磁偏角方向，E=东，W=西
字段12：模式，A=自动，D=差分，E=估测，N=数据无效(3.0协议内容)
字段13：校验值
*/

	unsigned char crc_char;
	unsigned char crc_asc[3];	//crc_asc[2]=='\0'

	// $GNRMC,091343.000,A,3038.15262,N,10358.98338,E,0.00,0.00,140423,,,A,V*0A
	int year = 0,month = 0,day = 0,hh = 0,mm = 0,ss = 0,ms=0;
	char status=0;//A=定位，V=未定位
	int latitude1=0,latitude2 = 0,latitude3=0;char latitude=0;	//纬度
	int longitude1=0,longitude2 = 0,longitude3=0;char longitude =0;	//经度
	float speed=0.0;float azimuth=0.0;

	struct tm tm_val;
	long utc_time;

	unsigned short timeToFpga[4]={0};

    while (1)
    {
        recv=uart_recv(uart_fd,recvBuff,UART_BUF_SIZE);
        if (recv>0)
        {
			UartMsgStruct_Push(recvBuff,recv);
			if(UartMsgStruct_FindCmd()>0)
			{
				if (gDataBuff[3] == 'R' && gDataBuff[4]=='M' && gDataBuff[5] == 'C')
				{
					gDataBuff[gDataBuffIndex]='\0';
					// printf("%s\n",gDataBuff);

					crc_char= NMEA_check_sum(gDataBuff+1,gDataBuffIndex-4-1);// "*XX\r" '$'
					sprintf(crc_asc,"%X%X",crc_char>>4,crc_char & 0x0f);

					// printf("crc %X%X	",crc_char>>4,crc_char & 0x0f);
					if (crc_asc[0] == gDataBuff[gDataBuffIndex-3] && crc_asc[1] == gDataBuff[gDataBuffIndex-2])
					{
						//noting to do 
						sscanf(gDataBuff,"%*[^,],%02d%02d%02d.%u,%c,%02d%02d.%u,%c,%03d%02d.%u,%c,%f,%f,%02d%02d%02d,(.*)",
							&hh,&mm,&ss,&ms,&status,
							&latitude1,&latitude2,&latitude3,&latitude,
							&longitude1,&longitude2,&longitude3,&longitude,
							&speed,&azimuth,
							&day,&month,&year);
						// printf("%02d%02d%02d-%02d%02d%02d latitude %c %02d.%02d longitude %c %03d.%02d\n",year,month,day,hh,mm,ss,
						// 	latitude,latitude1,latitude2,longitude,longitude1,longitude2);

						//转成UTC
						tm_val.tm_year = year + 2000 - 1900;
						tm_val.tm_mon = month - 1;
						tm_val.tm_mday = day;
						tm_val.tm_hour = hh;
						tm_val.tm_min = mm;
						tm_val.tm_sec = ss;
						utc_time = mktime(&tm_val); //+8 hour. 28800
						printf("get utc time %u\n",utc_time);

						timeToFpga[0] = ((unsigned char)(utc_time >> 24)<<8) | (unsigned char)(utc_time >> 16);
						timeToFpga[1] = ((unsigned char)(utc_time >> 8)<<8) | (unsigned char)(utc_time >> 0);

						write_fpga_register(0,5,timeToFpga[0]);
						write_fpga_register(0,6,timeToFpga[1]);//second
						write_fpga_register(0,7,timeToFpga[2]);
						write_fpga_register(0,8,timeToFpga[3]);//nsecond

						write_fpga_register(0,9,1);
					}
					else
					{
						printf("error crc!");
					}
				}
				memset(gDataBuff,0,gDataBuffIndex);
				gDataBuffIndex = 0;
			}
        }
        // usleep(1000*500);
    }
    return 0;
}

