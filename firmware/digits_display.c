#include "digits_display.h"
#include "defines.h"

const u16 SYMBOLH[21] = {0xC000, 0xF900, 0xA400, 0xB000, 0x9900, 0x9200, 0x8200, 0xF800, 0x8000, 0x9000,//цифры без точки
                         0x4000, 0x7900, 0x2400, 0x3000, 0x1900, 0x1200, 0x0200, 0x7800, 0x0000, 0x1000,//цифры с точкой
                         0xFF00};//погашенный символ
const u16 DIGIT[8] =   {0x0010, 0x0020, 0x0040, 0x0080, 0x0001, 0x0002, 0x0004, 0x0008};
u8 NUMBER[8];
u16 SPI_DATA[8];

void outRefToDIGITS(u16 ref1, u16 ref2) {
  int i=3;
  int j=0;
  int digit = 0;
  for (i=7; i>=0; i--){
    if (i>3){
      NUMBER[i] = (u8)(ref2 % 10);
      if (i == 5) NUMBER[i] +=10;//знак с точкой
      ref2 /= 10;
    } else {
      NUMBER[i] = (u8)(ref1 % 10);
      if (i == 1) NUMBER[i] += 10;
      ref1 /= 10;
    }
  }
  
  i=3;
  for (j=0;j<8;j++){
    digit = NUMBER[j];
    SPI_DATA[j] = SYMBOLH[digit] + DIGIT[j];
  }
}

void outDigitsToSPI() {
  static int i=0;
  static int latch=0;
  if(latch) {
    latch=0;
    DO_LCLK_DWN;
    SPI_I2S_SendData (SPI1, SPI_DATA[i]);
    i++;
    if (i==8)  i=0;
  }
  else {
    DO_LCLK_UP;
    latch=1;
  }
}