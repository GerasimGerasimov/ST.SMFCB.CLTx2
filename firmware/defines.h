//светодиод работы
#define RUN_ON   GPIOB->BRR = GPIO_Pin_9
#define RUN_OFF   GPIOB->BSRR = GPIO_Pin_9

//светодиод связи
#define LINK_ON   GPIOB->BRR = GPIO_Pin_8
#define LINK_STATUS   (GPIOB->ODR&GPIO_Pin_8)
#define LINK_OFF   GPIOB->BSRR = GPIO_Pin_8
#define LINK_CNT_MAX 50000 

//светодиод 1 и 2 потенциометра
#define LED1_ON   GPIOA->BSRR = GPIO_Pin_3
#define LED1_OFF   GPIOA->BRR = GPIO_Pin_3
#define LED2_ON   GPIOC->BSRR = GPIO_Pin_15
#define LED2_OFF   GPIOC->BRR = GPIO_Pin_15

//нога LCLK SPI
#define DO_LCLK_UP   GPIOB->BSRR = GPIO_Pin_12
#define DO_LCLK_DWN  GPIOB->BRR = GPIO_Pin_12

//состояния дип переключателя
#define SET1_NETWORK_REF   (GPIOA->IDR & GPIO_Pin_15)
#define SET2_NETWORK_REF   (GPIOB->IDR & GPIO_Pin_4)
#define SET3_STATUS   (GPIOB->IDR & GPIO_Pin_6)
#define SET4_STATUS   (GPIOB->IDR & GPIO_Pin_7)