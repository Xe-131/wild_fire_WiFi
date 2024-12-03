/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* 开发板硬件bsp头文件 */
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_esp8266.h"
#include "bsp_esp8266_test.h"

/* 标准库头文件 */
#include <string.h>

/**************************** 任务句柄 ********************************/
/* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle = NULL;
/* 主控接收完PC数据，转发到ESP*/
static TaskHandle_t KEY_Task_Handle = NULL;
/* 主控接收完ESP数据，转发到PC*/
static TaskHandle_t ESP_Task_Handle = NULL;

/********************************** 内核对象句柄 *********************************/
SemaphoreHandle_t BinarySem_Handle =NULL;
SemaphoreHandle_t BinarySem_Handle_2 =NULL;

/******************************* 全局变量声明 ************************************/
extern char Usart_Rx_Buf[USART_RBUFF_SIZE];
 
/******************************* 宏定义 ************************************/

/********************************* 函数声明 *************************************/
/* 用于创建任务 */
static void AppTaskCreate(void);
/* KEY_Task_Handle任务实现 */
static void KEY_Task(void* pvParameters);
/* ESP_Task_Handle任务实现 */
static void ESP_Task(void* pvParameters);
/* 用于初始化板载相关资源 */
static void BSP_Init(void);

/********************************* MAIN **************************/
int main(void)
{	
	/* 定义一个创建信息返回值，默认为pdPASS */
  BaseType_t xReturn = pdPASS;
  
  /* 开发板硬件初始化 */
  BSP_Init();
  
  printf("硬件初始化完成\r\n");
  
   /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  				/* 任务入口函数 */
                        (const char*    )"AppTaskCreate",				/* 任务名字 */
                        (uint16_t       )512,  									/* 任务栈大小 */
                        (void*          )NULL,									/* 任务入口函数参数 */
                        (UBaseType_t    )1, 										/* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
  /* 启动任务调度 */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   
  else
    return -1;  
	/* 正常不会执行到这里 */
  while(1);    
}


static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  //进入临界区
  taskENTER_CRITICAL();           
	
  /* 创建 BinarySem */
  BinarySem_Handle = xSemaphoreCreateBinary();	 
  BinarySem_Handle_2 = xSemaphoreCreateBinary();
	
	if(NULL != BinarySem_Handle)
    printf("BinarySem_Handle二值信号量创建成功\r\n");
	if(NULL != BinarySem_Handle_2)
    printf("BinarySem_Handle_2二值信号量创建成功\r\n");
	
  /* 创建KEY_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )KEY_Task,  /* 任务入口函数 */
                        (const char*    )"KEY_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )3, /* 任务的优先级 */
                        (TaskHandle_t*  )&KEY_Task_Handle);/* 任务控制块指针 */ 
  if(pdPASS == xReturn)
    printf("创建KEY_Task任务成功\r\n");
  
  /* 创建ESP_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )ESP_Task,  /* 任务入口函数 */
                        (const char*    )"ESP_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )3, /* 任务的优先级 */
                        (TaskHandle_t*  )&ESP_Task_Handle);/* 任务控制块指针 */ 
  if(pdPASS == xReturn)
    printf("创建ESP_Task任务成功\r\n");	
	
	
	
  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  
  taskEXIT_CRITICAL();            //退出临界区
}

// 等待中断释放信号量，一旦获得，将rx buffer 里面的数据发送给ESP，并将rx buffer 清零
static void KEY_Task(void* parameter)
{	
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  while (1)
  {
    //获取二值信号量 xSemaphore,没获取到则一直等待
		xReturn = xSemaphoreTake(BinarySem_Handle,/* 二值信号量句柄 */
                              portMAX_DELAY); /* 等待时间 */
    if(pdPASS == xReturn)
    {
      LED2_TOGGLE;
			macESP8266_Usart ( "PC-->ANDROID: %s\r\n", Usart_Rx_Buf );
      memset(Usart_Rx_Buf,0,USART_RBUFF_SIZE);/* 清零 */
    }
  }
}

//// 等待中断释放信号量，一旦获得，将rx buffer 里面的数据发送回电脑
//static void ESP_Task(void* parameter)
//{	
//	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
//  while (1)
//  {
//    //获取二值信号量 xSemaphore,没获取到则一直等待
//		xReturn = xSemaphoreTake(BinarySem_Handle_2,/* 二值信号量句柄 */
//                              portMAX_DELAY); /* 等待时间 */
//    if(pdPASS == xReturn)
//    {
//			LED1_TOGGLE;
//			for(int i = 0; i < strEsp8266_Fram_Record .InfBit .FramLength; i++)               
//			{
//				 USART_SendData( DEBUG_USARTx ,strEsp8266_Fram_Record .Data_RX_BUF[i]);    //转发给PC
//				 while(USART_GetFlagStatus(DEBUG_USARTx,USART_FLAG_TC)==RESET){}
//			}
//			 strEsp8266_Fram_Record .InfBit .FramLength = 0;                             //接收数据长度置零
//			 strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;                           //接收标志置零

//    }
//  }
//}

// 等待中断释放信号量，一旦获得，将rx buffer 里面的数据发送回电脑
static void ESP_Task(void* parameter)
{	
	while(1){
		/* 如果接收到了ESP8266的数据 */
		if(strEsp8266_Fram_Record.InfBit.FramFinishFlag)
		{                                                      
			for(int i = 0; i < strEsp8266_Fram_Record .InfBit .FramLength; i++)               
			{
				 USART_SendData( DEBUG_USARTx ,strEsp8266_Fram_Record .Data_RX_BUF[i]);    //转发给ESP8266
				 while(USART_GetFlagStatus(DEBUG_USARTx,USART_FLAG_TC)==RESET){}
			}
			 strEsp8266_Fram_Record .InfBit .FramLength = 0;                             //接收数据长度置零
			 strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;                           //接收标志置零

		}
	}
}

static void BSP_Init(void)
{

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	/* LED 初始化 */
	// 初始化GPIO
	LED_GPIO_Config();

	
	// UASRT1，也是printf 重定向的串口
	/* DMA初始化	*/
	// 初始化串口RX 到内存的DMA，包括使能DMA 中断
	USARTx_DMA_Config();
	
	/* 串口初始化	*/
	USART_Config();
	
	// ESP 初始化
	ESP8266_Init (); 
	//对ESP8266进行AT 配置
	ESP8266_StaTcpClient_Unvarnish_ConfigTest();

}

/********************************END OF FILE****************************/
