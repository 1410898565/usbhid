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

/*****************************************************************
* 包含头文件
******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/hid.h>
#include <linux/hiddev.h>
	
#include <sys/stat.h>   
#include <sys/select.h>
#include <sys/time.h>

/*****************************************************************
* 宏定义(仅在当前C文件使用的宏定义写在当前C文件中，否则需写在H文件中)
******************************************************************/

/*****************************************************************
* 结构定义(仅在当前C文件使用的结构体写在当前C文件中，否则需写在H文件中)
******************************************************************/

/*****************************************************************
* 全局变量定义
******************************************************************/

/*****************************************************************
* 静态变量定义
******************************************************************/

/*****************************************************************
* 外部变量声明（如果全局变量没有在其它的H文件声明，引用时需在此处声明，
*如果已在其它H文件声明，则只需包含此H文件即可）
******************************************************************/

/*****************************************************************
* 函数原型声明
******************************************************************/

/*****************************************************************
* 函数定义
*注意，编写函数需首先定义所有的局部变量等，不允许在
*函数的中间出现新的变量的定义。
******************************************************************/


/*************************************************
** Function：	hidAPI_GetVendorProduct
** Description:	获取vendor和product
** Input:	none
** Output：	none

** Return:	none
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/

int hidAPI_GetVendorProduct(int fd, unsigned short * vid, unsigned short * pid)
{
	struct hiddev_devinfo hidinfo;

	if(ioctl(fd, HIDIOCGDEVINFO, &hidinfo)<0)
	{
		perror("hid-ioctl-devinfo");
		return -1;
	}

	*vid = hidinfo.vendor;
	*pid = hidinfo.product;

	return 0;
}


/*************************************************
** Function：	hidAPI_GetStringDesc
** Description:	获取设备描述
** Input:	index：1-Manuf； 2-Prod
** Output：	none

** Return:	none
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/

int hidAPI_GetStringDesc(int fd, int index, char s[256])
{
	struct hiddev_string_descriptor devstr;

	devstr.index = index;
	
	if(ioctl(fd, HIDIOCGSTRING, &devstr)<0)
	{
		perror("hid-ioctl-string");
		return -1;
	}

	strncpy(s, devstr.value, HID_STRING_SIZE);

	return 0;
}


/*************************************************
** Function：	hidAPI_Send
** Description:	发送数据
** Input:	none
** Output：	none

** Return:	none
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/

int hidAPI_Send(int fd, char * data, int len)
{
	struct hiddev_usage_ref uref;
	struct hiddev_report_info rinfo;
	struct hiddev_field_info finfo;
	int i,j,ret;

	rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
	rinfo.report_id = HID_REPORT_ID_FIRST;
	ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
	if(ret<0)
	{
		perror("hid-ioctl-outrepinfo");
		return -1;
	}

	// Get Fields
	for (i = 0; i < rinfo.num_fields; i++)
	{
		finfo.report_type = rinfo.report_type;
		finfo.report_id   = rinfo.report_id;
		finfo.field_index = i;
		ioctl(fd, HIDIOCGFIELDINFO, &finfo);

		// Get usages
		//printf("(OUT Max Usage: %d) ", finfo.maxusage);
		// si len es menor a finfo.maxusage, mando len bytes y completo con ceros
		if(len < finfo.maxusage) finfo.maxusage = len;

		for (j = 0; j < finfo.maxusage; j++)
		{
			uref.report_type = finfo.report_type;
			uref.report_id   = finfo.report_id;
			uref.field_index = i;
			uref.usage_index = j;
			uref.value = data[j];
			ioctl(fd,HIDIOCSUSAGE, &uref);
		}

		//send report
		ret = ioctl(fd,HIDIOCSREPORT,&rinfo);
		if(ret<0)
		{
			perror("ioctl-setout");
			return -1;
		}
	}

	return finfo.maxusage;
}


/*************************************************
** Function：	hidAPI_Recv
** Description:	接收数据
** Input:	none
** Output：	none

** Return:	none
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/
int hidAPI_Recv(int fd, char * data, int len)
{
	struct hiddev_report_info rinfo;
	struct hiddev_field_info finfo;
	struct hiddev_usage_ref uref;
	int i,j,ret;

	rinfo.report_type = HID_REPORT_TYPE_INPUT;
	rinfo.report_id = HID_REPORT_ID_FIRST;
	ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
	if(ret<0)
	{
		perror("hid-ioctl-inrepinfo");
		return -1;
	}

	// Get Fields
	for (i = 0; i < rinfo.num_fields; i++)
	{
		finfo.report_type = rinfo.report_type;
		finfo.report_id   = rinfo.report_id;
		finfo.field_index = i;
		ioctl(fd, HIDIOCGFIELDINFO, &finfo);

		// Get usages
		//printf("(IN Max Usage: %d) ", finfo.maxusage);
		//si len es menor que finfo.maxusage, leer solamente len bytes
		if(len < finfo.maxusage) finfo.maxusage = len;

		for (j = 0; j < finfo.maxusage; j++)
		{
			uref.report_type = finfo.report_type;
			uref.report_id	 = finfo.report_id;
			uref.field_index = i;
			uref.usage_index = j;
			ioctl(fd, HIDIOCGUCODE, &uref);
			ioctl(fd, HIDIOCGUSAGE, &uref);

			data[j] = uref.value;
		}
	}

	return finfo.maxusage;
}


/*************************************************
** Function：	hidAPI_Close
** Description:	关闭hid设备
** Input:	none
** Output：	none

** Return:	none
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/
int hidAPI_Close(int hidFd)
{
	if(hidFd>0)
		close(hidFd);
	return 0;
}


/*************************************************
** Function：	hidAPI_Open
** Description:	打开hid设备
** Input:	none
** Output：	none

** Return:	none
** Author：骆军城
** Date：2022-01-25

** Modification History：
** Author：
** Date：
** Description：
*************************************************/

int hidAPI_Open(char *phidDevName)
{
	return open(phidDevName, O_RDWR);
}


