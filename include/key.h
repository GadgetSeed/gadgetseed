/** @file
    @brief	DADG109Aキーコード定義

    @date	2008.02.10
    @author	Takashi SHUDO
*/

#ifndef INCLUDE_KEY_H
#define INCLUDE_KEY_H

/*
  event.arg への代入値
*/
/*
  GadgetBaseキーボードキーコード
*/

				// ----- Shift Num   Num+Shift
				// ----- ----- ----- -----
#define KEY_GB_Q	200	// q     Q     !     !
#define KEY_GB_W	201	// w     W     "     "
#define KEY_GB_E	202	// e     E     #     #
#define KEY_GB_R	203	// r     R     7     7
#define KEY_GB_T	204	// t     T     8     8
#define KEY_GB_Y	205	// y     Y     9     9
#define KEY_GB_U	206	// u     U     *     *
#define KEY_GB_I	207	// i     I     -     -
#define KEY_GB_O	208	// o     O     `     `
#define KEY_GB_P	209	// p     P     |     ＼
#define KEY_GB_F1	210	// F1    F5    F1    F5

#define KEY_GB_A	211	// a     A     $     $
#define KEY_GB_S	212	// s     S     %     %
#define KEY_GB_D	213	// d     D     &     &
#define KEY_GB_F	214	// f     F     4     4
#define KEY_GB_G	215	// g     G     5     5
#define KEY_GB_H	216	// h     H     6     6
#define KEY_GB_J	217	// j     J     /     /
#define KEY_GB_K	218	// k     K     +     +
#define KEY_GB_L	219	// l     L     @     @
#define KEY_GB_COLON	220	// :     ;     ^     ^
#define KEY_GB_F2	221	// F2    F6    F2    F6

#define KEY_GB_Z	222	// z     Z     '     '
#define KEY_GB_X	223	// x     X     ?     ?
#define KEY_GB_C	224	// c     C     ~     ~
#define KEY_GB_V	225	// v     V     1     1
#define KEY_GB_B	226	// b     B     2     2
#define KEY_GB_N	227	// n     N     3     3
#define KEY_GB_M	228	// m     M     .     .
#define KEY_GB_UP	229	// ↑    PUP   ↑    PUP
#define KEY_GB_ESC	230	// ESC   TAB   ESC   TAB
#define KEY_GB_BS	231	// BS    DEL   BS    DEL
#define KEY_GB_F3	232	// F3    F7    F3    F7

#define KEY_GB_SHIFT	233	// Shift
#define KEY_GB_CTRL	234	// CTRL
#define KEY_GB_NUM	235	// Num
#define KEY_GB_SPACE	236	// SPACE 0     SPACE 0
#define KEY_GB_KAKKO	237	// (     [     .     {
#define KEY_GB_KOKKA	238	// )     ]     ,     }
#define KEY_GB_LEFT	239	// ←    HOME  ←    HOME
#define KEY_GB_DOWN	240	// ↓    PDOWN ↓    PDOWN
#define KEY_GB_RIGHT	241	// →    END   →    END
#define KEY_GB_ENTER	242	// Enter
#define KEY_GB_F4	243	// F4    F8    F4    F8

#define KEY_GB_RF	244	// <<
#define KEY_GB_STOP	245	// STOP
#define KEY_GB_PLAY	246	// PLAY
#define KEY_GB_FF	247	// >>
#define KEY_GB_MODE	248	// MODE
#define KEY_GB_MUTE	249	// MUTE
#define KEY_GB_VOLUP	250	// Volume UP
#define KEY_GB_VOLDOWN	251	// Volume DOWN
#define KEY_GB_HOME	252	// HOME
#define KEY_POWER	255	// 電源キー

#define KEY_GB_TOPNUM	KEY_GB_Q
#define KEY_GB_BOTTOMNUM	KEY_GB_F4


/*
  PS/2 USB PCキーボードキーコード
*/
#define KEY_HANZEN	1	// 半角/全角
#define KEY_1		2	// 1 !
#define KEY_2		3	// 2 "
#define KEY_3		4	// 3 #
#define KEY_4		5	// 4 $
#define KEY_5		6	// 5 %
#define KEY_6		7	// 6 &
#define KEY_7		8	// 7 '
#define KEY_8		9	// 8 (
#define KEY_9		10	// 9 )
#define KEY_0		11	// 0
#define KEY_HYPHEN	12	// - =
#define KEY_HAT		13	// ^ ~
#define KEY_YEN		14	// \ ｜
#define KEY_BACKSPACES	15	// Backspace
#define KEY_TAB		16	// Tab
#define KEY_Q		17	// Q
#define KEY_W		18	// W
#define KEY_E		19	// E
#define KEY_R		20	// R
#define KEY_T		21	// T
#define KEY_Y		22	// Y
#define KEY_U		23	// U
#define KEY_I		24	// I
#define KEY_O		25	// O
#define KEY_P		26	// P
#define KEY_ATMARK	27	// @ `
#define KEY_LSB		28	// [ {
// 29
#define KEY_EISUU	30	// 英数
#define KEY_A		31	// A
#define KEY_S		32	// S
#define KEY_D		33	// D
#define KEY_F		34	// F
#define KEY_G		35	// G
#define KEY_H		36	// H
#define KEY_J		37	// J
#define KEY_K		38	// K
#define KEY_L		39	// L
#define KEY_SEMICOLON	40	// ; +
#define KEY_COLON	41	// : *
#define KEY_RSB		42	// ] }
#define KEY_ENTER	43	// Enter
#define KEY_LSHIFT	44	// Left Shift
// 45
#define KEY_Z		46	// Z
#define KEY_X		47	// X
#define KEY_C		48	// C
#define KEY_V		49	// V
#define KEY_B		50	// B
#define KEY_N		51	// N
#define KEY_M		52	// M
#define KEY_COMMA	53	// , <
#define KEY_PERIOD	54	// . >
#define KEY_SLASH	55	// / ?
#define KEY_BACKSLASH	56	// \ _
#define KEY_RIGHTSHIFT	57	// Right Shift
#define KEY_LEFTCTRL	58	// Left Ctrl
#define KEY_LEFTALT	60	// Left Alt
#define KEY_SPACE	61	// Spacebar
#define KEY_RIGHTALT	62	// Right Alt
#define KEY_RIGHTCTRL	64	// Right Ctrl
#define KEY_INSERT	75	// Insert
#define KEY_DELETE	76	// Delete
#define KEY_LEFT	79	// ←
#define KEY_HOME	80	// Home
#define KEY_END		81	// End
#define KEY_UP		83	// ↑
#define KEY_DOWN	84	// ↓
#define KEY_PGUP	85	// Page Up
#define KEY_PGDN	86	// Page Down
#define KEY_RIGHT	89	// →
#define KEY_NUMLOCK	90	// Num Lock
#define KEY_KP7		91	// Keypad 7
#define KEY_KP4		92	// Keypad 4
#define KEY_KP1		93	// Keypad 1
#define KEY_KPSLASH	95	// Keypad /
#define KEY_KP8		96	// Keypad 8
#define KEY_KP5		97	// Keypad 5
#define KEY_KP2		98	// Keypad 2
#define KEY_KP0		99	// Keypad 0
#define KEY_KPASTERISC	100	// Keypad *
#define KEY_KP9		101	// Keypad 9
#define KEY_KP6		102	// Keypad 6
#define KEY_KP3		103	// Keypad 3
#define KEY_KPPERIOD	104	// Keypad .
#define KEY_KPHYPHEN	105	// Keypad -
#define KEY_KPPLUS	106	// Keypad +
#define KEY_KPENTER	108	// Keypad Enter
#define KEY_ESC		110	// Esc
#define KEY_F1		112	// F1
#define KEY_F2		113	// F2
#define KEY_F3		114	// F3
#define KEY_F4		115	// F4
#define KEY_F5		116	// F5
#define KEY_F6		117	// F6
#define KEY_F7		118	// F7
#define KEY_F8		119	// F8
#define KEY_F9		120	// F9
#define KEY_F10		121	// F10
#define KEY_F11		122	// F11
#define KEY_F12		123	// F12
#define KEY_PRINTSCREEN	124	// Print Screen
#define KEY_SCROLLLOCK	125	// Scroll Lock
#define KEY_PAUSE	126	// Pause
#define KEY_LEFTWINDOWS	127	// Left Windows
#define KEY_RIGHTWINDOWS	128	// Right Windows
#define KEY_APPLICATION	129	// Application
#define KEY_MUHENKAN	131	// 無変換
#define KEY_HENKAN	132	// 変換
#define KEY_HIRAKATA	133	// ひらがな カタカナ

#endif // INCLUDE_KEY_H
