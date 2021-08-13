
#ifndef __I2CBASE_INC__
#define __I2CBASE_INC__

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;


int     l2l                 (int dat)                           ;
int     i2c_wr_mem_word     (uint32 addr, uint32 *dat, int len) ;
int     i2c_rd_mem_word     (uint32 addr, uint32 *dat, int len) ;
int     i2c_wr_word         (uint32 addr, uint32 data)          ;
uint32  i2c_rd_word         (uint32 addr)                       ;
int     i2c_open            (void)                              ;
int     i2c_close           (void)                              ;


int i2c_power_on();
int i2c_power_off();

int _delayns(int dat);
int _delayus(int dat);
int _delayms(int dat);
int _over (char *dat);


int test_pass();
int test_fail();




#endif