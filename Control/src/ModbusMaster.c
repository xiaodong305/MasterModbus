/**
  ******************************************************************************
  * @file    ModbusMaster.c
  * @author  
  * @version V1.0
  * @date    2022-06-09
  * @brief   Modbus����Э��  �����ļ�  list.c/.h  malloc.c/.h  ModbusMaster.c/.h
   * 0.��ش��ڳ�ʼ�� ���ȿ��� uart + dma ���տ����ж� 
  *	1.�޸ļĴ����ʵ�	 �Լ� ModbusMasterReceiveReadCmdDeal ����
  *	2.�޸ķ��ͽӿ� ��д ModbusMasterStartSendBufferFun ����  ����������䵽 ModbusMasterDealType.Send.Buffer 
  *    ���������е����ݷ���
  * 3.�޸ķ��ͽ������ݳ���  ��ModbusMaster.h �� MasterModbusSENDBUFF_SIZE  MasterModbusRECEIVEBUFF_SIZE �޸�
  * 4.ָ���������顣�ѽ������ݷŵ� ModbusMasterDealType.Receive.Buffer ������
  * 5.�޸����÷��ͽ��շ������ź��� ��д ModbusMasterSetTxRxMode ����
  * 6.��ӷ�������ж�  
  *		ModbusMasterDealType.SetTxRxModeCall(MasterModbusRxEn);
  *		ModbusMasterDealType.tx_finish_flag = 1;	
  * 7.��ӽ�������ж� 
  *   	ModbusMasterDealType.SetTxRxModeCall(MasterModbusTxEn);
  *		ModbusMasterDealType.rx_finish_flag = 1;
  *		ModbusMasterDealType.Receive.len = len;
  * 8.��main��������� ModbusMasterDealType.InitCall(&ModbusMasterDealType);//Modbus������ʼ��
  * 9.�ڶ�ʱ������ӳ�ʱ�ж�  ��1ms���붨ʱ������� ModbusMasterDealType.TimeOutCall(&ModbusMasterDealType);
  * 10.��while��ѭ������� 	  ModbusMasterDealType.ReceiveTaskDealCall(&ModbusMasterDealType);
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
	/*                                   �豸1�Ĵ�����ַ��Ϣ start                                        */
	[TestRegister1										][MasterModbusSlave1] = {TestRegister1					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave1].TestValue1, 			MasterModbus_R },//
	[TestRegister2 										][MasterModbusSlave1] = {TestRegister2 					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave1].TestValue2,				MasterModbus_R},//
	 
	[MasterNullRegister									][MasterModbusSlave1] = {MasterNullRegister 			+ ModbusMasterBaseAdd,											&ValueNull,																MasterModbus_R },
	/*                                   �豸1�Ĵ�����ַ��Ϣ end                                        */

	/*                                   �豸2�Ĵ�����ַ��Ϣ start                                        */
	[TestRegister1										][MasterModbusSlave2] = {TestRegister1					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave2].TestValue1, 			MasterModbus_R },//ʵʱ������16λ
	[TestRegister2 										][MasterModbusSlave2] = {TestRegister2 					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave2].TestValue2,				MasterModbus_R},//ʵʱ������16λ
																		   
	[MasterNullRegister									][MasterModbusSlave2] = {MasterNullRegister 				+ ModbusMasterBaseAdd,											&ValueNull,																MasterModbus_R },
	/*                                   �豸2�Ĵ�����ַ��Ϣ end                                        */
	
	/*                                   �豸3�Ĵ�����ַ��Ϣ start                                        */
	[TestRegister1										][MasterModbusSlave3] = {TestRegister1					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave3].TestValue1, 			MasterModbus_R },//ʵʱ������16λ
	[TestRegister2 										][MasterModbusSlave3] = {TestRegister2 					+ ModbusMasterBaseAdd,											&ModbusMasterDealType.RegisterValue[MasterModbusSlave3].TestValue2,				MasterModbus_R},//ʵʱ������16λ
																	   
	[MasterNullRegister									][MasterModbusSlave3] = {MasterNullRegister 				+ ModbusMasterBaseAdd,											&ValueNull,																MasterModbus_R },
	/*                                   �豸3�Ĵ�����ַ��Ϣ end                                        */

};

//��������   ModbusMasterSetTxRxMode
//���ߣ�     
//���ڣ�    2022-10-27
//���ܣ�    �������ط�����д������������������¹��� 
//			ModbusMasterSetTxRxMode(MasterModbusTxEn); //�л�Ϊ����ģʽ
//			ModbusMasterSetTxRxMode(MasterModbusRxEn); //�л�Ϊ����ģʽ
//��ע		�������궨����ͷ�ļ��ж��� #define MasterModbusTxEn 1
//										#define MasterModbusRxEn 0
//���������Value  MasterModbusTxEn ����ģʽ  MasterModbusRxEn ����ģʽ
//����ֵ��  �޷���
//�޸ļ�¼��
static void ModbusMasterSetTxRxModeDefault(uint8_t Value)
{
	if(Value == 0){
		HAL_GPIO_WritePin(MASTER_MODBUS_TXRX_GPIO_Port,MASTER_MODBUS_TXRX_Pin,GPIO_PIN_RESET);
	}else{
		HAL_GPIO_WritePin(MASTER_MODBUS_TXRX_GPIO_Port,MASTER_MODBUS_TXRX_Pin,GPIO_PIN_SET);
	}
}
//��������  ModbusMasterStartSendBufferFun
//���ߣ�    
//���ڣ�    2022-10-27
//���ܣ�    �������ط�����д������������������¹��� 
//			����Ҫ���͵����ݳ��ȣ����Ҫ���͵����ݵ� ModbusMasterDealType.Send.Buffer����ʼ���ݷ���
//���������len
//����ֵ��  �޷���
//�޸ļ�¼��
static void ModbusMasterStartSendBufferFunDefault(uint16_t len)
{
	
	HAL_UART_Transmit_DMA(&MasterModbushuart,ModbusMasterDealType.Send.Buffer,len);
}
//��������  ModbusMasterCommunicationInit
//���ߣ�    
//���ڣ�    2022-05-26
//���ܣ�    ��APPͨ�ų�ʼ������
//�����������
//����ֵ��  �޷���
//�޸ļ�¼��
static void ModbusMasterCommunicationInit(ModbusMasterDealTypeDef *p)
{
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	p->SendDataList.head = (list_t *)mymalloc(SRAMIN,sizeof(list_t)); 
	if(p->SendDataList.head != NULL){
		list_init(p->SendDataList.head); //��ʼ������
		p->SendDataList.ListNum = list_len(p->SendDataList.head);
	}else{
		//�ڴ治��
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
//��������  ModbusMasterStartSendBuffer
//���ߣ�    
//���ڣ�    2022-05-26
//���ܣ�    Ҫ���͵�����ȫ����䵽���飬ִ�д˺����������ݷ���  ��ע���˺�����Ҫ�궨��ע��uart���ͺ���
//���������ModbusMasterDealTypeDef *p
//����ֵ��  ���ͣ�char)
//          0����������
//			-1:���ݳ������ͻ��峤�ȴ��󲢷��ʹ���֡��Ϣ
//�޸ļ�¼��
static signed char ModbusMasterStartSendBuffer(ModbusMasterDealTypeDef *p)
{
	signed char ret = 0;
	ModbusMasterDealType.TimeCnt = 0;
	if(p->Send.len <= ModbusMasterDealType.Send.Maxlen){
		//p->SetTxRxModeCall(MasterModbusTxEn);
		p->StartSendBufferCall(p->Send.len);
	}else{
		//���ݳ������ͻ��峤�ȴ���
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
��������: void crc_cal_value(.)

����˵����CRC8У��ֵ����

����˵��: *data_value---����buffer
          data_length---���ݳ���
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
			list_append(p->SendDataList.head, q); //׷�ӽ��
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
			list_append(p->SendDataList.head, q); //׷�ӽ��
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
			list_append(p->SendDataList.head, q); //׷�ӽ��
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

		if(p->Send.Buffer[0] != p->Receive.Buffer[0] && p->Receive.Buffer[0] != 0){//��ַ��ƥ�䣬��Ӧ��  DeviceAddr != p->Receive.Buffer[0]
			ModbusMasterSendListDelete(p);//���ϱ���ɾ������
		}else if(check == 0){ //У�����
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
			ModbusMasterSendListDelete(p);//ɾ������
			p->ModbusErrorCnt = 0;
			memset(p->Send.Buffer,0,p->Send.len);
		}
		
		if(p->ModbusErrorCnt > 3){
			ModbusMasterSendListDelete(p);//���ϱ���ɾ������
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

