/*****************************************************************
 ** �ļ���:  hidapi.h
 ** 2020-2022 �麣������ӿƼ����޹�˾ Co.Ltd
 
 ** ������: �����
 ** ��  ��: 2022-01
 ** ��  ��: hidapiӦ�ýӿ�
 ** ��  ��: V1.0

 ** �޸���:
 ** ��  ��:
 ** �޸�����:
 ** ��  ��: 
******************************************************************/

/*****************************************************************
* ����ͷ�ļ�
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
* �궨��(���ڵ�ǰC�ļ�ʹ�õĺ궨��д�ڵ�ǰC�ļ��У�������д��H�ļ���)
******************************************************************/

/*****************************************************************
* �ṹ����(���ڵ�ǰC�ļ�ʹ�õĽṹ��д�ڵ�ǰC�ļ��У�������д��H�ļ���)
******************************************************************/

/*****************************************************************
* ȫ�ֱ�������
******************************************************************/

/*****************************************************************
* ��̬��������
******************************************************************/

/*****************************************************************
* �ⲿ�������������ȫ�ֱ���û����������H�ļ�����������ʱ���ڴ˴�������
*�����������H�ļ���������ֻ�������H�ļ����ɣ�
******************************************************************/

/*****************************************************************
* ����ԭ������
******************************************************************/

/*****************************************************************
* ��������
*ע�⣬��д���������ȶ������еľֲ������ȣ���������
*�������м�����µı����Ķ��塣
******************************************************************/


/*************************************************
** Function��	hidAPI_GetVendorProduct
** Description:	��ȡvendor��product
** Input:	none
** Output��	none

** Return:	none
** Author�������
** Date��2022-01-25

** Modification History��
** Author��
** Date��
** Description��
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
** Function��	hidAPI_GetStringDesc
** Description:	��ȡ�豸����
** Input:	index��1-Manuf�� 2-Prod
** Output��	none

** Return:	none
** Author�������
** Date��2022-01-25

** Modification History��
** Author��
** Date��
** Description��
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
** Function��	hidAPI_Send
** Description:	��������
** Input:	none
** Output��	none

** Return:	none
** Author�������
** Date��2022-01-25

** Modification History��
** Author��
** Date��
** Description��
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
** Function��	hidAPI_Recv
** Description:	��������
** Input:	none
** Output��	none

** Return:	none
** Author�������
** Date��2022-01-25

** Modification History��
** Author��
** Date��
** Description��
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
** Function��	hidAPI_Close
** Description:	�ر�hid�豸
** Input:	none
** Output��	none

** Return:	none
** Author�������
** Date��2022-01-25

** Modification History��
** Author��
** Date��
** Description��
*************************************************/
int hidAPI_Close(int hidFd)
{
	if(hidFd>0)
		close(hidFd);
	return 0;
}


/*************************************************
** Function��	hidAPI_Open
** Description:	��hid�豸
** Input:	none
** Output��	none

** Return:	none
** Author�������
** Date��2022-01-25

** Modification History��
** Author��
** Date��
** Description��
*************************************************/

int hidAPI_Open(char *phidDevName)
{
	return open(phidDevName, O_RDWR);
}


