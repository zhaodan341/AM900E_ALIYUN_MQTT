#include "bsp.h"			/* �ײ�Ӳ������ */

/*
*********************************************************************************************************
*	�� �� ��: SendToTCP
*	����˵��: AM21E��ƽ̨�������� 
*	��    �Σ�
*					��_pIPstr		 ��IP��ַ
*					��_pIPstr    : �˿�
*					: _pDevIDstr ��ƽ̨����ID
*					��_pAPIkey 	 ���豸APIKEY
*					��_pData 	   ��Ҫ���͵�����
*	�� �� ֵ: 1 ���ɹ��� 0�����ӷ�����ʧ�ܣ�-1������ʧ�ܣ�-2�����ͳ��ȳ���
*********************************************************************************************************
*/

int8_t SendToTCP(char *_pIPstr,uint32_t _vPortstr,char *_pData)
{
	char Buf[64];
	int8_t ret = 0;
	char  *_pSendfBuf;		//���ͻ���
	int32_t vSendLen;			//���ͳ���
	
	vSendLen = strlen(_pData);
	
	if(vSendLen >= 2048)	//���ȳ��ޣ��˳�
	{
			return -2;
	}
	
		_pSendfBuf = (char *) malloc(vSendLen);																	//����һ���ڴ�
	
		sprintf(_pSendfBuf,"%s\r",_pData);
		_pSendfBuf[vSendLen] = 0x1A;																						//���һλ���ӽ�����
	
	  sprintf(Buf,"AT+CIPSTART=\"TCP\",\"%s\",%d\r",_pIPstr,_vPortstr);				//��IP �� �˿��ȸ�ʽ��																					
		
		/* ��ѯһ��TCP����״̬	*/								
			if (AT_SendToCmdData("AT+CIPSTATUS","STATE:IP INITIAL",100))						//��һ�ο�������Ҫ��ʼ��TCP�����IP��ַ�Ͷ˿�
			{		
					if(AT_SendToCmdData(Buf,"CONNECT OK",5000))												//�ȴ����ӳɹ�
					{				
						 ret = 1;			
					}else{			
						free(_pSendfBuf);//�ͷ��ڴ�
						return ret = 0;					
					}
					AT_SendToCmdData("AT+CIPSEND",">",1);														 //����������, ����" > " �ַ���û�л��У�����ʱ����һ��   
					if(AT_SendToCmdData(_pSendfBuf,"SEND OK",1000))											//�������ݵ�������
					{
						ret = 1;
					}else{
						free(_pSendfBuf);//�ͷ��ڴ�
						return ret = -1;
					}		

			}else if(AT_SendToCmdData("AT+CIPSTATUS","STATE:CONNECT OK",100))
			{
			
					AT_SendToCmdData("AT+CIPSEND",">",1);														    
					if(AT_SendToCmdData(_pSendfBuf,"SEND OK",1000))													//�������ݵ�������
					{
						ret = 1;
					}else{
						free(_pSendfBuf);//�ͷ��ڴ�
						return ret = -1;
					}			
			}
			else if(AT_SendToCmdData("AT+CIPSTATUS","STATE:CLOSED",100))
			{
					if(AT_SendToCmdData(Buf,"CONNECT OK",5000))												//�ȴ����ӳɹ�
					{				
						 ret = 1;			
					}else{			
						AT_SendToCmdData("AT+CIPCLOSE","CLOSE OK",1000);
						free(_pSendfBuf);//�ͷ��ڴ�
						return ret = 0;					
					}
					AT_SendToCmdData("AT+CIPSEND",">",1);														 //����������, ����" > " �ַ���û�л��У�����ʱ����һ��   
					if(AT_SendToCmdData(_pSendfBuf,"SEND OK",1000))											//�������ݵ�������
					{
						ret = 1;
					}else{
						free(_pSendfBuf);//�ͷ��ڴ�
						return ret = -1;
					}	
			}
			free(_pSendfBuf);//�ͷ��ڴ�
			return ret;
}

