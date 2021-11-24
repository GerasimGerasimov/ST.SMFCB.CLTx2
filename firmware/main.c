#include "stm32f10x.h"
#include "bastypes.h"
#include "ramdata.h"
#include "flashdata.h"
#include "crc16.h"
#include "init.h"
#include "modbus.h"
#include "uart1rs485.h"
#include "defines.h" 
#include "digits_display.h"

u16 getAdcValue(u8 ADC_Channel) {
  return ADC_GetInjectedConversionValue(ADC1, ADC_Channel);
}

u16 getChanel1Ref(bool select, u16 NetworkRef, u16 AdcValue) {
  float res = 0;
    if (select) {
      res = NetworkRef;
    } else {
      res = (float) AdcValue;
      res = (u16)((res * 20000.0) / 4095.0);
      if ((u32)res % 10 < 5)
        res = res / 10;
      else
        res = (res/10)+1;
    }
    return (u16) res;
}

inline u16 getRefToDACValue(u16 ref, u16 Kscale) {
 return (u16)(((float)ref*Kscale/10000*4095)/20);
}

void networking(){
   static int link_cnt=0;
   if (U1_SwCNT()) {
    (LINK_STATUS)? (LINK_ON): (LINK_OFF);//если приняли валидное сообщение по 485 - моргнули светиком
    link_cnt = 0;   
   }
   else
   {
    link_cnt++;
    if (link_cnt > LINK_CNT_MAX) {link_cnt = 0; LINK_OFF;}
   }
}

inline void sendOutsToDAC() {
  DAC_SetChannel1Data( DAC_Align_12b_R, RAM_DATA.OutChanel1);
  DAC_SetChannel2Data( DAC_Align_12b_R, RAM_DATA.OutChanel2);
}

int main(void) {
  Init();
  RUN_OFF;
  LINK_OFF;

  while (1)
  { 
   RUN_ON;

   networking();
   
   RAM_DATA.LocalRef1 = getChanel1Ref((bool)(SET1_NETWORK_REF !=0) , RAM_DATA.NetworkRef1, getAdcValue(ADC_InjectedChannel_1));
   (SET1_NETWORK_REF)? (LED1_OFF) : (LED1_ON);

   RAM_DATA.LocalRef2 = getChanel1Ref((bool)(SET2_NETWORK_REF != 0), RAM_DATA.NetworkRef2, getAdcValue(ADC_InjectedChannel_2));
   (SET2_NETWORK_REF)? (LED2_OFF) : (LED2_ON);

    RAM_DATA.OutChanel1 = getRefToDACValue(RAM_DATA.LocalRef1, FLASH_DATA.KDAC1);
    RAM_DATA.OutChanel2 = getRefToDACValue(RAM_DATA.LocalRef2, FLASH_DATA.KDAC2);

    sendOutsToDAC();

    outRefToDIGITS(RAM_DATA.LocalRef1, RAM_DATA.LocalRef2);
  }
}
