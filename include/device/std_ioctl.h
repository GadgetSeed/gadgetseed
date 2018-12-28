/** @file
    @brief	デバイスドライバ ioctl 用マクロ定義

    @date	2015.09.26
    @author	Takashi SHUDO
*/

#ifndef STD_IOCTL_H
#define STD_IOCTL_H

#define DEV_TIMER	(0x00)	///< タイマ
#define DEV_UART	(0x01)	///< UART
#define DEV_RTC		(0x02)	///< RTC(時計)
#define DEV_I2C		(0x03)	///< I2C
#define DEV_SPI		(0x04)	///< SPI
#define DEV_SD		(0x05)	///< ストレージデバイス
#define DEV_VIDEO	(0x06)	///< ビデオ出力デバイス
#define DEV_VIDEOIO	(0x07)	///< ビデオ出力デバイス制御用IO
#define DEV_BUZZER	(0x08)	///< 圧電ブザー等
#define DEV_SOUND	(0x09)	///< 圧電ブザー等シーケンス
#define DEV_IRQ		(0x0A)	///< 外部割り込み(IRQ)
#define DEV_ETHER	(0x0B)	///< Etherデバイス
#define DEV_GPIO	(0x0C)	///< GPIOデバイス
#define DEV_AUDIO	(0x0D)	///< AUDIOデバイス
#define DEV_TS		(0x0E)	///< タッチセンサデバイス
#define DEV_ENVSNSR	(0x0F)	///< 環境センサ(温度センサ等)
#define DEV_LOGBUF	(0xFF)	///< ログバッファ

#define STDIOCTL(dev,ioctl)	((unsigned int)(((dev)<<24)+((ioctl)<<16)))	///< ioctlデバイスタイプ毎のコマンド定義マクロ
#define IOCTL(ioctl)	((unsigned int)((ioctl) & 0xffff0000))			///< ioctlコマンド

#endif //STD_IOCTL_H
