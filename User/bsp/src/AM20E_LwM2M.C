#include "bsp.h"			/* �ײ�Ӳ������ */
 
 
#define OBJIDNUM			  1			/* ������� */
#define DEVOBJNUM01			3303	/* �豸���� */
/*
#define DEVOBJNUM02			3...	
#define DEVOBJNUM03			3...	
#define DEVOBJNUM04			3...	
*/

#define	OBJ01ResourceNum   7		/* ����1 ��Դ7�� */
#define	OBJ02ResourceNum	 3		/* ����2 ��Դ2�� */

char EXE_arguments[64];
const uint32_t ObjAttArray01[OBJ01ResourceNum] = {5700,5701,5601,5602,5603,5604,5605};	  /* ��������,�����Ը������� */

/* ��ʼ���ṹ�� */
typedef struct
{
			uint32_t ucObjID;			/* ����ID */
			uint32_t ucInstance;	/* ʵ������,Ĭ��һ��ʵ�� */
	 const char  *strmap;			/* ʵ��λͼ */
			uint8_t  ucAttribute;	/* ���Ը��� */
			uint8_t	 ucAction;		/* �������� */
	const uint32_t  *AttArray;/* ��������,�����Ը������� */
}LwM2M_T;


/* ���ṹ�� */
typedef struct
{
			uint32_t msgid;				/* ��ϢID */
			uint32_t objectid;		/* ����ID */
			uint32_t instanceid;  /* ʵ��ID */
			 int32_t resourceid;	/* ��ԴID */
}READINFO_T;

/* д�ṹ�� */
typedef struct
{
	
			uint32_t msgid;				/* ��ϢID */
			uint32_t objectid;		/* ����ID */
			uint32_t instanceid;  /* ʵ��ID */
			uint32_t resourceid;	/* ��ԴID */
	    uint32_t valuetype;		/* ��������ID */
			/*
					1��string	//��֧��
					2��opaque
					3��interger
					4��float
					5��bool
			*/
		 uint32_t valuelen;			/* ���ݳ��� */
		 uint32_t	value;				/* ����ֵ */
	
			/* ��һ����Ϣ ������Ϣ���м���Ϣ��֧�� */
}WRITEINFO_T;

/* ִ�нṹ�� */
typedef struct
{
	
			uint32_t msgid;				/* ��ϢID */
			uint32_t objectid;		/* ����ID */
			uint32_t instanceid;  /* ʵ��ID */
			uint32_t resourceid;	/* ��ԴID */
			uint32_t len;					/* ���Գ��� */
	      char *arguments;    /* ���� */
	
}EXECUTEINFO_T;


READINFO_T     t_ReadINFO 		= { 0 };		/* ���ṹ�� */		
WRITEINFO_T    t_WriteINFO	 	= { 0 };		/* д�ṹ�� */	
EXECUTEINFO_T  t_ExecuteINFO  = { 0,0,0,0,0,EXE_arguments};		/* ִ�нṹ�� */	

/* �����������ֵ */
LwM2M_T pDevModule[OBJIDNUM] = 
//{
	{ DEVOBJNUM01,1,"1",OBJ01ResourceNum,OBJ01ResourceNum,ObjAttArray01};	  	/* �豸ģ�� ���� һ������3303, 1��ʵ��, 2������, ������������  */
//};

/*
*********************************************************************************************************
*	�� �� ��: GPRS_PowerOn
*	����˵��: ģ���ϵ�. �����ڲ����ж��Ƿ��Ѿ�����������ѿ�����ֱ�ӷ���1
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
uint8_t GPRS_PowerOn(void)
{
	uint8_t ret = 0;
	uint8_t CSQ = 0;
//	char CGSN[24] = { 0 };
//	char CIMI[24] = { 0 };
	/* �ж��Ƿ񿪻� */
	ret =	AT_SendToCmdData("AT","OK",5,1000);
	ret =	AT_ResponseCSQ(&CSQ,5000);
//	ret =	AT_ResponseCGSN(CGSN,5000);
//	ret =	AT_ResponseCIMI(CIMI,5000);
	
	printf("CSQ :%d\r\n",CSQ);
//	printf("CGSN:%s\r\n",CGSN);
//	printf("CIMI:%s\r\n",CIMI);
	
	/* ��ѯģ���Ƿ�ɹ�פ�� */
	ret =	AT_SendToCmdData("AT+CEREG?","+CEREG: 1, 1",5,1000);
	ret =	AT_SendToCmdData("AT+CGACT=1,1","OK",5,1000);
//  	ret =	AT_SendToCmdData("AT+XIIC?","OK",5,1000);

	comClearRxFifo(COM_GPRS);	/* ���㴮�ڽ��ջ����� */

	return ret;
}

/*
*********************************************************************************************************
*	�� �� ��: ParaAnalysis
*	����˵��: ƽ̨�·���Ϣ������������Ǹ�����
*	��    �Σ�_pMsg �õ�����,_usTimeOut��ʱʱ��
*	�� �� ֵ: ������Ϣ���� 1:MIPLOBSE 2:MIPLDISC 3:+MIPLREAD; 4:+MIPLWRITE; 5:+MIPLEXECUTE
*********************************************************************************************************
*/

uint16_t ParaAnalysis(char _pBuf[128], uint32_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint8_t  ret = 0;
//	char _pBuf[128] = { 0 };
	
	/* ��Ϣ�����б� */
	char *MsgContent[] = {
	"+MIPLOBSE",
	"+MIPLDISC",
	"+MIPLREAD",
	"+MIPLWRIT",
	"+MIPLEXEC",
	};
	bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* ʹ��������ʱ����Ϊ��ʱ���� */
	while(1)
	{
		if (bsp_CheckTimer(GPRS_TMR_ID))
		{
			ret = 0;		/* ��ʱ */
			break;
		}
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */
			
			_pBuf[pos++] = ucData;
					if (ucData == '\n')		 /* �õ����к� */					
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
*	��Ϣ������+MIPLOBSERVE:0,106484,1,3303,0,-1
*	�� �� ��: ObserveAnalysis
*	����˵��: ƽ̨��Ӧ
*	��    �Σ�
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/

uint8_t ObserveAnalysis(char *_ucaBuf,uint32_t *msgid, uint32_t *objectid)
{	
	uint8_t i = 0;
	char _pBuf[64] = { 0 };
	char *_pPOS;
	while(1)
	{
			_pPOS = strchr(_ucaBuf,',');	/* �ҵ���һ�� ',' */
			_pPOS = _pPOS + 1;						/* ��ַ�ȼ�1 �ҵ���Ϣid */
			while((*_pPOS) != ',')
			{
				
				_pBuf[i] = *_pPOS;			
				i++;											
				_pPOS++;
										
			}
			*msgid = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */
			i = 0;
			_pPOS = _pPOS + 3;				/* ��ַ�ȼ�1 �ҵ�����id */
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
*	��Ϣ������+MIPLDISCOVER:0,40949,3303
*	�� �� ��: DiscoverAnalysis
*	����˵��: ƽ̨��Ӧ
*	��    �Σ�
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/

uint8_t DiscoverAnalysis(char *_ucaBuf,uint32_t *msgid, uint32_t *objectid)
{	
	uint8_t i = 0;
	char _pBuf[64] = { 0 };
	char *_pPOS;
	while(1)
	{
			_pPOS = strchr(_ucaBuf,',');	/* �ҵ���һ�� ',' */
			_pPOS = _pPOS + 1;						/* ��ַ�ȼ�1 �ҵ���Ϣid */
			while((*_pPOS) != ',')
			{
				
				_pBuf[i] = *_pPOS;			
				i++;											
				_pPOS++;
										
			}
			*msgid = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ�����id */
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
*	��Ϣ������+MIPLREAD:0,39280,3303,0,-1
*	�� �� ��: ReadAnalysis
*	����˵��: �� ������Ϣ����
*	��    �Σ�
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/

uint8_t ReadAnalysis(char *_ucaBuf)
{	
	uint8_t i = 0;
	char _pBuf[64] = { 0 };
	char *_pPOS;
	while(1)
	{
			_pPOS = strchr(_ucaBuf,',');	/* �ҵ���һ�� ',' */
			_pPOS = _pPOS + 1;						/* ��ַ�ȼ�1 �ҵ���Ϣid */
			while((*_pPOS) != ',')
			{
				
				_pBuf[i] = *_pPOS;			
				i++;											
				_pPOS++;
										
			}
			t_ReadINFO.msgid = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ�����id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ReadINFO.objectid = StrToIntFix(_pBuf,strlen(_pBuf));		
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */		
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ�ʵ��id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ReadINFO.instanceid = StrToIntFix(_pBuf,strlen(_pBuf));	
		
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */	
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ�ʵ��id */
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
*	��Ϣ������+MIPLWRITE:0,43357,3306,0,5850,5,1,0,0,0
*	�� �� ��: WriteAnalysis
*	����˵��: д ������Ϣ����
*	��    �Σ�
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
uint8_t WriteAnalysis(char *_ucaBuf)
{
	uint8_t i = 0;
	char _pBuf[64] = { 0 };
	char *_pPOS;
	while(1)
	{
			_pPOS = strchr(_ucaBuf,',');	/* �ҵ���һ�� ',' */
			_pPOS = _pPOS + 1;						/* ��ַ�ȼ�1 �ҵ���Ϣid */
			while((*_pPOS) != ',')
			{
				
				_pBuf[i] = *_pPOS;			
				i++;											
				_pPOS++;
										
			}
			t_WriteINFO.msgid = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ�����id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.objectid = StrToIntFix(_pBuf,strlen(_pBuf));		
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */	
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ�ʵ��id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.instanceid = StrToIntFix(_pBuf,strlen(_pBuf));		
				
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */	
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ���Դid */		
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.resourceid = StrToIntFix(_pBuf,strlen(_pBuf));	
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */	
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ���������id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.valuetype = StrToIntFix(_pBuf,strlen(_pBuf));	
					
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */	
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ����ݳ���ID,��֧���ַ��� */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_WriteINFO.valuelen = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */	
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ�����ֵ */
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
*	��Ϣ������+MIPLEXECUTE:0,43357,3306,0,5850,5,��reset��
*	�� �� ��: ExecuteAnalysis
*	����˵��: ִ��	 ������Ϣ����
*	��    �Σ�
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
uint8_t ExecuteAnalysis(char *_ucaBuf)
{
	uint8_t i = 0;
	char _pBuf[64] = { 0 };
	char *_pPOS;
	while(1)
	{
			_pPOS = strchr(_ucaBuf,',');	/* �ҵ���һ�� ',' */
			_pPOS = _pPOS + 1;						/* ��ַ�ȼ�1 �ҵ���Ϣid */
			while((*_pPOS) != ',')
			{
				
				_pBuf[i] = *_pPOS;			
				i++;											
				_pPOS++;
										
			}
			t_ExecuteINFO.msgid = StrToIntFix(_pBuf,strlen(_pBuf));
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ�����id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ExecuteINFO.objectid = StrToIntFix(_pBuf,strlen(_pBuf));		
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */	
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ�ʵ��id */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ExecuteINFO.instanceid = StrToIntFix(_pBuf,strlen(_pBuf));		
				
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */	
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ���Դid */		
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ExecuteINFO.resourceid = StrToIntFix(_pBuf,strlen(_pBuf));	
			
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */	
			i = 0;
			_pPOS = _pPOS + 1;				/* ��ַ�ȼ�1 �ҵ����Գ��� */
			while((*_pPOS) != ',')
			{
				_pBuf[i] = *_pPOS;	
				_pPOS++;
				 i++;											
			}
			t_ExecuteINFO.len = StrToIntFix(_pBuf,strlen(_pBuf));	
		
			memset(_pBuf,0,sizeof(_pBuf));				/* ����������� */	
			i = 0;
			_pPOS = _pPOS + 2;				/* ��ַ�ȼ�1 �ҵ��������� */
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
*	�� �� ��: MsgAnalysis
*	����˵��: ������Ϣ����
*	��    �Σ�
*	�� �� ֵ: 1�������ɹ� 0������ʧ�� 
*********************************************************************************************************
*/

uint8_t MsgAnalysis(uint32_t _usTimeOut)
{
		uint8_t MsgNum =0;
		uint32_t MsgID = 0;
		uint32_t MsgIDbuf[20] = { 0 };
	
		uint32_t ObjID = 0;
		uint32_t ResponseType;			/* ƽ̨Ӧ����Ϣ���� 1:MIPLOBSE; 2:MIPLDISC; 3:+MIPLREAD; 4:+MIPLWRITE; 5:+MIPLEXECUTE */
		char _pMsgBuf[128] = { 0 };	
		char _pSendBuf[128] = { 0 };
		char	ucArrayBuf[128];
		bsp_StartAutoTimer(GENERAL_TMR_ID, _usTimeOut);		/* ʹ��������ʱ����Ϊ��ʱ���� */
		while(1)
		{
				if (bsp_CheckTimer(GENERAL_TMR_ID))
				{				
					/* �ڴ˼��������Դ�ϱ� */
					if(MsgID != 0 )
					{
							sprintf(_pSendBuf,"AT+MIPLNOTIFY=0,%d,3303,0,5700,4,3,9.8,0,0",MsgIDbuf[0]);
							AT_SendToCmdData(_pSendBuf,"+MIPLEVENT:0,26",5,1000);
							memset(_pSendBuf,0,sizeof(_pSendBuf));  /* ������� */
					}
				}			
				
				
					ResponseType = ParaAnalysis(_pMsgBuf,1000);
					if(ResponseType == 1)
					{
							ObserveAnalysis(_pMsgBuf,&MsgID,&ObjID);
							MsgIDbuf[MsgNum++] = MsgID;
						
							sprintf(_pSendBuf,"AT+MIPLOBSERVERSP=0,%d,1",MsgID);
							AT_SendToCmdData(_pSendBuf,"+MIPLEVENT:0,21",5,1000);
							memset(_pSendBuf,0,sizeof(_pSendBuf));  /* ������� */
					}
					else if(ResponseType == 2)
					{
					    DiscoverAnalysis(_pMsgBuf,&MsgID,&ObjID);						
							if(ObjID == pDevModule[0].ucObjID)
							{
								ArrayToStr(pDevModule[0].AttArray,OBJ01ResourceNum,ucArrayBuf);
								sprintf(_pSendBuf,"AT+MIPLDISCOVERRSP=0,%d,1,%d,\"%s\"",MsgID,							/* ��ϢID */
																																				strlen(ucArrayBuf),	/* �������ݳ��� */
																																				ucArrayBuf);				/* �������� */
							}
							AT_SendToCmdData(_pSendBuf,"+MIPLEVENT:0,21",5,1000);
							memset(_pSendBuf,0,sizeof(_pSendBuf));  	/* ������� */
					}						
					else if(ResponseType == 3)						//���� ��
					{
						ReadAnalysis(_pMsgBuf);					
						printf("ReadAnalysis:  %d,%d,%d,%d\r\n",t_ReadINFO.msgid,				//��ϢID
																										t_ReadINFO.objectid,		//����ID
																										t_ReadINFO.instanceid,	//ʵ��ID 
																										t_ReadINFO.resourceid); //��ԴID
						
						switch (t_ReadINFO.objectid)
            {
							/* ��⵽ƽ̨�� */
            	case DEVOBJNUM01 :
																//AT+MIPLREADRSP=0,39280,1,3303,0,5700,4,13,7.57421538099,0,0
																/* ͨ�ű�ʶ, ��ϢID, �������, ����ID, ʵ��ID, ��ԴID, ������������, ���͵����ݳ���, ���͵�����, indexĬ��0, flagĬ��0  */											
																	if(t_ReadINFO.resourceid == pDevModule[0].AttArray[0])
																	{
																		sprintf(_pSendBuf,"AT+MIPLREADRSP=0,%d,1,%d,0,%d,4,3,7.5,0,0",t_ReadINFO.msgid,t_ReadINFO.objectid,pDevModule[0].AttArray[0]);
																		AT_SendToCmdData(_pSendBuf,"OK",5,1000);	
																		memset(_pSendBuf,0,sizeof(_pSendBuf));  /* ������� */
																		break; //���� for ѭ��
																	}else if(t_ReadINFO.resourceid == pDevModule[0].AttArray[1])
																	{
																		sprintf(_pSendBuf,"AT+MIPLREADRSP=0,%d,1,%d,0,%d,4,4,7.57,0,0",t_ReadINFO.msgid,t_ReadINFO.objectid,pDevModule[0].AttArray[1]);
																		AT_SendToCmdData(_pSendBuf,"OK",5,1000);	
																		memset(_pSendBuf,0,sizeof(_pSendBuf));  /* ������� */														
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
					}else if(ResponseType == 4)			//���� д
					{
						WriteAnalysis(_pMsgBuf);
						printf("WriteAnalysis:  %d,%d,%d,%d,%d,%d,%d \r\n",  t_WriteINFO.msgid,			//��ϢID
																																 t_WriteINFO.objectid,	//����ID
																																 t_WriteINFO.instanceid,//ʵ��ID
																																 t_WriteINFO.resourceid,//��ԴID
																																 t_WriteINFO.valuetype,	//ֵ����
																																 t_WriteINFO.valuelen,	//ֵ����
																																 t_WriteINFO.value);		//ֵ
						/* AT+MIPLWRITERSP=0,43357,2 */
						/* �׼���ʶ, ��ϢID, 2:��ȷ */
						sprintf(_pSendBuf,"AT+MIPLWRITERSP=0,%d,2",t_WriteINFO.msgid);
						AT_SendToCmdData(_pSendBuf,"OK",5,1000);
						memset(_pSendBuf,0,sizeof(_pSendBuf));  /* ������� */
						
					}else if(ResponseType == 5)			//���� ִ��
					{
						ExecuteAnalysis(_pMsgBuf);
						printf("ExecuteAnalysis:  %d,%d,%d,%d,%d,%s\r\n",t_ExecuteINFO.msgid,					//��ϢID
																														 t_ExecuteINFO.objectid,			//����ID
																														 t_ExecuteINFO.instanceid,		//ʵ��ID
																														 t_ExecuteINFO.resourceid,		//��ԴID
																														 t_ExecuteINFO.len,						//ֵ����
																														 t_ExecuteINFO.arguments);		//ֵ
						/* AT+MIPLEXECUTERSP=0,43357,2 */
						/* �׼���ʶ, ��ϢID, 2:��ȷ */
						sprintf(_pSendBuf,"AT+MIPLEXECUTERSP=0,%d,2",t_ExecuteINFO.msgid);
						AT_SendToCmdData(_pSendBuf,"OK",5,1000);
						memset(_pSendBuf,0,sizeof(_pSendBuf));  /* ������� */
					}					
		}
}

/*
*********************************************************************************************************
*	�� �� ��: LwM2MCreateOBJ
*	����˵��: ��������ʵ��
*	��    �Σ���
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
uint8_t LwM2MCreateOBJ(void)
{
		uint8_t i;
		uint8_t ret = 0;
//		int8_t Resub;		/* ��Ӧ���� */
//		uint8_t ResubNUM = 0;		/* ��Ӧ���ĸ���*/
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
			sprintf(ucBuf,"AT+MIPLADDOBJ=0,%d,%d,\"%s\",%d,%d",pDevModule[i].ucObjID,			/* ����ID */
																												 pDevModule[i].ucInstance,		/* ʵ������ */
																												 pDevModule[i].strmap,				/* ʵ��λͼ */
																												 pDevModule[i].ucAttribute,	/* ���Ը��� */
																												 pDevModule[i].ucAction);		/* ������� */
			if(AT_SendToCmdData(ucBuf,"OK",5,1000) == 0)
			{
				return ret;
			}
		}
		AT_SendToCmdData("AT+MIPLOPEN=0,3600","OK",5,1000);		/* ģ������ע�� */
		
		i = 0;
		bsp_StartTimer(GPRS_TMR_ID, 10000);		/* ʹ��������ʱ��3����Ϊ��ʱ���� */
		while(1)
		{
			if (bsp_CheckTimer(GPRS_TMR_ID))
			{
				ret = 0;	/* ��ʱ */
				break;
			}			
			if(GPRS_WaitResponse(MIPLEVENT[i],2000))
			{
				++i;
				printf("break %d\r\n ",i);
			}
			if(i >= 4)
			{
				ret = 1;	/* �ɹ� */
				break;
			}	
		}
		/* �ȴ�ƽ̨��ӦӦ�� */
		printf("\r\nSuccessful connection \r\n ");
		return ret;
}


/*
*********************************************************************************************************
*	�� �� ��: AM20E_init
*	����˵��: AM20E������ʼ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AM20E_init(void)
{	
	/* �ڴ˴����ӿ��ƿ������� */
	
	
	/* GPRSģ����ز�����ȡ������״̬ */
	if(GPRS_PowerOn())
	{		
		LwM2MCreateOBJ();
		return;
	}else{
		goto exit;
	}
	
	/* ��Ӧʧ�ܺ����ʧ�ܴ��� */
exit:
	while(1)
	{	
	    printf(" AM20E Response failure   \n ");
			printf(" AM20E system Reswet ...  \r\n ");
			
		 /* ϵͳ��λǰִ�д��� */
		 
		 /* ϵͳ��λ */
//		SystemReset();			
		  while(1);
	}
}

	




















