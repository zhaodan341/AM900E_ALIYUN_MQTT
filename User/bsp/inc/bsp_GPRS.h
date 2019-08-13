/*
*********************************************************************************************************
*
*	ģ������ : GPRSģ����������
*	�ļ����� : bsp_SIM800.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*********************************************************************************************************
*/

#ifndef __BSP_GPRS_H
#define __BSP_GPRS_H

#define COM_GPRS	COM2			/* ѡ�񴮿� */
//#define COM_GPRS	COM6		/* ѡ�񴮿� */

/* ����������仰, �����յ����ַ����͵����Դ���1 */
#define GPRS_TO_COM1_EN

/* 
  ��ģ�鲿�ֺ����õ��������ʱ�����1��ID�� �����������ñ�ģ��ĺ���ʱ����ע��رܶ�ʱ�� TMR_COUNT - 1��
  bsp_StartTimer(3, _usTimeOut);
  
  TMR_COUNT �� bsp_timer.h �ļ�����
*/

#define AT_CR		'\r'
#define AT_LF		'\n'

/* AT+CREG? ����Ӧ���е�����״̬���� 	��ǰ����ע��״̬  GPRS_GetNetStatus() */
enum
{
	CREG_NO_REG = 0,  	/* 0��û��ע�ᣬME���ڲ�û������ѰҪע����µ���Ӫ�� */
	CREG_LOCAL_OK = 1,	/* 1��ע���˱������� */
	CREG_SEARCH = 2,	  /* 2��û��ע�ᣬ��MS������ѰҪע����µ���Ӫ�� */
	CREG_REJECT = 3,	  /* 3��ע�ᱻ�ܾ� */
	CREG_UNKNOW = 4,	  /* 4��δ֪ԭ�� */
	CREG_REMOTE_OK = 5, /* 5��ע������������ */
};


typedef struct
{
	char Manfacture[12];			/* ���� SIMCOM_Ltd */
	char Model[12];		  			/* �ͺ� GPRS */
	char Revision[15 + 1];		/* �̼��汾 R13.08 */
	//char IMEI[15 + 1];			/* IMEI �� ��Ҫͨ��AT+GSN��� */
}GPRS_INFO_T;

/* ���ⲿ���õĺ������� */
void bsp_InitGPRS(void);
void GPRS_Reset(void);

uint8_t GPRS_PowerOn(void);
void GPRS_PowerOff(void);
void GPRS_SendAT(char *_Cmd);

uint8_t AT_SendToCmdData(char *_pCMDstr,char *_pBACKstr,uint8_t vsNum, uint16_t _usTimeOut);

void GPRS_PrintRxData(uint8_t _ch);
uint8_t GPRS_WaitResponse(char *_pAckStr, uint16_t _usTimeOut);
uint16_t GPRS_ReadResponse(char *_pBuf, uint16_t _usBufSize, uint16_t _usTimeOut);

uint8_t GPRS_GetHardInfo(GPRS_INFO_T *_pInfo);
uint8_t GPRS_GetNetStatus(void);


void  ArrayToStr(const uint32_t* arr,  uint8_t size,  char* output);
int32_t StrToIntFix(char *_pStr, uint8_t _ucLen);

uint8_t AT_ResponseCSQ(uint8_t *_vCSQ,uint32_t _usTimeOut);
uint8_t AT_ResponseCGSN(char _vCGSN[24],uint32_t _usTimeOut);
uint8_t AT_ResponseCIMI(char _vCIMI[24],uint32_t _usTimeOut);



#endif



