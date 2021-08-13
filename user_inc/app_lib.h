
#ifndef __APP_INC__
#define __APP_INC__

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

class Capp{
private:
public:
    Ci2c_bus    i2c_bus;
	uint32		i2c_slave_address;

    Capp(){
       i2c_slave_address = 0x30;
    }

	int open_usb(void){
		i2c_bus.open_usb();
		return 0;
	}

	int close_usb(void){
		i2c_bus.close_usb();
		return 0;
	}

	int i2c_power_on(){
		return i2c_bus.i2c_power_on();
	}

	int i2c_power_off(){
		return i2c_bus.i2c_power_off();
	}

	int i2c_wr_mem_word (uint32 addr, uint32 *dat, int len){
		i2c_bus.i2c_bus_wr32(i2c_slave_address, addr, (uint8*)dat, len*4);
		return 0;
	}

	int i2c_rd_mem_word (uint32 addr, uint32  *dat, int len){
		i2c_bus.i2c_bus_rd32(i2c_slave_address, addr, (uint8*)dat, len*4);
		return 0;
	}

	int i2c_wr_word(uint32 addr, uint32 data){
		i2c_wr_mem_word(addr, &data, 1);
		return 0;
	}

	uint32 i2c_rd_word(uint32 addr){
		uint32 data;

		i2c_rd_mem_word(addr, &data, 1);
		return data;
	}


};



#endif



