/** @file
    @brief	Cortex-M SysTickドライバ

    @date	2013.03.10
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "asm.h"
#include "device.h"
#include "device/timer_ioctl.h"
#include "interrupt.h"
#include "timer.h"
#include "tkprintf.h"
#include "task/task.h"

#include "stm32f4xx_hal.h"

static unsigned long long timer_count = 0;

static void (* inth_func)(void *sp);

/*
  SysTick割り込みハンドラ
*/
void inthdr_systick(unsigned int intnum, void *sp)
{
	int i;

	timer_count += GSC_KERNEL_TIMER_INTERVAL_MSEC;

	for(i=0; i<GSC_KERNEL_TIMER_INTERVAL_MSEC; i++) {
		HAL_IncTick();	// HALドライバ用
	}

	if(inth_func) {
		inth_func(sp);
	}
}

/*
  SysTick初期化
  GSC_KERNEL_TIMER_INTERVAL_MSEC(ms)間隔でタイマ割り込み発生
*/
static void start_systick(void)
{
	long clock = HAL_RCC_GetHCLKFreq();

	HAL_SYSTICK_Config(clock/1000 * GSC_KERNEL_TIMER_INTERVAL_MSEC);
	tkprintf("System Clock : %ld MHz\n", clock/1000000);
	/*
	  タイマ割り込みプライオリティが最高になっているので最低に戻し
	  ている。多重割り込みは対応していないので他の割り込みと同じレ
	  ベルにしなければならない
	*/
	NVIC_SetPriority(SysTick_IRQn, 0);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

/*
  SysTick停止
*/
static void stop_systick(void)
{
	SysTick->CTRL &= SysTick_CTRL_ENABLE_Msk;
}

static int systick_register(struct st_device *dev, char *param)
{
	inth_func = 0;

	register_interrupt(IRQ2VECT(SysTick_IRQn), inthdr_systick);

	return 0;
}

static int systick_unregister(struct st_device *dev)
{
	unregister_interrupt(IRQ2VECT(SysTick_IRQn));

	return 0;
}

static int systick_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_TIMER_GETTIME:
		{
			// GSC_KERNEL_TIMER_INTERVAL_MSEC の分解能以下のms単位の時間を返す
			int rtn = (SysTick->LOAD - SysTick->VAL)/(HAL_RCC_GetHCLKFreq()/1000);
			return rtn;
		}
		break;

	case IOCMD_TIMER_GETSYSTIME:
		{
			unsigned long long utime = (timer_count * 1000)
					+ (SysTick->LOAD - SysTick->VAL)/(HAL_RCC_GetHCLKFreq()/1000000);
			*((unsigned long long *)param) = utime;
			return 0;
		}
		break;

	case IOCMD_TIMER_START:
		start_systick();
		return 0;
		break;

	case IOCMD_TIMER_STOP:
		stop_systick();
		return 0;
		break;

	case IOCMD_TIMER_SETFUNC:
		inth_func = (void (*)(void *))param;
		return 0;
		break;

	default:
		SYSERR_PRINT("Unknown ioctl(%08X)\n", com);
		return -1;
	}

	return -1;
}

static int systick_suspend(struct st_device *dev)
{
	stop_systick();	// SysTick停止

	return 0;
}

static int systick_resume(struct st_device *dev)
{
	start_systick();	// SysTickスタート

	return 0;
}

const struct st_device cortexm_systick_device = {
	.name		= DEF_DEV_NAME_TIMER,
	.explan		= "Cortex-M SysTick Driver",
	.register_dev	= systick_register,
	.unregister_dev = systick_unregister,
	.ioctl		= systick_ioctl,
	.suspend	= systick_suspend,
	.resume		= systick_resume,
}; //!< SysTickデバイスドライバ
