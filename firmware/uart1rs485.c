//modbus rtu ��������� ������������ ������ � ���������� ���������� �����
//RS485 ���������
//������������:
//UART1 (RX, TX, DIR, �������� ������������� 115200)
//Timer4 (������ ��������� ���������� Modbus RTU)

//������ ������-�� ����� ����� ���������� � �������� �����
//� ���� ����� ���� ������ ���� � ������������� ����
//� �������� ��������� ����� � GALS
#include "uart1rs485.h"
#include "bastypes.h"
#include "mbtypes.h"
#include "flashdata.h"
#include "ramdata.h"
#include "modbus.h"
#include "stm32f10x.h"

TClient uart1data;
void RxDMA1Ch5 (void);//��������� DMA �� ������ ������ �� UART
void TxDMA1Ch4 (void);//��������� DMA �� �������� ������ � UART
void U1SetTimer(unsigned int Delay);  //������� ������� �� ��������� ����� ��������� ������

#define SetDIR1ToRX    GPIO_WriteBit(GPIOA, GPIO_Pin_8,  (BitAction)(0));
#define SetDIR1ToTX    GPIO_WriteBit(GPIOA, GPIO_Pin_8,  (BitAction)(1));

#define U1RXBUFFSIZE  255 //������ ������ ��������

u8 U1_RX_DATA_READY = 0;//���� ����� ������ �� ����
u8 U1_TX_WAIT = 0;//���� �������� ������ ����!!!
u8 B_rate_cur = 0;
 
const u32 U1BPS[]={  
  4800,// 0 
  9600,// 1 
 19200,// 2 
 57600,// 3 
115200,// 4
230400// 4

};

void uart1rs485_init (void){
  SetDIR1ToRX;//������� RS485 �� ����
  uart1data.DevAddr = FLASH_DATA.MODBUS1.b[0];//����� ���������� � ���� ������
  uart1data.Idx = 0;//����� ������ � ������
  uart1data.TXCount = 0;
  uart1data.ClntTimeOut = 200;//4800=200���
  uart1data.ID = 1;//����� ����������
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_BaudRate = U1BPS[FLASH_DATA.MODBUS1.b[1]];
  B_rate_cur = FLASH_DATA.MODBUS1.b[1];
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);
  //USART1->BRR = U2BPS[(u8)FLASH_DATA.MODBUS_slave.b[1]]; //U1BPS[FLASH_DATA.MODBUS1.b[1]];//26;//52;//104;// 12.0000.000��� / 115200bps = 104h �������� �����
  USART1->CR1 |=  USART_CR1_RE;//��������� �������
  USART1->CR1 |=  USART_CR1_TE;//��������� ����������
  USART1->CR1 |=  USART_CR1_UE;//��������� UART2
  U1_TX_WAIT = 0;
  U1_RX_DATA_READY = 0;
  RxDMA1Ch5();//��������� DMA �� ������ ������ �� UART
}

//�������� BPS � DEVADDR ��� UART2 ���� ����������, �� ������� ���������� �������������

void uart1rs485_ReInit (void){ 
  
  USART_InitTypeDef USART_InitStructure;
  
  if (B_rate_cur != FLASH_DATA.MODBUS1.b[1]) 
  {
    USART_InitStructure.USART_BaudRate = U1BPS[FLASH_DATA.MODBUS1.b[1]];
    B_rate_cur = FLASH_DATA.MODBUS1.b[1];
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
  }
  if (uart1data.DevAddr !=FLASH_DATA.MODBUS1.b[0])
  {
    uart1data.DevAddr = FLASH_DATA.MODBUS1.b[0];
  }
}


void U1SetTimer(unsigned int Delay)
{
  SetDIR1ToTX;//���������� ������� �� ��������, ������� �����

  //������� �������
  TIM_Cmd(TIM1, DISABLE);  
  TIM1->CNT = 0;
  TIM1->ARR = Delay;
  TIM_Cmd(TIM1, ENABLE);//TIMER3_START;
  TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);//���� ����� ����� ���������.
}

void TIM1_user_U1(void)
{
  TIM1->SR = 0;//�������� ���� ����������
  TIM1->CR1 &= ~TIM_CR1_CEN;//���������� ������
  TxDMA1Ch4();  //��������� � ������ ���
}

void U1_ResetModbusTimerForWaitTransmit(void)
{
  //��� ��������� 
  SetDIR1ToTX;//���������� ������� �� ��������, ������� �����
  TxDMA1Ch4();  //��������� � ������ ���
  U1_TX_WAIT = 1;
}

u8 U1_SwCNT (void)
{
  
  if (U1_TX_WAIT) return 0; 
  if (U1_RX_DATA_READY !=0) 
  {
    U1_RX_DATA_READY = 0;
    uart1data.OnCallBackReceive = &U1_ResetModbusTimerForWaitTransmit;
    //������������� �������
    if (command_decode(&uart1data)) 
    {      
      if (uart1data.TXCount != 0)
      {
        U1SetTimer(uart1data.ClntTimeOut);
        U1_TX_WAIT = 1;
      }
      return 1;
    }
      //U1_TX_WAIT = 0;
      RxDMA1Ch5();//��������������� ������ ��������
  }
  return 0;
}

void usart1DMA_init (void)
{
  DMA_InitTypeDef DMA_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  DMA_DeInit(DMA1_Channel4);//�� ������� ������
  DMA_DeInit(DMA1_Channel5);//�� ������� ������
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USART1->DR);//�������� - ������� ������ UART
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &uart1data.Buffer[0];//������� - ��� ����� (������ 256 ����)
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//����������� �� ��������� � ������ (�����)
  DMA_InitStructure.DMA_BufferSize = 0;//������� ���� ���������
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//����� ��������� �� ����������������
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//����� (������ �� �����) ����������������
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//������ ������ ��������� ����
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//������ ������ ������ ����
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//�� ���������� ������ DMA ���������������
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel4, &DMA_InitStructure); 
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//����������� �� ��������� � ������ (�����)
  DMA_Init(DMA1_Channel5, &DMA_InitStructure); 
}


void TxDMA1Ch4 (void) {//��������� DMA �� �������� ������ � UART
  
  DMA1_Channel4->CCR  &= ~DMA_CCR4_EN;//DMA_Cmd(DMA1_Channel7, DISABLE);//�������� DMA ��� ��������� ������� � ���������
  DMA1_Channel4->CNDTR = uart1data.TXCount;//������� ���� ���������
  USART1->SR  &=  ~USART_SR_TC;   //�������� ���� ��������� ��������
  USART1->CR3 |=  USART_CR3_DMAT;
  DMA1->IFCR |= DMA_IFCR_CTCIF4 | DMA_IFCR_CGIF4 | DMA_IFCR_CHTIF4 | DMA_IFCR_CTEIF4;//����� ��� ����� ���������� 
  USART1->CR1 |=  USART_CR1_TE;   //��������� ����������
  DMA1_Channel4->CCR  |= DMA_CCR4_EN;//DMA_Cmd(DMA1_Channel7, ENABLE);//������� DMA... � �� �������� �� ������ ���������� ������ �� ��
  USART1->CR1 |=  USART_CR1_TCIE; //������� ���������� �� ��������� ��������
} 

void RxDMA1Ch5 (void) {//��������� DMA �� ������ ������ �� UART

  SetDIR1ToRX;//�������� ������� �� ����
  DMA1_Channel5->CCR  &= ~DMA_CCR5_EN;//DMA_Cmd(DMA1_Channel6, DISABLE);//�������� DMA ��� ��������� ������� � ���������
  DMA1_Channel5->CNDTR = U1RXBUFFSIZE;//256 ���� ������ ������������ ������
  USART1->CR3 |=  USART_CR3_DMAR;
  DMA1->IFCR |= DMA_IFCR_CTCIF5 | DMA_IFCR_CGIF5 | DMA_IFCR_CHTIF5 | DMA_IFCR_CTEIF5;//����� ��� ����� ���������� 
  DMA1_Channel5->CCR  |= DMA_CCR5_EN;//DMA_Cmd(DMA1_Channel6, ENABLE);//������� DMA... � �� �������� ���������� ����������� ������ � �������� �����
  USART1->CR1 |=  USART_CR1_IDLEIE;//��������� ���������� �� ����� ������
  USART1->CR1 |=  USART_CR1_RE;//��������� �������
}

void USART1_IRQHandler(void)
{  
  u32 IIR = USART1->SR;
    if ((IIR & USART_SR_TC) && (USART1->CR1 & USART_CR1_TCIE)) // �������� �������� (��������� ���� ��������� ������� � ����)
      { 
        USART1->SR  &=  ~USART_SR_TC;   //�������� ���� ��������� ��������
        USART1->CR1 &=  ~USART_CR1_TCIE;//��������� ���������� �� ��������� ��������
        USART1->CR3 &=  ~USART_CR3_DMAT;//��������� UART-�� ���������� �� DMA
        DMA1_Channel4->CCR  &= ~DMA_CCR4_EN;//DMA_Cmd(DMA1_Channel7, DISABLE);//��������� DMA �����������
        //����������� �� ����
        RxDMA1Ch5();//��������� DMA �� ������ ������ �� UART
        U1_TX_WAIT = 0;
        return;
      }
    if ((IIR & USART_SR_IDLE) & (USART1->CR1 & USART_CR1_IDLEIE)) // ����� ������� ��� ����� ���������� ����� � 1 IDLE ����
      {        
        IIR = USART1->DR; //����� ����� IDLE
        IIR = USART1->SR;
        USART1->CR1 &=  ~USART_CR1_RE;    //��������� �������
        USART1->CR1 &=  ~USART_CR1_IDLEIE;//��������� ���������� �� ����� ������
        USART1->CR3 &=  ~USART_CR3_DMAR;  //��������� DMA RX
        DMA1_Channel5->CCR  &= ~DMA_CCR5_EN;//DMA_Cmd(DMA1_Channel6, DISABLE);//��������� DMA �� ����
        uart1data.Idx = (u8)(U1RXBUFFSIZE - DMA1_Channel5->CNDTR);//���-�� �������� ����
        U1_RX_DATA_READY = 1;//��������� ���� ��������� ����� ��� ����� ������ ������
        U1_TX_WAIT = 0;//��� �������� ��������
      }
}






