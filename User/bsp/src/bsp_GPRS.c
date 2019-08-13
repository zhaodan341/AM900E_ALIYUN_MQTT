#include "bsp.h"

/*
*********************************************************************************************************
*	�� �� ��: GPRS_PrintRxData
*	����˵��: ��ӡSTM32��GPRS�յ������ݵ�COM1���ڣ���Ҫ���ڸ��ٵ���
*	��    ��: _ch : �յ�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GPRS_PrintRxData(uint8_t _ch)
{
	#ifdef GPRS_TO_COM1_EN
		comSendChar(COM1, _ch);		/* �����յ����ݴ�ӡ�����Դ���1 */
	#endif
}

/*
*********************************************************************************************************
*	�� �� ��: GPRS_PowerOff
*	����˵��: ����GPRSģ��ػ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GPRS_PowerOff(void)
{
	/* Ӳ���ػ� */

}

/*
*********************************************************************************************************
*	�� �� ��: GPRS_WaitResponse
*	����˵��: �ȴ�GPRS����ָ����Ӧ���ַ���. ����ȴ� OK
*	��    ��: _pAckStr : Ӧ����ַ����� ���Ȳ��ó���255
*			 _usTimeOut : ����ִ�г�ʱ��0��ʾһֱ�ȴ�. >����ʾ��ʱʱ�䣬��λ1ms
*	�� �� ֵ: 1 ��ʾ�ɹ�  0 ��ʾʧ��
*********************************************************************************************************
*/
uint8_t GPRS_WaitResponse(char *_pAckStr, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint8_t ucRxBuf[256];
	uint16_t pos = 0;
	uint32_t len;
	uint8_t ret;

	len = strlen(_pAckStr);
	if (len > 255)
	{
		return 0;
	}

	/* _usTimeOut == 0 ��ʾ���޵ȴ� */
	if (_usTimeOut > 0)
	{
		bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* ʹ�������ʱ��3����Ϊ��ʱ���� */
	}
	while (1)
	{
		bsp_Idle();				/* CPU����ִ�еĲ����� �� bsp.c �� bsp.h �ļ� */

		if (_usTimeOut > 0)
		{
			if (bsp_CheckTimer(GPRS_TMR_ID))
			{
				ret = 0;	/* ��ʱ */
				break;
			}
		}

		if (comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */

			if (ucData == '\n')
			{
							if (pos > 0)	/* ��2���յ��س����� */
							{
											if (memcmp(ucRxBuf, _pAckStr,  len) == 0)
											{
												ret = 1;	/* �յ�ָ����Ӧ�����ݣ����سɹ� */
												break;
											}
											else
											{
												pos = 0;
											}
							}
							else
							{
											 pos = 0;
							}
			}
			else
			{
							if (pos < sizeof(ucRxBuf))
							{
										/* ֻ����ɼ��ַ� */
										if (ucData >= ' ')
										{
											ucRxBuf[pos++] = ucData;
										}
							}
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	�� �� ��: GPRS_ReadResponse
*	����˵��: ��ȡGPRS����Ӧ���ַ������ú��������ַ��䳬ʱ�жϽ����� ��������Ҫ����AT����ͺ�����
*	��    ��: _pBuf : ���ģ�鷵�ص������ַ���
*			  _usBufSize : ��������󳤶�
*			 _usTimeOut : ����ִ�г�ʱ��0��ʾһֱ�ȴ�. >0 ��ʾ��ʱʱ�䣬��λ1ms
*	�� �� ֵ: 0 ��ʾ���󣨳�ʱ��  > 0 ��ʾӦ������ݳ���
*********************************************************************************************************
*/
uint16_t GPRS_ReadResponse(char *_pBuf, uint16_t _usBufSize, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint8_t ret;
	uint8_t status = 0;		/* ����״̬ */

	/* _usTimeOut == 0 ��ʾ���޵ȴ� */
	if (_usTimeOut > 0)
	{
		bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* ʹ�������ʱ����Ϊ��ʱ���� */
	}
	while (1)
	{
		bsp_Idle();					/* CPU����ִ�еĲ����� �� bsp.c �� bsp.h �ļ� */

		if (status == 2)		/* ���ڽ�����ЧӦ��׶Σ�ͨ���ַ��䳬ʱ�ж����ݽ������ */
		{
			if (bsp_CheckTimer(GPRS_TMR_ID))
			{
				_pBuf[pos]	 = 0;	/* ��β��0�� ���ں���������ʶ���ַ������� */
				ret = pos;				/* �ɹ��� �������ݳ��� */
				break;
			}
		}
		else
		{
			if (_usTimeOut > 0)
			{
				if (bsp_CheckTimer(GPRS_TMR_ID))
				{
					ret = 0;	/* ��ʱ */
					break;
				}
			}
		}
		
		if (comGetChar(COM_GPRS, &ucData))
		{			
			GPRS_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */

			switch (status)
			{
				case 0:									  /* ���ַ� */
					if (ucData == AT_CR)		/* ������ַ��ǻس�����ʾ AT������� */
					{
						_pBuf[pos++] = ucData;		/* ������յ������� */
						status = 2;	 /* ��Ϊ�յ�ģ��Ӧ���� */
					}
					else					 /* ���ַ��� A ��ʾ AT������� */
					{
						status = 1;	 /* �����������͵�AT�����ַ�����������Ӧ�����ݣ�ֱ������ CR�ַ� */
					}
					break;
					
				case 1:			/* AT������Խ׶�, ����������. �����ȴ� */
					if (ucData == AT_CR)
					{
						status = 2;
					}
					break;
					
				case 2:			/* ��ʼ����ģ��Ӧ���� */
										/* ֻҪ�յ�ģ���Ӧ���ַ���������ַ��䳬ʱ�жϽ�������ʱ�����ܳ�ʱ�������� */
					bsp_StartTimer(GPRS_TMR_ID, 500);
					if (pos < _usBufSize - 1)
					{
						_pBuf[pos++] = ucData;		/* ������յ������� */
					}
					break;
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	�� �� ��: GPRS_SendAT
*	����˵��: ��GSMģ�鷢��AT��� �������Զ���AT�ַ���������<CR>�ַ�
*	��    ��: _Str : AT�����ַ�����������ĩβ�Ļس�<CR>. ���ַ�0����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void GPRS_SendAT(char *_Cmd)
{
	comClearRxFifo(COM_GPRS);	/* ���㴮�ڽ��ջ����� */	
	
	comSendBuf(COM_GPRS, (uint8_t *)_Cmd, strlen(_Cmd));
	comSendBuf(COM_GPRS, "\r", 1);
}

/*
*********************************************************************************************************
*	�� �� ��: AT_SendToCmdData
*	����˵��: ���Ϳ���ָ�����ݵ�ģ��
*	��    �Σ�_pCMDstr		 : AT ָ��
*					: _pBACKstr    ����ѯ����ָ��
*					:  vsNum			 : ���ʴ���������1��
*					: _usTimeOut	 : ���ʱ��
*	�� �� ֵ: 1 ��ʾ�ɹ�  0 ��ʾʧ��
*********************************************************************************************************
*/
uint8_t AT_SendToCmdData(char *_pCMDstr,char *_pBACKstr,uint8_t vsNum, uint16_t _usTimeOut)
{
	uint8_t ret = 0;
	uint8_t i;
	
	if(vsNum == 0)
	{
		vsNum = 1;
	}
	
	while(vsNum)
	{	
		--vsNum;
		GPRS_SendAT(_pCMDstr);
		for (i = 0; i < 5; i++)													//ϵͳĬ��������� 5 ��
		{
			if (GPRS_WaitResponse(_pBACKstr, _usTimeOut))
					{
						ret = 1;
						return 	ret;
					}
		}	
	}
		return ret;		
}


/*
*********************************************************************************************************
*	�� �� ��: ArrayToStr
*	����˵��: ����������ת��ָ�����ַ���
*	��    ��: arr����������; size������; output��������ַ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void  ArrayToStr(const uint32_t* arr,  uint8_t size,  char* output)
{
    char buf[512];
    int offset=0;
    for( int i=0; i<size; i++)
    {
        sprintf(buf+offset,"%d;",arr[i]);
        while(buf[offset]!=';')
            offset++;
        offset++;
    }
    buf[offset-1]=0;			//�����һ�� ';' д '\0'
    strcpy(output,buf);
}

/*
*********************************************************************************************************
*	�� �� ��: StrToIntFix
*	����˵��: ��ASCII���ַ���ת����ʮ����, ��������
*	��    ��: _pStr :��ת����ASCII�봮. �����Զ��Ż�0����
*			 _ucLen : �̶�����
*	�� �� ֵ: ����������ֵ
*********************************************************************************************************
*/
int32_t StrToIntFix(char *_pStr, uint8_t _ucLen)
{
	uint8_t flag;
	char *p;
	uint32_t ulInt;
	uint8_t i;
	uint8_t ucTemp;

	p = _pStr;
	if (*p == '-')
	{
		flag = 1;	/* ���� */
		p++;
		_ucLen--;
	}
	else
	{
		flag = 0;
	}

	ulInt = 0;
	for (i = 0; i < _ucLen; i++)
	{
		ucTemp = *p;
		if (ucTemp == '.')	/* ����С���㣬�Զ�����1���ֽ� */
		{
			p++;
			ucTemp = *p;
		}
		if ((ucTemp >= '0') && (ucTemp <= '9'))
		{
			ulInt = ulInt * 10 + (ucTemp - '0');
			p++;
		}
		else
		{
			break;
		}
	}

	if (flag == 1)
	{
		return -ulInt;
	}
	return ulInt;
}

/*
*********************************************************************************************************
*	�� �� ��: AT_ResponseCSQ
*	����˵��: AT ָ����Ӧ CSQ�����õ�CSQֵ
*	��    ��: _vCSQ ֵ,_usTimeOut��ʱ��
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
uint8_t AT_ResponseCSQ(uint8_t *_vCSQ,uint32_t _usTimeOut)
{
	uint8_t ucData;
	uint8_t ret_value = 0;
	uint16_t pos = 0;
	char *strATCMDCSQ = ": ";
	char _pBuf[128] = { 0 };
	char *_pCSQ;
	
	/* ���͹ؼ� AT ָ�� */
	GPRS_SendAT("AT+CSQ");
	bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* ʹ�������ʱ����Ϊ��ʱ���� */
	while(1)
	{
		if (bsp_CheckTimer(GPRS_TMR_ID))
		{
			ret_value = 0;	/* ��ʱ */
			break;
		}
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */
			
			_pBuf[pos++] = ucData;
					if (ucData == '\n')		 /* �õ����к� */					
					{
								_pCSQ = strstr(_pBuf,strATCMDCSQ);
						
									if(_pCSQ != NULL)		/* ��ַ��⵽���ǿ� */
									{
									
												*_vCSQ = *(_pCSQ+2) - 0x30;
												if(*(_pCSQ+3) == ',')
												{
													ret_value = 1;		//��⵽һλ�ź�ֵ
													break;
												}
												else
												{
													*_vCSQ =((*_vCSQ) * 10)+((*(_pCSQ+3)) - 0x30);//��⵽��λ�ź�ֵ
													ret_value = 1;
													break;
												}	
									}
									else 
									{
										pos = 0;
										memset(_pBuf,0,sizeof(_pBuf));
									}
					}		
			}
		}
	return ret_value;
}

/*****************************************
			AM20E:
					AT+CGSN

					+CGSN: 869060030133387
					OK
					AT+CIMI

					+CIMI: 460040180727133
					OK

			AM21E:
					AT+CGSN=1

					000000000000000

					OK
					AT+CIMI

					460041936113887

					OK
*****************************************/


/*
*********************************************************************************************************
*	�� �� ��: AT_ResponseCGSN 
*	����˵��: AT ָ����Ӧ CGSN, �õ���Ʒ���к�
*	��    ��: _vCGSN ��Ʒ���к�,_usTimeOut��ʱ��
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
uint8_t AT_ResponseCGSN(char _vCGSN[24],uint32_t _usTimeOut)
{
	uint8_t ucData;
	uint8_t i = 0;
	uint8_t ret_value = 0;
	uint16_t pos = 0;
	char strATCMDCGSN = ':';
	char _pBuf[128] = { 0 };
	char *_pCGSN;	
	
	char *ReContent[] =
	{
			"+CGSN",
			"ERROR",
	};		
	/* ���͹ؼ� AT ָ�� */
	GPRS_SendAT("AT+CGSN");
	bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* ʹ�������ʱ����Ϊ��ʱ���� */
	while(1)
	{
		if (bsp_CheckTimer(GPRS_TMR_ID))
		{
			ret_value = 0;	/* ��ʱ */
			break;
		}
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */
			
			_pBuf[pos++] = ucData;
					if (ucData == '\n')		 /* �õ����к� */					
					{
							if(memcmp(_pBuf,ReContent[0],5) == 0)
							{						
									_pCGSN = strchr(_pBuf,strATCMDCGSN);				
									_pCGSN = _pCGSN + 2;				/* ��ַ�ȼ�2 �ҵ���ȷ����*/
									while((*_pCGSN) != 0x0D)
									{									
										_vCGSN[i] = *_pCGSN;			
										i++;											
										_pCGSN++;																
									}
									ret_value = 1;
									break;
							}else if(memcmp(_pBuf,ReContent[1],5) == 0)
							{
									ret_value = 0;	/* �����˳� */
									break;
							}else{
									pos = 0;
									memset(_pBuf,0,sizeof(_pBuf));
							}
					}		
			}
		}
	return ret_value;
}

/*
*********************************************************************************************************
*	�� �� ��: AT_ResponseCIMI 
*	����˵��: AT ָ����Ӧ CGSN, �õ������ƶ��û�ʶ����
*	��    ��: _vCGSN ��Ʒ���к�,_usTimeOut��ʱ��
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
uint8_t AT_ResponseCIMI(char _vCIMI[24],uint32_t _usTimeOut)
{
	uint8_t ucData;
	uint8_t i = 0;
	uint8_t ret_value = 0;
	uint16_t pos = 0;
	char strATCMDCIMI = ':';
	char _pBuf[128] = { 0 };
	char *_pCIMI;	
	
	char *ReContent[] =
	{
			"+CIMI",
			"ERROR",
	};		
	/* ���͹ؼ� AT ָ�� */
	GPRS_SendAT("AT+CIMI");
	bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* ʹ�������ʱ����Ϊ��ʱ���� */
	while(1)
	{
		if (bsp_CheckTimer(GPRS_TMR_ID))
		{
			ret_value = 0;	/* ��ʱ */
			break;
		}
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */
			
			_pBuf[pos++] = ucData;
					if (ucData == '\n')		 /* �õ����к� */					
					{
							if(memcmp(_pBuf,ReContent[0],5) == 0)
							{						
									_pCIMI = strchr(_pBuf,strATCMDCIMI);				
									_pCIMI = _pCIMI + 2;				/* ��ַ�ȼ�2 �ҵ���ȷ����*/
									while((*_pCIMI) != 0x0D)
									{									
										_vCIMI[i] = *_pCIMI;			
										i++;											
										_pCIMI++;																
									}
									ret_value = 1;
									break;
							}else if(memcmp(_pBuf,ReContent[1],5) == 0)
							{
									ret_value = 0;	/* �����˳� */
									break;
							}else{
									pos = 0;
									memset(_pBuf,0,sizeof(_pBuf));
							}
					}		
			}
		}
	return ret_value;
}


