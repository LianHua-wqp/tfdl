#ifndef _FPGA_H_
#define _FPGA_H_


 
int init_fpga();
int read_fpga_register (unsigned char chl_no,unsigned char reg_address,unsigned short * value_from_read);
int write_fpga_register (unsigned char chl_no,unsigned char reg_address,unsigned short value_to_write);

int spi_test();
#endif