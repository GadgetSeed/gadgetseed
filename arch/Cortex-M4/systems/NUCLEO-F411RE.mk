ifeq ($(DEV_ENABLE_ADC),YES)
ARCH_DRIVERS += adc
endif

ifeq ($(DEV_ENABLE_I2C),YES)
ARCH_DRIVERS += i2c
endif

ifeq ($(DEV_ENABLE_TEMPSENSOR_BME280),YES)
DRIVERS += bme280
endif

ifeq ($(DEV_ENABLE_RTC),YES)
ARCH_DRIVERS += rtc
endif

ifeq ($(DEV_ENABLE_BUTTON),YES)
ARCH_DRIVERS += gpio_button
endif

ifeq ($(DEV_ENABLE_LED),YES)
ARCH_DRIVERS += stm32f4xx_nucleo_led
endif

ifeq ($(DEV_ENABLE_BUZZER),YES)
ARCH_DRIVERS += tim3_buzzer
DRIVERS += sound
endif

ifeq ($(DEV_ENABLE_NULL),YES)
DRIVERS += null
endif
