//modbus rtu интерфейс организующий доступ к параметрам процессора через
//RS485 интерфейс
//Используется:
//UART1 (RX, TX, DIR, скорость фиксированная 115200)
//Timer4 (отсчёт временных интервалов Modbus RTU)

//должен откуда-то брать адрес устройства и скорость связи
//я хочу чтобы юзер меньше знал о существовании АРМа
//и встроить параметры связи в GALS
#include "uart1rs485.h"
#include "bastypes.h"
#include "mbtypes.h"
#include "flashdata.h"
#include "ramdata.h"
#include "modbus.h"
#include "stm32f10x.h"

TClient uart1data;
void RxDMA1Ch5 (void);//настройка DMA на чтение данных из UART
void TxDMA1Ch4 (void);//настройка DMA на передачу данных в UART
void U1SetTimer(unsigned int Delay);  //зарядка таймера на подождать перед отправкой данных

#define SetDIR1ToRX    GPIO_WriteBit(GPIOA, GPIO_Pin_8,  (BitAction)(0));
#define SetDIR1ToTX    GPIO_WriteBit(GPIOA, GPIO_Pin_8,  (BitAction)(1));

#define U1RXBUFFSIZE  255 //размер буфера приёмника

u8 U1_RX_DATA_READY = 0;//флаг приёма пакета не ждем
u8 U1_TX_WAIT = 0;//флаг отправки пакета ждем!!!
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
  SetDIR1ToRX;//драйвер RS485 на приём
  uart1data.DevAddr = FLASH_DATA.MODBUS1.b[0];//адрес устройства в сети модбас
  uart1data.Idx = 0;//буфер начать с начала
  uart1data.TXCount = 0;
  uart1data.ClntTimeOut = 200;//4800=200мкс
  uart1data.ID = 1;//номер интерфейса
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
  //USART1->BRR = U2BPS[(u8)FLASH_DATA.MODBUS_slave.b[1]]; //U1BPS[FLASH_DATA.MODBUS1.b[1]];//26;//52;//104;// 12.0000.000МГц / 115200bps = 104h скорость связи
  USART1->CR1 |=  USART_CR1_RE;//разрешить приёмник
  USART1->CR1 |=  USART_CR1_TE;//разрешить передатчик
  USART1->CR1 |=  USART_CR1_UE;//разрешить UART2
  U1_TX_WAIT = 0;
  U1_RX_DATA_READY = 0;
  RxDMA1Ch5();//настройка DMA на чтение данных из UART
}

//сравнить BPS и DEVADDR для UART2 если отличаются, то сделать повторнуюю инициализацию

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
  SetDIR1ToTX;//переключаю драйвер на передачу, дернули ногой

  //зарядка таймера
  TIM_Cmd(TIM1, DISABLE);  
  TIM1->CNT = 0;
  TIM1->ARR = Delay;
  TIM_Cmd(TIM1, ENABLE);//TIMER3_START;
  TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);//ждем когда пауза сработает.
}

void TIM1_user_U1(void)
{
  TIM1->SR = 0;//сбросить флаг прерывания
  TIM1->CR1 &= ~TIM_CR1_CEN;//остановить таймер
  TxDMA1Ch4();  //настройка и запуск дма
}

void U1_ResetModbusTimerForWaitTransmit(void)
{
  //тут подождать 
  SetDIR1ToTX;//переключаю драйвер на передачу, дернули ногой
  TxDMA1Ch4();  //настройка и запуск дма
  U1_TX_WAIT = 1;
}

u8 U1_SwCNT (void)
{
  
  if (U1_TX_WAIT) return 0; 
  if (U1_RX_DATA_READY !=0) 
  {
    U1_RX_DATA_READY = 0;
    uart1data.OnCallBackReceive = &U1_ResetModbusTimerForWaitTransmit;
    //декодирование команды
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
      RxDMA1Ch5();//восстановлением работы приёмника
  }
  return 0;
}

void usart1DMA_init (void)
{
  DMA_InitTypeDef DMA_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  DMA_DeInit(DMA1_Channel4);//на всякимй случай
  DMA_DeInit(DMA1_Channel5);//на всякимй случай
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USART1->DR);//источник - регистр данных UART
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &uart1data.Buffer[0];//приёмник - мой буфер (размер 256 байт)
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//направление из переферии в память (буфер)
  DMA_InitStructure.DMA_BufferSize = 0;//сколько байт отправить
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//адрес переферии не инкрементируется
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//адрес (ссылка на буфер) инкрементируется
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//размер данных переферии БАЙТ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//размер данных буфера БАЙТ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//по заполнению буфера DMA останавливается
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel4, &DMA_InitStructure); 
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//направление из переферии в память (буфер)
  DMA_Init(DMA1_Channel5, &DMA_InitStructure); 
}


void TxDMA1Ch4 (void) {//настройка DMA на передачу данных в UART
  
  DMA1_Channel4->CCR  &= ~DMA_CCR4_EN;//DMA_Cmd(DMA1_Channel7, DISABLE);//отключаю DMA для получения доступа к регистрам
  DMA1_Channel4->CNDTR = uart1data.TXCount;//сколько байт отправить
  USART1->SR  &=  ~USART_SR_TC;   //сбросить флаг окончания передачи
  USART1->CR3 |=  USART_CR3_DMAT;
  DMA1->IFCR |= DMA_IFCR_CTCIF4 | DMA_IFCR_CGIF4 | DMA_IFCR_CHTIF4 | DMA_IFCR_CTEIF4;//очищу все флаги прерываний 
  USART1->CR1 |=  USART_CR1_TE;   //разрешить передатчик
  DMA1_Channel4->CCR  |= DMA_CCR4_EN;//DMA_Cmd(DMA1_Channel7, ENABLE);//включаю DMA... и он начинает из буфера выкидывать данные на ТХ
  USART1->CR1 |=  USART_CR1_TCIE; //разрешу прерывания по окончанию передачи
} 

void RxDMA1Ch5 (void) {//настройка DMA на чтение данных из UART

  SetDIR1ToRX;//включить драйвер на приём
  DMA1_Channel5->CCR  &= ~DMA_CCR5_EN;//DMA_Cmd(DMA1_Channel6, DISABLE);//отключаю DMA для получения доступа к регистрам
  DMA1_Channel5->CNDTR = U1RXBUFFSIZE;//256 байт размер принимающего буфера
  USART1->CR3 |=  USART_CR3_DMAR;
  DMA1->IFCR |= DMA_IFCR_CTCIF5 | DMA_IFCR_CGIF5 | DMA_IFCR_CHTIF5 | DMA_IFCR_CTEIF5;//очищу все флаги прерываний 
  DMA1_Channel5->CCR  |= DMA_CCR5_EN;//DMA_Cmd(DMA1_Channel6, ENABLE);//включаю DMA... и он начинает складывать поступающие данные в заданный буфер
  USART1->CR1 |=  USART_CR1_IDLEIE;//разрешить прерывания по приёму данных
  USART1->CR1 |=  USART_CR1_RE;//разрешить приёмник
}

void USART1_IRQHandler(void)
{  
  u32 IIR = USART1->SR;
    if ((IIR & USART_SR_TC) && (USART1->CR1 & USART_CR1_TCIE)) // Передача окончена (последний байт полностью передан в порт)
      { 
        USART1->SR  &=  ~USART_SR_TC;   //сбросить флаг окончания передачи
        USART1->CR1 &=  ~USART_CR1_TCIE;//запретить прерывание по окончании передачи
        USART1->CR3 &=  ~USART_CR3_DMAT;//запретить UART-ту передавать по DMA
        DMA1_Channel4->CCR  &= ~DMA_CCR4_EN;//DMA_Cmd(DMA1_Channel7, DISABLE);//выключить DMA передатчика
        //переключить на приём
        RxDMA1Ch5();//настройка DMA на чтение данных из UART
        U1_TX_WAIT = 0;
        return;
      }
    if ((IIR & USART_SR_IDLE) & (USART1->CR1 & USART_CR1_IDLEIE)) // Между байтами при приёме обнаружена пауза в 1 IDLE байт
      {        
        IIR = USART1->DR; //сброс флага IDLE
        IIR = USART1->SR;
        USART1->CR1 &=  ~USART_CR1_RE;    //запретить приёмник
        USART1->CR1 &=  ~USART_CR1_IDLEIE;//запретить прерывания по приёму данных
        USART1->CR3 &=  ~USART_CR3_DMAR;  //запретить DMA RX
        DMA1_Channel5->CCR  &= ~DMA_CCR5_EN;//DMA_Cmd(DMA1_Channel6, DISABLE);//выключить DMA на приём
        uart1data.Idx = (u8)(U1RXBUFFSIZE - DMA1_Channel5->CNDTR);//кол-во принятых байт
        U1_RX_DATA_READY = 1;//выставляю флаг основному циклу что пакет данных принят
        U1_TX_WAIT = 0;//нет ожидания передачи
      }
}






