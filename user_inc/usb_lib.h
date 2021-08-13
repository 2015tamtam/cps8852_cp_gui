
#ifndef __USB_LIB_INC__
#define __USB_LIB_INC__

/* Private define ------------------------------------------------------------*/
// #define     USB_VID         0x0483  //这个值在单片机程序的设备描述符中定义
// #define     USB_PID         0x5750  //这个值在单片机程序的设备描述符中定义
// #define     REPORT_COUNT    64      //端点长度
// #define     REPORT_ID       0x00


/* Private define ------------------------------------------------------------*/
#define     USB_VID         0x28e9  //这个值在单片机程序的设备描述符中定义
#define     USB_PID         0x028a  //这个值在单片机程序的设备描述符中定义
#define     REPORT_COUNT    63      //端点长度
#define     REPORT_ID       0x02

class Cusb_hid
{
private:
    int     flag;
    BYTE    recvDataBuf[1024];
    BYTE    reportBuf  [1024];

    void buf_init(void){
        for(int i=0; i<1024; i++){
            reportBuf  [i] = 0;
            recvDataBuf[i] = 0;
        }
        reportBuf[0] = REPORT_ID;
    }

    int char2int(char c){
        int num = 0;

        if(('A' <= c) && ('F' >= c)){
            num = (10 + c - 'A');
        }
        if(('a' <= c) && ('f' >= c)){
            num = (10 + c - 'a');
        }
        if(('0' <= c) && ('9' >= c)){
            num = (c - '0');
        }

        return num;
    }

    int hex2int32(char *str){
        int num;

        num  = char2int(str[0]) * 0x1000;
        num += char2int(str[1]) * 0x0100;
        num += char2int(str[2]) * 0x0010;
        num += char2int(str[3]) * 0x0001;

        return num;
    }

    int check_vid(char *path){
        int i;

        for(i=0; i<(strlen(path)-8); i++){
            if( (path[i+0] == 'v') &&
                (path[i+1] == 'i') &&
                (path[i+2] == 'd') &&
                (path[i+3] == '_') ){

                return hex2int32(path+i+4);
            }
        }
        return 0;
    }

    int check_pid(char *path){
        int i;

        for(i=0; i<(strlen(path)-8); i++){
            if( (path[i+0] == 'p') &&
                (path[i+1] == 'i') &&
                (path[i+2] == 'd') &&
                (path[i+3] == '_') ){

                return hex2int32(path+i+4);
            }
        }
        return 0;
    }


public:
    HANDLE  hDev;
    DWORD   bytes;

    Cusb_hid(){
        flag = 0;
        buf_init();
    }

   ~Cusb_hid(){
        close_usb();
    }

    int open_usb(void){
        hDev = OpenMyHIDDevice(0); // 打开设备，不使用重叠（异步）方式 ;
        if (hDev == INVALID_HANDLE_VALUE){
            printf("INVALID_HANDLE_VALUE\n");
            //AfxMessageBox("OPEN USB FAIL");
            return 0;
        }

        flag = 1;
        return 1;
    }

    int close_usb(void){
        if(flag){
            CloseHandle(hDev);
        }
        flag = 0;
        return flag;
    }

    HANDLE OpenMyHIDDevice(int overlapped)
    {
        HANDLE hidHandle;
        GUID hidGuid;
        HidD_GetHidGuid(&hidGuid);
        HDEVINFO hDevInfo = SetupDiGetClassDevs(&hidGuid,NULL,NULL,(DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
        if (hDevInfo == INVALID_HANDLE_VALUE)
        {
            return INVALID_HANDLE_VALUE;
        }
        SP_DEVICE_INTERFACE_DATA devInfoData;
        devInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
        int deviceNo = 0;
        SetLastError(NO_ERROR);
        while (GetLastError() != ERROR_NO_MORE_ITEMS)
        {
            if (SetupDiEnumInterfaceDevice (hDevInfo,0,&hidGuid,deviceNo,&devInfoData))
            {
                ULONG  requiredLength = 0;
                SetupDiGetInterfaceDeviceDetail(hDevInfo,
                    &devInfoData,
                    NULL,
                    0,
                    &requiredLength,
                    NULL);
                PSP_INTERFACE_DEVICE_DETAIL_DATA devDetail = (SP_INTERFACE_DEVICE_DETAIL_DATA*)malloc(requiredLength);
                devDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
                if(!SetupDiGetInterfaceDeviceDetail(hDevInfo,
                    &devInfoData,
                    devDetail,
                    requiredLength,
                    NULL,
                    NULL))
                {
                    free(devDetail);
                    SetupDiDestroyDeviceInfoList(hDevInfo);
                    return INVALID_HANDLE_VALUE;
                }

                int path_vid = check_vid(devDetail->DevicePath);
                int path_pid = check_pid(devDetail->DevicePath);

                hidHandle = INVALID_HANDLE_VALUE;
                if( USB_VID != path_vid){
                    ++deviceNo;
                    continue;
                }
                if( USB_PID != path_pid){
                    ++deviceNo;
                    continue;
                }

                if (overlapped)
                {
                    hidHandle = CreateFile(devDetail->DevicePath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_OVERLAPPED,
                        NULL);
                }
                else
                {
                    hidHandle = CreateFile(devDetail->DevicePath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);
                }
                free(devDetail);
                if (hidHandle==INVALID_HANDLE_VALUE)
                {
                    SetupDiDestroyDeviceInfoList(hDevInfo);
                    return INVALID_HANDLE_VALUE;
                }
                _HIDD_ATTRIBUTES hidAttributes;
                if(!HidD_GetAttributes(hidHandle, &hidAttributes))
                {
                    CloseHandle(hidHandle);
                    SetupDiDestroyDeviceInfoList(hDevInfo);
                    return INVALID_HANDLE_VALUE;
                }
                if (USB_VID == hidAttributes.VendorID
                    && USB_PID  == hidAttributes.ProductID)
                {
                    printf("FIND HID-USB\n");
                    break;
                }
                else
                {
                    CloseHandle(hidHandle);
                    ++deviceNo;
                }
            }
        }
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return hidHandle;
    }

    int wr(char *dat, int len){
        int i;
        buf_init();

        for(i=0; i<len; i++){
            reportBuf[i+1] = dat[i];
        }

        if (!WriteFile(hDev, reportBuf, REPORT_COUNT+1, &bytes, NULL)){
            printf("write data error! %d\n",GetLastError());
            return -1;
        }
        return 0;
    }

    int rd(char *dat, int len){
        int i;

        // 从设备读取数据，注意，第三个参数值必须大于等于REPORT_COUNT+1，否则会返回1784错误
        if(!ReadFile(hDev, recvDataBuf, REPORT_COUNT+1, &bytes, NULL)){ // 读取设备发给主机的数据
            printf("read data error! %d\n",GetLastError());
            return -1;
        }

        for(i=0; i<len; i++){
            dat[i] = recvDataBuf[i+1];
        }

        return 0;
    }
};


#endif



