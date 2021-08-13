
#ifndef __I2C_LIB_INC__
#define __I2C_LIB_INC__

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

typedef struct {
    uint16  id       ;
    uint8   cmd      ;
    uint8   len      ;
    uint8   dat[48]  ;
    uint16  check_sum;
}i2c_cmd_pkg;

class Ci2c_bus{
private:
    uint8       msg_buf[64];
public:
    Cusb_hid    m_usb;

    Ci2c_bus(){
        memset(msg_buf, 0, 64);
    }

   ~Ci2c_bus(){

    }

    int open_usb(void){
        return m_usb.open_usb();
    }

    int close_usb(void){
        return m_usb.close_usb();
    }

    int usb_msg(char *info){
        m_usb.wr(info, 64);
        m_usb.rd(info, 64);
		return 0;
    }

    // CRC-16/MODBUS
    uint16 Ci2c_bus::crc_chk_value(uint8 *message, uint32 len)
    {
        int i, j;
        uint16 crc_value = 0xffff;

        for (i = 0; i < len; i++){
            crc_value ^= *message++;
            for (j = 0; j < 8; j++){
                if (crc_value & 0x0001)
                    crc_value = crc_value >> 1 ^ 0xA001;
                 else
                    crc_value >>= 1;
            }

        }

        return crc_value;
    }

    uint32 data_trans(uint8 *msg_buf){
        i2c_cmd_pkg *p = (i2c_cmd_pkg*)msg_buf;

        uint16 id = (p->id + 1) & 0xffff;

        p->check_sum = crc_chk_value(msg_buf, 52);

        usb_msg((char *)msg_buf);

        if((id) != p->id){
            printf("CHECK SUM FAIL1 !\n");
            return -1;
        }

        if(crc_chk_value(msg_buf, 52) != p->check_sum){
            printf("CHECK SUM FAIL2 , %x : %x!\n",  p->check_sum, crc_chk_value(msg_buf, 52));
            return -1;
        }

        return 0;
    }

	uint32 i2c_power_on(){
        int             i;
        i2c_cmd_pkg*   wp;
        i2c_cmd_pkg*   rp;
        int             pkg_len;
        wp = (i2c_cmd_pkg *)msg_buf;
        rp = (i2c_cmd_pkg *)msg_buf;

		wp->id  += 1;
		wp->cmd  = 0xA0; // start + write(nbyte)
		wp->len  = 48;

		for(i=0; i<48; i++){
			wp->dat[i] = 0;
		}
	
		data_trans(msg_buf);
		return 0;
	}

	uint32 i2c_power_off(){
        int             i;
        i2c_cmd_pkg*   wp;
        i2c_cmd_pkg*   rp;
        int             pkg_len;
        wp = (i2c_cmd_pkg *)msg_buf;
        rp = (i2c_cmd_pkg *)msg_buf;
		
		wp->id  += 1;
		wp->cmd  = 0xA1; // start + write(nbyte)
		wp->len  = 48;
		
		for(i=0; i<48; i++){
			wp->dat[i] = 0;
		}
		
		data_trans(msg_buf);
		return 0;
	}



    uint32 i2c_bus_wr8(uint8 i2c_device_addr, uint8 memory_addr, uint8* dat, uint32 len){
        int             i;
        i2c_cmd_pkg*   wp;
        i2c_cmd_pkg*   rp;
        int             pkg_len;
        wp = (i2c_cmd_pkg *)msg_buf;
        rp = (i2c_cmd_pkg *)msg_buf;

        pkg_len = len + 2;

        if(pkg_len > 48){
            wp->id  += 1;
            wp->cmd  = 0x11; // start + write(nbyte)
            wp->len  = 48;

            wp->dat[0] = (i2c_device_addr << 1);
            wp->dat[1] = memory_addr;

            for(i=2; i<48; i++){
                wp->dat[i] = *dat;
                dat+=1;
            }

            data_trans(msg_buf);

            pkg_len -= 48;

            while(pkg_len > 48){
                pkg_len -= 48;

                wp->id  += 1;
                wp->cmd  = 0x12; // write(nbyte)
                wp->len  = 48;

                for(i=0; i<48; i++){
                    wp->dat[i] = *dat;
                    dat+=1;
                }
                data_trans(msg_buf);
            }

            if(pkg_len != 0){
                wp->id  += 1;
                wp->cmd  = 0x13; // write(nbyte) + stop
                wp->len  = pkg_len;

                for(i=0; i<pkg_len; i++){
                    wp->dat[i] = *dat;
                    dat+=1;
                }
                data_trans(msg_buf);
            }
        }
        else { // pkg_len <= 48
            wp->id  += 1;
            wp->cmd  = 0x10; //start + write(nbyte) + stop
            wp->len  = pkg_len;

            wp->dat[0] = (i2c_device_addr << 1);
            wp->dat[1] = memory_addr;

            for(i=2; i<pkg_len; i++){
                wp->dat[i] = *dat;
                dat+=1;
            }
            data_trans(msg_buf);
        }
        return 0;
    }

    uint32 i2c_bus_rd8(uint8 i2c_device_addr, uint8 memory_addr, uint8* dat, uint32 len){
        int             i;
        i2c_cmd_pkg*   wp;
        i2c_cmd_pkg*   rp;
        int             pkg_len;
        wp = (i2c_cmd_pkg *)msg_buf;
        rp = (i2c_cmd_pkg *)msg_buf;
        pkg_len = (len + 3)*2;

        if(pkg_len > 48){
            wp->id  += 1;
            wp->cmd  = 0x20; //
            wp->len  = 48;

            wp->dat[0] = 0x01;              // i2c_wr_with_start(wr)
            wp->dat[1] = (i2c_device_addr << 1);

            wp->dat[2] = 0x02;              // i2c_wr_with_normal
            wp->dat[3] = (memory_addr);

            wp->dat[4] = 0x01;              // i2c_wr_with_start(rd)
            wp->dat[5] = (i2c_device_addr << 1) | 0x1;

            for(i=0; i<(21); i++){
                wp->dat[(i*2) + 6] = 0x04;  // i2c_rd_with_normal
                wp->dat[(i*2) + 7] = 0x00;
            }

            data_trans(msg_buf);

            for(i=0; i<21; i++){
                *dat = rp->dat[(i*2) + 7];
                dat+=1;
            }

            pkg_len = len - 21;

            while(pkg_len > 48){
                pkg_len -= 48;
                wp->id  += 1;
                wp->cmd  = 0x21; // read(nbyte)
                wp->len  = 48;

                for(i=0; i<48; i++){
                    wp->dat[i] = 0x00;
                }

                data_trans(msg_buf);

                for(i=0; i<48; i++){
                    *dat = rp->dat[i];
                    dat+=1;
                }
            }

            if(pkg_len != 0){
                wp->id  += 1;
                wp->cmd  = 0x22; // read(nbyte) + stop
                wp->len  = pkg_len;

                data_trans(msg_buf);

                for(i=0; i<pkg_len; i++){
                    *dat = rp->dat[i];
                    dat+=1;
                }
            }
        }
        else { // pkg_len <= 48
            wp->id  += 1;
            wp->cmd  = 0x20; //
            wp->len  = pkg_len;

            wp->dat[0] = 0x01;              // i2c_wr_with_start(wr)
            wp->dat[1] = (i2c_device_addr << 1);

            wp->dat[2] = 0x02;              // i2c_wr_with_normal
            wp->dat[3] = (memory_addr);

            wp->dat[4] = 0x01;              // i2c_wr_with_start(rd)
            wp->dat[5] = (i2c_device_addr << 1) | 0x1;

            for(i=0; i<(len-1); i++){
                wp->dat[(i*2) + 6] = 0x04;  // i2c_rd_with_normal
                wp->dat[(i*2) + 7] = 0x00;
            }

            wp->dat[(i*2) + 6] = 0x05;      // i2c_rd_with_stop
            wp->dat[(i*2) + 7] = 0x00;

            data_trans(msg_buf);

            for(i=0; i<len; i++){
                *dat = rp->dat[(i*2) + 7];
                dat+=1;
            }
        }

        return 0;
    }

    uint32 i2c_bus_wr16(uint8 i2c_device_addr, uint32 memory_addr, uint8* dat, uint32 len){
        int             i;
        i2c_cmd_pkg*   wp;
        i2c_cmd_pkg*   rp;
        int             pkg_len;
        wp = (i2c_cmd_pkg *)msg_buf;
        rp = (i2c_cmd_pkg *)msg_buf;

        pkg_len = len + 3;

        if(pkg_len > 48){
            wp->id  += 1;
            wp->cmd  = 0x11; // start + write(nbyte)
            wp->len  = 48;

            wp->dat[0] = (i2c_device_addr << 1);
            wp->dat[1] = (memory_addr >> 8) & 0xff;
            wp->dat[2] = (memory_addr >> 0) & 0xff;

            for(i=3; i<48; i++){
                wp->dat[i] = *dat;
                dat+=1;
            }

            data_trans(msg_buf);

            pkg_len -= 48;

            while(pkg_len > 48){
                pkg_len -= 48;

                wp->id  += 1;
                wp->cmd  = 0x12; // write(nbyte)
                wp->len  = 48;

                for(i=0; i<48; i++){
                    wp->dat[i] = *dat;
                    dat+=1;
                }
                data_trans(msg_buf);
            }

            if(pkg_len != 0){
                wp->id  += 1;
                wp->cmd  = 0x13; // write(nbyte) + stop
                wp->len  = pkg_len;

                for(i=0; i<pkg_len; i++){
                    wp->dat[i] = *dat;
                    dat+=1;
                }
                data_trans(msg_buf);
            }
        }
        else { // pkg_len <= 48
            wp->id  += 1;
            wp->cmd  = 0x10; //start + write(nbyte) + stop
            wp->len  = pkg_len;

            wp->dat[0] = (i2c_device_addr << 1);
		    wp->dat[1] = (memory_addr >> 8) & 0xff;
            wp->dat[2] = (memory_addr >> 0) & 0xff;

            for(i=3; i<pkg_len; i++){
                wp->dat[i] = *dat;
                dat+=1;
            }
            data_trans(msg_buf);
        }
        return 0;
    }

    uint32 i2c_bus_rd16(uint8 i2c_device_addr, uint32 memory_addr, uint8* dat, uint32 len){
        int             i;
        i2c_cmd_pkg*   wp;
        i2c_cmd_pkg*   rp;
        int             pkg_len;
        wp = (i2c_cmd_pkg *)msg_buf;
        rp = (i2c_cmd_pkg *)msg_buf;
        pkg_len = (len + 4)*2;

        if(pkg_len > 48){
            wp->id  += 1;
            wp->cmd  = 0x20; //
            wp->len  = 48;

            wp->dat[0] = 0x01;              // i2c_wr_with_start(wr)
            wp->dat[1] = (i2c_device_addr << 1);

            wp->dat[2] = 0x02;              // i2c_wr_with_normal
            wp->dat[3] = (memory_addr>>8) & 0xff;

            wp->dat[4] = 0x02;              // i2c_wr_with_normal
            wp->dat[5] = (memory_addr>>0) & 0xff;

            wp->dat[6] = 0x01;              // i2c_wr_with_start(rd)
            wp->dat[7] = (i2c_device_addr << 1) | 0x1;

            for(i=0; i<20; i++){
                wp->dat[(i*2) + 8] = 0x04;  // i2c_rd_with_normal
                wp->dat[(i*2) + 9] = 0x00;
            }

            data_trans(msg_buf);

            for(i=0; i<20; i++){
                *dat = rp->dat[(i*2) + 9];
                dat+=1;
            }

            pkg_len = len - 20;

            while(pkg_len > 48){
                pkg_len -= 48;
                wp->id  += 1;
                wp->cmd  = 0x21; // read(nbyte)
                wp->len  = 48;

                for(i=0; i<48; i++){
                    wp->dat[i] = 0x00;
                }

                data_trans(msg_buf);

                for(i=0; i<48; i++){
                    *dat = rp->dat[i];
                    dat+=1;
                }
            }

            if(pkg_len != 0){
                wp->id  += 1;
                wp->cmd  = 0x22; // read(nbyte) + stop
                wp->len  = pkg_len;

                data_trans(msg_buf);

                for(i=0; i<pkg_len; i++){
                    *dat = rp->dat[i];
                    dat+=1;
                }
            }
        }
        else { // pkg_len <= 48
            wp->id  += 1;
            wp->cmd  = 0x20; //
            wp->len  = pkg_len;

            wp->dat[0] = 0x01;              // i2c_wr_with_start(wr)
            wp->dat[1] = (i2c_device_addr << 1);

            wp->dat[2] = 0x02;              // i2c_wr_with_normal
            wp->dat[3] = (memory_addr>>8)&0xff;

			wp->dat[4] = 0x02;              // i2c_wr_with_normal
            wp->dat[5] = (memory_addr>>0)&0xff;

            wp->dat[6] = 0x01;              // i2c_wr_with_start(rd)
            wp->dat[7] = (i2c_device_addr << 1) | 0x1;

            for(i=0; i<(len-1); i++){
                wp->dat[(i*2) + 8] = 0x04;  // i2c_rd_with_normal
                wp->dat[(i*2) + 9] = 0x00;
            }

            wp->dat[(i*2) + 8] = 0x05;      // i2c_rd_with_stop
            wp->dat[(i*2) + 9] = 0x00;

            data_trans(msg_buf);

            for(i=0; i<len; i++){
                *dat = rp->dat[(i*2) + 9];
                dat+=1;
            }
        }

        return 0;
    }


    uint32 i2c_bus_wr32(uint8 i2c_device_addr, uint32 memory_addr, uint8* dat, uint32 len){
        int             i;
        i2c_cmd_pkg*   wp;
        i2c_cmd_pkg*   rp;
        int             pkg_len;
        wp = (i2c_cmd_pkg *)msg_buf;
        rp = (i2c_cmd_pkg *)msg_buf;

        pkg_len = len + 5;

        if(pkg_len > 48){
            wp->id  += 1;
            wp->cmd  = 0x11; // start + write(nbyte)
            wp->len  = 48;

            wp->dat[0] = (i2c_device_addr << 1);
            wp->dat[1] = (memory_addr >>24) & 0xff;
            wp->dat[2] = (memory_addr >>16) & 0xff;
            wp->dat[3] = (memory_addr >> 8) & 0xff;
            wp->dat[4] = (memory_addr >> 0) & 0xff;

            for(i=5; i<48; i++){
                wp->dat[i] = *dat;
                dat+=1;
            }

            data_trans(msg_buf);

            pkg_len -= 48;

            while(pkg_len > 48){
                pkg_len -= 48;

                wp->id  += 1;
                wp->cmd  = 0x12; // write(nbyte)
                wp->len  = 48;

                for(i=0; i<48; i++){
                    wp->dat[i] = *dat;
                    dat+=1;
                }
                data_trans(msg_buf);
            }

            if(pkg_len != 0){
                wp->id  += 1;
                wp->cmd  = 0x13; // write(nbyte) + stop
                wp->len  = pkg_len;

                for(i=0; i<pkg_len; i++){
                    wp->dat[i] = *dat;
                    dat+=1;
                }
                data_trans(msg_buf);
            }
        }
        else { // pkg_len <= 48
            wp->id  += 1;
            wp->cmd  = 0x10; //start + write(nbyte) + stop
            wp->len  = pkg_len;

            wp->dat[0] = (i2c_device_addr << 1);
		    wp->dat[1] = (memory_addr >>24) & 0xff;
            wp->dat[2] = (memory_addr >>16) & 0xff;
            wp->dat[3] = (memory_addr >> 8) & 0xff;
            wp->dat[4] = (memory_addr >> 0) & 0xff;

            for(i=5; i<pkg_len; i++){
                wp->dat[i] = *dat;
                dat+=1;
            }
            data_trans(msg_buf);
        }
        return 0;
    }


    uint32 i2c_bus_rd32(uint8 i2c_device_addr, uint32 memory_addr, uint8* dat, uint32 len){
        int             i;
        i2c_cmd_pkg*   wp;
        i2c_cmd_pkg*   rp;
        int             pkg_len;
        wp = (i2c_cmd_pkg *)msg_buf;
        rp = (i2c_cmd_pkg *)msg_buf;

        pkg_len = (len + 6)*2;

        if(pkg_len > 48){
            wp->id  += 1;
            wp->cmd  = 0x20; //
            wp->len  = 48;

            wp->dat[0] = 0x01;              // i2c_wr_with_start(wr)
            wp->dat[1] = (i2c_device_addr << 1);

            wp->dat[2] = 0x02;              // i2c_wr_with_normal
            wp->dat[3] = (memory_addr>>24) & 0xff;

            wp->dat[4] = 0x02;              // i2c_wr_with_normal
            wp->dat[5] = (memory_addr>>16) & 0xff;

            wp->dat[6] = 0x02;              // i2c_wr_with_normal
            wp->dat[7] = (memory_addr>>8) & 0xff;

            wp->dat[8] = 0x02;              // i2c_wr_with_normal
            wp->dat[9] = (memory_addr>>0) & 0xff;

            wp->dat[10] = 0x01;              // i2c_wr_with_start(rd)
            wp->dat[11] = (i2c_device_addr << 1) | 0x1;

            for(i=0; i<18; i++){
                wp->dat[(i*2) + 12] = 0x04;  // i2c_rd_with_normal
                wp->dat[(i*2) + 13] = 0x00;
            }

            data_trans(msg_buf);

            for(i=0; i<18; i++){
                *dat = rp->dat[(i*2) + 13];
                dat+=1;
            }

            pkg_len = len - 18;

            while(pkg_len > 48){
                pkg_len -= 48;
                wp->id  += 1;
                wp->cmd  = 0x21; // read(nbyte)
                wp->len  = 48;

                for(i=0; i<48; i++){
                    wp->dat[i] = 0x00;
                }

                data_trans(msg_buf);

                for(i=0; i<48; i++){
                    *dat = rp->dat[i];
                    dat+=1;
                }
            }

            if(pkg_len != 0){
                wp->id  += 1;
                wp->cmd  = 0x22; // read(nbyte) + stop
                wp->len  = pkg_len;

                data_trans(msg_buf);

                for(i=0; i<pkg_len; i++){
                    *dat = rp->dat[i];
                    dat+=1;
                }
            }
        }
        else { // pkg_len <= 48
            wp->id  += 1;
            wp->cmd  = 0x20; //
            wp->len  = pkg_len;

            wp->dat[0] = 0x01;              // i2c_wr_with_start(wr)
            wp->dat[1] = (i2c_device_addr << 1);

            wp->dat[2] = 0x02;              // i2c_wr_with_normal
            wp->dat[3] = (memory_addr>>24) & 0xff;

            wp->dat[4] = 0x02;              // i2c_wr_with_normal
            wp->dat[5] = (memory_addr>>16) & 0xff;

            wp->dat[6] = 0x02;              // i2c_wr_with_normal
            wp->dat[7] = (memory_addr>>8) & 0xff;

            wp->dat[8] = 0x02;              // i2c_wr_with_normal
            wp->dat[9] = (memory_addr>>0) & 0xff;

            wp->dat[10] = 0x01;              // i2c_wr_with_start(rd)
            wp->dat[11] = (i2c_device_addr << 1) | 0x1;

            for(i=0; i<(len-1); i++){
                wp->dat[(i*2) + 12] = 0x04;  // i2c_rd_with_normal
                wp->dat[(i*2) + 13] = 0x00;
            }

            wp->dat[(i*2) + 12] = 0x05;      // i2c_rd_with_stop
            wp->dat[(i*2) + 13] = 0x00;

            data_trans(msg_buf);

            for(i=0; i<len; i++){
                *dat = rp->dat[(i*2) + 13];
                dat+=1;
            }
        }

        return 0;
    }


};


static void init_srand(void){
    int             dat;
    SYSTEMTIME      sys;

    GetLocalTime(&sys);

    dat = sys.wDay + sys.wHour + (sys.wMinute*3600) + (sys.wSecond*60) + sys.wMilliseconds;

    srand(dat);
}

static int get_rand(){
    return (rand() << 16) | rand();
}

#endif



