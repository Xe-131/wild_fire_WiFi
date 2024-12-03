/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* ������Ӳ��bspͷ�ļ� */
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_esp8266.h"
#include "bsp_esp8266_test.h"

/* ��׼��ͷ�ļ� */
#include <string.h>

/**************************** ������ ********************************/
/* ���������� */
static TaskHandle_t AppTaskCreate_Handle = NULL;
/* ���ؽ�����PC���ݣ�ת����ESP*/
static TaskHandle_t KEY_Task_Handle = NULL;
/* ���ؽ�����ESP���ݣ�ת����PC*/
static TaskHandle_t ESP_Task_Handle = NULL;

/********************************** �ں˶����� *********************************/
SemaphoreHandle_t BinarySem_Handle =NULL;
SemaphoreHandle_t BinarySem_Handle_2 =NULL;

/******************************* ȫ�ֱ������� ************************************/
extern char Usart_Rx_Buf[USART_RBUFF_SIZE];
 
/******************************* �궨�� ************************************/

/********************************* �������� *************************************/
/* ���ڴ������� */
static void AppTaskCreate(void);
/* KEY_Task_Handle����ʵ�� */
static void KEY_Task(void* pvParameters);
/* ESP_Task_Handle����ʵ�� */
static void ESP_Task(void* pvParameters);
/* ���ڳ�ʼ�����������Դ */
static void BSP_Init(void);

/********************************* MAIN **************************/
int main(void)
{	
	/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  BaseType_t xReturn = pdPASS;
  
  /* ������Ӳ����ʼ�� */
  BSP_Init();
  
  printf("Ӳ����ʼ�����\r\n");
  
   /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  				/* ������ں��� */
                        (const char*    )"AppTaskCreate",				/* �������� */
                        (uint16_t       )512,  									/* ����ջ��С */
                        (void*          )NULL,									/* ������ں������� */
                        (UBaseType_t    )1, 										/* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
  /* ����������� */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   
  else
    return -1;  
	/* ��������ִ�е����� */
  while(1);    
}


static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  //�����ٽ���
  taskENTER_CRITICAL();           
	
  /* ���� BinarySem */
  BinarySem_Handle = xSemaphoreCreateBinary();	 
  BinarySem_Handle_2 = xSemaphoreCreateBinary();
	
	if(NULL != BinarySem_Handle)
    printf("BinarySem_Handle��ֵ�ź��������ɹ�\r\n");
	if(NULL != BinarySem_Handle_2)
    printf("BinarySem_Handle_2��ֵ�ź��������ɹ�\r\n");
	
  /* ����KEY_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )KEY_Task,  /* ������ں��� */
                        (const char*    )"KEY_Task",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )3, /* ��������ȼ� */
                        (TaskHandle_t*  )&KEY_Task_Handle);/* ������ƿ�ָ�� */ 
  if(pdPASS == xReturn)
    printf("����KEY_Task����ɹ�\r\n");
  
  /* ����ESP_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )ESP_Task,  /* ������ں��� */
                        (const char*    )"ESP_Task",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )3, /* ��������ȼ� */
                        (TaskHandle_t*  )&ESP_Task_Handle);/* ������ƿ�ָ�� */ 
  if(pdPASS == xReturn)
    printf("����ESP_Task����ɹ�\r\n");	
	
	
	
  vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
  
  taskEXIT_CRITICAL();            //�˳��ٽ���
}

// �ȴ��ж��ͷ��ź�����һ����ã���rx buffer ��������ݷ��͸�ESP������rx buffer ����
static void KEY_Task(void* parameter)
{	
	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  while (1)
  {
    //��ȡ��ֵ�ź��� xSemaphore,û��ȡ����һֱ�ȴ�
		xReturn = xSemaphoreTake(BinarySem_Handle,/* ��ֵ�ź������ */
                              portMAX_DELAY); /* �ȴ�ʱ�� */
    if(pdPASS == xReturn)
    {
      LED2_TOGGLE;
			macESP8266_Usart ( "PC-->ANDROID: %s\r\n", Usart_Rx_Buf );
      memset(Usart_Rx_Buf,0,USART_RBUFF_SIZE);/* ���� */
    }
  }
}

//// �ȴ��ж��ͷ��ź�����һ����ã���rx buffer ��������ݷ��ͻص���
//static void ESP_Task(void* parameter)
//{	
//	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
//  while (1)
//  {
//    //��ȡ��ֵ�ź��� xSemaphore,û��ȡ����һֱ�ȴ�
//		xReturn = xSemaphoreTake(BinarySem_Handle_2,/* ��ֵ�ź������ */
//                              portMAX_DELAY); /* �ȴ�ʱ�� */
//    if(pdPASS == xReturn)
//    {
//			LED1_TOGGLE;
//			for(int i = 0; i < strEsp8266_Fram_Record .InfBit .FramLength; i++)               
//			{
//				 USART_SendData( DEBUG_USARTx ,strEsp8266_Fram_Record .Data_RX_BUF[i]);    //ת����PC
//				 while(USART_GetFlagStatus(DEBUG_USARTx,USART_FLAG_TC)==RESET){}
//			}
//			 strEsp8266_Fram_Record .InfBit .FramLength = 0;                             //�������ݳ�������
//			 strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;                           //���ձ�־����

//    }
//  }
//}

// �ȴ��ж��ͷ��ź�����һ����ã���rx buffer ��������ݷ��ͻص���
static void ESP_Task(void* parameter)
{	
	while(1){
		/* ������յ���ESP8266������ */
		if(strEsp8266_Fram_Record.InfBit.FramFinishFlag)
		{                                                      
			for(int i = 0; i < strEsp8266_Fram_Record .InfBit .FramLength; i++)               
			{
				 USART_SendData( DEBUG_USARTx ,strEsp8266_Fram_Record .Data_RX_BUF[i]);    //ת����ESP8266
				 while(USART_GetFlagStatus(DEBUG_USARTx,USART_FLAG_TC)==RESET){}
			}
			 strEsp8266_Fram_Record .InfBit .FramLength = 0;                             //�������ݳ�������
			 strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;                           //���ձ�־����

		}
	}
}

static void BSP_Init(void)
{

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	/* LED ��ʼ�� */
	// ��ʼ��GPIO
	LED_GPIO_Config();

	
	// UASRT1��Ҳ��printf �ض���Ĵ���
	/* DMA��ʼ��	*/
	// ��ʼ������RX ���ڴ��DMA������ʹ��DMA �ж�
	USARTx_DMA_Config();
	
	/* ���ڳ�ʼ��	*/
	USART_Config();
	
	// ESP ��ʼ��
	ESP8266_Init (); 
	//��ESP8266����AT ����
	ESP8266_StaTcpClient_Unvarnish_ConfigTest();

}

/********************************END OF FILE****************************/
