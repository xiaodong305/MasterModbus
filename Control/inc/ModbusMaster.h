#ifndef __MODBUSMASTER_H
#define	__MODBUSMASTER_H

//Ĭ�ϰ����ļ�
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "malloc.h"	


//��Ҫ�޸ĵ�cpuͷ�ļ�
#include "main.h"


//�Լ���Ҫ�õ���ͷ�ļ�
#include "gpio.h"
#include "tim.h"	
#include "usart.h"


#define MasterModbushuart huart4
#define MasterModbusUart UART4

#define MasterModbusVERSION 10001

#define MasterModbusSENDBUFF_SIZE                   256				//���͵�������
#define MasterModbusRECEIVEBUFF_SIZE                  256

#define MasterModbusTxEn 1
#define MasterModbusRxEn 0

#define MasterModbusSlaveNume 3

#define MasterModbusSlave1 0
#define MasterModbusSlave2 1
#define MasterModbusSlave3 2

typedef enum {
	TestRegister1,	//���ԼĴ���1
	TestRegister2,	//���ԼĴ���2
	
	MasterNullRegister,																
} ModbusMasterRegister_enum;

#define ModbusMasterREGISTER_MAX 80
#define ModbusMasterBaseAdd 0X0000  
#define ModbusMasterBase1Add (0)
#define ModbusMasterBase2Add (0) 

#define DeviceAddr 0X01

typedef struct
{
	uint8_t Buffer[MasterModbusSENDBUFF_SIZE];
	uint16_t len;
	uint16_t Maxlen;
} ModbusMasterCommSendTypeDef;

typedef struct
{
	uint8_t Buffer[MasterModbusRECEIVEBUFF_SIZE];
	uint16_t len;
	uint16_t Maxlen;
} ModbusMasterCommReceiveTypeDef;

typedef enum {
	MasterModbus_RW,	//�Ĵ����ɶ�д
    MasterModbus_W,	//�Ĵ���ֻд
	MasterModbus_R,	//�Ĵ���ֻ��
} ModbusMasterProperty_enum;

typedef enum {
	ModbusMasterReadRegister = 0x3,	//���Ĵ���
    ModbusMasterWriteRegister = 0x6,	//д�Ĵ���
	ModbusMasterWriteMultipleRegister = 0x10,	//д����Ĵ���
} ModbusMasterFunCode_enum;

typedef struct
{
	uint16_t Addr; //�Ĵ�����ַ
	uint16_t *Value;//�Ĵ���ֵ
	ModbusMasterProperty_enum Property;//�Ĵ�����д����
} ModbusMasterRegisterTypeDef;

typedef enum {
	ModbusMasterInvalidFunCode = 1,	//�Ƿ�������
    ModbusMasterInvaliddataAddr = 2,	//�Ƿ������ݵ�ַ
	ModbusMasterInvalidFataValue = 3,	//�Ƿ�������ֵ
	ModbusMasterCRCCheckError = 4,	//CRCУ�����
	ModbusMasterDataLengthError = 7,	//���ݳ��ȳ�����Χ
	ModbusMasterDataOnlyRead = 8,	//дֻ������
} ModbusMasterErrorCode_enum;

typedef struct //modbus �������ݽṹ
{
	uint8_t length;//���ݳ���
	uint8_t *Array;//����ָ��
	
} ModbusMasterSendDataArrayStruct;

typedef struct //modbus ��������ָ��ṹ��
{
	list_t *head;//����ͷ
	uint8_t ListNum;//�������
	
} ModbusMasterSendDataStruct;

/**********************************************************/

/**********************************************************/


typedef struct //modbus �Ĵ���ֵ
{
	uint16_t TestValue1;//
	uint16_t TestValue2;//
	
}ModbusMasterRegisterValueStruct;
typedef struct _ModbusMasterDealTypeDef
{
	
	
	ModbusMasterCommSendTypeDef Send;//����
	ModbusMasterCommReceiveTypeDef Receive;//����
	
	uint8_t DeviceNumber;//�豸��վ��
	ModbusMasterSendDataStruct SendDataList;//���Ͷ���
	
	ModbusMasterRegisterValueStruct RegisterValue[MasterModbusSlaveNume];//�Ĵ���ֵ
	
	TimeTypeDef TimeType;
	uint32_t TimeCnt;//ͨ�ų�ʱ�ж�

	uint16_t ModbusErrorCnt;
	uint16_t ModbusReceiveCnt;
	uint16_t ModbusReceiveCntOK;
	
	uint8_t   rx_finish_flag;       //Ӧ�������ɱ�־
	uint8_t   tx_finish_flag;       //Ӧ������ɱ�־

	uint8_t (*SendReadCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t Addr, uint32_t num);//��ȡ�Ĵ������� 03����
	uint8_t (*SendWriteCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t Addr, uint32_t num);//д�Ĵ������� 06����
	uint8_t (*SendWriteMultipleCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t Addr, uint32_t num);//д�����Ĵ��� 10 ����

	uint8_t (*SendReadAllCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Addr, uint32_t num);//��ȡ�Ĵ������� 03����
	uint8_t (*SendWriteAllCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Addr, uint32_t num);//д�Ĵ������� 06����
	uint8_t (*SendWriteMultipleAllCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Addr, uint32_t num);//д�����Ĵ��� 10 ����


	void (*SetTxRxModeCall)(uint8_t Value);//���ͽ���ģʽ�л�
	void (*StartSendBufferCall)(uint16_t len);//��������
	
	void (*SendTaskDealCall)(struct _ModbusMasterDealTypeDef *p);
	void (*ReceiveTaskDealCall)(struct _ModbusMasterDealTypeDef *p);
	
	void (*TimeOutCall)(struct _ModbusMasterDealTypeDef *p);
	
	void (*InitCall)(struct _ModbusMasterDealTypeDef *p);
}ModbusMasterDealTypeDef;

#define ModbusMasterDEAL_DEFAULT() { \
	.ModbusErrorCnt = 0,\
	.ModbusReceiveCnt = 0,\
	.ModbusReceiveCntOK = 0,\
	.Send = {\
		.Maxlen = MasterModbusSENDBUFF_SIZE,\
	},\
	.Receive = {\
		.Maxlen = MasterModbusRECEIVEBUFF_SIZE,\
	},\
	.RegisterValue[MasterModbusSlave1] = {\
		.TestValue1 = 1,\
	},\
	.RegisterValue[MasterModbusSlave2] = {\
		.TestValue1 = 1,\
	},\
	.RegisterValue[MasterModbusSlave3] = {\
		.TestValue1 = 1,\
	},\
	.DeviceNumber = 1,\
    .rx_finish_flag = 0,\
    .tx_finish_flag = 0,\
	.SendReadCall = ModbusMasterSendRead,\
	.SendWriteCall = ModbusMasterSendWrite,\
	.SendWriteMultipleCall = ModbusMasterSendWriteMultiple,\
	.SendReadAllCall = ModbusMasterSendReadAll,\
	.SendWriteAllCall = ModbusMasterSendWriteAll,\
	.SendWriteMultipleAllCall = ModbusMasterSendWriteMultipleAll,\
	.SetTxRxModeCall = ModbusMasterSetTxRxModeDefault,\
	.StartSendBufferCall = ModbusMasterStartSendBufferFunDefault,\
	.SendTaskDealCall = ModbusMasterSendTaskDeal,\
	.ReceiveTaskDealCall = ModbusMasterReceiveDeal,\
	.TimeOutCall = ModbusMasterTimeOut,\
	.InitCall = ModbusMasterCommunicationInit,\
}
extern ModbusMasterDealTypeDef ModbusMasterDealType;


#endif /* __MODBUSMASTER_H */
