/** @file
    @brief	STM32 GPIO LCD (Arduino LCD Shield)

	Registor width 16 : ILI9325 (aitendo UL024TF)
	Registor width  8 : ILI9341 (Kuman K60)

   @date	2017.10.15
   @author	Takashi SHUDO

   PA0	LCD_RD
   PA1	LCD_WR
   PA4	LCD_RS	YP(Analog IN) ADC1 IN4
   PB0	LCD_CS	XM(Analog IN) ADC1 IN8
   PC1	LCD_RESET

   PA9	LCD_D0	YM(Digital)
   PC7	LCD_D1	XP(Digital)
   PA10	LCD_D2
   PB3	LCD_D3
   PB5	LCD_D4
   PB4	LCD_D5
   PB10	LCD_D6
   PA8	LCD_D7
*/

//#define GSC_DEV_ENABLE_TOUCHSENSOR

#include "sysconfig.h"
#include "device.h"
#include "tprintf.h"
#include "tkprintf.h"
#include "system.h"
#include "device/vio_ioctl.h"
#include "task/syscall.h"
#include "task/mutex.h"
#include "sysevent.h"
#include "timer.h"

#include "stm32f4xx_hal.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"


static unsigned char fore_color_h = 0;
static unsigned char fore_color_l = 0;
static unsigned char back_color_h = 0;
static unsigned char back_color_l = 0;

static unsigned long f_bsrr_a_h;
static unsigned long f_bsrr_b_h;
static unsigned long f_bsrr_c_h;
static unsigned long f_bsrr_a_l;
static unsigned long f_bsrr_b_l;
static unsigned long f_bsrr_c_l;

static unsigned long b_bsrr_a_h;
static unsigned long b_bsrr_b_h;
static unsigned long b_bsrr_c_h;
static unsigned long b_bsrr_a_l;
static unsigned long b_bsrr_b_l;
static unsigned long b_bsrr_c_l;

#ifdef GSC_DEV_ENABLE_TOUCHSENSOR	///< $gsc タッチセンサデバイスを有効にする
static struct st_mutex gpio_mutex;
#define lock()		mutex_lock(&gpio_mutex, 0/*1000*/)
#define unlock()	mutex_unlock(&gpio_mutex)
#else
#define lock()
#define unlock()
#endif

#define DATA_TO_GPIOA_BSRR(data)	(\
			((((unsigned long)(data & (1<<0))?1:0)<<9) |\
			 (((unsigned long)(data & (1<<2))?1:0)<<10) |\
			 (((unsigned long)(data & (1<<7))?1:0)<<8))\
			|\
			(((((unsigned long)(data & (1<<0))?0:1)<<9) |\
			  (((unsigned long)(data & (1<<2))?0:1)<<10) |\
			  (((unsigned long)(data & (1<<7))?0:1)<<8)) << 16)\
		       )

#define DATA_TO_GPIOB_BSRR(data)	(\
			((((unsigned long)(data & (1<<3))?1:0)<<3) |\
			 (((unsigned long)(data & (1<<4))?1:0)<<5) |\
			 (((unsigned long)(data & (1<<5))?1:0)<<4) |\
			 (((unsigned long)(data & (1<<6))?1:0)<<10))\
			|\
			(((((unsigned long)(data & (1<<3))?0:1)<<3) |\
			  (((unsigned long)(data & (1<<4))?0:1)<<5) |\
			  (((unsigned long)(data & (1<<5))?0:1)<<4) |\
			  (((unsigned long)(data & (1<<6))?0:1)<<10)) << 16)\
		       )

#define DATA_TO_GPIOC_BSRR(data)	(\
			(((unsigned long)(data & (1<<1))?1:0)<<7)\
			|\
			((((unsigned long)(data & (1<<1))?0:1)<<7) << 16)\
		       )

static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__GPIOC_CLK_ENABLE();
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();

	/*Configure GPIO pins : PC1 PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PA0 PA1 PA4 PA8 
	  PA9 PA10 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_8 
			|GPIO_PIN_9|GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB10 PB3 PB4 
	  PB5 PB6 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4 
			|GPIO_PIN_5|GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
}

//#define HAL_USE

static inline void rd_active(void)
{
#ifdef HAL_USE
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
#else
	GPIOA->BSRR = (((unsigned long)GPIO_PIN_0) << 16);
	nop();
#endif
}

static inline void rd_idle(void)
{
#ifdef HAL_USE
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
#else
	nop();
	GPIOA->BSRR = (unsigned long)GPIO_PIN_0;
#endif
}

static inline void wr_active(void)
{
#ifdef HAL_USE
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
#else
	GPIOA->BSRR = (((unsigned long)GPIO_PIN_1) << 16);
	nop();
#endif
}

static inline void wr_idle(void)
{
#ifdef HAL_USE
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
#else
	nop();
	GPIOA->BSRR = (unsigned long)GPIO_PIN_1;
#endif
}

static inline void cd_command(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
}

static inline void cd_data(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

static inline void set_reset(unsigned long arg)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, arg);
}

static inline void set_cs(unsigned long arg)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, arg);
}

static void dir_out_data(void)
{
	// D0-D7を出力へ

	GPIO_InitTypeDef GPIO_InitStruct;

	/*Configure GPIO pins : PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PA8 PA9 PA10 */
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB10 PB3 PB4 PB5 */
	GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4
			|GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void dir_in_data(void)
{
	// D0-D7を入力へ

	GPIO_InitTypeDef GPIO_InitStruct;

	/*Configure GPIO pins : PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PA8 PA9 PA10 */
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB10 PB3 PB4 PB5 */
	GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4 
			|GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static unsigned char read_8bits(void)
{
#if 0
	return (((unsigned char)(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9))  << 0) |
		((unsigned char)(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7))  << 1) |
		((unsigned char)(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10)) << 2) |
		((unsigned char)(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3))  << 3) |
		((unsigned char)(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5))  << 4) |
		((unsigned char)(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4))  << 5) |
		((unsigned char)(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)) << 6) |
		((unsigned char)(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8))  << 7));
#else
	return
			(((GPIOA->IDR & 0x0100) >> 8)  << 7) |
			(((GPIOB->IDR & 0x0400) >> 10) << 6) |
			(((GPIOB->IDR & 0x0010) >> 4)  << 5) |
			(((GPIOB->IDR & 0x0020) >> 5)  << 4) |
			(((GPIOB->IDR & 0x0008) >> 3)  << 3) |
			(((GPIOA->IDR & 0x0400) >> 10) << 2) |
			(((GPIOC->IDR & 0x0080) >> 7)  << 1) |
			(((GPIOA->IDR & 0x0200) >> 9)  << 0);
#endif
}

static void write_8bits(unsigned char data)
{
#ifdef HAL_USE
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9,  data & (1<<0));
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7,  data & (1<<1));
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, data & (1<<2));
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3,  data & (1<<3));
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,  data & (1<<4));
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4,  data & (1<<5));
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, data & (1<<6));
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8,  data & (1<<7));
#else
#if 0
	/*
	 * こうするなら他のGPIOドライバと競合しないようにMUTEXが必要
	 */
	GPIOA->ODR = ((GPIOA->IDR & (~((1<<9) | (1<<10) | (1<<8)))) | (
				      (((unsigned short)(data & (1<<0))?1:0)<<9) |
				      (((unsigned short)(data & (1<<2))?1:0)<<10) |
				      (((unsigned short)(data & (1<<7))?1:0)<<8)));
	GPIOB->ODR = ((GPIOB->IDR & (~((1<<3) | (1<<5) | (1<<4) | (1<<10)))) | (
				      (((unsigned short)(data & (1<<3))?1:0)<<3) |
				      (((unsigned short)(data & (1<<4))?1:0)<<5) |
				      (((unsigned short)(data & (1<<5))?1:0)<<4) |
				      (((unsigned short)(data & (1<<6))?1:0)<<10)));
	GPIOC->ODR = ((GPIOC->IDR & (~((1<<7)))) | (
				      (((unsigned short)(data & (1<<1))?1:0)<<7)));
#else
	GPIOA->BSRR = DATA_TO_GPIOA_BSRR(data);	       
	GPIOB->BSRR = DATA_TO_GPIOB_BSRR(data);
	GPIOC->BSRR = DATA_TO_GPIOC_BSRR(data);
#endif
#endif
}

/*
 *
 */

static int gpio_read(struct st_device *dev, void *data, unsigned int size)
{
	int i;
	unsigned char *dp = data;

	lock();
	cd_data();
	dir_in_data();

	for(i=0; i<size/2; i++) {
		unsigned char oh, ol;
#define LCD_PIXEL_3BYTES
#ifdef LCD_PIXEL_3BYTES
		unsigned char ih, im, il;
		rd_active();
		ih  = read_8bits();
		rd_idle();
		DKPRINTF(0x02, " %02X", ih);

		rd_active();
		im = read_8bits();
		rd_idle();
		DKPRINTF(0x02, " %02X", im);

		rd_active();
		il = read_8bits();
		rd_idle();
		DKPRINTF(0x02, " %02X\n", il);

		oh = (ih & 0xf8) | ((im & 0xe0) >> 5);
		ol = ((im  & 0x1c) << 3) | ((il & 0xf8) >> 3);
#else
		rd_active();
		oh  = read_8bits();
		rd_idle();
		DKPRINTF(0x02, " %02X", ih);

		rd_active();
		ol = read_8bits();
		rd_idle();
		DKPRINTF(0x02, " %02X", im);
#endif

		*(dp + 0) = ol;
		*(dp + 1) = oh;

		dp += 2;
	}

	dir_out_data();
	unlock();

	KXDUMP(0x01, data, size);

	return size;
}

static int gpio_write(struct st_device *dev, const void *data, unsigned int size)
{
	int i;
	const unsigned char *dp = data;

	lock();
	cd_data();

	for(i=0; i<size/2; i++) {
		// Little Endian
		write_8bits(*(dp + 1));
		wr_active();
		wr_idle();
		write_8bits(*dp);
		wr_active();
		wr_idle();
		dp += 2;
	}

	unlock();

	return size;
}

static inline void write_fore_word(void)
{
	GPIOA->BSRR = f_bsrr_a_h;
	GPIOB->BSRR = f_bsrr_b_h;
	GPIOC->BSRR = f_bsrr_c_h;
	wr_active();
	wr_idle();

	GPIOA->BSRR = f_bsrr_a_l;
	GPIOB->BSRR = f_bsrr_b_l;
	GPIOC->BSRR = f_bsrr_c_l;
	wr_active();
	wr_idle();
}

static inline void write_back_word(void)
{
	GPIOA->BSRR = b_bsrr_a_h;
	GPIOB->BSRR = b_bsrr_b_h;
	GPIOC->BSRR = b_bsrr_c_h;
	wr_active();
	wr_idle();

	GPIOA->BSRR = b_bsrr_a_l;
	GPIOB->BSRR = b_bsrr_b_l;
	GPIOC->BSRR = b_bsrr_c_l;
	wr_active();
	wr_idle();
}

static int gpio_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	int rtn = 0;

	DKFPRINTF(0x02, "com = %08lX, arg =  %08lX\n", com, arg);

	switch(IOCTL(com)) {
	case IOCMD_VIO_LOCK_BUS:
		lock();
		break;

	case IOCMD_VIO_UNLOCK_BUS:
		unlock();
		break;

	case IOCMD_VIO_SET_RESET:	// PC1(LCD_RESET)
		lock();
		set_reset(arg);
		unlock();
		break;

	case IOCMD_VIO_SET_CS:	// CS ASSERT/NEGATE
		lock();
		set_cs(arg);
		unlock();
		break;

	case IOCMD_VIO_WRITE_COMMAND:
		{
			lock();
			cd_command();

#ifdef LCD_ADDR_16
			write_8bits((unsigned char)((arg >> 8) & 0xff));
			wr_active();
			wr_idle();
#endif

			write_8bits((unsigned char)((arg >> 0) & 0xff));
			wr_active();
			wr_idle();
			unlock();
		}
		break;

	case IOCMD_VIO_WRITE_REG8:
		{
#ifdef LCD_ADDR_16
			unsigned char ad_h = ((arg >> 24) & 0xff);
#endif
			unsigned char ad_l = ((arg >> 16) & 0xff);
			unsigned char dt_l = ((arg >>  0) & 0xff);

			lock();
			cd_command();
#ifdef LCD_ADDR_16
			write_8bits(ad_h);
			wr_active();
			wr_idle();
#endif
			write_8bits(ad_l);
			wr_active();
			wr_idle();

			cd_data();
			write_8bits(dt_l);
			wr_active();
			wr_idle();
			unlock();
		}
		break;

	case IOCMD_VIO_WRITE_REG16:
		{
#ifdef LCD_ADDR_16
			unsigned char ad_h = ((arg >> 24) & 0xff);
#endif
			unsigned char ad_l = ((arg >> 16) & 0xff);
			unsigned char dt_h = ((arg >>  8) & 0xff);
			unsigned char dt_l = ((arg >>  0) & 0xff);

			lock();
			cd_command();
#ifdef LCD_ADDR_16
			write_8bits(ad_h);
			wr_active();
			wr_idle();
#endif
			write_8bits(ad_l);
			wr_active();
			wr_idle();

			cd_data();
			write_8bits(dt_h);
			wr_active();
			wr_idle();

			write_8bits(dt_l);
			wr_active();
			wr_idle();
			unlock();
		}
		break;

	case IOCMD_VIO_READ_REG32:
		{
#ifdef LCD_ADDR_16
			unsigned char ad_h = ((arg >> 8) & 0xff);
#endif
			unsigned char ad_l = ((arg >> 0) & 0xff);
			unsigned long dt = 0;

			lock();
			cd_command();
#ifdef LCD_ADDR_16
			write_8bits(ad_h);
			wr_active();
			wr_idle();
#endif
			write_8bits(ad_l);
			wr_active();
			wr_idle();

			cd_data();
			dir_in_data();

			rd_active();
			dt |= read_8bits();
			rd_idle();

			dt <<= 8;
			rd_active();
			dt |= read_8bits();
			rd_idle();

			dt <<= 8;
			rd_active();
			dt |= read_8bits();
			rd_idle();

			dt <<= 8;
			rd_active();
			dt |= read_8bits();
			rd_idle();

			dir_out_data();
			unlock();

			rtn = dt;
		}
		break;

	case IOCMD_VIO_WRITE_DATA8:
		{
			unsigned char dt = (arg & 0xff);

			lock();
			cd_data();
			write_8bits(dt);
			wr_active();
			wr_idle();
			unlock();
		}
		break;

	case IOCMD_VIO_WRITE_DATA16:
		{
			unsigned char dt_h = ((arg >> 8) & 0xff);
			unsigned char dt_l = ((arg >> 0) & 0xff);

			lock();
			cd_data();
			write_8bits(dt_h);
			wr_active();
			wr_idle();
			write_8bits(dt_l);
			wr_active();
			wr_idle();
			unlock();
		}
		break;

	case IOCMD_VIO_READ_DATA8:
		{
			long dt;

			lock();
			cd_data();
			dir_in_data();

			rd_active();
			dt = read_8bits();
			rd_idle();

			dir_out_data();
			unlock();

			rtn = dt;
		}
		break;

	case IOCMD_VIO_NOLOCK_WRITE_DATA16:
		{
			unsigned char dt_h = ((arg >> 8) & 0xff);
			unsigned char dt_l = ((arg >> 0) & 0xff);

			cd_data();
			write_8bits(dt_h);
			wr_active();
			wr_idle();
			write_8bits(dt_l);
			wr_active();
			wr_idle();
		}
		break;

	case IOCMD_VIO_WRITE_DATA32:
		{
			unsigned char dt_hh = ((arg >> 24) & 0xff);
			unsigned char dt_hl = ((arg >> 16) & 0xff);
			unsigned char dt_lh = ((arg >> 8) & 0xff);
			unsigned char dt_ll = ((arg >> 0) & 0xff);

			lock();
			cd_data();
			write_8bits(dt_hh);
			wr_active();
			wr_idle();
			write_8bits(dt_hl);
			wr_active();
			wr_idle();
			write_8bits(dt_lh);
			wr_active();
			wr_idle();
			write_8bits(dt_ll);
			wr_active();
			wr_idle();
			unlock();
		}
		break;

	case IOCMD_VIO_SET_WRITEDATA0:
		{
			fore_color_h = (arg >> 8) & 0xff;
			fore_color_l = (arg >> 0) & 0xff;

			f_bsrr_a_h = DATA_TO_GPIOA_BSRR(fore_color_h);
			f_bsrr_b_h = DATA_TO_GPIOB_BSRR(fore_color_h);
			f_bsrr_c_h = DATA_TO_GPIOC_BSRR(fore_color_h);
			f_bsrr_a_l = DATA_TO_GPIOA_BSRR(fore_color_l);
			f_bsrr_b_l = DATA_TO_GPIOB_BSRR(fore_color_l);
			f_bsrr_c_l = DATA_TO_GPIOC_BSRR(fore_color_l);
		}
		break;

	case IOCMD_VIO_SET_WRITEDATA1:
		{
			back_color_h = (arg >> 8) & 0xff;
			back_color_l = (arg >> 0) & 0xff;

			b_bsrr_a_h = DATA_TO_GPIOA_BSRR(back_color_h);
			b_bsrr_b_h = DATA_TO_GPIOB_BSRR(back_color_h);
			b_bsrr_c_h = DATA_TO_GPIOC_BSRR(back_color_h);
			b_bsrr_a_l = DATA_TO_GPIOA_BSRR(back_color_l);
			b_bsrr_b_l = DATA_TO_GPIOB_BSRR(back_color_l);
			b_bsrr_c_l = DATA_TO_GPIOC_BSRR(back_color_l);
		}
		break;

	case IOCMD_VIO_REPEAT_DATA:
		{
			long i;

			lock();
			cd_data();

			if(fore_color_h == fore_color_l) {
				GPIOA->BSRR = f_bsrr_a_h;
				GPIOB->BSRR = f_bsrr_b_h;
				GPIOC->BSRR = f_bsrr_c_h;
				for(i=0; i<arg; i++) {
					wr_active();
					wr_idle();
					wr_active();
					wr_idle();
				}
			} else {
				for(i=0; i<arg; i++) {
					write_fore_word();
				}
			}
			unlock();
		}
		break;

	case IOCMD_VIO_REPEAT_BITS:
		{
			int sbit = ((com >> 12) & 0x7);
			int count = (com & 0x0fff);
			unsigned char *data = (unsigned char *)param;
			unsigned char bit = (0x80 >> sbit);
			int i;

			lock();
			cd_data();

			DKPRINTF(0x02, "sbit=%d count=%d, data=%p\n", sbit, count, data);
			for(i=0; i<count; i++) {
				if(*data & bit) {
					write_fore_word();
				} else {
					write_back_word();
				}
				if(bit == 0x01) {
					bit = 0x80;
					data ++;
				} else {
					bit >>= 1;
				}
			}
			unlock();
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		rtn =  -1;
		break;
	}

	return rtn;
}

#ifdef GSC_DEV_ENABLE_TOUCHSENSOR
ADC_HandleTypeDef hadc1;

/* ADC1 init function */
static void MX_ADC1_Init(void)
{
	hadc1.Instance = ADC1;
	// hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
#if 0
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		SYSERR_PRINT("ADC1 Initialize error");
	}

	/**Configure for the selected ADC regular channel its
	 * corresponding rank in the sequencer and its sample time.
	 */
	// sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Channel = ADC_CHANNEL_8;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		SYSERR_PRINT("ADC1 Config error");
	}
#endif
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(hadc->Instance==ADC1) {
		__HAL_RCC_ADC1_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_4;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_0;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}

}

/*
  LCD     Arduino STM
  YP(AIN) A2      PA4
  XM(AIN) A3      PB0
  YM      8       PA9
  XP      9       PC7
 */
unsigned long read_pos_x(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_ChannelConfTypeDef sConfig;
	unsigned long val;

	//pinMode(_yp, INPUT);
	// PA4をアナログ入力
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	//GPIO_InitStruct.Pull = GPIO_PULLUP;
	//GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//pinMode(_ym, INPUT);
	// PA9をハイインピーダンスに
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	//GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	//GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//digitalWrite(_yp, LOW);
	//digitalWrite(_ym, LOW);

	//pinMode(_xp, OUTPUT);
	//digitalWrite(_xp, HIGH);
	// PC7をHigh
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	//GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);

	//pinMode(_xm, OUTPUT);
	//digitalWrite(_xm, LOW);
	// PB0をLow
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		SYSERR_PRINT("ADC1 Config error");
	}

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	val = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	//tkprintf("PA4 = %-6ld    ", val);

	//return (1023-analogRead(_yp));
	return val;
}

unsigned int read_pos_y(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_ChannelConfTypeDef sConfig;
	unsigned int val;

	//pinMode(_xp, INPUT);
	// PC7をハイインピーダンスに
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	//GPIO_InitStruct.Pull = GPIO_NOPULL;
	//GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	//pinMode(_xm, INPUT);
	// PB0をアナログ入力
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	//GPIO_InitStruct.Pull = GPIO_PULLUP;
	//GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//digitalWrite(_xp, LOW);
	//digitalWrite(_xm, LOW);

	//pinMode(_yp, OUTPUT);
	//digitalWrite(_yp, HIGH);
	// PA4をHigh
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	//pinMode(_ym, OUTPUT);
	//digitalWrite(_ym, LOW);
	// PA9をLow
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);

	sConfig.Channel = ADC_CHANNEL_8;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		SYSERR_PRINT("ADC1 Config error");
	}

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	val = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	//tkprintf("PB0 = %-6ld\n", val);

	return val;
}

#ifdef GSC_DEV_ENABLE_TOUCHSENSOR
#define SIZEOFSTACK	(1024*1)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFSTACK/sizeof(unsigned int)];
#endif

static void ts_timer_func(void *sp, unsigned long long systime)
{
	task_wakeup_id_ISR(sp, tcb.id);
}

static int ts_mxl = 16;
static int ts_xl = 576;
static int ts_mxr = (GSC_GRAPHICS_DISPLAY_WIDTH-16);
static int ts_xr = 3667;

static int ts_myt = 16;
static int ts_yt = 659;
static int ts_myb = (GSC_GRAPHICS_DISPLAY_HEIGHT-16);
static int ts_yb = 3446;

static volatile int flg_touch = 0;
static int lpos_x;
static int lpos_y;
static int ts_low_val[2];

static int ts_task(char *arg)
{
	struct st_sysevent ev;

	while(1) {
		int vx, vy;
		int px, py;

		lock();
		//!!! MUTEXに問題ありそう（２重Lock/Unlockを検出する場合有り）

		vx = read_pos_x();
		vy = read_pos_y();
		ts_low_val[0] = vx;
		ts_low_val[1] = vy;
		ev.private_data = (void *)ts_low_val;

		MX_GPIO_Init();

		unlock();

		if((vx < 4000) && (vy > 420)) {
			DKPRINTF(0x02, "VX= %4ld, VY= %4ld\n", vx, vy);

			// 座標の計算
			px = (((vx - ts_xl + ts_mxl) * (ts_mxr - ts_mxl)) / (ts_xr - ts_xl)) + ts_mxl;
			py = (((vy - ts_yt + ts_myt) * (ts_myb - ts_myt)) / (ts_yb - ts_yt)) + ts_myt;
			DKPRINTF(0x01, "X = %4ld, Y = %4ld\n", px, py);

			if(flg_touch == 0) {
				DKPRINTF(0x01, "TS START\n");
				ev.what = EVT_TOUCHSTART;
				ev.pos_x = px;
				ev.pos_y = py;
				lpos_x = px;
				lpos_y = py;
				flg_touch = 1;
				wait_utime(1000);	// FIXME これがないとSTARTが取れない？
				set_event(&ev);
			} else if((px != lpos_x) || (py != lpos_y)) {
				ev.what = EVT_TOUCHMOVE;
				ev.pos_x = px;
				ev.pos_y = py;
				lpos_x = px;
				lpos_y = py;
				set_event(&ev);
			} else {
				DKPRINTF(0x01, "TS STAY\n");
			}
		} else {
			if(flg_touch != 0) {
				DKPRINTF(0x02, "TS END\n");
				ev.what = EVT_TOUCHEND;
				ev.pos_x = lpos_x;
				ev.pos_y = lpos_y;
				flg_touch = 0;
				set_event(&ev);
			}
		}

		task_pause();
		//task_sleep(20);
	}

	return 0;
}
#endif // GSC_DEV_ENABLE_TOUCHSENSOR

static int gpio_register(struct st_device *dev, char *param)
{
#ifdef GSC_DEV_ENABLE_TOUCHSENSOR
	MX_ADC1_Init();
	__HAL_RCC_ADC1_CLK_ENABLE();
#endif
	MX_GPIO_Init();

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);	// LCD_RD Hi
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);	// LCD_WR Hi
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);	// LCD_RS Hi
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);	// LCD_CS Hi
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);	// LCD_RESET Lo

#ifdef GSC_DEV_ENABLE_TOUCHSENSOR
	mutex_register_ISR(&gpio_mutex, DEF_DEV_NAME_VIDEOIO);

	task_add(ts_task, "touch_sensor", TASK_PRIORITY_DEVICE_DRIVER, &tcb,
		 stack, SIZEOFSTACK, 0);

	register_timer_func(ts_timer_func, 20);
#endif

	return 0;
}

const struct st_device lcd_gpio_device = {
	.name		= DEF_DEV_NAME_VIDEOIO,
	.explan		= "STM32F4 LCD GPIO",
	.register_dev	= gpio_register,
	.read		= gpio_read,
	.write		= gpio_write,
	.ioctl		= gpio_ioctl,
};


#ifdef GSC_DEV_ENABLE_TOUCHSENSOR

#include "device/ts_ioctl.h"

static int ts_analog_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	int rtn = 0;

	DKFPRINTF(0x02, "com = %08lX, arg =  %08lX\n", com, arg);

	switch(IOCTL(com)) {
	case IOCMD_TS_SET_CALIB:
		{
			struct st_ts_calib_data *cd;
			cd = (struct st_ts_calib_data *)param;

			DKPRINTF(0x01, "LEFT   POS=%4d, VAL=%5d\n", cd->left_pos,   cd->left_val);
			DKPRINTF(0x01, "RIGHT  POS=%4d, VAL=%5d\n", cd->right_pos,  cd->right_val);
			DKPRINTF(0x01, "TOP    POS=%4d, VAL=%5d\n", cd->top_pos,    cd->top_val);
			DKPRINTF(0x01, "BOTTOM POS=%4d, VAL=%5d\n", cd->bottom_pos, cd->bottom_val);

			ts_mxl = cd->left_pos;
			ts_xl = cd->left_val;
			ts_mxr = cd->right_pos;
			ts_xr = cd->right_val;

			ts_myt = cd->top_pos;
			ts_yt = cd->top_val;
			ts_myb = cd->bottom_pos;
			ts_yb = cd->bottom_val;
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		rtn =  -1;
		break;
	}

	return rtn;
}

const struct st_device ts_analog_device = {
	.name		= DEF_DEV_NAME_TS,
	.explan		= "STM32F4 LCD Touch Sensor",
	.ioctl		= ts_analog_ioctl,
};

#endif // GSC_DEV_ENABLE_TOUCHSENSOR
