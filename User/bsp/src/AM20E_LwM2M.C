#include "bsp.h"			/* 底层硬件驱动 */
 
 
#define OBJIDNUM			  1			/* 对象个数 */
#define DEVOBJNUM01			3303	/* 设备对象 */
/*
#define DEVOBJNUM02			3...	
#define DEVOBJNUM03			3...	
#define DEVOBJNUM04			3...	
*/

#define	OBJ01ResourceNum   7		/* 对象1 资源7个 */
#define	OBJ02ResourceNum	 3		/* 对象2 资源2个 */

char EXE_arguments[64];
const uint32_t ObjAttArray01[OBJ01ResourceNum] = {5700,5701,5601,5602,5603,5604,5605};	  /* 属性数组,由属性个数决定 */

/* 初始化结构体 */
typedef struct
{
			uint32_t ucObjID;			/* 对象ID */
			uint32_t ucInstance;	/* 实例个数,默认一个实例 */
	 const char  *strmap;			/* 实例位图 */
			uint8_t  ucAttribute;	/* 属性个数 */
			uint8_t	 ucAction;		/* 操作个数 */
	const uint32_t  *AttArray;/* 属性数组,由属性个数决定 */
}LwM2M_T;


/* 读结构体 */
typedef struct
{
			uint32_t msgid;				/* 消息ID */
			uint32_t objectid;		/* 对象ID */
			uint32_t instanceid;  /* 实例ID */
			 int32_t resourceid;	/* 资源ID */
}READINFO_T;

/* 写结构体 */
typedef struct
{
	
			uint32_t msgid;				/* 消息ID */
			uint32_t objectid;		/* 对象ID */
			uint32_t instanceid;  /* 实例ID */
			uint32_t resourceid;	/* 资源ID */
	    uint32_t valuetype;		/* 数据类型ID */
			/*
					1：string	//不支持
					2：opaque
					3：interger
					4：float
					5：bool
			*/
		 uint32_t valuelen;			/* 数据长度 */
		 uint32_t	value;				/* 数据值 */
	
			/* 第一条消息 超长消息与中间消息不支持 */
}WRITEINFO_T;

/* 执行结构体 */
typedef struct
{
	
			uint32_t msgid;				/* 消息ID */
			uint32_t objectid;		/* 对象ID */
			uint32_t instanceid;  /* 实例ID */
			uint32_t resourceid;	/* 资源ID */
			uint32_t len;					/* 属性长度 */
	      char *arguments;    /* 属性 */
	
}EXECUTEINFO_T;


READINFO_T     t_ReadINFO 		= { 0 };		/* 读结构体 */		
WRITEINFO_T    t_WriteINFO	 	= { 0 };		/* 写结构体 */	
EXECUTEINFO_T  t_ExecuteINFO  = { 0,0,0,0,0,EXE_arguments};		/* 执行结构体 */	

/* 定义对象属性值 */
LwM2M_T pDevModule[OBJIDNUM] = 
//{
	{ DEVOBJNUM01,1,"1",OBJ01ResourceNum,OBJ01ResourceNum,ObjAttArray01};	  	/* 设备模组 包含 一个对象3303, 1个实例, 2个属性, 激活两个属性  */
//};

/*
*********************************************************************************************************
*	函 数 名: GPRS_PowerOn
*	功能说明: 模块上电. 函数内部先判断是否已经开机，如果已开机则直接返回1
*	形    参: 无
*	返 回 值: 1 表示成功  0: 表示异常
*********************************************************************************************************
*/
uint8_t GPRS_PowerOn(void)
{
	uint8_t ret = 0;
	uint8_t CSQ = 0;
//	char CGSN[24] = { 0 };
//	char CIMI[24] = { 0 };
	/* 判断是否开机 */
	ret =	AT_SendToCmdData("AT","OK",5,1000);
	ret =	AT_ResponseCSQ(&CSQ,5000);
//	ret =	AT_ResponseCGSN(CGSN,5000);
//	ret =	AT_ResponseCIMI(CIMI,5000);
	
	printf("CSQ :%d\r\n",CSQ);
//	printf("CGSN:%s\r\n",CGSN);
//	printf("CIMI:%s\r\n",CIMI);
	
	/* 查询模组是否成功驻网 */
	ret =	AT_SendToCmdData("AT+CEREG?","+CEREG: 1, 1",5,1000);
	ret =	AT_SendToCmdData("AT+CGACT=1,1","OK",5,1000);
//  	ret =	AT_SendToCmdData("AT+XIIC?","OK",5,1000);

	comClearRxFifo(COM_GPRS);	/* 清零串口接收缓冲区 */

	return ret;
}

/*
*********************************************************************************************************
*	函 数 名: ParaAnalysis
*	功能说明: 平台下发消息后解析参数是那个类型
*	形    参：_pMsg 得到参数,_usTimeOut超时时间
*	返 回 值: 返回消息类型 1:MIPLOBSE 2:MIPLDISC 3:+MIPLREAD; 4:+MIPLWRITE; 5:+MIPLEXECUTE
*********************************************************************************************************
*/

uint16_t ParaAnalysis(char _pBuf[128], uint32_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint8_t  ret = 0;
//	char _pBuf[128] = { 0 };
	
	/* 消息内容列表 */
	char *MsgContent[] = {
	"+MIPLOBSE",
	"+MIPLDISC",
	"+MIPLREAD",
	"+MIPLWRIT",
	"+MIPLEXEC",
	};
	bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* 使用软件定时器作为超时控制 */
	while(1)
	{
		if (bsp_CheckTimer(GPRS_TMR_ID))
		{
			ret = 0;		/* 超时 */
			break;
		}
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */
			
			_pBuf[pos++] = ucData;
					if (ucData == '\n')		 /* 得到换行后 */					
					{
							if(memcmp(_pBuf,MsgContent[0],9) == 0)
							{		
								ret = 1;
								break;
							}
							else if(memcmp(_pBuf,MsgContent[1],9) == 0)
							{		
								ret = 2;
								break;
							}
							else if(memcmp(_pBuf,MsgContent[2],9) == 0)
							{		
								ret = 3;
								break;
							}
							else if(memcmp(_pBuf,MsgContent[3],9) == 0)
							{			
								ret = 4;
								break;
							}
							else if(memcmp(_pBuf,MsgContent[4],9) == 0)
							{
								ret = 5;
								break;
							}
							else
							{
								pos = 0;
								memset(_pBuf,0,strlen(_pBuf));
							}
					}		
			}
	}
	
	return ret;
}

/*
*********************************************************************************************************
*	消息举例：+MIPLOBSERVE:0,106484,1,3303,0,-1
*	函 数 名: ObserveAnalysis
*	功能说明: 平台响应
*	形    参：
*	返 回 值: 1 表示成功  0: 表示异常
*********************************************************************************************************
*/

uint8_t ObserveAnalysis(char *_ucaBuf,uint32_t *msgid, uint32_t *objectid)
{	
	uint8_t i = 0;
	char _pBuf[64] = { 0 };
	char *_pPOS;
	while(1)
	{
			_pPOS = strchr(_ucaBuf,',');	/* 找到第一个 ',' */
			_pPOS = _pPOS + 1;						/* 地址先加1 找到信息id */
			while((*_pPOS) != ',')
			{
				
				_pBuf[i] = *_pPOS;			
				i++;											
				_pPOS++;
										
			}
			*msgid = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */
			i = 0;
			_pPOS = _pPOS + 3;				/* 地址先加1 找到对象id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			*objectid = StrToIntFix(_pBuf,strlen(_pBuf));				
			break;
	}	
	return 1;
}

/*
*********************************************************************************************************
*	消息举例：+MIPLDISCOVER:0,40949,3303
*	函 数 名: DiscoverAnalysis
*	功能说明: 平台响应
*	形    参：
*	返 回 值: 1 表示成功  0: 表示异常
*********************************************************************************************************
*/

uint8_t DiscoverAnalysis(char *_ucaBuf,uint32_t *msgid, uint32_t *objectid)
{	
	uint8_t i = 0;
	char _pBuf[64] = { 0 };
	char *_pPOS;
	while(1)
	{
			_pPOS = strchr(_ucaBuf,',');	/* 找到第一个 ',' */
			_pPOS = _pPOS + 1;						/* 地址先加1 找到信息id */
			while((*_pPOS) != ',')
			{
				
				_pBuf[i] = *_pPOS;			
				i++;											
				_pPOS++;
										
			}
			*msgid = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到对象id */
			while((*_pPOS) != 0x0D)
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			*objectid = StrToIntFix(_pBuf,strlen(_pBuf));				
			break;
	}	
	return 1;
}

/*
*********************************************************************************************************
*	消息举例：+MIPLREAD:0,39280,3303,0,-1
*	函 数 名: ReadAnalysis
*	功能说明: 读 解析消息类型
*	形    参：
*	返 回 值: 1 表示成功  0: 表示异常
*********************************************************************************************************
*/

uint8_t ReadAnalysis(char *_ucaBuf)
{	
	uint8_t i = 0;
	char _pBuf[64] = { 0 };
	char *_pPOS;
	while(1)
	{
			_pPOS = strchr(_ucaBuf,',');	/* 找到第一个 ',' */
			_pPOS = _pPOS + 1;						/* 地址先加1 找到信息id */
			while((*_pPOS) != ',')
			{
				
				_pBuf[i] = *_pPOS;			
				i++;											
				_pPOS++;
										
			}
			t_ReadINFO.msgid = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到对象id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ReadINFO.objectid = StrToIntFix(_pBuf,strlen(_pBuf));		
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */		
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到实例id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ReadINFO.instanceid = StrToIntFix(_pBuf,strlen(_pBuf));	
		
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */	
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到实例id */
			while((*_pPOS) != 0x0D)
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ReadINFO.resourceid = StrToIntFix(_pBuf,strlen(_pBuf));				
			
			break;
	}
	
	return 1;
}


/*
*********************************************************************************************************
*	消息举例：+MIPLWRITE:0,43357,3306,0,5850,5,1,0,0,0
*	函 数 名: WriteAnalysis
*	功能说明: 写 解析消息类型
*	形    参：
*	返 回 值: 1 表示成功  0: 表示异常
*********************************************************************************************************
*/
uint8_t WriteAnalysis(char *_ucaBuf)
{
	uint8_t i = 0;
	char _pBuf[64] = { 0 };
	char *_pPOS;
	while(1)
	{
			_pPOS = strchr(_ucaBuf,',');	/* 找到第一个 ',' */
			_pPOS = _pPOS + 1;						/* 地址先加1 找到消息id */
			while((*_pPOS) != ',')
			{
				
				_pBuf[i] = *_pPOS;			
				i++;											
				_pPOS++;
										
			}
			t_WriteINFO.msgid = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到对象id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.objectid = StrToIntFix(_pBuf,strlen(_pBuf));		
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */	
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到实例id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.instanceid = StrToIntFix(_pBuf,strlen(_pBuf));		
				
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */	
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到资源id */		
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.resourceid = StrToIntFix(_pBuf,strlen(_pBuf));	
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */	
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到数据类型id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.valuetype = StrToIntFix(_pBuf,strlen(_pBuf));	
					
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */	
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到数据长度ID,不支持字符串 */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.valuelen = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */	
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到数据值 */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.value = StrToIntFix(_pBuf,strlen(_pBuf));
			
			break;
	}
	
	return 1;	
}

/*
*********************************************************************************************************
*	消息举例：+MIPLEXECUTE:0,43357,3306,0,5850,5,“reset”
*	函 数 名: ExecuteAnalysis
*	功能说明: 执行	 解析消息类型
*	形    参：
*	返 回 值: 1 表示成功  0: 表示异常
*********************************************************************************************************
*/
uint8_t ExecuteAnalysis(char *_ucaBuf)
{
	uint8_t i = 0;
	char _pBuf[64] = { 0 };
	char *_pPOS;
	while(1)
	{
			_pPOS = strchr(_ucaBuf,',');	/* 找到第一个 ',' */
			_pPOS = _pPOS + 1;						/* 地址先加1 找到消息id */
			while((*_pPOS) != ',')
			{
				
				_pBuf[i] = *_pPOS;			
				i++;											
				_pPOS++;
										
			}
			t_ExecuteINFO.msgid = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到对象id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ExecuteINFO.objectid = StrToIntFix(_pBuf,strlen(_pBuf));		
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */	
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到实例id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ExecuteINFO.instanceid = StrToIntFix(_pBuf,strlen(_pBuf));		
				
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */	
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到资源id */		
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ExecuteINFO.resourceid = StrToIntFix(_pBuf,strlen(_pBuf));	
			
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */	
			i = 0;
			_pPOS = _pPOS + 1;				/* 地址先加1 找到属性长度 */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ExecuteINFO.len = StrToIntFix(_pBuf,strlen(_pBuf));	
		
			memset(_pBuf,0,sizeof(_pBuf));				/* 清楚缓存内容 */	
			i = 0;
			_pPOS = _pPOS + 2;				/* 地址先加1 找到属性内容 */
			while((*_pPOS) != '"')
			{
				t_ExecuteINFO.arguments[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}			
			break;
	}
	
	return 1;	
}

/*
*********************************************************************************************************
*	函 数 名: MsgAnalysis
*	功能说明: 解析消息类型
*	形    参：
*	返 回 值: 1：解析成功 0：解析失败 
*********************************************************************************************************
*/

uint8_t MsgAnalysis(uint32_t _usTimeOut)
{
		uint8_t MsgNum =0;
		uint32_t MsgID = 0;
		uint32_t MsgIDbuf[20] = { 0 };
	
		uint32_t ObjID = 0;
		uint32_t ResponseType;			/* 平台应答消息类型 1:MIPLOBSE; 2:MIPLDISC; 3:+MIPLREAD; 4:+MIPLWRITE; 5:+MIPLEXECUTE */
		char _pMsgBuf[128] = { 0 };	
		char _pSendBuf[128] = { 0 };
		char	ucArrayBuf[128];
		bsp_StartAutoTimer(GENERAL_TMR_ID, _usTimeOut);		/* 使用软件定时器作为超时控制 */
		while(1)
		{
				if (bsp_CheckTimer(GENERAL_TMR_ID))
				{				
					/* 在此加入基础资源上报 */
					if(MsgID != 0 )
					{
							sprintf(_pSendBuf,"AT+MIPLNOTIFY=0,%d,3303,0,5700,4,3,9.8,0,0",MsgIDbuf[0]);
							AT_SendToCmdData(_pSendBuf,"+MIPLEVENT:0,26",5,1000);
							memset(_pSendBuf,0,sizeof(_pSendBuf));  /* 清楚缓存 */
					}
				}			
				
				
					ResponseType = ParaAnalysis(_pMsgBuf,1000);
					if(ResponseType == 1)
					{
							ObserveAnalysis(_pMsgBuf,&MsgID,&ObjID);
							MsgIDbuf[MsgNum++] = MsgID;
						
							sprintf(_pSendBuf,"AT+MIPLOBSERVERSP=0,%d,1",MsgID);
							AT_SendToCmdData(_pSendBuf,"+MIPLEVENT:0,21",5,1000);
							memset(_pSendBuf,0,sizeof(_pSendBuf));  /* 清楚缓存 */
					}
					else if(ResponseType == 2)
					{
					    DiscoverAnalysis(_pMsgBuf,&MsgID,&ObjID);						
							if(ObjID == pDevModule[0].ucObjID)
							{
								ArrayToStr(pDevModule[0].AttArray,OBJ01ResourceNum,ucArrayBuf);
								sprintf(_pSendBuf,"AT+MIPLDISCOVERRSP=0,%d,1,%d,\"%s\"",MsgID,							/* 消息ID */
																																				strlen(ucArrayBuf),	/* 发送数据长度 */
																																				ucArrayBuf);				/* 数据内容 */
							}
							AT_SendToCmdData(_pSendBuf,"+MIPLEVENT:0,21",5,1000);
							memset(_pSendBuf,0,sizeof(_pSendBuf));  	/* 清楚缓存 */
					}						
					else if(ResponseType == 3)						//解析 读
					{
						ReadAnalysis(_pMsgBuf);					
						printf("ReadAnalysis:  %d,%d,%d,%d\r\n",t_ReadINFO.msgid,				//消息ID
																										t_ReadINFO.objectid,		//对象ID
																										t_ReadINFO.instanceid,	//实例ID 
																										t_ReadINFO.resourceid); //资源ID
						
						switch (t_ReadINFO.objectid)
            {
							/* 检测到平台读 */
            	case DEVOBJNUM01 :
																//AT+MIPLREADRSP=0,39280,1,3303,0,5700,4,13,7.57421538099,0,0
																/* 通信标识, 消息ID, 操作结果, 对象ID, 实例ID, 资源ID, 发送数据类型, 发送的数据长度, 发送的数据, index默认0, flag默认0  */											
																	if(t_ReadINFO.resourceid == pDevModule[0].AttArray[0])
																	{
																		sprintf(_pSendBuf,"AT+MIPLREADRSP=0,%d,1,%d,0,%d,4,3,7.5,0,0",t_ReadINFO.msgid,t_ReadINFO.objectid,pDevModule[0].AttArray[0]);
																		AT_SendToCmdData(_pSendBuf,"OK",5,1000);	
																		memset(_pSendBuf,0,sizeof(_pSendBuf));  /* 清楚缓存 */
																		break; //跳出 for 循环
																	}else if(t_ReadINFO.resourceid == pDevModule[0].AttArray[1])
																	{
																		sprintf(_pSendBuf,"AT+MIPLREADRSP=0,%d,1,%d,0,%d,4,4,7.57,0,0",t_ReadINFO.msgid,t_ReadINFO.objectid,pDevModule[0].AttArray[1]);
																		AT_SendToCmdData(_pSendBuf,"OK",5,1000);	
																		memset(_pSendBuf,0,sizeof(_pSendBuf));  /* 清楚缓存 */														
																	}
																	/*
																		else if(t_ReadINFO.resourceid == pDevModule[0].AttArray[2])
																		{
																				...
																		}
																		else if		...
																	*/
																																		
            		break;
							/*
								
							 case DEVOBJNUM02 : 	...
								
            		break;
							
							*/
            }
					}else if(ResponseType == 4)			//解析 写
					{
						WriteAnalysis(_pMsgBuf);
						printf("WriteAnalysis:  %d,%d,%d,%d,%d,%d,%d \r\n",  t_WriteINFO.msgid,			//消息ID
																																 t_WriteINFO.objectid,	//对象ID
																																 t_WriteINFO.instanceid,//实例ID
																																 t_WriteINFO.resourceid,//资源ID
																																 t_WriteINFO.valuetype,	//值类型
																																 t_WriteINFO.valuelen,	//值长度
																																 t_WriteINFO.value);		//值
						/* AT+MIPLWRITERSP=0,43357,2 */
						/* 套件标识, 消息ID, 2:正确 */
						sprintf(_pSendBuf,"AT+MIPLWRITERSP=0,%d,2",t_WriteINFO.msgid);
						AT_SendToCmdData(_pSendBuf,"OK",5,1000);
						memset(_pSendBuf,0,sizeof(_pSendBuf));  /* 清楚缓存 */
						
					}else if(ResponseType == 5)			//解析 执行
					{
						ExecuteAnalysis(_pMsgBuf);
						printf("ExecuteAnalysis:  %d,%d,%d,%d,%d,%s\r\n",t_ExecuteINFO.msgid,					//消息ID
																														 t_ExecuteINFO.objectid,			//对象ID
																														 t_ExecuteINFO.instanceid,		//实例ID
																														 t_ExecuteINFO.resourceid,		//资源ID
																														 t_ExecuteINFO.len,						//值长度
																														 t_ExecuteINFO.arguments);		//值
						/* AT+MIPLEXECUTERSP=0,43357,2 */
						/* 套件标识, 消息ID, 2:正确 */
						sprintf(_pSendBuf,"AT+MIPLEXECUTERSP=0,%d,2",t_ExecuteINFO.msgid);
						AT_SendToCmdData(_pSendBuf,"OK",5,1000);
						memset(_pSendBuf,0,sizeof(_pSendBuf));  /* 清楚缓存 */
					}					
		}
}

/*
*********************************************************************************************************
*	函 数 名: LwM2MCreateOBJ
*	功能说明: 创建对象实例
*	形    参：无
*	返 回 值: 1 表示成功  0: 表示异常
*********************************************************************************************************
*/
uint8_t LwM2MCreateOBJ(void)
{
		uint8_t i;
		uint8_t ret = 0;
//		int8_t Resub;		/* 响应订阅 */
//		uint8_t ResubNUM = 0;		/* 响应订阅个数*/
//		uint32_t msgID,ObjID;
		
		char	ucBuf[128];
	
	  char *MIPLEVENT[] = {
													"+MIPLEVENT:0,1",
													"+MIPLEVENT:0,2",
													"+MIPLEVENT:0,4",
													"+MIPLEVENT:0,6",
		};
		
		AT_SendToCmdData("AT+MIPLCREATE","+MIPLCREATE:0",5,1000);	
		for(i=0; i<OBJIDNUM; i++)
		{
			sprintf(ucBuf,"AT+MIPLADDOBJ=0,%d,%d,\"%s\",%d,%d",pDevModule[i].ucObjID,			/* 对象ID */
																												 pDevModule[i].ucInstance,		/* 实例个数 */
																												 pDevModule[i].strmap,				/* 实例位图 */
																												 pDevModule[i].ucAttribute,	/* 属性个数 */
																												 pDevModule[i].ucAction);		/* 激活个数 */
			if(AT_SendToCmdData(ucBuf,"OK",5,1000) == 0)
			{
				return ret;
			}
		}
		AT_SendToCmdData("AT+MIPLOPEN=0,3600","OK",5,1000);		/* 模组请求注册 */
		
		i = 0;
		bsp_StartTimer(GPRS_TMR_ID, 10000);		/* 使用软件定时器3，作为超时控制 */
		while(1)
		{
			if (bsp_CheckTimer(GPRS_TMR_ID))
			{
				ret = 0;	/* 超时 */
				break;
			}			
			if(GPRS_WaitResponse(MIPLEVENT[i],2000))
			{
				++i;
				printf("break %d\r\n ",i);
			}
			if(i >= 4)
			{
				ret = 1;	/* 成功 */
				break;
			}	
		}
		/* 等待平台响应应答 */
		printf("\r\nSuccessful connection \r\n ");
		return ret;
}


/*
*********************************************************************************************************
*	函 数 名: AM20E_init
*	功能说明: AM20E开机初始化
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void AM20E_init(void)
{	
	/* 在此处添加控制开机引脚 */
	
	
	/* GPRS模块相关参数读取检测相关状态 */
	if(GPRS_PowerOn())
	{		
		LwM2MCreateOBJ();
		return;
	}else{
		goto exit;
	}
	
	/* 响应失败后加入失败代码 */
exit:
	while(1)
	{	
	    printf(" AM20E Response failure   \n ");
			printf(" AM20E system Reswet ...  \r\n ");
			
		 /* 系统复位前执行代码 */
		 
		 /* 系统复位 */
//		SystemReset();			
		  while(1);
	}
}

	





















