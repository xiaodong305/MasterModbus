#ifndef __MODBUSMASTER_H
#define	__MODBUSMASTER_H

//默认包含文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "malloc.h"	


//需要修改的cpu头文件
#include "main.h"


//自己需要用到的头文件
#include "gpio.h"
#include "tim.h"	
#include "usart.h"


#define MasterModbushuart huart4
#define MasterModbusUart UART4

#define MasterModbusVERSION 10001

#define MasterModbusSENDBUFF_SIZE                   256				//发送的数据量
#define MasterModbusRECEIVEBUFF_SIZE                  256

#define MasterModbusTxEn 1
#define MasterModbusRxEn 0

#define MasterModbusSlaveNume 3

#define MasterModbusSlave1 0
#define MasterModbusSlave2 1
#define MasterModbusSlave3 2

typedef enum {
	TestRegister1,	//测试寄存器1
	TestRegister2,	//测试寄存器2
	
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
	MasterModbus_RW,	//寄存器可读写
    MasterModbus_W,	//寄存器只写
	MasterModbus_R,	//寄存器只读
} ModbusMasterProperty_enum;

typedef enum {
	ModbusMasterReadRegister = 0x3,	//读寄存器
    ModbusMasterWriteRegister = 0x6,	//写寄存器
	ModbusMasterWriteMultipleRegister = 0x10,	//写多个寄存器
} ModbusMasterFunCode_enum;

typedef struct
{
	uint16_t Addr; //寄存器地址
	uint16_t *Value;//寄存器值
	ModbusMasterProperty_enum Property;//寄存器读写属性
} ModbusMasterRegisterTypeDef;

typedef enum {
	ModbusMasterInvalidFunCode = 1,	//非法功能码
    ModbusMasterInvaliddataAddr = 2,	//非法的数据地址
	ModbusMasterInvalidFataValue = 3,	//非法的数据值
	ModbusMasterCRCCheckError = 4,	//CRC校验错误
	ModbusMasterDataLengthError = 7,	//数据长度超过范围
	ModbusMasterDataOnlyRead = 8,	//写只读数据
} ModbusMasterErrorCode_enum;

typedef struct //modbus 发送数据结构
{
	uint8_t length;//数据长度
	uint8_t *Array;//数组指针
	
} ModbusMasterSendDataArrayStruct;

typedef struct //modbus 发送数据指针结构体
{
	list_t *head;//链表头
	uint8_t ListNum;//链表个数
	
} ModbusMasterSendDataStruct;

/**********************************************************/

/**********************************************************/


typedef struct //modbus 寄存器值
{
	uint16_t TestValue1;//
	uint16_t TestValue2;//
	
}ModbusMasterRegisterValueStruct;
typedef struct _ModbusMasterDealTypeDef
{
	
	
	ModbusMasterCommSendTypeDef Send;//发送
	ModbusMasterCommReceiveTypeDef Receive;//接收
	
	uint8_t DeviceNumber;//设备从站号
	ModbusMasterSendDataStruct SendDataList;//发送队列
	
	ModbusMasterRegisterValueStruct RegisterValue[MasterModbusSlaveNume];//寄存器值
	
	TimeTypeDef TimeType;
	uint32_t TimeCnt;//通信超时判断

	uint16_t ModbusErrorCnt;
	uint16_t ModbusReceiveCnt;
	uint16_t ModbusReceiveCntOK;
	
	uint8_t   rx_finish_flag;       //应答接收完成标志
	uint8_t   tx_finish_flag;       //应答发送完成标志

	uint8_t (*SendReadCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t Addr, uint32_t num);//读取寄存器数据 03命令
	uint8_t (*SendWriteCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t Addr, uint32_t num);//写寄存器数据 06命令
	uint8_t (*SendWriteMultipleCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t Addr, uint32_t num);//写过个寄存器 10 命令

	uint8_t (*SendReadAllCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Addr, uint32_t num);//读取寄存器数据 03命令
	uint8_t (*SendWriteAllCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Addr, uint32_t num);//写寄存器数据 06命令
	uint8_t (*SendWriteMultipleAllCall)(struct _ModbusMasterDealTypeDef *p,uint16_t Addr, uint32_t num);//写过个寄存器 10 命令


	void (*SetTxRxModeCall)(uint8_t Value);//发送接收模式切换
	void (*StartSendBufferCall)(uint16_t len);//开启发送
	
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
