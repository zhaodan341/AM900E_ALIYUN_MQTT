#include "bsp.h"			/* 底层硬件驱动 */

/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"Aliyun test"
#define EXAMPLE_DATE	"2019-08-09"
#define DEMO_VER			"1.0"

/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{	
	/* 硬件初始化 */
	bsp_Init();		
	/* 打印例程信息到串口1 */
	PrintfLogo();	

	AM900E_init();
	
	/* 进入主程序循环体 */
	while (1)
	{
			bsp_Idle();																			/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */
			MQTT_ParamsAnalysis(60000);
	}
}







/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("\n\r");
	printf("*************************************************************\r\n");
	printf("* Routine name    : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* Routine version : %s\r\n", DEMO_VER);		  /* 打印例程版本 */
	printf("* publish date    : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST固件库版本，这3个定义宏在stm32f10x.h文件中 */
	printf("* Hardware version: V%d.%d.%d (STM32F10x_StdPeriph_Driver)\r\n", __STM32F10X_STDPERIPH_VERSION_MAIN,
			__STM32F10X_STDPERIPH_VERSION_SUB1,__STM32F10X_STDPERIPH_VERSION_SUB2);
	printf("*************************************************************\r\n");
}




