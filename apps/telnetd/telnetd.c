/** @file
    @brief	telnetd for LwIP

    @date	2017.08.19
    @author	Takashi SHUDO
*/

#include "str.h"
#include "tprintf.h"
#include "sysevent.h"
#include "console.h"
#include "key.h"
#include "shell.h"
#include "task/syscall.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


#define LISTEN_PORT	23

#define TELOPT_TB	0x00	// Transmit Binary
#define TELOPT_ECHO	0x01	// Echo
#define TELOPT_SGA	0x03	// Suppress Go Ahead
#define TELOPT_TSO	0x05	// Telnet Status Option
#define TELOPT_TTM	0x06	// Telnet Timing Mark
#define TELOPT_TY	0x18	// Terminal Type
#define TELOPT_EOL	0x19	// End of Record
#define TELOPT_TS	0x20	// Terminal Speed
#define TELOPT_RFC	0x21	// Remote Flow Control
#define TELOPT_TLM	0x22	// Telnet Line Mode
#define TELOPT_XDL	0x23	// X Display Location
#define TELOPT_NEO	0x27	// New Environment Option

#define TELCMD_SE	0xF0	// Sub-negotiaon End
#define TELCMD_NOP	0xF1	// No Operation
#define TELCMD_DM	0xF2	// Data Mark
#define TELCMD_BRK	0xF3	// Break
#define TELCMD_IP	0xF4	// Interrupt Process
#define TELCMD_AO	0xF5	// Abort Output
#define TELCMD_AYT	0xF6	// Are You There
#define TELCMD_EC	0xF7	// Erase Character
#define TELCMD_EL	0xF8	// Erase Line
#define TELCMD_GA	0xF9	// Go Ahead
#define TELCMD_SB	0xFA	// Sub-negotiation Begin
#define TELCMD_WILL	0xFB	// Will
#define TELCMD_WONT	0xFC	// Won't
#define TELCMD_DO	0xFD	// Do
#define TELCMD_DONT	0xFE	// Don't
#define TELCMD_IAC	0xFF	// Interpret as Command

const unsigned char tel_option[] = {
	TELCMD_IAC, TELCMD_WILL, TELOPT_SGA,	// Suppress Go Ahead
	TELCMD_IAC, TELCMD_DONT, TELOPT_TY,	// Terminal type
	TELCMD_IAC, TELCMD_DONT, TELOPT_EOL,	// End of Record
	TELCMD_IAC, TELCMD_DONT, TELOPT_TS,	// Terminal Speed
	TELCMD_IAC, TELCMD_DONT, TELOPT_RFC,	// Remote Flow Control
	TELCMD_IAC, TELCMD_DONT, TELOPT_TLM,	// Linemode
	TELCMD_IAC, TELCMD_DONT, TELOPT_NEO,	// New Environment Option
	TELCMD_IAC, TELCMD_WILL, TELOPT_TSO,	// Status
	TELCMD_IAC, TELCMD_DONT, TELOPT_XDL,	// X Display Location
	TELCMD_IAC, TELCMD_WILL, TELOPT_ECHO,	// Echo
	0
};

extern const struct st_shell_command com_dump;
extern const struct st_shell_command com_memedit;
extern const struct st_shell_command com_memedit_b;
extern const struct st_shell_command com_memedit_w;
extern const struct st_shell_command com_memedit_l;
extern const struct st_shell_command com_sfload;

static const uchar prompt[] = ": ";
struct st_shell net_shell;
#define MAX_BUF	500
static unsigned char buf[MAX_BUF];
static int tblen;

struct netconn *conn, *newconn;
static int flg_connected = 0;
static unsigned long nread_timout = 0;

static int netio_sync(struct st_device *dev)
{
	if(tblen != 0) {
		netconn_write(newconn, buf, tblen, NETCONN_COPY);
		tblen = 0;
	}

	return 0;
}

static int netio_read(struct st_device *dev, void *data, unsigned int count)
{
	struct netbuf *inbuf;
	unsigned char *dp = data;
	char *buf;
	u16_t buflen;
	err_t err;
	long rtn = 0;

	netconn_set_recvtimeout(newconn, nread_timout);//!!!

	err = netconn_recv(newconn, &inbuf);

	if (err == ERR_OK) {
		int i;
		netbuf_data(inbuf, (void**)&buf, &buflen);
		DTFPRINTF(0x01, "len = %d\n", buflen);
		XDUMP(0x01, (unsigned char *)buf, buflen);
		for(i=0; i<buflen; i++) {
			*dp = *buf;
			dp ++;
			buf ++;
		}
		netbuf_delete(inbuf);
		rtn = buflen;
	}

	return rtn;
}

static int netio_write(struct st_device *dev, const void *td, unsigned int count)
{
	int i;
	const unsigned char *dp = td;

	for(i=0; i<count; i++) {
		DTPRINTF(0x01, "T:[%02X]\n", *td);
		buf[tblen] = *dp;
		tblen ++;

		if((tblen >= MAX_BUF) || (*dp == ASCII_LF)) {
			netio_sync(dev);
		}

		dp ++;
	}

	return count;
}

static int netio_select(struct st_device *dev, unsigned int timeout)
{
	DTFPRINTF(0x01, "%d\n", timeout);

	nread_timout = timeout;

	return timeout;
}

const struct st_device netio_device = {
	.name		= "netio",
	.explan		= "LwIP network io device",
	.read		= netio_read,
	.write		= netio_write,
	.sync		= netio_sync,
	.select		= netio_select,
};

extern struct st_tcb telnetd_tcb;

int proc_telnetd(unsigned char *buf, int len)
{
	int i;
	int flg_iac = 0;
	unsigned char *dp = buf;

	for(i=0; i<len; i++) {
		DTPRINTF(0x01, "R:[%02X]\n", *dp);
		switch(*dp) {
		case TELCMD_IAC:
			flg_iac = 1;
			i++;
			dp++;
			switch(*dp) {
			case TELCMD_WILL:
				DTPRINTF(0x01, "Will ");
				break;

			case TELCMD_WONT:
				DTPRINTF(0x01, "Won't ");
				break;

			case TELCMD_DO:
				DTPRINTF(0x01, "Do ");
				break;

			case TELCMD_DONT:
				DTPRINTF(0x01, "Don't ");
				break;

			default:
				DTPRINTF(0x01, "TELCMD %02X ", *dp);
				break;
			}
			i++;
			dp++;
			DTPRINTF(0x01, "%02X\n", *dp);
			break;

		default:
			set_console_out_device((struct st_device *)&netio_device);
			set_console_in_device((struct st_device *)&netio_device);
			if(task_shell(&net_shell, *dp) == -1) {
				set_console_out_device(0);
				return 0;
			}
			set_console_in_device(0);
			netio_sync(0);
			set_console_out_device(0);
			break;
		}
		dp++;
	}

	if(flg_iac) {
		strncopy((unsigned char *)&buf[0], (unsigned char *)tel_option, MAX_BUF);
		netconn_write(newconn, buf, sizeof(tel_option)-1, NETCONN_COPY);
	}

	return 0;
}

static void telnetd_netconn_serve(struct netconn *conn)
{
	struct netbuf *inbuf;
	char *buf;
	u16_t buflen;
	err_t err;

	netconn_write(newconn, ": ", 2, NETCONN_COPY);

	while(flg_connected) {
		err = netconn_recv(conn, &inbuf);

		if (err == ERR_OK) {
			netbuf_data(inbuf, (void**)&buf, &buflen);
			DTPRINTF(0x01, "TELNET recv len %d\n", buflen);

			proc_telnetd((unsigned char *)buf, buflen);
		}

		netbuf_delete(inbuf);
	}
}


static int telnet_exit(int argc, uchar *argv[])
{
	flg_connected = 0;

	return 0;
}

const struct st_shell_command com_telnet_exit = {
	"exit", 0, telnet_exit, 0, "Exit telnet"
};

extern struct st_shell_command * const com_list[];
extern int shell_com_count;
struct st_shell_command * net_com_list[12]; // [TODO]

int telnetd_task(char *arg)
{
	err_t err;
	int i;

	for(i=0; i<shell_com_count; i++) {
		net_com_list[i] = com_list[i];
	}
	net_com_list[i] = (struct st_shell_command *)&com_telnet_exit;
	net_com_list[i+1] = 0;

	tblen = 0;

	tprintf("Start telnetd\n");

	init_shell(&net_shell, net_com_list, prompt);

	/* Create a new TCP connection handle */
	conn = netconn_new(NETCONN_TCP);
	LWIP_ERROR("telnetd: invalid conn", (conn != NULL), return -1;);

	/* Bind to port 23 (TELNET) with default IP address */
	netconn_bind(conn, NULL, LISTEN_PORT);

	while(1) {
		/* Put the connection into LISTEN state */
		netconn_listen(conn);
		tprintf("Wait for telnet connect.\n");

		err = netconn_accept(conn, &newconn);
		if (err == ERR_OK) {
			tprintf("Connect from %d.%d.%d.%d\n",
				ip4_addr1(&(newconn->pcb.tcp->remote_ip)),
				ip4_addr2(&(newconn->pcb.tcp->remote_ip)),
				ip4_addr3(&(newconn->pcb.tcp->remote_ip)),
				ip4_addr4(&(newconn->pcb.tcp->remote_ip))
				);
			flg_connected = 1;
			telnetd_netconn_serve(newconn);
			netconn_delete(newconn);
		} else {
			tprintf("accept error %d\n", err);
		}
	}

	return 0;
}


#define SIZEOFSTACK	(1024*4)
struct st_tcb telnetd_tcb;
unsigned int telnetd_stack[SIZEOFSTACK/sizeof(unsigned int)] ATTR_STACK;

void startup_telnetd(void)
{
	task_exec(telnetd_task, "telnetd", TASK_PRIORITY_APP_HIGH, &telnetd_tcb,
		  telnetd_stack, SIZEOFSTACK, 0);
}
