/** @file
    @brief	STM32H747I-Discovery LED ドライバ

    @date	2020.02.16
    @authoer	Takashi SHUDO
*/

#include "device.h"

#include "stm32h747i_discovery.h"

static unsigned char led_stat;

/**
   @brief	LEDドライバの登録

   @param[in]	param	未使用

   @return	!=0:エラー
*/
static int led_register(struct st_device *dev, char *param)
{
	BSP_LED_Init(LED1);
	BSP_LED_Init(LED2);
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);

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
		BSP_LED_On(LED1);
	} else {
		BSP_LED_Off(LED1);
	}

	if(ch & 0x02) {
		BSP_LED_On(LED2);
	} else {
		BSP_LED_Off(LED2);
	}

	if(ch & 0x04) {
		BSP_LED_On(LED3);
	} else {
		BSP_LED_Off(LED3);
	}

	if(ch & 0x08) {
		BSP_LED_On(LED4);
	} else {
		BSP_LED_Off(LED4);
	}

	led_stat = (ch & 0x0f);

	return 1;
}

static int led_suspend(struct st_device *dev)
{
	return 0;
}

static int led_resume(struct st_device *dev)
{
	return 0;
}

const struct st_device led_device = {
	.name		= DEF_DEV_NAME_LED,
	.explan		= "STM32H747I-Discovery LED",
	.register_dev	= led_register,
	.getc		= led_getc,
	.putc		= led_putc,
	.suspend	= led_suspend,
	.resume		= led_resume,
}; //!< LEDドライバ
