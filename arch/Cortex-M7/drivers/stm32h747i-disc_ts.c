/** @file
    @brief	STM32H747I Discovery Touch Sensor

    @date	2019.01.19
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "device.h"
#include "interrupt.h"
#include "device/video_ioctl.h"
#include "device/ts_ioctl.h"
#include "timer.h"
#include "tkprintf.h"
#include "sysevent.h"
#include "task/syscall.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


#include "stm32h747i_discovery_ts.h"
#include "stm32h747i_discovery_bus.h"

#define USE_INTERRUPT

#define LCD_WIDTH	GSC_GRAPHICS_DISPLAY_WIDTH	///< LCD幅ドット数
#define LCD_HEIGHT	GSC_GRAPHICS_DISPLAY_HEIGHT	///< LCD高さドット数

#define MOVEEVENTINTERVAL	20	///< EVT_TOUCHMOVEの最小送信間隔

static struct st_event ts_evq;
#define MAX_TS_EVENT_QUEUE	4
static unsigned char ts_data[MAX_TS_EVENT_QUEUE];
static struct st_mutex ts_mutex;

static unsigned short levent;
static unsigned int ltime;
static unsigned short lpos_x;
static unsigned short lpos_y;

static TS_Init_t hTS;

static void init_ts(void)
{
	int rt;

	hTS.Width = LCD_WIDTH;
	hTS.Height = LCD_HEIGHT;
	hTS.Orientation = TS_SWAP_XY | TS_SWAP_Y;
	hTS.Accuracy = 0;

	rt = BSP_TS_Init(0, &hTS);
	if(rt != BSP_ERROR_NONE) {
		SYSERR_PRINT("BSP_TS_Init() error\n");
	}
}

#ifdef USE_INTERRUPT
static void inthdr_exti9_5(unsigned int intnum, void *sp)
{
	DKFPRINTF(0x01, "INT %d(%d)\n", (int)intnum, HAL_GPIO_ReadPin(TS_INT_GPIO_PORT, TS_INT_PIN));

	HAL_NVIC_DisableIRQ((IRQn_Type)(TS_INT_EXTI_IRQn));
	__HAL_GPIO_EXTI_CLEAR_IT(TS_INT_PIN);

	event_wakeup_ISR(sp, &ts_evq, 0);
}
#endif

#define SIZEOFSTACK	(1024*4)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFSTACK/sizeof(unsigned int)] ATTR_STACK;

extern const struct st_device ts_device;

static int ts_task(void *arg)
{
	TS_State_t  TS_State;
	struct st_sysevent ev;
	int rt;

	while(1) {
#ifdef USE_INTERRUPT
		if(event_wait(&ts_evq, 0, 50) < 0) {
			continue;
		}
#else
		task_sleep(MOVEEVENTINTERVAL);
#endif

		lock_device((struct st_device *)&ts_device, 0);
		rt = BSP_TS_GetState(0, &TS_State);
		if(rt != BSP_ERROR_NONE) {
			/*
			  I2C通信が正常に動作せずTSの状態が読み出せなくなる場合がある。(WM8994も操作不可となる)
			  以下の処理で強制的に復帰している。
			  原因を調査[TODO]
			 */
			BSP_TS_DeInit(0);
			BSP_I2C4_DeInit();
			BSP_I2C4_Init();
			init_ts();
			BSP_TS_EnableIT(0);
		}
		DKFPRINTF(0x01, "[%08ld] D:%d X:%d Y:%d\n", (int)get_kernel_time(), TS_State.touchDetected, TS_State.touchX[0], TS_State.touchY[0]);
#ifdef USE_INTERRUPT
		HAL_NVIC_EnableIRQ((IRQn_Type)(TS_INT_EXTI_IRQn));
#endif
		unlock_device((struct st_device *)&ts_device);

		switch(levent) {
		case EVT_NULL:
		case EVT_TOUCHEND:
			if(TS_State.TouchDetected != 0) {
				ev.what = EVT_TOUCHSTART;
				ev.pos_x = TS_State.TouchX;
				ev.pos_y = TS_State.TouchY;
				ltime = get_kernel_time();
			} else {
				// イベントなし
				continue;
			}
			break;

		case EVT_TOUCHSTART:
		case EVT_TOUCHMOVE:
			if(TS_State.TouchDetected != 0) {
				if((lpos_x != TS_State.TouchX) ||
				   (lpos_y != TS_State.TouchY)) {
					unsigned int ntime = get_kernel_time();
					if((ntime - ltime) >= MOVEEVENTINTERVAL) {
						// イベントが密集するのを防ぐ
						ev.what = EVT_TOUCHMOVE;
						ev.pos_x = TS_State.TouchX;
						ev.pos_y = TS_State.TouchY;
						ltime = ntime;
					} else {
						// イベントなし
						continue;
					}
				} else {
					// イベントなし
					continue;
				}
			} else {
				ev.what = EVT_TOUCHEND;
				ev.pos_x = lpos_x;
				ev.pos_y = lpos_y;
			}
			break;

		default:
			break;
		}

		levent = ev.what;
		lpos_x = ev.pos_x;
		lpos_y = ev.pos_y;

		push_event_interrupt(0, &ev);
	}

	return 0;
}

static int ts_register(struct st_device *dev, char *param)
{
	levent = EVT_NULL;
	ltime = 0;
	lpos_x = 0;
	lpos_y = 0;

	eventqueue_register_ISR(&ts_evq, "touch_sn", ts_data, sizeof(unsigned char), MAX_TS_EVENT_QUEUE);

#ifdef USE_INTERRUPT
	register_interrupt(IRQ2VECT(EXTI9_5_IRQn), inthdr_exti9_5);
#endif

	task_add(ts_task, "touch_sensor", TASK_PRIORITY_DEVICE_DRIVER, &tcb,
		 stack, SIZEOFSTACK, 0);

	init_ts();

#ifdef USE_INTERRUPT
	BSP_TS_EnableIT(0);
	// 多重割り込みはカーネルが対応していないので
	HAL_NVIC_SetPriority(TS_INT_EXTI_IRQn, 0, 0);	// 割り込みプライオリティは最低(0)
#endif

	return 0;
}

const struct st_device ts_device = {
	.name		= DEF_DEV_NAME_TS,
	.explan		= "STM32H747I-Disc Touch Sensor",
	.mutex		= &ts_mutex,
	.register_dev	= ts_register,
};
