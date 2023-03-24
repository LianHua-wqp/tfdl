// #include <stdio.h>
// void hello()
// {
//     printf("hello world\n"); //peiwqeq
// }

#include <stdio.h>
#include "hello.h"
 
//参数是函数指针
void some_c_func(callback_fcn callback)
{
	int arg = 2;
	printf("C.some_c_func(): calling callback with arg = %d\n", arg);
	int response = callback(2);
	printf("C.some_c_func(): callback responded with %d\n", response);
}