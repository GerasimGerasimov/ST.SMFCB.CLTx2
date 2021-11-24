#include "memutil.h"
#include "mbtypes.h"
#include "flashdata.h"


void u8_mem_cpy( u8 *s, u8 *d, u8 c);
//void swp_copy_u16( u8 *s, u16 *d, u8 c);
void FlashSectorWrite(u32 FlashSectorAddr, u32 Buffer);
void CopyFlashToTmpBuffer(u32 Addr);

__no_init a32a16a8 FlashTmpBuffer;//��� IAP ������ ������������� � "�������" ������


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

//�������� ���������� ��������� ������� ���� ������ �� ��������� ����� (�������� �������)
void CopyFlashToTmpBuffer(u32 Addr){
  u32 *source = (u32 *) Addr;
  u32 *dest   = (u32 *) FlashTmpBuffer.dwFlashTmpBuffer;
  u32  count  = FlashTmpBufferSize_dw;
 // u32  count  = FlashTmpBufferSize_w;
  while (count != 0) {//21 �������
    *dest++ = *source++;
    count--;
  };
}

//������ ��������� � ����� ������� �� ��������� ������
//Source - ��������� �� �������� ������
//Position - ��������� �� ��������� �����
//Count - ���-�� ���������� ����


void u8_mem_cpy( u8 *s, u8 *d, u8 c){
  while (c) {
    c--;
    *d++ = *s++;
  }
}

/*
//swp_copy_u16( u8 *s, u16 *d, u8 c)
//���������� � ������������� ����
//u8  *s ����� ������ - ������ ����� �������� �������� ������
//u16 *d ����� ������ - ���� ����� ������ ������������ ������ (���� �� 2 �����)
//u8   c ���������� �������������� ���� � �������� ������ (����� �������� � �����+1)
void swp_copy_u16( u8 *s, u16 *d, u8 c){
  bauint w;
  while (c) {
    c--;
    //������ � �������
    w.b[1] = *s++;
    w.b[0] = *s++;
    *d++ = w.i;
  }
}
*/


