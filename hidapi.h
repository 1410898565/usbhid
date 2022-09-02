/*****************************************************************
 ** 文件名:  hidapi.h
 ** 2020-2022 珠海禾田电子科技有限公司 Co.Ltd
 
 ** 创建人: 骆军城
 ** 日  期: 2022-01
 ** 描  述: hidapi应用接口
 ** 版  本: V1.0

 ** 修改人:
 ** 日  期:
 ** 修改描述:
 ** 版  本: 
******************************************************************/

#ifndef __HT_HIDAPI_H_
#define	__HT_HIDAPI_H_


#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************
* 包含头文件(如非特殊需要，H文件不建议包含其它H文件)
******************************************************************/

#include <stdint.h>

/*****************************************************************
* 宏定义
******************************************************************/

/*****************************************************************
* 结构定义
******************************************************************/


/*****************************************************************
* 函数原型声明
******************************************************************/

int hidAPI_GetVendorProduct(int fd, unsigned short * vid, unsigned short * pid);
int hidAPI_GetStringDesc(int fd, int index, char s[256]);
int hidAPI_Send(int fd, uint8_t * data, int len);
int hidAPI_Recv(int fd, char * data, int len);

int hidAPI_Close(int hidFd);
int hidAPI_Open(char *phidDevName);

#ifdef __cplusplus
}
#endif

#endif 



