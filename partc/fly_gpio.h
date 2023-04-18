/**
 * @brief   AXI GPIO
 * @author  xuewang
 * @date    2022/08/30
*/
/*
AXI GPIO和EMIO以及MIO的应用层使用方法一致，通过/sys/class/gpio文件夹下可以查看对应的gpio控制器，然后里面的ngpio对应着相应的位数，比如gpio_0对应的为118（54+64），axi-gpio对应的数字即为axi-gpio对应的位数，例如数字为2，那么支持从基地址开始的两位gpio的export，进而操作direction和 Value控制gpio的输入输出以及值。

*/
#ifndef _FLY_GPIO_H_
#define _FLY_GPIO_H_

int write_gpio(char *GPIO_NUM_STR,char *value);
int read_gpio(char *GPIO_NUM_STR);
int init_gpio(char *GPIO_NUM_STR);

int PullHighGpio();
#endif