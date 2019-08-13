#include "bsp.h"			/* 底层硬件驱动 */


/* ClientIdSNMAC ：自定义，可以是设备IMEI 也可以是别的名字  */
#define ClientIdSNMAC			"FESA234FBDS25"

/* 对应平台相关信息 */
#define ProductKey				"a12J7oLVMpF"
#define DeviceName				"dev02"	
#define DeviceSecret			"zq9EXO3Lhi9DcIVEIzyO2xTtUoDuB6d6"	

#define	Aliyunhostname		"\"218.11.0.64\""
#define Aliyunhostport		"1883"
#define	Aliyunkeeplive		 120 		//单位 s

/* use fixed timestamp */
#define TIMESTAMP_VALUE    "789"

#define DEV_SIGN_SOURCE_MAXLEN    (200)
#define DEV_SIGN_HOSTNAME_MAXLEN  (128)
#define DEV_SIGN_CLIENT_ID_MAXLEN (200)
#define DEV_SIGN_USERNAME_MAXLEN  (64)
#define DEV_SIGN_PASSWORD_MAXLEN  (65)

typedef struct
{
	/* 注意：最长字符串64个 */
		char  identifier[64];		//标识符	跟平台所设置的功能标志符要一致
		char	dataValue[64];		//数据值  数据值大小与平台设置的功能标识符相关
}T_iotx_params,*_piotx_params;


/* 5个类型值对应一下数据的平台数据类型 */
typedef struct
{
	float 		Temp;
	float			Humidity;
	int32_t 	Speed;
	double  	WorkTime;
	float 		ReWater;
}T_iotx_type,*_piotx_type;


static T_iotx_type		aliFunData = { 20.55,40.75,50,20,1 };
//阿里云功能定义初始化
static T_iotx_params	aliFunDef[5] = 		
{
	{"IndoorTemperature","0"},
	{"RelativeHumidity"	,"0"},
	{"StirringSpeed"		,"0"},
	{"WorkTime"					,"0"},
	{"RemainingWater"		,"0"},
};

/* 设备 Hmacsha256 值*/
static char DEV_Hmacsha256[DEV_SIGN_PASSWORD_MAXLEN];
static char ATstr[1024] = { 0 };
static char sourcestr[512] = { 0 };	
static char ATcomstrbuf[1024] = { 0 };


/*
*********************************************************************************************************
*	函 数 名: _iotx_data_group
*	功能说明: 将功能名称组转换字符串
*	形    参: _piotx_type 功能结构体，output 输出字符串
*	返 回 值: 无
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
*	函 数 名: _iotx_group_string
*	功能说明: 将功能名称和对应数据转化为字符串
*	形    参: _piotx_type 功能结构体，output 输出字符串 最大长度1024个字节
*	返 回 值: 无
*********************************************************************************************************
*/
void _iotx_group_string(T_iotx_type *_piotx_type,char output[1024])
{
	uint8_t i;
	char datasource[64] = {0}; 
	sprintf(datasource,"%.2f",_piotx_type->Temp);										//	保护小数点后两位
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
*	函 数 名: AM900E_GPRS_PowerOn
*	功能说明: 模块上电. 函数内部先判断是否已经开机，如果已开机则直接返回1
*	形    参: 无
*	返 回 值: 1 表示成功  0: 表示异常
*********************************************************************************************************
*/
uint8_t AM900E_GPRS_PowerOn(void)
{
	uint8_t ret = 0;
	uint8_t CSQ = 0;
	
	/* 判断是否开机 */
	ret =	AT_SendToCmdData("AT","OK",5,1000);
	ret =	AT_ResponseCSQ(&CSQ,5000);
	
	printf("CSQ :%d\r\n",CSQ);

	
	/* 查询模组是否成功驻网 */
	ret =	AT_SendToCmdData("AT+CGATT=1","OK",5,1000);				//挂接GPRS网络
	ret =	AT_SendToCmdData("AT+CGDCONT=1,\"IP\",\"cmmtm\"","OK",5,1000);	//设置PDP上下文
	ret =	AT_SendToCmdData("AT+CGACT=1,1","OK",5,1000);			//激活PDP上下文
	
	
	comClearRxFifo(COM_GPRS);	/* 清零串口接收缓冲区 */
	return ret;
}


/*
*********************************************************************************************************
*	函 数 名: _hex2str
*	功能说明: HEX转字符串
*	形    参: *input 输入16进制 *output 输出16进制字符串
*	返 回 值: 无
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
*	函 数 名: ALIMQTTAnalysis
*	功能说明: 收到模组信息判断
*	形    参：_pMsg 得到参数,_usTimeOut超时时间
*	返 回 值: 1 表示设备掉线 2 收到订阅数据 0 失败
*********************************************************************************************************
*/

uint16_t ALIMQTTAnalysis(char *_MsgBuf,uint16_t *Msglen,uint32_t _usTimeOut)
{
	uint8_t ucData;
	uint16_t pos = 0;
	uint16_t msgpos = 0;
	uint8_t  ret = 0;
	char _pBuf[128] = { 0 };
	
	/* 消息内容列表 */
	char *MsgContent[] = {
	"MQTT DISCONNECT",				//设备掉线
	"MQTT DL:"							//收到订阅数据
	};
	bsp_StartTimer(GPRS_TMR_ID, _usTimeOut);		/* 使用软件定时器作为超时控制 */
	while(1)
	{
		bsp_Idle();	
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
*	函 数 名: ALIMQTTSUB
*	功能说明: 收到平台发送的信息
*	形    参：_pBuf 信息回执
*	返 回 值: 1 表示设备掉线 2 收到订阅数据 0 失败
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
			GPRS_PrintRxData(ucData);		/* 将接收到数据打印到调试串口1 */		
			ret = 1;
			_pBuf[pos++] = ucData;	
		}else{
			return ret;
		}
	}	
}
/*
*********************************************************************************************************
*	函 数 名: _iotx_generate_sign_string
*	功能说明: 生成哈希密匙 hmacsha1
*	形    参: device_id    ：SN/MAC （自定义）
*					：device_name  ：设备名字
*					： product_key ：产品KEY
*					：device_secret：设备密匙
*					：sign_string	 ：哈希值
*	返 回 值: 1 表示成功  0: 表示异常
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
*	函 数 名: IOT_Sign_MQTT_Init
*	功能说明: 协议初始化
*	形    参: 无
*	返 回 值: 1 表示成功  0: 表示异常
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
		
		/* 等一下MQTT连接成功后还会再次返回一个 "OK" 串口要清除 */
		bsp_DelayMS(100);	
		comClearRxFifo(COM_GPRS);	/* 清零串口接收缓冲区 */
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
		comClearRxFifo(COM_GPRS);	/* 清零串口接收缓冲区 */
		
		
		/* 订阅  a12J7oLVMpF/dev02/user/get  */
		ret = AT_SendToCmdData("AT+MQTTSUB=1,1,1,\"/a12J7oLVMpF/dev02/user/get\",0","SUBACK packet id 1 count 1 granted qos 1",5,5000);
		bsp_DelayMS(100);	
		comClearRxFifo(COM_GPRS);	/* 清零串口接收缓冲区 */
		
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
			
		 /* 系统复位前执行代码 */
		 
		 /* 系统复位 */
//		SystemReset();			
		  while(1);
	}
}


/*
*********************************************************************************************************
*	函 数 名: MQTT_ParamsAnalysis
*	功能说明: mqtt参数解析消息类型
*	形    参：形参设置时间 是根据	Aliyunkeeplive	时间设置，要小于Aliyunkeeplive时间
*	返 回 值: 1：解析成功 0：解析失败 
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
		bsp_StartAutoTimer(GENERAL_TMR_ID, _usTimeOut);		/* 使用软件定时器作为超时控制 */
		while(1)
		{
				bsp_Idle();	
				if (bsp_CheckTimer(GENERAL_TMR_ID))
				{				
					/* 在此加入基础资源上报 */					
					aliFunData.Temp 		= aliFunData.Temp + 0.1;
					aliFunData.Humidity = 0;
					aliFunData.WorkTime = 0;
					aliFunData.Speed		= 0;
					aliFunData.ReWater	= 0;
					
					_iotx_group_string(&aliFunData,ATcomstrbuf);
					ret = AT_SendToCmdData(ATcomstrbuf,"PUBREC dup 0, packet id 3",5,5000);
					memset(ATcomstrbuf,0,sizeof(ATcomstrbuf));
					bsp_DelayMS(100);	
					comClearRxFifo(COM_GPRS);	/* 清零串口接收缓冲区 */
				}	
				ResponseType = ALIMQTTAnalysis(_pMsgBuf,&Msglen,1000);	//1s查询一次				
				if(ResponseType == 1)		//设备掉线，重连
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
							if(ret == 0)	//连接失败
							{
									/* 系统复位 */
									// SystemReset();
									while(1);
							}
				}else if(ResponseType == 2)		//接受到订阅数据
				{				
						printf("_pMsgBuf:%s\r\n",_pMsgBuf);
				}
				
		}

}


