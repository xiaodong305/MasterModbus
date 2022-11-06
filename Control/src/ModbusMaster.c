/**
  ******************************************************************************
  * @file    ModbusMaster.c
  * @author  
  * @version V1.0
  * @date    2022-06-09
  * @brief   Modbus主机协议  包含文件  list.c/.h  malloc.c/.h  ModbusMaster.c/.h
   * 0.相关串口初始化 优先考虑 uart + dma 接收空闲中断 
  *	1.修改寄存器词典	 以及 ModbusMasterReceiveReadCmdDeal 函数
  *	2.修改发送接口 重写 ModbusMasterStartSendBufferFun 函数  发送数据填充到 ModbusMasterDealType.Send.Buffer 
  *    并把数组中的数据发出
  * 3.修改发送接收数据长度  在ModbusMaster.h 中 MasterModbusSENDBUFF_SIZE  MasterModbusRECEIVEBUFF_SIZE 修改
  * 4.指定接收数组。把接收数据放到 ModbusMasterDealType.Receive.Buffer 数组中
  * 5.修改设置发送接收方向引脚函数 重写 ModbusMasterSetTxRxMode 函数
  * 6.添加发送完成中断  
  *		ModbusMasterDealType.SetTxRxModeCall(MasterModbusRxEn);
  *		ModbusMasterDealType.tx_finish_flag = 1;	
  * 7.添加接收完成中断 
  *   	ModbusMasterDealType.SetTxRxModeCall(MasterModbusTxEn);
  *		ModbusMasterDealType.rx_finish_flag = 1;
  *		ModbusMasterDealType.Receive.len = len;
  * 8.在main函数中添加 ModbusMasterDealType.InitCall(&ModbusMasterDealType);//Modbus主机初始化
  * 9.在定时器中添加超时判断  在1ms毫秒定时器中添加 ModbusMasterDealType.TimeOutCall(&ModbusMasterDealType);
  * 10.在while死循环中添加 	  ModbusMasterDealType.ReceiveTaskDealCall(&ModbusMasterDealType);
  *							  ModbusMasterDealType.SendTaskDealCall(&ModbusMasterDealType);
  ******************************************************************************
  */
#include "ModbusMaster.h"


static uint16_t ValueNull = 0;
  
static uint8_t ListArray[MasterModbusSENDBUFF_SIZE];

static uint8_t MasterModbusSlaveAddr[MasterModbusSlaveNume] = {
	1,2,3
};

static ModbusMasterRegisterTypeDef ModbusMasterRegister[][MasterModbusSlaveNume]={//ModbusMasterREGISTER_MAX
	/*                                   设备1寄存器地址信息 start                                        */
	[TestRegister1										][MasterModbusSlave1] = {TestRegister1					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave1].TestValue1, 			MasterModbus_R },//
	[TestRegister2 										][MasterModbusSlave1] = {TestRegister2 					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave1].TestValue2,				MasterModbus_R},//
	 
	[MasterNullRegister									][MasterModbusSlave1] = {MasterNullRegister 			+ ModbusMasterBaseAdd,											&ValueNull,																MasterModbus_R },
	/*                                   设备1寄存器地址信息 end                                        */

	/*                                   设备2寄存器地址信息 start                                        */
	[TestRegister1										][MasterModbusSlave2] = {TestRegister1					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave2].TestValue1, 			MasterModbus_R },//实时重量低16位
	[TestRegister2 										][MasterModbusSlave2] = {TestRegister2 					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave2].TestValue2,				MasterModbus_R},//实时重量高16位
																		   
	[MasterNullRegister									][MasterModbusSlave2] = {MasterNullRegister 				+ ModbusMasterBaseAdd,											&ValueNull,																MasterModbus_R },
	/*                                   设备2寄存器地址信息 end                                        */
	
	/*                                   设备3寄存器地址信息 start                                        */
	[TestRegister1										][MasterModbusSlave3] = {TestRegister1					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave3].TestValue1, 			MasterModbus_R },//实时重量低16位
	[TestRegister2 										][MasterModbusSlave3] = {TestRegister2 					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave3].TestValue2,				MasterModbus_R},//实时重量高16位
																	   
	[MasterNullRegister									][MasterModbusSlave3] = {MasterNullRegister 				+ ModbusMasterBaseAdd,											&ValueNull,																MasterModbus_R },
	/*                                   设备3寄存器地址信息 end                                        */

};

//函数名：   ModbusMasterSetTxRxMode
//作者：     
//日期：    2022-10-27
//功能：    在其他地方重新写或者在这里面完成以下功能 
//			ModbusMasterSetTxRxMode(MasterModbusTxEn); //切换为发送模式
//			ModbusMasterSetTxRxMode(MasterModbusRxEn); //切换为接收模式
//备注		这两个宏定义在头文件中定义 #define MasterModbusTxEn 1
//										#define MasterModbusRxEn 0
//输入参数：Value  MasterModbusTxEn 发送模式  MasterModbusRxEn 接收模式
//返回值：  无返回
//修改记录：
static void ModbusMasterSetTxRxModeDefault(uint8_t Value)
{
	if(Value == 0){
		HAL_GPIO_WritePin(MASTER_MODBUS_TXRX_GPIO_Port,MASTER_MODBUS_TXRX_Pin,GPIO_PIN_RESET);
	}else{
		HAL_GPIO_WritePin(MASTER_MODBUS_TXRX_GPIO_Port,MASTER_MODBUS_TXRX_Pin,GPIO_PIN_SET);
	}
}
//函数名：  ModbusMasterStartSendBufferFun
//作者：    
//日期：    2022-10-27
//功能：    在其他地方重新写或者在这里面完成以下功能 
//			传入要发送的数据长度，填充要发送的数据到 ModbusMasterDealType.Send.Buffer，开始数据发送
//输入参数：len
//返回值：  无返回
//修改记录：
static void ModbusMasterStartSendBufferFunDefault(uint16_t len)
{
	
	HAL_UART_Transmit_DMA(&MasterModbushuart,ModbusMasterDealType.Send.Buffer,len);
}
//函数名：  ModbusMasterCommunicationInit
//作者：    
//日期：    2022-05-26
//功能：    与APP通信初始化函数
//输入参数：无
//返回值：  无返回
//修改记录：
static void ModbusMasterCommunicationInit(ModbusMasterDealTypeDef *p)
{
	my_mem_init(SRAMIN);		//初始化内部内存池
	p->SendDataList.head = (list_t *)mymalloc(SRAMIN,sizeof(list_t)); 
	if(p->SendDataList.head != NULL){
		list_init(p->SendDataList.head); //初始化链表
		p->SendDataList.ListNum = list_len(p->SendDataList.head);
	}else{
		//内存不足
	}
	p->SetTxRxModeCall(MasterModbusTxEn);
    
}
 

static uint16_t ModbusMasterReceiveReadCmdDeal(ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t ArrayAddr,uint16_t data,uint8_t *flag)
{
	uint16_t cnt = 0;
	//int sdata = 0;
	//uint32_t TreadmillSpeed = 0;
	
	

	switch (ArrayAddr)
	{
		
		case TestRegister1://	          
					*ModbusMasterRegister[ArrayAddr][Slave].Value = data;
					break;
		case TestRegister2://	          
					*ModbusMasterRegister[ArrayAddr][Slave].Value = data;
					break;
		
		default:
			*flag = 1;
			cnt++;
			break;
	}
	return cnt;
}
//函数名：  ModbusMasterStartSendBuffer
//作者：    
//日期：    2022-05-26
//功能：    要发送的数据全部填充到数组，执行此函数进行数据发送  备注：此函数需要宏定义注册uart发送函数
//输入参数：ModbusMasterDealTypeDef *p
//返回值：  类型（char)
//          0：发送正常
//			-1:数据超过发送缓冲长度错误并发送错误帧信息
//修改记录：
static signed char ModbusMasterStartSendBuffer(ModbusMasterDealTypeDef *p)
{
	signed char ret = 0;
	ModbusMasterDealType.TimeCnt = 0;
	if(p->Send.len <= ModbusMasterDealType.Send.Maxlen){
		//p->SetTxRxModeCall(MasterModbusTxEn);
		p->StartSendBufferCall(p->Send.len);
	}else{
		//数据超过发送缓冲长度错误
		ret = -1;
	}
	return ret;
	
}
static uint8_t ModbusMasterSendListDelete(ModbusMasterDealTypeDef *p)
{
	myfree(SRAMIN,((ModbusMasterSendDataArrayStruct *)(p->SendDataList.head->head->data))->Array);
	((ModbusMasterSendDataArrayStruct *)(p->SendDataList.head->head->data))->Array = NULL;
	myfree(SRAMIN,p->SendDataList.head->head->data);
	p->SendDataList.head->head->data = NULL;
	list_pop(p->SendDataList.head,0);
	p->SendDataList.ListNum = list_len(p->SendDataList.head);	
	return p->SendDataList.ListNum;
}

/* =============================================================================
函数名称: void crc_cal_value(.)

函数说明：CRC8校验值计算

参数说明: *data_value---数据buffer
          data_length---数据长度
==============================================================================*/
static uint16_t crc_cal_value(uint8_t *data_value, uint16_t data_length)
{
	uint16_t  i;
	uint16_t  crc_value = 0xffff;
	
	while(data_length--)
	{
		crc_value ^= *data_value++;
		for(i=0; i<8; i++)
		{
			if(crc_value&0x0001)
			{
				crc_value=(crc_value>>1)^0xa001;
			}
			else
			{
				crc_value=crc_value>>1;
			}
		}
	}
	
	return(crc_value);
}

static ModbusMasterRegister_enum GetMasterRegisterAddr(uint8_t Slave,uint16_t Addr)
{
	for(int i = TestRegister1; i <= MasterNullRegister; i++){ 
		if(ModbusMasterRegister[i][Slave].Addr == Addr){
			return (ModbusMasterRegister_enum)i;//
		}
	}
	return MasterNullRegister;
}


static uint8_t ModbusMasterSendRead(ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t Addr, uint32_t num)
{
	ModbusMasterSendDataArrayStruct *q = mymalloc(SRAMIN,sizeof(ModbusMasterSendDataArrayStruct));
	//uint32_t data = 0;
	uint8_t len = 0;
	//uint16_t cnt = 0;
	uint16_t crc16 = 0;
	//uint16_t ArrayAddr = 0;
	//uint16_t AddrMax = Addr + num - 1;
	

	
	ListArray[0] = MasterModbusSlaveAddr[Slave];
	ListArray[1] = 0x03;	
	
	ListArray[2] = (Addr >> 8) & 0x00ff;
	ListArray[3] = (Addr >> 0) & 0x00ff;	
	
	ListArray[4] = (num >> 8) & 0x00ff;
	ListArray[5] = (num >> 0) & 0x00ff;	
	
	crc16 = crc_cal_value(ListArray,6);
	ListArray[6] = crc16 & 0x00ff;
	ListArray[7] = (crc16 >> 8)& 0x00ff;
	len = 8;
	if(q != NULL){
		q->length = len;
		q->Array = (uint8_t *)mymalloc(SRAMIN,len); 
		if(q->Array != NULL){
			memcpy(q->Array,ListArray,q->length);
			list_append(p->SendDataList.head, q); //追加结点
			p->SendDataList.ListNum = list_len(p->SendDataList.head);	
		}
	}

	
	return len; 
}	

static uint8_t ModbusMasterSendWrite(ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t Addr, uint32_t num)
{
	ModbusMasterSendDataArrayStruct *q = mymalloc(SRAMIN,sizeof(ModbusMasterSendDataArrayStruct));
	//uint32_t data = 0;
	uint8_t len = 0;
	//uint16_t cnt = 0;
	uint16_t crc16 = 0;
	//uint16_t ArrayAddr = 0;
	//uint16_t AddrMax = Addr + num - 1;
	

	
	ListArray[0] = MasterModbusSlaveAddr[Slave];//p->DeviceNumber;
	ListArray[1] = 0x06;	
	
	ListArray[2] = (Addr >> 8) & 0x00ff;
	ListArray[3] = (Addr >> 0) & 0x00ff;	
	
	ListArray[4] = (*ModbusMasterRegister[GetMasterRegisterAddr(Slave,Addr)][Slave].Value >> 8) & 0x00ff;
	ListArray[5] = (*ModbusMasterRegister[GetMasterRegisterAddr(Slave,Addr)][Slave].Value >> 0) & 0x00ff;
		
	crc16 = crc_cal_value(ListArray,6);
	ListArray[6] = crc16 & 0x00ff;
	ListArray[7] = (crc16 >> 8)& 0x00ff;
	len = 8;
	if(q != NULL){
		q->length = len;
		q->Array = (uint8_t *)mymalloc(SRAMIN,len); 
		if(q->Array != NULL){
			memcpy(q->Array,ListArray,q->length);
			list_append(p->SendDataList.head, q); //追加结点
			p->SendDataList.ListNum = list_len(p->SendDataList.head);	
		}
	}

	
	return len; 
}	

static uint8_t ModbusMasterSendWriteMultiple(ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t Addr, uint32_t num)
{
	ModbusMasterSendDataArrayStruct *q = mymalloc(SRAMIN,sizeof(ModbusMasterSendDataArrayStruct));
	//uint32_t data = 0;
	uint8_t len = 0;
	//uint16_t cnt = 0;
	uint16_t crc16 = 0;
	//uint16_t ArrayAddr = 0;
	//uint16_t AddrMax = Addr + num - 1;
	uint16_t num1 = num*2;
	
	
	ListArray[0] = MasterModbusSlaveAddr[Slave];
	ListArray[1] = 0x10;	
	
	ListArray[2] = (Addr >> 8) & 0x00ff;
	ListArray[3] = (Addr >> 0) & 0x00ff;	
	
	ListArray[4] = (num >> 8) & 0x00ff;
	ListArray[5] = (num >> 0) & 0x00ff;	
	
	ListArray[6] = (num1) & 0x00ff;	
	for(int i = 0; i < num; i++){
		ListArray[7 + 2 * i] = (*ModbusMasterRegister[GetMasterRegisterAddr(Slave,Addr + i)][Slave].Value >> 8) & 0x00ff;
		ListArray[8 + 2 * i] = (*ModbusMasterRegister[GetMasterRegisterAddr(Slave,Addr + i)][Slave].Value >> 0) & 0x00ff;	
	}
	
	crc16 = crc_cal_value(ListArray,7 + num*2);
	ListArray[9 + num*2] = crc16 & 0x00ff;
	ListArray[10 + num*2] = (crc16 >> 8)& 0x00ff;
	
	len = 9 + num*2;

	if(q != NULL){
		q->length = len;
		q->Array = (uint8_t *)mymalloc(SRAMIN,len); 
		if(q->Array != NULL){
			memcpy(q->Array,ListArray,q->length);
			list_append(p->SendDataList.head, q); //追加结点
			p->SendDataList.ListNum = list_len(p->SendDataList.head);	
		}
	}
	return len; 
	
}






static void ModbusMasterReceiveReadMultipleFuncDataToRam(ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t Addr, uint32_t num)
{
	uint16_t data = 0;
	uint8_t flag = 0;
	//uint16_t cnt = 0;
	//uint16_t crc16 = 0;
	uint16_t ArrayAddr = 0;
	uint16_t AddrMax = Addr + num - 1;
	
	ArrayAddr = GetMasterRegisterAddr(Slave,Addr);
	//if(ArrayAddr != 0)
	{
		if(AddrMax < ModbusMasterRegister[MasterNullRegister][Slave].Addr){
			for(int i = 0; i < num; i++){
				data = (p->Receive.Buffer[3 + i*2] << 8) + p->Receive.Buffer[4 + i*2];
				ModbusMasterReceiveReadCmdDeal(p,Slave,ArrayAddr + i,data,&flag);
			}		
		}	
	}

	
}
static uint16_t ModbusMasterReceiveWriteCmdDeal(ModbusMasterDealTypeDef *p,uint16_t Slave,uint16_t ArrayAddr,uint32_t data,uint8_t *flag)
{
	uint16_t cnt = 0;
	//int sdata = 0;
	
	return cnt;
}


static uint8_t GetMasterRegisterAddrState(uint16_t Slave,uint16_t addr)
{ 
	if(addr >= ModbusMasterRegister[TestRegister1][Slave].Addr && addr < ModbusMasterRegister[MasterNullRegister][Slave].Addr)
		return 1;
	else
		return 0;
}

static void ModbusMasterErrDeal(ModbusMasterDealTypeDef *p,ModbusMasterErrorCode_enum error)
{
	uint16_t crc16 = 0;
	p->Send.Buffer[0] = p->Receive.Buffer[0];
	p->Send.Buffer[1] = p->Receive.Buffer[1] | 0x80;
	p->Send.Buffer[2] = error;
	crc16 = crc_cal_value(p->Send.Buffer,3);
	p->Send.Buffer[3] = crc16 & 0x00ff;
	p->Send.Buffer[4] = (crc16 >> 8)& 0x00ff;
	p->Send.len = 5;
	
}

static void ModbusMasterTimeOut(ModbusMasterDealTypeDef *p)
{
	if(p->tx_finish_flag == 1 && p->rx_finish_flag == 0){
		p->TimeCnt++;
		if(p->TimeCnt > 3000){
			p->tx_finish_flag = 0;
			p->rx_finish_flag = 0;
			p->SetTxRxModeCall(MasterModbusTxEn);
			p->SendDataList.ListNum = list_len(p->SendDataList.head);
		}
	}else{
		p->TimeCnt = 0;
	}
}
static uint16_t ModbusMasterGetSlaveArrayAddr(uint16_t Slave)
{
	uint8_t i;
	for(i = 0; i < MasterModbusSlaveNume; i ++){
		if(Slave == MasterModbusSlaveAddr[i]){
			return i;
		}
	}
	return 0xff;
}
static void ModbusMasterReceiveDeal(ModbusMasterDealTypeDef *p)
{
	uint8_t check;
	uint16_t crc16 = 0;
	uint16_t addr_value = 0;
	uint16_t data_num  = 0;
	//uint16_t data  = 0;
	uint8_t flag = 0;
	uint8_t Slave = 0;
	if(p->rx_finish_flag == 1 && p->tx_finish_flag == 1){
		p->rx_finish_flag = 0; 
		p->tx_finish_flag = 0;
		p->ModbusReceiveCnt ++;
		if(p->Receive.len > 3){
			crc16 = (uint16_t)(p->Receive.Buffer[p->Receive.len-1] << 8) + p->Receive.Buffer[p->Receive.len-2];
			if(crc_cal_value(p->Receive.Buffer,p->Receive.len - 2) == crc16){
				check = 1;
			}else{
				check = 0;
			}
		}else{
			check = 0;
		}

		if(p->Send.Buffer[0] != p->Receive.Buffer[0] && p->Receive.Buffer[0] != 0){//地址不匹配，不应答  DeviceAddr != p->Receive.Buffer[0]
			ModbusMasterSendListDelete(p);//对上报错，删除链表
		}else if(check == 0){ //校验错误
			ModbusMasterStartSendBuffer(p);

			p->ModbusErrorCnt++;
		}else if(p->Receive.Buffer[1] != ModbusMasterReadRegister && p->Receive.Buffer[1] != ModbusMasterWriteRegister && p->Receive.Buffer[1] != ModbusMasterWriteMultipleRegister){
			ModbusMasterStartSendBuffer(p);
			p->ModbusErrorCnt ++;
		}else{
			p->ModbusReceiveCntOK ++;
			Slave = ModbusMasterGetSlaveArrayAddr(p->Receive.Buffer[0]);
			switch (p->Receive.Buffer[1])
            {
            	case ModbusMasterReadRegister:
					
					addr_value = (p->Send.Buffer[2] << 8) + p->Send.Buffer[3];
					data_num = (p->Receive.Buffer[2]);
					ModbusMasterReceiveReadMultipleFuncDataToRam(p,Slave,addr_value,data_num/2);
				
            		break;
            	case ModbusMasterWriteRegister:
					addr_value = (p->Send.Buffer[2] << 8) + p->Send.Buffer[3];
					data_num = 4;
					ModbusMasterReceiveWriteCmdDeal(p,Slave,addr_value,data_num/4,&flag);
				
            		break;
				case ModbusMasterWriteMultipleRegister:
					addr_value = (p->Send.Buffer[2] << 8) + p->Send.Buffer[3];
					data_num = (p->Receive.Buffer[4] << 8) + p->Receive.Buffer[5];;
					ModbusMasterReceiveWriteCmdDeal(p,Slave,addr_value,data_num,&flag);
				
            		break;
            	default:
            		break;
            } 
			ModbusMasterSendListDelete(p);//删除链表
			p->ModbusErrorCnt = 0;
			memset(p->Send.Buffer,0,p->Send.len);
		}
		
		if(p->ModbusErrorCnt > 3){
			ModbusMasterSendListDelete(p);//对上报错，删除链表
		}
  
	}
 
}
static void ModbusMasterSendTaskDeal(ModbusMasterDealTypeDef *p)
{
	ModbusMasterSendDataArrayStruct *q;
	if(p->rx_finish_flag == 0 && p->tx_finish_flag == 0){
		if(p->SendDataList.ListNum != 0){
			q = (ModbusMasterSendDataArrayStruct *)(p->SendDataList.head->head->data);
			memcpy(p->Send.Buffer,q->Array,q->length);
			p->Send.len = q->length;
			ModbusMasterStartSendBuffer(p);
			p->tx_finish_flag = 3;
		}
		
	}
}

static uint8_t ModbusMasterSendReadAll(struct _ModbusMasterDealTypeDef *p,uint16_t Addr, uint32_t num)//
{
	uint8_t i;
	uint8_t len;
	for(i = 0; i < MasterModbusSlaveNume; i++){
		len = p->SendReadCall(p,i,Addr,num);
	}
	return len;
}
static uint8_t ModbusMasterSendWriteAll(struct _ModbusMasterDealTypeDef *p,uint16_t Addr, uint32_t num)
{
	uint8_t i;
	uint8_t len;
	for(i = 0; i < MasterModbusSlaveNume; i++){
		len = p->SendWriteCall(p,i,Addr,num);
	}
	return len;
}
static uint8_t ModbusMasterSendWriteMultipleAll(struct _ModbusMasterDealTypeDef *p,uint16_t Addr, uint32_t num)
{
	uint8_t i;
	uint8_t len;
	for(i = 0; i < MasterModbusSlaveNume; i++){
		len = p->SendWriteMultipleCall(p,i,Addr,num);
	}
	return len;
}

ModbusMasterDealTypeDef ModbusMasterDealType = ModbusMasterDEAL_DEFAULT();

