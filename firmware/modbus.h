#ifndef MODBUS_H
#define MODBUS_H
#include "mbtypes.h"

//диапазон адресов оперативных данных подключенного устройства
#define r_min_DEVICE_RAM_DATA 0x0000
#define r_max_DEVICE_RAM_DATA 0x00FF


//диапазон адресов копии FLASH (уставок) подключенного устройства
#define r_min_DEVICE_FLASH_DATA 0x2000
#define r_max_DEVICE_FLASH_DATA 0x207F



extern void ModbusInit(void);
extern bool ModbusMemRead(TClient *pC);
extern bool ModbusMemWrite(TClient *pC);
extern bool command_decode(TClient *pC);
extern void frame_end(TClient *pC);

void CopyFlashToTmpBuffer_(u32 Addr, u32 Buff);
extern void FlashWrite(u32 DATA_BASE);

#endif
