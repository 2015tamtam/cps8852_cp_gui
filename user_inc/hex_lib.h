#ifndef __HEX_LIB_H__
#define __HEX_LIB_H__

#define _CRT_SECURE_NO_WARNINGS

static unsigned char str2int8(char *msg){
    char tmp[3];

    tmp[0] = msg[0];
    tmp[1] = msg[1];
    tmp[2] = 0x00;

    return strtol(tmp, NULL, 16);
}

static int hex2bin(char *file_name, unsigned char *out){
    int             i;
    int             idx = 0;
    int             max = 0;
    FILE*           fp;

    char            buf[1024];
    unsigned char   dat[1024];
    unsigned char   check_sum;
    char*           p;
    int             flag = 0;

    for(i=0; i<65536; i++){
        out[i] = 0x00;
    }

    fp = fopen(file_name,"r");

    if(NULL == fp){
        printf("Can't Open file: %s\n", file_name);
        return 0;
    }
    
    while(0 == flag){
        p = buf;
        for(i=0; i<1024; i++){
            p[i]   = 0x00;
            dat[i] = 0x00;
        }
        if(EOF == fscanf(fp, "%s", p)){
            break;
        }
        if(':' != p[0]){
            flag = 1;
            break;
        }
        p++;

     
        for(i=0; i<strlen(p); i++){
            if(!(((p[i] >= '0') && (p[i] <= '9')) ||
                 ((p[i] >= 'a') && (p[i] <= 'f')) ||
                 ((p[i] >= 'A') && (p[i] <= 'F')))){
                flag = 1;
                break;
            }
        }

        if(flag)break;

        for(i=0; i<strlen(p); i=i+2){
            dat[i/2] = str2int8(p+i);
        }

        check_sum = 0;
        for(i=0; i<strlen(p)/2; i++){
            check_sum += dat[i];
        }

        if(0 != check_sum){
            flag = 1;
            break;
        }

        if(0 == dat[3]){
            idx = (dat[1] * 256) + dat[2];
            //printf("%04X :", idx);
            for(i=0; i<dat[0]; i++){
                out[idx] = dat[i+4];
                
                //printf(" %02X", dat[i+4]);
                
                if(idx >= max)max = idx;

                idx += 1;
            }
            //printf("\n");
        }
        else if(1 == dat[3]){
            break;
        }
    }

    fclose(fp);

    if(flag)max = 0;

    return (max+1);
}
#endif
