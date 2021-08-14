/** @file
    @brief	iperfサーバ

    @date	2020.03.04
    @auther	Takashi SHUDO
*/

#include "task/syscall.h"
#include "tprintf.h"
#include "lwip/apps/lwiperf.h"

static void lwiperf_report(void *arg, enum lwiperf_report_type report_type,
			   const ip_addr_t* local_addr, u16_t local_port, const ip_addr_t* remote_addr, u16_t remote_port,
			   u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
	tprintf("report_type = %d\n", report_type);

	switch(report_type) {
	case LWIPERF_TCP_DONE_SERVER: /** The client side test is done */
		tprintf("iperf server done\n");
		break;

	case LWIPERF_TCP_DONE_CLIENT: /** Local error lead to test abort */
		break;

	case LWIPERF_TCP_ABORTED_LOCAL: /** Data check error lead to test abort */
		break;

	case LWIPERF_TCP_ABORTED_LOCAL_DATAERROR: /** Transmit error lead to test abort */
		break;

	case LWIPERF_TCP_ABORTED_LOCAL_TXERROR: /** Remote side aborted the test */
		break;

	case LWIPERF_TCP_ABORTED_REMOTE:
		break;

	default:
		break;
	}

	tprintf("Local  IP   : %s\n", ip4addr_ntoa(local_addr));
	tprintf("Local  PORT : %d\n", local_port);
	tprintf("Remote IP   : %s\n", ip4addr_ntoa(remote_addr));
	tprintf("Remote PORT : %d\n", remote_port);
	tprintf("Transferred bytes  : %u\n", bytes_transferred);
	tprintf("Duration(ms)       : %u\n", ms_duration);
	tprintf("Band width(Kbit/s) : %u\n", bandwidth_kbitpsec);
}

void start_lwiperf_server(void)
{
	lwiperf_start_tcp_server_default(lwiperf_report, NULL);
}

static int iperf_server_task(void *arg)
{
	start_lwiperf_server();

	return 0;
}


#define SIZEOFAPPTS	(1024*4)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)] ATTR_STACK;

void startup_iperf_server(void)
{
	tprintf("Startup iperf server\n");

	task_exec(iperf_server_task, "iperf-server", TASK_PRIORITY_APP_HIGH, &tcb,
		  stack, SIZEOFAPPTS, 0);
}
