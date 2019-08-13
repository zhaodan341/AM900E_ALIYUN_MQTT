#include "bsp.h"			/* �ײ�Ӳ������ */


/* ClientIdSNMAC ���Զ��壬�������豸IMEI Ҳ�����Ǳ������  */
#define ClientIdSNMAC			"FESA234FBDS25"

/* ��Ӧƽ̨�����Ϣ */
#define ProductKey				"a12J7oLVMpF"
#define DeviceName				"dev02"	
#define DeviceSecret			"zq9EXO3Lhi9DcIVEIzyO2xTtUoDuB6d6"	

#define	Aliyunhostname		"\"218.11.0.64\""
#define Aliyunhostport		"1883"
#define	Aliyunkeeplive		 120 		//��λ s

/* use fixed timestamp */
#define TIMESTAMP_VALUE    "789"

#define DEV_SIGN_SOURCE_MAXLEN    (200)
#define DEV_SIGN_HOSTNAME_MAXLEN  (128)
#define DEV_SIGN_CLIENT_ID_MAXLEN (200)
#define DEV_SIGN_USERNAME_MAXLEN  (64)
#define DEV_SIGN_PASSWORD_MAXLEN  (65)

typedef struct
{
	/* ע�⣺��ַ���64�� */
		char  identifier[64];		//��ʶ��	��ƽ̨�����õĹ��ܱ�־��Ҫһ��
		char	dataValue[64];		//����ֵ  ����ֵ��С��ƽ̨���õĹ��ܱ�ʶ�����
}T_iotx_params,*_piotx_params;


/* 5������ֵ��Ӧһ�����ݵ�ƽ̨�������� */
typedef struct
{
	float 		Temp;
	float			Humidity;
	int32_t 	Speed;
	double  	WorkTime;
	float 		ReWater;
}T_iotx_type,*_piotx_type;


static T_iotx_type		aliFunData = { 20.55,40.75,50,20,1 };
//�����ƹ��ܶ����ʼ��
static T_iotx_params	aliFunDef[5] = 		
{
	{"IndoorTemperature","0"},
	{"RelativeHumidity"	,"0"},
	{"StirringSpeed"		,"0"},
	{"WorkTime"					,"0"},
	{"RemainingWater"		,"0"},
};

/* �豸 Hmacsha256 ֵ*/
static char DEV_Hmacsha256[DEV_SIGN_PASSWORD_MAXLEN];
static char ATstr[1024] = { 0 };
static char sourcestr[512] = { 0 };	
static char ATcomstrbuf[1024] = { 0 };


/*
*********************************************************************************************************
*	�� �� ��: _iotx_data_group
*	����˵��: ������������ת���ַ���
*	��    ��: _piotx_type ���ܽṹ�壬output ����ַ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _iotx_data_group(T_iotx_params *aliFunDef,char *output)
{
		char datasource[64] = {0};
		char *ESC = "\\";
		memset(datasource, 0, 64);
		sprintf(datasource,"%s\"%s%s\":%s,",ESC,aliFunDef->identifier,ESC,aliFunDef->dataValue);		
		memcpy(output,datasource, strlen(datasource));
}


/*
*********************************************************************************************************
*	�� �� ��: _iotx_group_string
*	����˵��: ���������ƺͶ�Ӧ����ת��Ϊ�ַ���
*	��    ��: _piotx_type ���ܽṹ�壬output ����ַ��� ��󳤶�1024���ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void _iotx_group_string(T_iotx_type *_piotx_type,char output[1024])
{
	uint8_t i;
	char datasource[64] = {0}; 
	sprintf(datasource,"%.2f",_piotx_type->Temp);										//	����С�������λ
	memcpy(aliFunDef[0].dataValue, datasource, strlen(datasource));
	memset(datasource, 0, 64);
	
	sprintf(datasource,"%.2f",_piotx_type->Humidity);
	memcpy(aliFunDef[1].dataValue, datasource, strlen(datasource));
	memset(datasource, 0, 64);
	
	sprintf(datasource,"%d",_piotx_type->Speed);
	memcpy(aliFunDef[2].dataValue, datasource, strlen(datasource));
	memset(datasource, 0, 64);
	
	sprintf(datasource,"%.2lf",_piotx_type->WorkTime);
	memcpy(aliFunDef[3].dataValue, datasource, strlen(datasource));
	memset(datasource, 0, 64);
	
	sprintf(datasource,"%.2f",_piotx_type->ReWater);
	memcpy(aliFunDef[4].dataValue, datasource, strlen(datasource));
	memset(datasource, 0, 64);
	memset(sourcestr, 0, 1024);	
	for(i=0;i<5;i++)
	{		
		_iotx_data_group(&aliFunDef[i],datasource);
		memcpy(sourcestr + strlen(sourcestr), datasource, strlen(datasource));
		memset(datasource, 0, 64);
	}
	sprintf(ATstr,"AT+MQTTPUB=\"{\\\"id\\\" : \\\"789\\\", \\\"version\\\":\\\"1.0\\\", \\\"params\\\" :{%s},\\\"method\\\":\\\"thing.event.property.post\\\"}\",512,2,0,0,\"/sys/a12J7oLVMpF/dev02/thing/event/property/post\",3",sourcestr);
	memcpy(output,ATstr, strlen(ATstr));
}

/*
*********************************************************************************************************
*	�� �� ��: AM900E_GPRS_PowerOn
*	����˵��: ģ���ϵ�. �����ڲ����ж��Ƿ��Ѿ�����������ѿ�����ֱ�ӷ���1
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
uint8_t AM900E_GPRS_PowerOn(void)
{
	uint8_t ret = 0;
	uint8_t CSQ = 0;
	
	/* �ж��Ƿ񿪻� */
	ret =	AT_SendToCmdData("AT","OK",5,1000);
	ret =	AT_ResponseCSQ(&CSQ,5000);
	
	printf("CSQ :%d\r\n",CSQ);

	
	/* ��ѯģ���Ƿ�ɹ�פ�� */
	ret =	AT_SendToCmdData("AT+CGATT=1","OK",5,1000);				//�ҽ�GPRS����
	ret =	AT_SendToCmdData("AT+CGDCONT=1,\"IP\",\"cmmtm\"","OK",5,1000);	//����PDP������
	ret =	AT_SendToCmdData("AT+CGACT=1,1","OK",5,1000);			//����PDP������
	
	
	comClearRxFifo(COM_GPRS);	/* ���㴮�ڽ��ջ����� */
	return ret;
}


/*
*********************************************************************************************************
*	�� �� ��: _hex2str
*	����˵��: HEXת�ַ���
*	��    ��: *input ����16���� *output ���16�����ַ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void _hex2str(uint8_t *input, uint16_t input_len, char *output)
{
    char *zEncode = "0123456789ABCDEF";
    int i = 0, j = 0;

    for (i = 0; i < input_len; i++) {
        output[j++] = zEncode[(input[i] >> 4) & 0xf];
        output[j++] = zEncode[(input[i]) & 0xf];
    }
}

/*
*********************************************************************************************************
*	�� �� ��: ALIMQTTAnalysis
*	����˵��: �յ�ģ����Ϣ�ж�
*	��    �Σ�_pMsg �õ�����,_usTimeOut��ʱʱ��
*	�� �� ֵ: 1 ��ʾ�豸���� 2 �յ��������� 0 ʧ��
*********************************************************************************************************
*/

uint16_t ALIMQTTAnalysis(char *_MsgBuf,uint16_t *Msglen,uint32_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint16_t msgpos = 0;
	uint8_t  ret = 0;
	char _pBuf[128] = { 0 };
	
	/* ��Ϣ�����б� */
	char *MsgContent[] = {
	"MQTT DISCONNECT",				//�豸����
	"MQTT DL:"							//�յ���������
	};
	bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* ʹ�������ʱ����Ϊ��ʱ���� */
	while(1)
	{
		bsp_Idle();	
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
							if(memcmp(_pBuf,MsgContent[0],7) == 0)
							{		
								_MsgBuf = NULL;
								*Msglen = 0;
								ret = 1;
								break;
							}
							else if(memcmp(_pBuf,MsgContent[1],7) == 0)
							{
										while(1)
										{
												bsp_DelayMS(50);
												if(comGetChar(COM_GPRS, &ucData))
												{													
													_MsgBuf[msgpos++] = ucData;													
												}else{
													 *Msglen = msgpos;
														break;
												}
										}
								ret = 2;
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
*	�� �� ��: ALIMQTTSUB
*	����˵��: �յ�ƽ̨���͵���Ϣ
*	��    �Σ�_pBuf ��Ϣ��ִ
*	�� �� ֵ: 1 ��ʾ�豸���� 2 �յ��������� 0 ʧ��
*********************************************************************************************************
*/

uint16_t ALIMQTTSUB(char _pBuf[128])
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint8_t  ret = 0;
//	char _pBuf[128] = { 0 };
	while(1)
	{
		bsp_Idle();	
		if(comGetChar(COM_GPRS, &ucData))
		{
			GPRS_PrintRxData(ucData);		/* �����յ����ݴ�ӡ�����Դ���1 */		
			ret = 1;
			_pBuf[pos++] = ucData;	
		}else{
			return ret;
		}
	}	
}
/*
*********************************************************************************************************
*	�� �� ��: _iotx_generate_sign_string
*	����˵��: ���ɹ�ϣ�ܳ� hmacsha1
*	��    ��: device_id    ��SN/MAC ���Զ��壩
*					��device_name  ���豸����
*					�� product_key ����ƷKEY
*					��device_secret���豸�ܳ�
*					��sign_string	 ����ϣֵ
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
int _iotx_generate_sign_string(const char *device_id, const char *device_name, const char *product_key, const char *device_secret, char *sign_string)
{
    char signsource[DEV_SIGN_SOURCE_MAXLEN] = {0};
    uint16_t signsource_len = 0;
    uint8_t sign_hex[32] = {0};

    signsource_len = strlen(device_id) + strlen(device_name) + strlen(product_key) + strlen(TIMESTAMP_VALUE);
    if (signsource_len >= DEV_SIGN_SOURCE_MAXLEN) {
        return 0;
    }

    memset(signsource, 0, DEV_SIGN_SOURCE_MAXLEN);
    memcpy(signsource, "clientId", strlen("clientId"));
    memcpy(signsource + strlen(signsource), device_id, strlen(device_id));
    memcpy(signsource + strlen(signsource), "deviceName", strlen("deviceName"));
    memcpy(signsource + strlen(signsource), device_name, strlen(device_name));
    memcpy(signsource + strlen(signsource), "productKey", strlen("productKey"));
    memcpy(signsource + strlen(signsource), product_key, strlen(product_key));
    memcpy(signsource + strlen(signsource), "timestamp", strlen("timestamp"));
    memcpy(signsource + strlen(signsource), TIMESTAMP_VALUE, strlen(TIMESTAMP_VALUE));

		
//		printf("%s\r\n",signsource);
    utils_hmac_sha256((uint8_t *)signsource, strlen(signsource), (uint8_t *)device_secret,
                      strlen(device_secret), sign_hex);

    _hex2str(sign_hex, 32, sign_string);

    return 1;
}


/*
*********************************************************************************************************
*	�� �� ��: IOT_Sign_MQTT_Init
*	����˵��: Э���ʼ��
*	��    ��: ��
*	�� �� ֵ: 1 ��ʾ�ɹ�  0: ��ʾ�쳣
*********************************************************************************************************
*/
uint8_t IOT_Sign_MQTT_Init(void)
{
		uint8_t ret = 0;
		_iotx_generate_sign_string(ClientIdSNMAC,DeviceName,ProductKey,DeviceSecret,DEV_Hmacsha256);	
		
		sprintf(ATcomstrbuf,"AT+MQTTCONN=%s,%s,\"%s|securemode=3,signmethod=hmacsha256,timestamp=789|\",\"%s&%s\",\"%s\",%d,1,0",
						Aliyunhostname,
						Aliyunhostport,
						ClientIdSNMAC,
						DeviceName,
						ProductKey,
						DEV_Hmacsha256,
						Aliyunkeeplive
					 );
		ret = AT_SendToCmdData(ATcomstrbuf,"CONNACK session present 0, rc 0",5,5000);	
		
		/* ��һ��MQTT���ӳɹ��󻹻��ٴη���һ�� "OK" ����Ҫ��� */
		bsp_DelayMS(100);	
		comClearRxFifo(COM_GPRS);	/* ���㴮�ڽ��ջ����� */
		memset(ATcomstrbuf,0,sizeof(ATcomstrbuf));
	
		aliFunData.Temp 		= 0;
		aliFunData.Humidity = 0;
		aliFunData.WorkTime = 0;
		aliFunData.Speed		= 0;
		aliFunData.ReWater	= 0;
		
		_iotx_group_string(&aliFunData,ATcomstrbuf);
		ret = AT_SendToCmdData(ATcomstrbuf,"PUBREC dup 0, packet id 3",5,5000);
		memset(ATcomstrbuf,0,sizeof(ATcomstrbuf));
		bsp_DelayMS(100);	
		comClearRxFifo(COM_GPRS);	/* ���㴮�ڽ��ջ����� */
		
		
		/* ����  a12J7oLVMpF/dev02/user/get  */
		ret = AT_SendToCmdData("AT+MQTTSUB=1,1,1,\"/a12J7oLVMpF/dev02/user/get\",0","SUBACK packet id 1 count 1 granted qos 1",5,5000);
		bsp_DelayMS(100);	
		comClearRxFifo(COM_GPRS);	/* ���㴮�ڽ��ջ����� */
		
//		printf("%s\r\n",ATcomstrbuf);
		return ret;
}

void AM900E_init(void)
{
	if(AM900E_GPRS_PowerOn())
	{		
		IOT_Sign_MQTT_Init();
		return;
	}else{
		goto exit;
	}
	
exit:
	while(1)
	{	
	    printf(" AM900E Response failure   \n ");
			printf(" AM900E system Reswet ...  \r\n ");
			
		 /* ϵͳ��λǰִ�д��� */
		 
		 /* ϵͳ��λ */
//		SystemReset();			
		  while(1);
	}
}


/*
*********************************************************************************************************
*	�� �� ��: MQTT_ParamsAnalysis
*	����˵��: mqtt����������Ϣ����
*	��    �Σ��β�����ʱ�� �Ǹ���	Aliyunkeeplive	ʱ�����ã�ҪС��Aliyunkeepliveʱ��
*	�� �� ֵ: 1�������ɹ� 0������ʧ�� 
*********************************************************************************************************
*/
uint8_t MQTT_ParamsAnalysis(uint32_t _usTimeOut)
{
		uint8_t ret = 0;
		uint8_t ResponseType;
		uint16_t Msglen;
		char _pMsgBuf[128] = { 0 };	
		
		if(_usTimeOut > Aliyunkeeplive * 1000)
		{
			 _usTimeOut  = (Aliyunkeeplive * 1000) / 2;
		}
		bsp_StartAutoTimer(GENERAL_TMR_ID, _usTimeOut);		/* ʹ�������ʱ����Ϊ��ʱ���� */
		while(1)
		{
				bsp_Idle();	
				if (bsp_CheckTimer(GENERAL_TMR_ID))
				{				
					/* �ڴ˼��������Դ�ϱ� */					
					aliFunData.Temp 		= aliFunData.Temp + 0.1;
					aliFunData.Humidity = 0;
					aliFunData.WorkTime = 0;
					aliFunData.Speed		= 0;
					aliFunData.ReWater	= 0;
					
					_iotx_group_string(&aliFunData,ATcomstrbuf);
					ret = AT_SendToCmdData(ATcomstrbuf,"PUBREC dup 0, packet id 3",5,5000);
					memset(ATcomstrbuf,0,sizeof(ATcomstrbuf));
					bsp_DelayMS(100);	
					comClearRxFifo(COM_GPRS);	/* ���㴮�ڽ��ջ����� */
				}	
				ResponseType = ALIMQTTAnalysis(_pMsgBuf,&Msglen,1000);	//1s��ѯһ��				
				if(ResponseType == 1)		//�豸���ߣ�����
				{
							_iotx_generate_sign_string(ClientIdSNMAC,DeviceName,ProductKey,DeviceSecret,DEV_Hmacsha256);							
							sprintf(ATcomstrbuf,"AT+MQTTCONN=%s,%s,\"%s|securemode=3,signmethod=hmacsha256,timestamp=789|\",\"%s&%s\",\"%s\",%d,1,0",
											Aliyunhostname,
											Aliyunhostport,
											ClientIdSNMAC,
											DeviceName,
											ProductKey,
											DEV_Hmacsha256,
											Aliyunkeeplive
										 );
							ret = AT_SendToCmdData(ATcomstrbuf,"CONNACK session present 0, rc 0",5,5000);	
							if(ret == 0)	//����ʧ��
							{
									/* ϵͳ��λ */
									// SystemReset();
									while(1);
							}
				}else if(ResponseType == 2)		//���ܵ���������
				{				
						printf("_pMsgBuf:%s\r\n",_pMsgBuf);
				}
				
		}

}


