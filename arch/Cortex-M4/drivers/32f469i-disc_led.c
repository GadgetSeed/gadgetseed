/** @file
    @brief	32F469IDISCOVERY LEDドライバ

    @date	2018.08.15
    @author	Takashi SHUDO

    LD1(Green)	PG6
    LD2(Orange)	PD4
    LD3(Red)	PD5
    LD4(Blue)	PK3
*/

#include "device.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

/* Legacy GPIO Type Definition */
typedef struct {
	__IO uint32_t MODER;    /*!< GPIO port mode register,               Address offset: 0x00      */
	__IO uint32_t OTYPER;   /*!< GPIO port output type register,        Address offset: 0x04      */
	__IO uint32_t OSPEEDR;  /*!< GPIO port output speed register,       Address offset: 0x08      */
	__IO uint32_t PUPDR;    /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C      */
	__IO uint32_t IDR;      /*!< GPIO port input data register,         Address offset: 0x10      */
	__IO uint32_t ODR;      /*!< GPIO port output data register,        Address offset: 0x14      */
	__IO uint16_t BSRR;     /*!< GPIO port bit set/reset register,      Address offset: 0x18      */
	__IO uint16_t BRR;      /*!< GPIO port bit set/reset register,      Address offset: 0x1A      */
	__IO uint32_t LCKR;     /*!< GPIO port configuration lock register, Address offset: 0x1C      */
	__IO uint32_t AFR[2];   /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
} GPIO_LegacyTypeDef;

/* LED1 */ 
#define GPIO_LED1		(GPIOG)
#define RCC_AHBPeriph_GPIO_LED1	(RCC_AHB1ENR_GPIOGEN)
#define LED_D1			(GPIO_PIN_6)
#define LED1_IO_HI(x)		(((GPIO_LegacyTypeDef *)GPIO_LED1)->BSRR = x)
#define LED1_IO_LO(x)		(((GPIO_LegacyTypeDef *)GPIO_LED1)->BRR = x)
#define LED_D1_OFF()		LED1_IO_HI(LED_D1)		/* Active Low */
#define LED_D1_ON()		LED1_IO_LO(LED_D1)		/* Active Low */
/* LED2 */
#define GPIO_LED2		(GPIOD)
#define RCC_AHBPeriph_GPIO_LED2	(RCC_AHB1ENR_GPIODEN)
#define LED_D2			(GPIO_PIN_4)
#define LED2_IO_HI(x)		(((GPIO_LegacyTypeDef *)GPIO_LED2)->BSRR = x)
#define LED2_IO_LO(x)		(((GPIO_LegacyTypeDef *)GPIO_LED2)->BRR = x)
#define LED_D2_OFF()		LED2_IO_HI(LED_D2)		/* Active Low */
#define LED_D2_ON()		LED2_IO_LO(LED_D2)		/* Active Low */
/* LED3 */
#define GPIO_LED3		(GPIOD)
#define RCC_AHBPeriph_GPIO_LED3	(RCC_AHB1ENR_GPIODEN)
#define LED_D3			(GPIO_PIN_5)
#define LED3_IO_HI(x)		(((GPIO_LegacyTypeDef *)GPIO_LED3)->BSRR = x)
#define LED3_IO_LO(x)		(((GPIO_LegacyTypeDef *)GPIO_LED3)->BRR = x)
#define LED_D3_OFF()		LED3_IO_HI(LED_D3)		/* Active Low */
#define LED_D3_ON()		LED3_IO_LO(LED_D3)		/* Active Low */
/* LED4 */
#define GPIO_LED4		(GPIOK)
#define RCC_AHBPeriph_GPIO_LED4	(RCC_AHB1ENR_GPIOKEN)
#define LED_D4			(GPIO_PIN_3)
#define LED4_IO_HI(x)		(((GPIO_LegacyTypeDef *)GPIO_LED4)->BSRR = x)
#define LED4_IO_LO(x)		(((GPIO_LegacyTypeDef *)GPIO_LED4)->BRR = x)
#define LED_D4_OFF()		LED4_IO_HI(LED_D4)		/* Active Low */
#define LED_D4_ON()		LED4_IO_LO(LED_D4)		/* Active Low */

static unsigned char led_stat;

static void LED_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO_LED clock */
	RCC->AHB1ENR |= (RCC_AHBPeriph_GPIO_LED1 | RCC_AHBPeriph_GPIO_LED2 | RCC_AHBPeriph_GPIO_LED4);

	/* Configure GPIO for LEDs as Output push-pull */
	GPIO_InitStructure.Pin 			= LED_D1;
	GPIO_InitStructure.Mode 		= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull 		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	GPIO_InitStructure.Alternate 	= 0;
	HAL_GPIO_Init(GPIO_LED1, &GPIO_InitStructure);

	GPIO_InitStructure.Pin 			= LED_D2 | LED_D3;
	HAL_GPIO_Init(GPIO_LED2, &GPIO_InitStructure);

	GPIO_InitStructure.Pin 			= LED_D4;
	HAL_GPIO_Init(GPIO_LED4, &GPIO_InitStructure);

	LED_D1_OFF();
	LED_D2_OFF();
	LED_D3_OFF();
	LED_D4_OFF();
}

/**
   @brief	LEDドライバの登録

   @param[in]	param	未使用

   @return	!=0:エラー
*/
static int led_register(struct st_device *dev, char *param)
{
	LED_Configuration();

	led_stat = 0;

	return 0;
}

/**
   @brief	LEDの状態を取得する

   @param[out]	rd	LEDデータポインタ

   @return	!=0:データあり 0:データ無し
*/
static int led_getc(struct st_device *dev, unsigned char *rd)
{
	*rd = led_stat;

	return 1;
}

/**
   @brief	LEDの状態を設定する

   @param[out]	ch	LEDデータポインタ

   @return	1:正常終了,!=0:エラー
*/
static int led_putc(struct st_device *dev, unsigned char ch)
{
	if(ch & 0x01) {
		LED_D1_ON();
	} else {
		LED_D1_OFF();
	}

	if(ch & 0x02) {
		LED_D2_ON();
	} else {
		LED_D2_OFF();
	}

	if(ch & 0x04) {
		LED_D3_ON();
	} else {
		LED_D3_OFF();
	}

	if(ch & 0x08) {
		LED_D4_ON();
	} else {
		LED_D4_OFF();
	}

	led_stat = (ch & 0x0f);

	return 1;
}

const struct st_device led_device = {
	.name		= DEF_DEV_NAME_LED,
	.explan		= "32F469I-Discovery LED",
	.register_dev	= led_register,
	.getc		= led_getc,
	.putc		= led_putc,
}; //!< LEDドライバ
