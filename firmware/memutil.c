#include "memutil.h"
#include "mbtypes.h"
#include "flashdata.h"


void u8_mem_cpy( u8 *s, u8 *d, u8 c);
//void swp_copy_u16( u8 *s, u16 *d, u8 c);
void FlashSectorWrite(u32 FlashSectorAddr, u32 Buffer);
void CopyFlashToTmpBuffer(u32 Addr);

__no_init a32a16a8 FlashTmpBuffer;//для IAP должен располагаться в "обычной" памяти


void FlashSectorWrite(u32 FlashSectorAddr, u32 Buffer)
{
  volatile FLASH_Status FLASHStatus;
  u32 *source = (u32 *) Buffer;
  u32 Data;
  u32 Count = FlashTmpBufferSize_dw;

  FLASH_Unlock();  // Unlock the Flash Program Erase controller
  /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  FLASHStatus = FLASH_ErasePage(FlashSectorAddr);// Erase the FLASH pages
  while(Count !=0 )
  {
    Data = *source;
    FLASHStatus = FLASH_ProgramWord(FlashSectorAddr, Data);
    FlashSectorAddr += 4;
    source ++;
    Count --;
  }
}

//копирует содержимое заданного сектора флэш памяти во временный буфер (двойными словами)
void CopyFlashToTmpBuffer(u32 Addr){
  u32 *source = (u32 *) Addr;
  u32 *dest   = (u32 *) FlashTmpBuffer.dwFlashTmpBuffer;
  u32  count  = FlashTmpBufferSize_dw;
 // u32  count  = FlashTmpBufferSize_w;
  while (count != 0) {//21 команды
    *dest++ = *source++;
    count--;
  };
}

//вносит изменения в копию сектора во временном буфере
//Source - указатель на источник данных
//Position - указатель на временный буфер
//Count - кол-во заменяемых байт


void u8_mem_cpy( u8 *s, u8 *d, u8 c){
  while (c) {
    c--;
    *d++ = *s++;
  }
}

/*
//swp_copy_u16( u8 *s, u16 *d, u8 c)
//копировать с перестановкой байт
//u8  *s адрес буфера - откуда нужно забирать принятые данные
//u16 *d адрес буфера - куда нужно ложить обработанные данные (пишу по 2 байта)
//u8   c количество информационных байт в принятом пакете (потом превращу в слова+1)
void swp_copy_u16( u8 *s, u16 *d, u8 c){
  bauint w;
  while (c) {
    c--;
    //сваплю и копирую
    w.b[1] = *s++;
    w.b[0] = *s++;
    *d++ = w.i;
  }
}
*/


