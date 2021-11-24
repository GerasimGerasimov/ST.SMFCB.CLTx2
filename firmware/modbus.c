#include "mbtypes.h"
#include "bastypes.h"
#include "modbus.h"
#include "crc16.h"
#include "memutil.h"
#include "uart1rs485.h"
#include "id.h"
#include "flashdata.h"
#include "ramdata.h"

u8 aFlashTmpBuffer[256];

void ModbusInit(void);
void REinit(void);
void ModbusRamRead(u32 DATA_BASE, TClient *pC);
void ModbusRamWrite(u32 DATA_BASE, TClient *pC);
void ModbusCDWrite(u32 DATA_BASE, TClient *pC);//запись калибровочных данных
void ModbusFlashWrite(u32 DATA_BASE, TClient *pC);
/////////
void ModbusFlashWrite_(u32 DATA_BASE, TClient *pC);
////

void frame_end(TClient *pC);

void ModbusInit(void){
  //FilesStructureInit();
}
void frame_end(TClient *pC)
{
  bauint crc;
  crc.i = crc16(&pC->Buffer[0], pC->TXCount);//подсчет CRC буфера
  //запись CRC в конец буфера
  pC->Buffer[pC->TXCount++] = crc.b[1];//crc >> 8;//crc;//crc >> 8     ;//CRCHi;
  pC->Buffer[pC->TXCount++] = crc.b[0];//crc;//crc >> 8;//crc  ;//CRCLo;
}

u8 GetDeviceIDLength(void){
u8 i=0;
 while (DeviceID[i++]!=0);
 return i;
}

void GetDeviceID(TClient *pC){
u8 i=0;
u8 j;
   j = pC->Buffer[_u_byte_cnt] = pC->TXCount = GetDeviceIDLength();
   do
     pC->Buffer[_u_data_section_cm03+i]=DeviceID[i];
   while ((i++)!=j);
   pC->TXCount+=2;//прибавить длину заголовка
   frame_end(pC);
}

bool ModbusMemRead(TClient *pC){
  bauint w;


  //получаю номер регистра в w.i
  w.b[0] = pC->Buffer[_u_start_addr_lo];
  w.b[1] = pC->Buffer[_u_start_addr_hi];
  //выбор программы чтения памяти
  //RAM_DATA

  //оперативные данные 
  if ((w.i >= r_min_DEVICE_RAM_DATA) && (w.i <= r_max_DEVICE_RAM_DATA)) 
  {   ModbusRamRead((u32) &RAM_DATA, pC);
      return (TRUE);
  }
  
  //DEVICE_FLASH_DATA данные уставок подключенного устройства
  if ((w.i >= r_min_DEVICE_FLASH_DATA) && (w.i <= r_max_DEVICE_FLASH_DATA)) 
  {
    ModbusRamRead((u32)&FLASH_DATA, pC);
    return (TRUE);
  }
  
   //не нашёл подходящей области
  //ModbusDataRetranslate(&lnks_RTR_WRITE, pC, TRUE);//пробую ретранслировать, вдруг Устройство знает
  return (FALSE);
}



typedef struct TRetReg{//ретранслируемый регистр
  u16 Disp_reg; //номер регистра в LPC
  u16 Device_reg;//соответсвующий ему номер регистра в F020
} TRetReg;

bool ModbusMemWrite(TClient *pC){
  bauint w;
  //получаю номер регистра в w.i
  w.b[0] = pC->Buffer[_u_start_addr_lo];
  w.b[1] = pC->Buffer[_u_start_addr_hi];
  
  //выбор программы записи памяти
  
  //DEVICE_RAM_DATA
  if ((w.i >= r_min_DEVICE_RAM_DATA) && (w.i <= r_max_DEVICE_RAM_DATA)) {
    //ретранслировать команду записи Устройству
    ModbusRamWrite((u32)&RAM_DATA, pC);
    
  return(TRUE);
  }
  
  //для flash дисплея   
    if ((w.i >= r_min_DEVICE_FLASH_DATA) && (w.i <= r_max_DEVICE_FLASH_DATA)) 
    {
      ModbusFlashWrite_((u32)&FLASH_DATA ,pC);
     uart1rs485_ReInit();
      return (TRUE);
  }


    //не нашёл подходящей области
  //ModbusDataRetranslate(&lnks_RTR_WRITE, pC, TRUE);//пробую ретранслировать, вдруг Устройство знает
  return (FALSE);
}

void ModbusRamRead(u32 DATA_BASE, TClient *pC){
  bauint w;
  u32 i;
  //RAM_DATA_BASE начало адресов ОЗУ, для получения абсолютного адреса
  //к значению полученному из номера регистра прибавить RAM_DATA_BASE
  //1) получить из буфера номер регистра
  //1.1) преобразовать его в адрес в памяти
  w.b[0] = pC->Buffer[_u_start_addr_lo];
  w.b[1] = pC->Buffer[_u_start_addr_hi] & 0x0F;
  i = w.i;
  i = (i << 1) & 0x1FFF;
  u16 *ModbusAddrSet = (u16 *)(DATA_BASE + i);
  //2)получим кол-во передаваемых регистров
  u8 ModbusAddrCount = pC->Buffer[_u_word_count_lo];
  //4) заполнение буфера
  //4.1) записать в буфер кол-во передаваемых байт
  pC->Buffer[_u_byte_cnt]=pC->TXCount=(u16)(ModbusAddrCount << 1);
  pC->TXCount+=3;
  //4.2) записать в буфер содержимое регистров (старшими байтами вперёд)
  u8 j = _u_data_section_cm03;
  do {
    w.i = *ModbusAddrSet++;
    pC->Buffer[j++]= w.b[1];
    pC->Buffer[j++]= w.b[0];
  } while (--ModbusAddrCount != 0);
  
  frame_end(pC);
}

void ModbusRamWrite(u32 DATA_BASE, TClient *pC){
  bauint w;
  u32 i;
  //1) получить из буфера номер регистра
  //1.1) преобразовать его в адрес в памяти
  w.b[0] = pC->Buffer[_u_start_addr_lo];
  w.b[1] = pC->Buffer[_u_start_addr_hi] & 0x0F;
  i = w.i;
  i = (i << 1) & 0x1FFF;
  u16 *ModbusAddrSet = (u16 *)(DATA_BASE + i);
  //2)получим кол-во переданных регистров
  u8 ModbusAddrCount = pC->Buffer[_u_word_count_lo];
  //3)чтение буфера в память
  u8 j = _u_data_section_cm10;
  do {
    w.b[1] = pC->Buffer[j++];
    w.b[0] = pC->Buffer[j++];
    *ModbusAddrSet++ = w.i;
  } while (--ModbusAddrCount != 0);
  pC->TXCount = 6;
  frame_end(pC);
}


bool command_decode(TClient *pC){
  u8 cmd;
  //проверка контрольной суммы пакета
  //Если адрес устройства "0" - то пакет широковещательный, отвечать на него нельзя!
  if (crc16(&pC->Buffer[0], pC->Idx) == 0){
    //проверка адреса устройствa
    if (pC->Buffer[_u_dev_addr] == pC->DevAddr){
      //адрес совпал, контрольная сумма пакета правильная
      //Расшифровка поля комад
   
      cmd = pC->Buffer[_u_cmd_code];
      switch (cmd){
	case 0x03: if (ModbusMemRead(pC)) {return (TRUE);}//multiplay registers read
                   return (FALSE);
	case 0x10: if (ModbusMemWrite(pC)) { return (TRUE);}//multiplay registers write
        
                   //REinit(); 
                   return (FALSE);
	case 0x11: GetDeviceID(pC);//flash_time = 1; //читаем флеш 
                   return (TRUE);
	default:  return (FALSE);
      };
    };
  };
  return (FALSE);
}



//копирует содержимое заданного сектора флэш памяти во временный буфер (двойными словами)
void CopyFlashToTmpBuffer_(u32 Addr, u32 Buff)
{
  u32 *source = (u32 *) Addr;
  u32 *dest   = (u32 *) Buff;
  u32  count  = FlashTmpBufferSize_dw;
  while (count != 0) {//21 команды
    *dest++ = *source++;
    count--;
  };
}




void ModbusFlashWrite_(u32 DATA_BASE, TClient *pC){
u16 crc;
  u16 *dest;
  u8 *source;
  bauint w; //for swaping modbus packets
  //0)копировать основной сектор флэша во временный буфер aFlashTmpBuffer
  CopyFlashToTmpBuffer_((u32)&FLASH_DATA, (u32)&aFlashTmpBuffer);
  //1) получить из буфера номер регистра 
  //1.1) преобразовать его в адрес в памяти
  //2)получим кол-во переданных регистров
  u8 ModbusAddrCount = pC->Buffer[_u_word_count_lo];
  //3)внести изменения во временный буфер
  source = &pC->Buffer[_u_data_section_cm10];
  dest = (u16*)&aFlashTmpBuffer[0] + (pC->Buffer[_u_start_addr_lo]);//RAM_DATA.Iload....<<1
  do {
    w.b[1] = *source++;
    w.b[0] = *source++;
    *dest++ = w.i;
  } while (--ModbusAddrCount != 0);
  //4)данные находятся во временном буфере, теперь:
  //4.1)Подсчитать контрольную сумму временного буфера
  crc = crc16(&aFlashTmpBuffer[0], FlashTmpBufferSize_bytes-2);
  //4.2)добавить контрольную сумму в конец временного буфера
  aFlashTmpBuffer[254] = (crc >> 8) & 0x00ff;
  aFlashTmpBuffer[255] = crc & 0x00ff;
  //5)стереть резервный сектор флэша
  //6)записать туда данные из временного буфера
  __disable_irq(); // handles nested interrupt
  FlashSectorWrite((u32)&BKFLASH_DATA, (u32) &aFlashTmpBuffer);//
  //6.1)Проверить CRC буфера, если не испортился писать дальше
     if (crc16((u8*)&BKFLASH_DATA, FlashTmpBufferSize_bytes)==0)
   {
     FlashSectorWrite((u32)&FLASH_DATA, (u32) &aFlashTmpBuffer);// пишем в основной сектор
   }
   else 
   {
     //Блок оказался кривым, резервный сектор флеша испорчен.
     //Попытаться Восстановить из основного, если не получится
     //не пытаться переписать основной сектор
      FlashSectorWrite((u32)&FLASH_DATA, (u32)&BKFLASH_DATA);

      //тут по идее какую нибудь ошибку бы выдать
     }

  __enable_irq(); // handles nested interrupt
  
  pC->TXCount=6;
  frame_end(pC);
}  

