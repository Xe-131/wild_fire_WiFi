#ifndef  __ESP8266_TEST_H
#define	 __ESP8266_TEST_H



#include "stm32f10x.h"



/********************************** �û���Ҫ���õĲ���**********************************/
#define      macUser_ESP8266_ApSsid                       "Xe-131"                //Ҫ���ӵ��ȵ������
#define      macUser_ESP8266_ApPwd                        "123456789"           //Ҫ���ӵ��ȵ����Կ

#define      macUser_ESP8266_TcpServer_IP                 "192.168.225.2"      //Ҫ���ӵķ������� IP
#define      macUser_ESP8266_TcpServer_Port               "10000"               //Ҫ���ӵķ������Ķ˿�



/********************************** �ⲿȫ�ֱ��� ***************************************/
extern volatile uint8_t ucTcpClosedFlag;



/********************************** ���Ժ������� ***************************************/
void ESP8266_StaTcpClient_Unvarnish_ConfigTest(void);
void ESP8266_CheckRecvDataTest(void);

#endif

