#include "bsp.h"

/*
*********************************************************************************************************
*	函 数 名: GPRS_PrintRxData
*	功能说明: 打印STM32从GPRS收到的数据到COM1串口，主要用于跟踪调试
*	形    参: _ch : 收到的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void GPRS_PrintRxData(uint8_t _ch)
{
	#ifdef GPRS_TO_COM1_EN
		comSendChar(COM1, _ch);		/* 将接收到数据打印到调试串口1 */
	#endif
}

/*
*********************************************************************************************************
*	函 数 名: GPRS_PowerOff
*	功能说明: 控制GPRS模块关机
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void GPRS_PowerOff(void)
{
	/* 硬件关机 */

}

/*
*********************************************************************************************************
*	函 数 名: GPRS_WaitResponse
*	功能说明: 等待GPRS返回指定的应答字符串. 比如等待 OK
*	形    参: _pAckStr : 应答的字符串， 长度不得超过255
*			 _usTimeOut : 命令执行超时，0表示一直等待. >０表示超时时间，单位1ms
*	返 回 值: 1 表示成功  0 表示失败
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

	/* _usTimeOut == 0 表示无限等待 */
	if (_usTimeOut > 0)
	{
		bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* 使用软件定时器3，作为超时控制 */
	}
	while (1)
	{
		bsp_Idle();				/* CPU空闲执行的操作， 见 bsp.c 和 bsp.h 文件 */

		if (_usTimeOut > 0)
		{
			if (bsp_CheckTimer(GPRS_TMR_ID))
			{
				ret = 0;	/* 超时 */
				break;
			}
		}

		if (comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */

			if (ucData == '\n')
			{
							if (pos > 0)	/* 第2次收到回车换行 */
							{
											if (memcmp(ucRxBuf, _pAckStr,  len) == 0)
											{
												ret = 1;	/* 收到指定的应答数据，返回成功 */
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
										/* 只保存可见字符 */
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
*	函 数 名: GPRS_ReadResponse
*	功能说明: 读取GPRS返回应答字符串。该函数根据字符间超时判断结束。 本函数需要紧跟AT命令发送函数。
*	形    参: _pBuf : 存放模块返回的完整字符串
*			  _usBufSize : 缓冲区最大长度
*			 _usTimeOut : 命令执行超时，0表示一直等待. >0 表示超时时间，单位1ms
*	返 回 值: 0 表示错误（超时）  > 0 表示应答的数据长度
*********************************************************************************************************
*/
uint16_t GPRS_ReadResponse(char *_pBuf, uint16_t _usBufSize, uint16_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint8_t ret;
	uint8_t status = 0;		/* 接收状态 */

	/* _usTimeOut == 0 表示无限等待 */
	if (_usTimeOut > 0)
	{
		bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* 使用软件定时器作为超时控制 */
	}
	while (1)
	{
		bsp_Idle();					/* CPU空闲执行的操作， 见 bsp.c 和 bsp.h 文件 */

		if (status == 2)		/* 正在接收有效应答阶段，通过字符间超时判断数据接收完毕 */
		{
			if (bsp_CheckTimer(GPRS_TMR_ID))
			{
				_pBuf[pos]	 = 0;	/* 结尾加0， 便于函数调用者识别字符串结束 */
				ret = pos;				/* 成功。 返回数据长度 */
				break;
			}
		}
		else
		{
			if (_usTimeOut > 0)
			{
				if (bsp_CheckTimer(GPRS_TMR_ID))
				{
					ret = 0;	/* 超时 */
					break;
				}
			}
		}
		
		if (comGetChar(COM_GPRS, &ucData))
		{			
			GPRS_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */

			switch (status)
			{
				case 0:									  /* 首字符 */
					if (ucData == AT_CR)		/* 如果首字符是回车，表示 AT命令不会显 */
					{
						_pBuf[pos++] = ucData;		/* 保存接收到的数据 */
						status = 2;	 /* 认为收到模块应答结果 */
					}
					else					 /* 首字符是 A 表示 AT命令回显 */
					{
						status = 1;	 /* 这是主机发送的AT命令字符串，不保存应答数据，直到遇到 CR字符 */
					}
					break;
					
				case 1:			/* AT命令回显阶段, 不保存数据. 继续等待 */
					if (ucData == AT_CR)
					{
						status = 2;
					}
					break;
					
				case 2:			/* 开始接收模块应答结果 */
										/* 只要收到模块的应答字符，则采用字符间超时判断结束，此时命令总超时不起作用 */
					bsp_StartTimer(GPRS_TMR_ID, 500);
					if (pos < _usBufSize - 1)
					{
						_pBuf[pos++] = ucData;		/* 保存接收到的数据 */
					}
					break;
			}
		}
	}
	return ret;
}

/*
*********************************************************************************************************
*	函 数 名: GPRS_SendAT
*	功能说明: 向GSM模块发送AT命令。 本函数自动在AT字符串口增加<CR>字符
*	形    参: _Str : AT命令字符串，不包括末尾的回车<CR>. 以字符0结束
*	返 回 值: 无
*********************************************************************************************************
*/
void GPRS_SendAT(char *_Cmd)
{
	comClearRxFifo(COM_GPRS);	/* 清零串口接收缓冲区 */	
	
	comSendBuf(COM_GPRS, (uint8_t *)_Cmd, strlen(_Cmd));
	comSendBuf(COM_GPRS, "\r", 1);
}

/*
*********************************************************************************************************
*	函 数 名: AT_SendToCmdData
*	功能说明: 发送控制指令数据到模组
*	形    参：_pCMDstr		 : AT 指令
*					: _pBACKstr    ：查询返回指令
*					:  vsNum			 : 访问次数，最少1次
*					: _usTimeOut	 : 溢出时间
*	返 回 值: 1 表示成功  0 表示失败
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
		for (i = 0; i < 5; i++)													//系统默认连续检测 5 次
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
*	函 数 名: ArrayToStr
*	功能说明: 将整数数组转换指定的字符串
*	形    参: arr：整数数组; size：长度; output：输出的字符串
*	返 回 值: 无
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
    buf[offset-1]=0;			//将最后一个 ';' 写 '\0'
    strcpy(output,buf);
}

/*
*********************************************************************************************************
*	函 数 名: StrToIntFix
*	功能说明: 将ASCII码字符串转换成十进制, 给定长度
*	形    参: _pStr :待转换的ASCII码串. 可以以逗号或0结束
*			 _ucLen : 固定长度
*	返 回 值: 二进制整数值
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
		flag = 1;	/* 负数 */
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
		if (ucTemp == '.')	/* 遇到小数点，自动跳过1个字节 */
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
*	函 数 名: AT_ResponseCSQ
*	功能说明: AT 指令响应 CSQ，并得到CSQ值
*	形    参: _vCSQ 值,_usTimeOut超时间
*	返 回 值: 1 表示成功  0: 表示异常
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
	
	/* 发送关键 AT 指令 */
	GPRS_SendAT("AT+CSQ");
	bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* 使用软件定时器作为超时控制 */
	while(1)
	{
		if (bsp_CheckTimer(GPRS_TMR_ID))
		{
			ret_value = 0;	/* 超时 */
			break;
		}
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */
			
			_pBuf[pos++] = ucData;
					if (ucData == '\n')		 /* 得到换行后 */					
					{
								_pCSQ = strstr(_pBuf,strATCMDCSQ);
						
									if(_pCSQ != NULL)		/* 地址检测到不是空 */
									{
									
												*_vCSQ = *(_pCSQ+2) - 0x30;
												if(*(_pCSQ+3) == ',')
												{
													ret_value = 1;		//检测到一位信号值
													break;
												}
												else
												{
													*_vCSQ =((*_vCSQ) * 10)+((*(_pCSQ+3)) - 0x30);//检测到两位信号值
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
*	函 数 名: AT_ResponseCGSN 
*	功能说明: AT 指令响应 CGSN, 得到产品序列号
*	形    参: _vCGSN 产品序列号,_usTimeOut超时间
*	返 回 值: 1 表示成功  0: 表示异常
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
	/* 发送关键 AT 指令 */
	GPRS_SendAT("AT+CGSN");
	bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* 使用软件定时器作为超时控制 */
	while(1)
	{
		if (bsp_CheckTimer(GPRS_TMR_ID))
		{
			ret_value = 0;	/* 超时 */
			break;
		}
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */
			
			_pBuf[pos++] = ucData;
					if (ucData == '\n')		 /* 得到换行后 */					
					{
							if(memcmp(_pBuf,ReContent[0],5) == 0)
							{						
									_pCGSN = strchr(_pBuf,strATCMDCGSN);				
									_pCGSN = _pCGSN + 2;				/* 地址先加2 找到正确数据*/
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
									ret_value = 0;	/* 错误退出 */
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
*	函 数 名: AT_ResponseCIMI 
*	功能说明: AT 指令响应 CGSN, 得到国际移动用户识别码
*	形    参: _vCGSN 产品序列号,_usTimeOut超时间
*	返 回 值: 1 表示成功  0: 表示异常
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
	/* 发送关键 AT 指令 */
	GPRS_SendAT("AT+CIMI");
	bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* 使用软件定时器作为超时控制 */
	while(1)
	{
		if (bsp_CheckTimer(GPRS_TMR_ID))
		{
			ret_value = 0;	/* 超时 */
			break;
		}
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */
			
			_pBuf[pos++] = ucData;
					if (ucData == '\n')		 /* 得到换行后 */					
					{
							if(memcmp(_pBuf,ReContent[0],5) == 0)
							{						
									_pCIMI = strchr(_pBuf,strATCMDCIMI);				
									_pCIMI = _pCIMI + 2;				/* 地址先加2 找到正确数据*/
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
									ret_value = 0;	/* 错误退出 */
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


