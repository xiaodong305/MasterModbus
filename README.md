# MasterModbus
stm32f407 CubeMx
0.相关串口初始化 优先考虑 uart + dma 接收空闲中断 
1.修改寄存器词典	 以及 ModbusMasterReceiveReadCmdDeal 函数
2.修改发送接口 重写 ModbusMasterStartSendBufferFun 函数  发送数据填充到 ModbusMasterDealType.Send.Buffer 
    并把数组中的数据发出
3.修改发送接收数据长度  在ModbusMaster.h 中 MasterModbusSENDBUFF_SIZE  MasterModbusRECEIVEBUFF_SIZE 修改
4.指定接收数组。把接收数据放到 ModbusMasterDealType.Receive.Buffer 数组中
5.修改设置发送接收方向引脚函数 重写 ModbusMasterSetTxRxMode 函数
6.添加发送完成中断  
		ModbusMasterDealType.SetTxRxModeCall(MasterModbusRxEn);
		ModbusMasterDealType.tx_finish_flag = 1;	
7.添加接收完成中断 
   	ModbusMasterDealType.SetTxRxModeCall(MasterModbusTxEn);
		ModbusMasterDealType.rx_finish_flag = 1;
		ModbusMasterDealType.Receive.len = len;
8.在main函数中添加 ModbusMasterDealType.InitCall(&ModbusMasterDealType);//Modbus主机初始化
9.在定时器中添加超时判断  在1ms毫秒定时器中添加 ModbusMasterDealType.TimeOutCall(&ModbusMasterDealType);
10.在while死循环中添加 	  
ModbusMasterDealType.ReceiveTaskDealCall(&ModbusMasterDealType);
ModbusMasterDealType.SendTaskDealCall(&ModbusMasterDealType);
 
 个人能力有限，仅供参考，
