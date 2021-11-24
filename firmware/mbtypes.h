#ifndef __MBTYPES_H
#define __MBTYPES_H
#include "stm32f10x.h"
		/* FatFs configurations and declarations */

// MODBUS commans structure (common block)
#define _u_dev_addr      0
#define _u_cmd_code      1
#define _u_start_addr_hi 2
#define _u_start_addr_lo 3
#define _u_word_count_hi 4
#define _u_word_count_lo 5
// MODBUS commans structure ( additional data field for 10h command)
#define _u_byte_cnt_cm10     6
#define _u_data_section_cm10 7
// MODBUS commans structure ( additional data field for 03h command)
#define _u_data_section_cm03 3

// MODBUS commans structure (byte count, as rule for transmiting response from device)
#define _u_byte_cnt 2

#define illegal_data_address       0x02
#define error_in_data_size         0x03
#define error_in_request_data_size 0x04
#define error_message_lenght       0x03

typedef
  union TClntStat { // Byte-addressable UINT
     u8 Stat;
	 struct {
           unsigned bReceiveValidPacket : 1; //����� ������ �������� �����, �� ������ ���� ���� ��� ������ ������
           unsigned bPacketTransmited   : 1; //����� �������� �����, ������ ������� ������, ��� ����� ������������� �� ������ ��������
           unsigned bWaitForReceive     : 1;
           unsigned bWaitForTransmit    : 1;
	 }bits;
  } TClntStat;

typedef struct _TClient_
{
  u8  Buffer[256];//��� ������������ �������� ������, � ��� �� ��������� ������ ��� ��������
  u16  Idx;        //������ ��� ����������� �� ������
  u16  DevAddr;    //����� ���������� �� ������� ����� ����������� (��� ������� ��� ������ �� ������� �� �������,
                  //� �� 0-�����(���������)���� ����� �����������. ��� �������� ���������� ������, ���� ������
                  //������������ � DevAddr � "0" ���� ��� ���, �� ����� ��������� �������)
  u16  TXCount;    //���-�� ���� ������� ����� ��������
  TClntStat Status;//��������� ����������
  u16  ClntTimeOut;//FE ��� ������ ����������� �� �������� ���������� (�������� ��������)
  void (*OnCallBack)(void);//����� �� ����� ������ �� 020, ���������� ����������.
  void (*OnCallBackReceive)(void);//��� ���������� �������� ������ � ������, ���������� ������ ����������� � ��������
                                  //����, ����� ���� ��� ������ ������������, ����� ��� ������ ������ ������������ �� ��������
  u8  GAP;
  u8  ID; //����� ���������� 0-UART0, 1-UART1, 2-UART2, 3-UART3
  u8  BPS;//�������� �����
  //��� �������� � ������� ����� ���������
  //FILINFO fno;
  //DIR dir;
  //FIL f;
} TClient;

#endif
