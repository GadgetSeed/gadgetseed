/** @file
 * @brief	STM32 RTC
 *
 * @date	2015.08.12
 * @author	Takashi SHUDO
 */

#include "device.h"
#include "device/rtc_ioctl.h"
#include "datetime.h"
#include "tkprintf.h"
#include "system.h"

#include "stm32f4xx_hal.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{
#if 0
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	RTC_AlarmTypeDef sAlarm;
#endif
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  
	/**Initialize RTC and set the Time and Date 
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	HAL_RTC_Init(&hrtc);

#if 0
	sTime.Hours = 0;
	sTime.Minutes = 0;
	sTime.Seconds = 0;
	sTime.SubSeconds = 0;
	sTime.TimeFormat = RTC_HOURFORMAT12_AM;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BCD);

	sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	sDate.Month = RTC_MONTH_JANUARY;
	sDate.Date = 1;
	sDate.Year = 0;
	HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BCD);

	/**Enable the Alarm A 
	 */
	sAlarm.AlarmTime.Hours = 0;
	sAlarm.AlarmTime.Minutes = 0;
	sAlarm.AlarmTime.Seconds = 0;
	sAlarm.AlarmTime.SubSeconds = 0;
	sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = 1;
	sAlarm.Alarm = RTC_ALARM_A;
	HAL_RTC_SetAlarm(&hrtc, &sAlarm, FORMAT_BCD);

	/**Enable the Alarm B 
	 */
	sAlarm.Alarm = RTC_ALARM_B;
	HAL_RTC_SetAlarm(&hrtc, &sAlarm, FORMAT_BCD);

	/**Enable the WakeUp 
	 */
	HAL_RTCEx_SetWakeUpTimer(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
#endif
}

void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
#ifdef GSC_TARGET_SYSTEM_32F469IDISCOVERY
	RCC_OscInitTypeDef Osc;

	/*##-1- Configue LSE as RTC clock soucre ###################################*/
	Osc.OscillatorType = RCC_OSCILLATORTYPE_LSE;
	Osc.LSEState = RCC_LSE_ON;
	Osc.HSIState = RCC_HSI_OFF;
	Osc.HSICalibrationValue = 0;
	Osc.LSIState = RCC_LSI_OFF;
	Osc.PLL.PLLState = RCC_PLL_NONE;
	HAL_RCC_OscConfig(&Osc);

	__HAL_RCC_RTC_CLKPRESCALER(RCC_RTCCLKSOURCE_LSE);
	__HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);
#endif

	if(hrtc->Instance==RTC) {
		__HAL_RCC_RTC_ENABLE();
	}
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_RTC_AlarmAEventCallback could be implemented in the user file
   */
}

void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_RTCEx_AlarmBEventCallback could be implemented in the user file
   */
}

static int rtc_register(struct st_device *dev, char *param)
{
	MX_RTC_Init();

	return 0;
}


static int rtc_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	struct st_datetime *tp = (struct st_datetime *)param;

	switch(com) {
	case IOCMD_RTC_SET:
		{
			RTC_DateTypeDef sDate;
			RTC_TimeTypeDef sTime;

			sDate.Year = tp->year - 2000;
			sDate.Month = tp->month;
			sDate.Date = tp->day;
			sDate.WeekDay = tp->dayofweek;

			sTime.TimeFormat = RTC_HOURFORMAT12_AM;
			sTime.Hours = tp->hour;
			sTime.Minutes = tp->min;
			sTime.Seconds = tp->sec;
			sTime.SubSeconds = 256 - ((tp->msec * 256) / 1000);
			sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
			sTime.StoreOperation = RTC_STOREOPERATION_RESET;

			HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BIN);
			HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BIN);

			return 0;
		}
		break;

	case IOCMD_RTC_GET:
		{
			RTC_TimeTypeDef gTime;
			RTC_DateTypeDef gDate;

			HAL_RTC_GetTime(&hrtc, &gTime, FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc, &gDate, FORMAT_BIN);

			DKPRINTF(0x01, "%04d-%02d-%02d ",
				 2000 + gDate.Year, gDate.Month, gDate.Date);
			DKPRINTF(0x01, "%02d:%02d:%02d\n",
				 gTime.Hours, gTime.Minutes, gTime.Seconds);

			tp->year = gDate.Year + 2000;
			tp->month = gDate.Month;
			tp->day = gDate.Date;
			tp->dayofweek = gDate.WeekDay;

			if(gTime.TimeFormat == RTC_HOURFORMAT12_AM) {
				tp->hour = gTime.Hours;
			} else {
				tp->hour = gTime.Hours + 12;
			}
			tp->min = gTime.Minutes;
			tp->sec = gTime.Seconds;
			tp->msec = ((256 - gTime.SubSeconds) * 1000) / 256;

			return 0;
		}
		break;

	default:
		SYSERR_PRINT("Unknown ioctl(%08lX)\n", com);
		break;
	}

	return -1;
}

const struct st_device rtc_device = {
	.name		= DEF_DEV_NAME_RTC,
	.explan		= "STM32F4 RTC",
	.register_dev	= rtc_register,
	.ioctl		= rtc_ioctl,
};
