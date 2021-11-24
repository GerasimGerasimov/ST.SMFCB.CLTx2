#ifndef BASTYPES_H
#define BASTYPES_H
#include "stm32f10x.h"// basic types

typedef u8 *pu8;

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;

typedef
  union bauint { // Byte-addressable UINT
     u16 i; // int: Var.i
     u8 b[2]; // u char: Var.b[0]:Var.b[1]
  } bauint;


typedef
  union bavu16 { // Byte-addressable UINT
     vu16 i; // int: Var.i
     vu8 b[2]; // u char: Var.b[0]:Var.b[1]
  } bavu16;

typedef
  union baulong { // Byte-addressable UINT
     ulong L;    // 1 DWORD
     uint  i[2]; // 2 WORD
     uchar b[4]; // 4 BYTES
  } baulong;

typedef
  union bauqword { // Byte-addressable QWORD 64bit
    uchar  b[8]; //8  BYTES
    uint   i[4]; //4  WORD
    ulong  L[2]; //2 DWORD
    signed long long int Q; // QWORD
  }bauqword;
 
typedef
  union biauint { // Byte-addressable UINT
     uint i; // int: Var.i
     uchar b[2]; // u char: Var.b[0]:Var.b[1]
	 struct {
	 	unsigned b0:1;
	 	unsigned b1:1;
	 	unsigned b2:1;
	 	unsigned b3:1;
	 	unsigned b4:1;
	 	unsigned b5:1;
	 	unsigned b6:1;
	 	unsigned b7:1;
	 	unsigned b8:1;
	 	unsigned b9:1;
	 	unsigned b10:1;
	 	unsigned b11:1;
	 	unsigned b12:1;
	 	unsigned b13:1;
	 	unsigned b14:1;
	 	unsigned b15:1;
	 }bits;
  } biauint;

typedef
  union biauchar { // Byte-addressable UINT
     uchar b;
	 struct {
	 	unsigned b0:1;
	 	unsigned b1:1;
	 	unsigned b2:1;
	 	unsigned b3:1;
	 	unsigned b4:1;
	 	unsigned b5:1;
	 	unsigned b6:1;
	 	unsigned b7:1;
	 }bits;
  } biauchar;

struct TFLASH_DATA {
    bavu16 MODBUS1;//device addres and baud rate for uart1 (modbus1); 0x2000   
    u16 KDAC1; //калибровочный коэффициент 1 канала
    u16 KDAC2; //калибровочный коэффициент 2 канала
 };


typedef union {
    u32	   i;
    struct {
      unsigned b0: 1;
      unsigned b1 : 1;
      unsigned b2 : 1;
      unsigned b3 : 1;
      unsigned b4 : 1;
      unsigned b5 : 1;
      unsigned b6 : 1;
      unsigned b7 : 1;
      unsigned b8 : 1;
      unsigned b9 : 1;
      unsigned b10 : 1;
      unsigned b11 : 1;
      unsigned b12 : 1;
      unsigned b13 : 1;
      unsigned b14 : 1;
      unsigned b15 : 1;
      unsigned b16 : 1;
      unsigned b17 : 1;
      unsigned b18 : 1;
      unsigned b19 : 1;
      unsigned b20 : 1;
      unsigned b21 : 1;
      unsigned b22  : 1;
      unsigned b23  : 1;
      unsigned b24  : 1;
      unsigned b25  : 1;
      unsigned b26 : 1;
      unsigned b27 : 1;
      unsigned b28 : 1;
      unsigned b29 : 1;
      unsigned b30 : 1;
      unsigned b31 : 1;
    } BA;
  } _FLG0;

//параметры расположеные в RAM
  struct TRAM_DATA {   
    //r0000, r0001 - x0000
    _FLG0 FLAGS;    //32bit флаги управления и индикации;
    //r0002.l - x0004
    u16 dig_index;      // счётчик индикаторных разрядов
    //r0002.h - x0005
    u16 digit;          // номер индикаторного разряда
    //r0003 - x0006
    u16 LocalRef1;       // 1 число заданное потенциометром
    //r0004 - x0008
    u16 LocalRef2;       // 2 число заданное потенциометром
    //r0005 - x000A
    u16 NetworkRef1;     // 1 число заданное через связь
    //r0006 - x000C
    u16 NetworkRef2;     // 2 число заданное через связь
    //r0007 - x000E
    u16 OutChanel1;       // 1 значение выводимое ЦАПом
    //r0008 - x0010
    u16 OutChanel2;       // 2 значение выводимое ЦАПом
} ;

#endif