#include "stm32f10x.h"
#include "bastypes.h"
#pragma section = ".fdsection"
const struct TFLASH_DATA FLASH_DATA @ ".fdsection" =
{
  0x0401, //device addres and baud rate for uart1 (modbus1);
  0x003C, //калибровочный коэффициент 1 канала
  0x003C, //калибровочный коэффициент 2 канала
};


#pragma section = ".bkfdsection"
const struct TFLASH_DATA BKFLASH_DATA @ ".bkfdsection" = 
{
  0x0401, //device addres and baud rate for uart1 (modbus1);
  0x003C, //калибровочный коэффициент 1 канала
  0x003C, //калибровочный коэффициент 2 канала
};

