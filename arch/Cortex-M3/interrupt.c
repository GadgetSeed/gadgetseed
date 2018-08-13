/** @file
    @brief	STM32 割り込みハンドラ

    @date	2013.03.10
    @author	Takashi SHUDO
*/

#include "interrupt.h"
#include "sysconfig.h"
#include "tkprintf.h"
#include "task/calltrace.h"
#include "task/task.h"

unsigned int last_int;
void *last_sp;

volatile int flg_interrput_proc = 0;
typedef void (*inthdr)(void);
typedef void (*gs_inthdr)(unsigned int intnum, void *sp);

int is_in_interrupt(void)
{
	return flg_interrput_proc;
}

static void unreg_inthdr(unsigned int intnum, void *sp)
{
	SYSERR_PRINT("Unknown interrupt(%u).\n", intnum);

	disp_regs((union st_regs *)sp);

	while(1)
	;
}

#define CFSR	(*(volatile unsigned int *)0xE000ED28)
#define MMSR	(*(volatile unsigned char *)0xE000ED28)
#define BFSR	(*(volatile unsigned char *)0xE000ED29)
#define UFSR	(*(volatile unsigned short *)0xE000ED2A)
#define HFSR	(*(volatile unsigned int *)0xE000ED2C)
#define DFSR	(*(volatile unsigned int *)0xE000ED30)
#define AFSR	(*(volatile unsigned int *)0xE000ED3C)

#define DHCSR	(*(volatile unsigned int *)0xE000EDF0)
#define DCRSR	(*(volatile unsigned int *)0xE000EDF4)
#define DCRDR	(*(volatile unsigned int *)0xE000EDF8)
#define DEMCR	(*(volatile unsigned int *)0xE000EDFC)

void disp_debug_info(void)
{
	tkprintf("CFSR  = %08X\n", CFSR);
	tkprintf(" MMSR = %02X\n", MMSR);
	tkprintf(" BFSR = %02X\n", BFSR);
	tkprintf(" UFSR = %04X\n", UFSR);
	tkprintf("HFSR  = %08X\n", HFSR);
	tkprintf("DFSR  = %08X\n", DFSR);
	tkprintf("AFSR  = %08X\n", AFSR);

	tkprintf("DHCSR = %08X\n", DHCSR);
	tkprintf("DCRSR = %08X\n", DCRSR);
	tkprintf("DCRDR = %08X\n", DCRDR);
	tkprintf("DEMCR = %08X\n", DEMCR);

	tkprintf("Last Int = %02X(%d)\n", (int)last_int, (int)last_int);
	tkprintf("Last SP  = %08X\n", (int)last_sp);
}


void fault_inthdr(unsigned int intnum, void *sp)
{
	switch(intnum) {
	case 3:
		SYSERR_PRINT("Hard Fault Interrupt(%d).\n", intnum);
		break;

	case 4:
		SYSERR_PRINT("Mem Manage Interrupt(%d).\n", intnum);
		break;

	case 5:
		SYSERR_PRINT("Buf Fault Interrupt(%d).\n", intnum);
		break;

	case 6:
		SYSERR_PRINT("Usage Fault Interrupt(%d).\n", intnum);
		break;

	default:
		SYSERR_PRINT("??? Interrupt(%d).\n", intnum);
		break;
	}

	disp_task_info();

	disp_regs((union st_regs *)sp);

	disp_debug_info();

	print_task();

	print_stack();

	print_queues();

	print_calltrace();

	while(1)
	;
}

void ur_inthdr_000(unsigned int intnum, void *sp){unreg_inthdr(0,sp);}
void ur_inthdr_001(unsigned int intnum, void *sp){unreg_inthdr(1,sp);}
void ur_inthdr_002(unsigned int intnum, void *sp){unreg_inthdr(2,sp);}
void ur_inthdr_003(unsigned int intnum, void *sp){unreg_inthdr(3,sp);}
void ur_inthdr_004(unsigned int intnum, void *sp){unreg_inthdr(4,sp);}
void ur_inthdr_005(unsigned int intnum, void *sp){unreg_inthdr(5,sp);}
void ur_inthdr_006(unsigned int intnum, void *sp){unreg_inthdr(6,sp);}
void ur_inthdr_007(unsigned int intnum, void *sp){unreg_inthdr(7,sp);}
void ur_inthdr_008(unsigned int intnum, void *sp){unreg_inthdr(8,sp);}
void ur_inthdr_009(unsigned int intnum, void *sp){unreg_inthdr(9,sp);}
void ur_inthdr_010(unsigned int intnum, void *sp){unreg_inthdr(10,sp);}
void ur_inthdr_011(unsigned int intnum, void *sp){unreg_inthdr(11,sp);}
void ur_inthdr_012(unsigned int intnum, void *sp){unreg_inthdr(12,sp);}
void ur_inthdr_013(unsigned int intnum, void *sp){unreg_inthdr(13,sp);}
void ur_inthdr_014(unsigned int intnum, void *sp){unreg_inthdr(14,sp);}
void ur_inthdr_015(unsigned int intnum, void *sp){unreg_inthdr(15,sp);}
void ur_inthdr_016(unsigned int intnum, void *sp){unreg_inthdr(16,sp);}
void ur_inthdr_017(unsigned int intnum, void *sp){unreg_inthdr(17,sp);}
void ur_inthdr_018(unsigned int intnum, void *sp){unreg_inthdr(18,sp);}
void ur_inthdr_019(unsigned int intnum, void *sp){unreg_inthdr(19,sp);}
void ur_inthdr_020(unsigned int intnum, void *sp){unreg_inthdr(20,sp);}
void ur_inthdr_021(unsigned int intnum, void *sp){unreg_inthdr(21,sp);}
void ur_inthdr_022(unsigned int intnum, void *sp){unreg_inthdr(22,sp);}
void ur_inthdr_023(unsigned int intnum, void *sp){unreg_inthdr(23,sp);}
void ur_inthdr_024(unsigned int intnum, void *sp){unreg_inthdr(24,sp);}
void ur_inthdr_025(unsigned int intnum, void *sp){unreg_inthdr(25,sp);}
void ur_inthdr_026(unsigned int intnum, void *sp){unreg_inthdr(26,sp);}
void ur_inthdr_027(unsigned int intnum, void *sp){unreg_inthdr(27,sp);}
void ur_inthdr_028(unsigned int intnum, void *sp){unreg_inthdr(28,sp);}
void ur_inthdr_029(unsigned int intnum, void *sp){unreg_inthdr(29,sp);}
void ur_inthdr_030(unsigned int intnum, void *sp){unreg_inthdr(30,sp);}
void ur_inthdr_031(unsigned int intnum, void *sp){unreg_inthdr(31,sp);}
void ur_inthdr_032(unsigned int intnum, void *sp){unreg_inthdr(32,sp);}
void ur_inthdr_033(unsigned int intnum, void *sp){unreg_inthdr(33,sp);}
void ur_inthdr_034(unsigned int intnum, void *sp){unreg_inthdr(34,sp);}
void ur_inthdr_035(unsigned int intnum, void *sp){unreg_inthdr(35,sp);}
void ur_inthdr_036(unsigned int intnum, void *sp){unreg_inthdr(36,sp);}
void ur_inthdr_037(unsigned int intnum, void *sp){unreg_inthdr(37,sp);}
void ur_inthdr_038(unsigned int intnum, void *sp){unreg_inthdr(38,sp);}
void ur_inthdr_039(unsigned int intnum, void *sp){unreg_inthdr(39,sp);}
void ur_inthdr_040(unsigned int intnum, void *sp){unreg_inthdr(40,sp);}
void ur_inthdr_041(unsigned int intnum, void *sp){unreg_inthdr(41,sp);}
void ur_inthdr_042(unsigned int intnum, void *sp){unreg_inthdr(42,sp);}
void ur_inthdr_043(unsigned int intnum, void *sp){unreg_inthdr(43,sp);}
void ur_inthdr_044(unsigned int intnum, void *sp){unreg_inthdr(44,sp);}
void ur_inthdr_045(unsigned int intnum, void *sp){unreg_inthdr(45,sp);}
void ur_inthdr_046(unsigned int intnum, void *sp){unreg_inthdr(46,sp);}
void ur_inthdr_047(unsigned int intnum, void *sp){unreg_inthdr(47,sp);}
void ur_inthdr_048(unsigned int intnum, void *sp){unreg_inthdr(48,sp);}
void ur_inthdr_049(unsigned int intnum, void *sp){unreg_inthdr(49,sp);}
void ur_inthdr_050(unsigned int intnum, void *sp){unreg_inthdr(50,sp);}
void ur_inthdr_051(unsigned int intnum, void *sp){unreg_inthdr(51,sp);}
void ur_inthdr_052(unsigned int intnum, void *sp){unreg_inthdr(52,sp);}
void ur_inthdr_053(unsigned int intnum, void *sp){unreg_inthdr(53,sp);}
void ur_inthdr_054(unsigned int intnum, void *sp){unreg_inthdr(54,sp);}
void ur_inthdr_055(unsigned int intnum, void *sp){unreg_inthdr(55,sp);}
void ur_inthdr_056(unsigned int intnum, void *sp){unreg_inthdr(56,sp);}
void ur_inthdr_057(unsigned int intnum, void *sp){unreg_inthdr(57,sp);}
void ur_inthdr_058(unsigned int intnum, void *sp){unreg_inthdr(58,sp);}
void ur_inthdr_059(unsigned int intnum, void *sp){unreg_inthdr(59,sp);}
void ur_inthdr_060(unsigned int intnum, void *sp){unreg_inthdr(60,sp);}
void ur_inthdr_061(unsigned int intnum, void *sp){unreg_inthdr(61,sp);}
void ur_inthdr_062(unsigned int intnum, void *sp){unreg_inthdr(62,sp);}
void ur_inthdr_063(unsigned int intnum, void *sp){unreg_inthdr(63,sp);}
void ur_inthdr_064(unsigned int intnum, void *sp){unreg_inthdr(64,sp);}
void ur_inthdr_065(unsigned int intnum, void *sp){unreg_inthdr(65,sp);}
void ur_inthdr_066(unsigned int intnum, void *sp){unreg_inthdr(66,sp);}
void ur_inthdr_067(unsigned int intnum, void *sp){unreg_inthdr(67,sp);}
void ur_inthdr_068(unsigned int intnum, void *sp){unreg_inthdr(68,sp);}
void ur_inthdr_069(unsigned int intnum, void *sp){unreg_inthdr(69,sp);}
void ur_inthdr_070(unsigned int intnum, void *sp){unreg_inthdr(70,sp);}
void ur_inthdr_071(unsigned int intnum, void *sp){unreg_inthdr(71,sp);}
void ur_inthdr_072(unsigned int intnum, void *sp){unreg_inthdr(72,sp);}
void ur_inthdr_073(unsigned int intnum, void *sp){unreg_inthdr(73,sp);}
void ur_inthdr_074(unsigned int intnum, void *sp){unreg_inthdr(74,sp);}
void ur_inthdr_075(unsigned int intnum, void *sp){unreg_inthdr(75,sp);}
void ur_inthdr_076(unsigned int intnum, void *sp){unreg_inthdr(76,sp);}
void ur_inthdr_077(unsigned int intnum, void *sp){unreg_inthdr(77,sp);}
void ur_inthdr_078(unsigned int intnum, void *sp){unreg_inthdr(78,sp);}
void ur_inthdr_079(unsigned int intnum, void *sp){unreg_inthdr(79,sp);}
void ur_inthdr_080(unsigned int intnum, void *sp){unreg_inthdr(80,sp);}
void ur_inthdr_081(unsigned int intnum, void *sp){unreg_inthdr(81,sp);}
void ur_inthdr_082(unsigned int intnum, void *sp){unreg_inthdr(82,sp);}
void ur_inthdr_083(unsigned int intnum, void *sp){unreg_inthdr(83,sp);}
void ur_inthdr_084(unsigned int intnum, void *sp){unreg_inthdr(84,sp);}
void ur_inthdr_085(unsigned int intnum, void *sp){unreg_inthdr(85,sp);}
void ur_inthdr_086(unsigned int intnum, void *sp){unreg_inthdr(86,sp);}
void ur_inthdr_087(unsigned int intnum, void *sp){unreg_inthdr(87,sp);}
void ur_inthdr_088(unsigned int intnum, void *sp){unreg_inthdr(88,sp);}
void ur_inthdr_089(unsigned int intnum, void *sp){unreg_inthdr(89,sp);}
void ur_inthdr_090(unsigned int intnum, void *sp){unreg_inthdr(90,sp);}
void ur_inthdr_091(unsigned int intnum, void *sp){unreg_inthdr(91,sp);}
void ur_inthdr_092(unsigned int intnum, void *sp){unreg_inthdr(92,sp);}
void ur_inthdr_093(unsigned int intnum, void *sp){unreg_inthdr(93,sp);}
void ur_inthdr_094(unsigned int intnum, void *sp){unreg_inthdr(94,sp);}
void ur_inthdr_095(unsigned int intnum, void *sp){unreg_inthdr(95,sp);}
void ur_inthdr_096(unsigned int intnum, void *sp){unreg_inthdr(96,sp);}
void ur_inthdr_097(unsigned int intnum, void *sp){unreg_inthdr(97,sp);}

extern void _endof_stack();
extern void start(void);

static const gs_inthdr init_gs_inthdr_table[MAXVECT] = {
	ur_inthdr_000,
	ur_inthdr_001,
	ur_inthdr_002,
	ur_inthdr_003,
	ur_inthdr_004,
	ur_inthdr_005,
	ur_inthdr_006,
	ur_inthdr_007,
	ur_inthdr_008,
	ur_inthdr_009,
	ur_inthdr_010,
	ur_inthdr_011,
	ur_inthdr_012,
	ur_inthdr_013,
	ur_inthdr_014,
	ur_inthdr_015,
	ur_inthdr_016,
	ur_inthdr_017,
	ur_inthdr_018,
	ur_inthdr_019,
	ur_inthdr_020,
	ur_inthdr_021,
	ur_inthdr_022,
	ur_inthdr_023,
	ur_inthdr_024,
	ur_inthdr_025,
	ur_inthdr_026,
	ur_inthdr_027,
	ur_inthdr_028,
	ur_inthdr_029,
	ur_inthdr_030,
	ur_inthdr_031,
	ur_inthdr_032,
	ur_inthdr_033,
	ur_inthdr_034,
	ur_inthdr_035,
	ur_inthdr_036,
	ur_inthdr_037,
	ur_inthdr_038,
	ur_inthdr_039,
	ur_inthdr_040,
	ur_inthdr_041,
	ur_inthdr_042,
	ur_inthdr_043,
	ur_inthdr_044,
	ur_inthdr_045,
	ur_inthdr_046,
	ur_inthdr_047,
	ur_inthdr_048,
	ur_inthdr_049,
	ur_inthdr_050,
	ur_inthdr_051,
	ur_inthdr_052,
	ur_inthdr_053,
	ur_inthdr_054,
	ur_inthdr_055,
	ur_inthdr_056,
	ur_inthdr_057,
	ur_inthdr_058,
	ur_inthdr_059,
	ur_inthdr_060,
	ur_inthdr_061,
	ur_inthdr_062,
	ur_inthdr_063,
	ur_inthdr_064,
	ur_inthdr_065,
	ur_inthdr_066,
	ur_inthdr_067,
	ur_inthdr_068,
	ur_inthdr_069,
	ur_inthdr_070,
	ur_inthdr_071,
	ur_inthdr_072,
	ur_inthdr_073,
	ur_inthdr_074,
	ur_inthdr_075,
	ur_inthdr_076,
	ur_inthdr_077,
	ur_inthdr_078,
	ur_inthdr_079,
	ur_inthdr_080,
	ur_inthdr_081,
	ur_inthdr_082,
	ur_inthdr_083,
	ur_inthdr_084,
	ur_inthdr_085,
	ur_inthdr_086,
	ur_inthdr_087,
	ur_inthdr_088,
	ur_inthdr_089,
	ur_inthdr_090,
	ur_inthdr_091,
	ur_inthdr_092,
	ur_inthdr_093,
	ur_inthdr_094,
	ur_inthdr_095,
	ur_inthdr_096,
	ur_inthdr_097,
};

extern void int000(void);
extern void int001(void);
extern void int002(void);
extern void int003(void);
extern void int004(void);
extern void int005(void);
extern void int006(void);
extern void int007(void);
extern void int008(void);
extern void int009(void);
extern void int010(void);
extern void int011(void);
extern void int012(void);
extern void int013(void);
extern void int014(void);
extern void int015(void);
extern void int016(void);
extern void int017(void);
extern void int018(void);
extern void int019(void);
extern void int020(void);
extern void int021(void);
extern void int022(void);
extern void int023(void);
extern void int024(void);
extern void int025(void);
extern void int026(void);
extern void int027(void);
extern void int028(void);
extern void int029(void);
extern void int030(void);
extern void int031(void);
extern void int032(void);
extern void int033(void);
extern void int034(void);
extern void int035(void);
extern void int036(void);
extern void int037(void);
extern void int038(void);
extern void int039(void);
extern void int040(void);
extern void int041(void);
extern void int042(void);
extern void int043(void);
extern void int044(void);
extern void int045(void);
extern void int046(void);
extern void int047(void);
extern void int048(void);
extern void int049(void);
extern void int050(void);
extern void int051(void);
extern void int052(void);
extern void int053(void);
extern void int054(void);
extern void int055(void);
extern void int056(void);
extern void int057(void);
extern void int058(void);
extern void int059(void);
extern void int060(void);
extern void int061(void);
extern void int062(void);
extern void int063(void);
extern void int064(void);
extern void int065(void);
extern void int066(void);
extern void int067(void);
extern void int068(void);
extern void int069(void);
extern void int070(void);
extern void int071(void);
extern void int072(void);
extern void int073(void);
extern void int074(void);
extern void int075(void);
extern void int076(void);
extern void int077(void);
extern void int078(void);
extern void int079(void);
extern void int080(void);
extern void int081(void);
extern void int082(void);
extern void int083(void);
extern void int084(void);
extern void int085(void);
extern void int086(void);
extern void int087(void);
extern void int088(void);
extern void int089(void);
extern void int090(void);
extern void int091(void);
extern void int092(void);
extern void int093(void);
extern void int094(void);
extern void int095(void);
extern void int096(void);
extern void int097(void);

const inthdr init_inthdr_table[MAXVECT] __attribute__ ((section("vect"))) = {
	_endof_stack,
	start,
	int002,	// 0x00000008	2  NMI
	int003,	// 0x0000000c	3  HardFault
	int004,	// 0x00000010	4  MemManage
	int005,	// 0x00000014	5  BusFault
	int006,	// 0x00000018	6  UsageFault
	int007,	// 0x0000001c	7  Reserved
	int008,	// 0x00000020	8  Reserved
	int009,	// 0x00000024	9  Reserved
	int010,	// 0x00000028	10 Reserved
	int011,	// 0x0000002c	11 SVCall
	int012,	// 0x00000030	12 Debug Monitor
	int013,	// 0x00000034	13 Reserved
	int014,	// 0x00000038	14 PendSV
	int015,	// 0x0000003c	15 SysTick
	int016,	// 0x00000040	16 WWDG
	int017,	// 0x00000044	17 PVD
	int018,	// 0x00000048	18 TAMP_STAMP
	int019,	// 0x0000004c	19 RTC_WKUP
	int020,	// 0x00000050	20 FLASH
	int021,	// 0x00000054	21 RCC
	int022,	// 0x00000058	22 EXTI0
	int023,	// 0x0000005c	23 EXTI1
	int024,	// 0x00000060	24 EXTI2
	int025,	// 0x00000064	25 EXTI3
	int026,	// 0x00000068	26 EXTI4
	int027,	// 0x0000006c	27 DMA1_Stream0
	int028,	// 0x00000070	24 DMA1_Stream1
	int029,	// 0x00000074	29 DMA1_Stream2
	int030,	// 0x00000078	30 DMA1_Stream3
	int031,	// 0x0000007c	31 DMA1_Stream4
	int032,	// 0x00000080	32 DMA1_Stream5
	int033,	// 0x00000084	33 DMA1_Stream6
	int034,	// 0x00000088	34 ADC
	int035,	// 0x0000008c	35 CAN1_TX
	int036,	// 0x00000090	36 CAN1_RX0
	int037,	// 0x00000094	37 CAN1_RX1
	int038,	// 0x00000098	38 CAN1_SCE
	int039,	// 0x0000009c	39 EXTI9_5
	int040,	// 0x000000a0	40 TIM1_BRK_TIM9 (0xa0)
	int041,	// 0x000000a4	41 TIM1_UP_TIM10
	int042,	// 0x000000a8	42 TIM1_TRG_COM_TIM11
	int043,	// 0x000000ac	43 TIM1_CC
	int044,	// 0x000000b0	44 TIM2
	int045,	// 0x000000b4	45 TIM3
	int046,	// 0x000000b8	46 TIM4
	int047,	// 0x000000cc	47 I2C1_EV
	int048,	// 0x000000c0	48 I2C1_ER
	int049,	// 0x000000c4	49 I2C2_EV
	int050,	// 0x000000c8	50 I2C2_ER
	int051,	// 0x000000cc	51 SPI1
	int052,	// 0x000000d0	52 SPI2
	int053,	// 0x000000d4	53 USART1
	int054,	// 0x000000d8	54 USART2
	int055,	// 0x000000dc	55 USART3
	int056,	// 0x000000e0	56 EXTI15_10
	int057,	// 0x000000e4	57 RTC_Alarm
	int058,	// 0x000000e8	58 OTG_FS_WKUP
	int059,	// 0x000000ec	59 TIM8_BRK_TIM12
	int060,	// 0x000000f0	60 TIM8_UP_TIM13
	int061,	// 0x000000f4	61 TIM8_TRG_COM_TIM14
	int062,	// 0x000000f8	62 TIM8_CC
	int063,	// 0x000000fc	63 DMA1_Stream7
	int064,	// 0x00000100	64 FSMC
	int065,	// 0x00000104	65 SDIO
	int066,	// 0x00000108	66 TIM5
	int067,	// 0x0000010c	67 SPI3
	int068,	// 0x00000110	68 USART4
	int069,	// 0x00000114	69 USART5
	int070,	// 0x00000118	70 TIM6_DAC
	int071,	// 0x0000011c	71 TIM7
	int072,	// 0x00000120	72 DMA2_Stream0
	int073,	// 0x00000124	73 DMA2_Stream1
	int074,	// 0x00000128	74 DMA2_Stream2
	int075,	// 0x0000012c	75 DMA2_Stream3
	int076,	// 0x00000130	76 DMA2_Stream4
	int077,	// 0x00000134	77 ETH
	int078,	// 0x00000138	78 ETH_WKUP
	int079,	// 0x0000013c	79 CAN2_TX
	int080,	// 0x00000140	80 CAN2_RX0
	int081,	// 0x00000144	81 CAN2_RX1
	int082,	// 0x00000148	82 CAN2_SCE
	int083,	// 0x0000014c	83 OTG_FS
	int084,	// 0x00000150	84 DMA2_Stream5
	int085,	// 0x00000154	85 DMA2_Stream6
	int086,	// 0x00000158	86 DMA2_Stream7
	int087,	// 0x0000015c	87 USART6
	int088,	// 0x00000160	88 I2C3_EV
	int089,	// 0x00000164	89 I2C3_ER
	int090,	// 0x00000168	90 OTG_HS_EP1_OUT
	int091,	// 0x0000016c	91 OTG_HS_EP1_IN
	int092,	// 0x00000170	92 OTG_HS_WKUP
	int093,	// 0x00000174	93 OTG_HS
	int094,	// 0x00000178	94 DCMI
	int095,	// 0x0000017c	95 CRYP
	int096,	// 0x00000180	96 HASH_RNG
	int097,	// 0x00000184	97 FPU
};

gs_inthdr inthdr_table[MAXVECT];

void init_interrupt_vector(void)
{
	int i;

	for(i=0; i<MAXVECT; i++) {
		inthdr_table[i] = init_gs_inthdr_table[i];
	}

//	inthdr_table[3] = fault_inthdr;
}

int register_interrupt(unsigned short vectnum,
		       void (* func)(unsigned int intnum, void *sp))
{
	if(inthdr_table[vectnum] == init_gs_inthdr_table[vectnum]) {
		inthdr_table[vectnum] = func;
	} else {
		SYSERR_PRINT("Interrupt Vector %d allready registered.\n",
			     vectnum);
		return -1;
	}

	return 0;
}

int unregister_interrupt(unsigned short vectnum)
{
	inthdr_table[vectnum] = init_gs_inthdr_table[vectnum];

	return 0;
}

#ifdef GSC_KERNEL_ENABLE_INTERRUPT_COUNT
static unsigned int interrupt_count[MAXVECT];

int get_interrupt_count(int intnum)
{
	if(inthdr_table[intnum] == init_gs_inthdr_table[intnum]) {
		return -1;
	} else {
		return (int)interrupt_count[intnum];
	}
}
#endif

void interrupt_func(unsigned int intnum, void *sp)
{
	void (* func)(unsigned int in, void *s);

#ifdef GSC_KERNEL_ENABLE_INTERRUPT_COUNT
	interrupt_count[intnum] ++;
#endif

	last_int = intnum;
	last_sp = sp;

	func = inthdr_table[intnum];

	flg_interrput_proc = 1;
	(* func)(intnum, sp);
	flg_interrput_proc = 0;
}
