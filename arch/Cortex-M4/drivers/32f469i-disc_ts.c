/** @file
    @brief	STM32F469I Discovery Touch Sensor

    @date	2018.08.17
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


#include "stm32469i_discovery_ts.h"

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

static void inthdr_exti9_5(unsigned int intnum, void *sp)
{
	DKFPRINTF(0x01, "INT %d(%d)\n", (int)intnum, HAL_GPIO_ReadPin(GPIOI, TS_INT_PIN));

	HAL_NVIC_DisableIRQ((IRQn_Type)(TS_INT_EXTI_IRQn));
	__HAL_GPIO_EXTI_CLEAR_IT(TS_INT_PIN);

	event_wakeup_ISR(sp, &ts_evq, 0);
}

#define SIZEOFSTACK	(1024*4)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFSTACK/sizeof(unsigned int)];

extern const struct st_device ts_device;

static int ts_task(char *arg)
{
	TS_StateTypeDef  TS_State;
	struct st_sysevent ev;

	while(1) {
		if(event_wait(&ts_evq, 0, 50) == 0) {
			continue;
		}

		lock_device((struct st_device *)&ts_device, 0);
		BSP_TS_GetState(&TS_State);
		DKFPRINTF(0x01, "[%08ld] D:%d X:%d Y:%d\n", (int)get_kernel_time(), TS_State.touchDetected, TS_State.touchX[0], TS_State.touchY[0]);
		HAL_NVIC_EnableIRQ((IRQn_Type)(TS_INT_EXTI_IRQn));
		unlock_device((struct st_device *)&ts_device);

		switch(levent) {
		case EVT_NULL:
		case EVT_TOUCHEND:
			if(TS_State.touchDetected != 0) {
				ev.what = EVT_TOUCHSTART;
				ev.pos_x = TS_State.touchX[0];
				ev.pos_y = TS_State.touchY[0];
				ltime = get_kernel_time();
			} else {
				// イベントなし
				continue;
			}
			break;

		case EVT_TOUCHSTART:
		case EVT_TOUCHMOVE:
			if(TS_State.touchDetected != 0) {
				if((lpos_x != TS_State.touchX[0]) ||
				   (lpos_y != TS_State.touchY[0])) {
					unsigned int ntime = get_kernel_time();
					if((ntime - ltime) >= MOVEEVENTINTERVAL) {
						// イベントが密集するのを防ぐ
						ev.what = EVT_TOUCHMOVE;
						ev.pos_x = TS_State.touchX[0];
						ev.pos_y = TS_State.touchY[0];
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

	register_interrupt(IRQ2VECT(TS_INT_EXTI_IRQn), inthdr_exti9_5);

	task_add(ts_task, "touch_sensor", 1, &tcb,
		 stack, SIZEOFSTACK, 0);

	BSP_TS_Init(LCD_WIDTH, LCD_HEIGHT);
	BSP_TS_ITConfig();
	// 多重割り込みはカーネルが対応していないので
	HAL_NVIC_SetPriority(TS_INT_EXTI_IRQn, 0, 0);	// 割り込みプライオリティは最低(0)

	return 0;
}

const struct st_device ts_device = {
	.name		= DEF_DEV_NAME_TS,
	.explan		= "32F469IDISCOVERY Touch Sensor",
	.mutex		= &ts_mutex,
	.register_dev	= ts_register,
};
