/** @file
    @brief	ネットワーク設定ダイアログボックス

    @date	2019.09.01
    @author	Takashi SHUDO
*/

#include "sysconfig.h"

#include "netset.h"
#include "str.h"
#include "font.h"
#include "tprintf.h"
#include "net.h"

#include "lwip/ip.h"
#include "lwip/dns.h"

#include "ui_style.h"
#include "ui_switch.h"
#include "ui_button.h"
#include "ui_edittext.h"
#include "tenkey.h"

#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
#define UI_SWITCH_WIDTH		96
#define UI_SWITCH_HEIGHT	32
#define NETSET_FONT	"num24x32"
#define EDITCHAR_WIDTH	24
#define DEFCHAR_HEIGHT	(24 + 4)
#elif GSC_GRAPHICS_DISPLAY_WIDTH >= 480
#define UI_SWITCH_WIDTH		64
#define UI_SWITCH_HEIGHT	24
#define NETSET_FONT	"12x16"
#define EDITCHAR_WIDTH	12
#define DEFCHAR_HEIGHT	(16 + 4)
#elif GSC_GRAPHICS_DISPLAY_WIDTH >= 320
#define UI_SWITCH_WIDTH		52
#define UI_SWITCH_HEIGHT	24
#define NETSET_FONT	"8x16"
#define EDITCHAR_WIDTH	8
#define DEFCHAR_HEIGHT	(16 + 4)
#endif

#if GSC_GRAPHICS_DISPLAY_HEIGHT == 480
#define PANEL_Y		40
#define BUTTON_TOP	0
#define BUTTON_HEIGHT	64
#define EDITCHAR_HEIGHT	40
#define IP_TEXT_Y	64
#define IP_ITEM_HEIGHT	72
#define TEXT_AREA_MARGIN	4
#define BTN_POS_Y_SET	372
#elif GSC_GRAPHICS_DISPLAY_HEIGHT == 320
#define PANEL_Y		24
#define BUTTON_TOP	24
#define BUTTON_HEIGHT	48
#define EDITCHAR_HEIGHT	22
#define IP_TEXT_Y	48
#define IP_ITEM_HEIGHT	48
#define TEXT_AREA_MARGIN	3
#define BTN_POS_Y_SET	244
#elif GSC_GRAPHICS_DISPLAY_HEIGHT == 272
#define PANEL_Y		0
#define BUTTON_TOP	0
#define BUTTON_HEIGHT	48
#define EDITCHAR_HEIGHT	22
#define IP_TEXT_Y	48
#define IP_ITEM_HEIGHT	48
#define TEXT_AREA_MARGIN	3
#define BTN_POS_Y_SET	(GSC_GRAPHICS_DISPLAY_HEIGHT - BUTTON_HEIGHT - BUTTON_INTERVAL)
#elif GSC_GRAPHICS_DISPLAY_HEIGHT == 240
#define PANEL_Y		0
#define BUTTON_TOP	0
#define BUTTON_HEIGHT	24
#define EDITCHAR_HEIGHT	20
#define IP_TEXT_Y	48
#define IP_ITEM_HEIGHT	46
#define TEXT_AREA_MARGIN	2
#define BTN_POS_Y_SET	(GSC_GRAPHICS_DISPLAY_HEIGHT - BUTTON_HEIGHT - BUTTON_INTERVAL)
#endif

#if GSC_GRAPHICS_DISPLAY_WIDTH == 320
#define PANEL_X		0
#define BUTTON_WIDTH	64
#define IPTEXT_X	0
#elif GSC_GRAPHICS_DISPLAY_WIDTH == 480
#define PANEL_X		8
#define BUTTON_WIDTH	96
#define IPTEXT_X	8
#elif GSC_GRAPHICS_DISPLAY_WIDTH == 800
#define PANEL_X		16
#define BUTTON_WIDTH	128
#define IPTEXT_X	8
#else
#define PANEL_X		0
#define BUTTON_WIDTH	48
#define IPTEXT_X	8
#endif

#define BUTTON_LEFT	160
#define BUTTON_INTERVAL	4

#define SET_BUTTON_WIDTH	BUTTON_WIDTH
#define CANCEL_BUTTON_WIDTH	BUTTON_WIDTH

#define BUTTON_X	16

#define BTN_POS_X_SET	(PANEL_X + BUTTON_X + SET_BUTTON_WIDTH + (BUTTON_INTERVAL*2))

#define BTN_POS_X_CANCEL	(PANEL_X + BUTTON_X)
#define BTN_POS_Y_CANCEL	(BTN_POS_Y_SET)

static struct st_ui_switch uisw_dhcp = {
	.id	= 1,
	.text_area = {
		.pos.x	= PANEL_X + IPTEXT_X,
		.pos.y	= PANEL_Y,
		.sur.width	= UI_SWITCH_WIDTH,
		.sur.height	= UI_SWITCH_HEIGHT,
	},
	.font_name	= GSC_FONTS_DEFAULT_FONT,
	.name		= (unsigned char *)"DHCP",
	.status	= UI_SWITCH_ST_NORMAL,
	.value	= 0,
};

#define IP_TEXT_WIDTH	(EDITCHAR_WIDTH*3)

#define IP_SUBNETMASK_Y		(IP_TEXT_Y + IP_ITEM_HEIGHT)
#define IP_DEFAULTGATEWAY_Y	(IP_TEXT_Y + IP_ITEM_HEIGHT*2)
#define IP_DNSSERVER_Y		(IP_TEXT_Y + IP_ITEM_HEIGHT*3)


uchar ipaddress_str[4][4] = {
	"192", "168", "1", "100"
};

static struct st_ui_edittext te_ipaddress[4] = {
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 0),
			.pos.y	= PANEL_Y + IP_TEXT_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.flg_active	= 1,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= ipaddress_str[0],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 1),
			.pos.y	= PANEL_Y + IP_TEXT_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= ipaddress_str[1],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 2),
			.pos.y	= PANEL_Y + IP_TEXT_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= ipaddress_str[2],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 3),
			.pos.y	= PANEL_Y + IP_TEXT_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= ipaddress_str[3],
		.max_text_length = 3,
	},
};


uchar subnetmask_str[4][4] = {
	"255", "255", "255", "0"
};

static struct st_ui_edittext te_subnetmask[4] = {
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 0),
			.pos.y	= PANEL_Y + IP_SUBNETMASK_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= subnetmask_str[0],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 1),
			.pos.y	= PANEL_Y + IP_SUBNETMASK_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= subnetmask_str[1],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 2),
			.pos.y	= PANEL_Y + IP_SUBNETMASK_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= subnetmask_str[2],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 3),
			.pos.y	= PANEL_Y + IP_SUBNETMASK_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= subnetmask_str[3],
		.max_text_length = 3,
	},
};


uchar defaultgateway_str[4][4] = {
	"192", "168", "1", "1"
};

static struct st_ui_edittext te_defaultgateway[4] = {
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 0),
			.pos.y	= PANEL_Y + IP_DEFAULTGATEWAY_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= defaultgateway_str[0],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 1),
			.pos.y	= PANEL_Y + IP_DEFAULTGATEWAY_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= defaultgateway_str[1],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 2),
			.pos.y	= PANEL_Y + IP_DEFAULTGATEWAY_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= defaultgateway_str[2],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 3),
			.pos.y	= PANEL_Y + IP_DEFAULTGATEWAY_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= defaultgateway_str[3],
		.max_text_length = 3,
	},
};


uchar dnsserver_str[4][4] = {
	"192", "168", "1", "1"
};

static struct st_ui_edittext te_dnsserver[4] = {
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 0),
			.pos.y	= PANEL_Y + IP_DNSSERVER_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= dnsserver_str[0],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 1),
			.pos.y	= PANEL_Y + IP_DNSSERVER_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= dnsserver_str[1],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 2),
			.pos.y	= PANEL_Y + IP_DNSSERVER_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= dnsserver_str[2],
		.max_text_length = 3,
	},
	{
		.view_area = {
			.pos.x	= PANEL_X + IPTEXT_X + ((IP_TEXT_WIDTH + EDITCHAR_WIDTH*2) * 3),
			.pos.y	= PANEL_Y + IP_DNSSERVER_Y,
			.sur.width	= IP_TEXT_WIDTH + (TEXT_AREA_MARGIN * 2),
			.sur.height	= EDITCHAR_HEIGHT,
		},
		.text_area_margin	= TEXT_AREA_MARGIN,
		.font_name	= NETSET_FONT,
		.flg_enable_multibyte_select	= 1,
		.text		= dnsserver_str[3],
		.max_text_length = 3,
	},
};


static struct st_ui_edittext *setip_textedit[] = {
	&te_ipaddress[0],
	&te_ipaddress[1],
	&te_ipaddress[2],
	&te_ipaddress[3],
	&te_subnetmask[0],
	&te_subnetmask[1],
	&te_subnetmask[2],
	&te_subnetmask[3],
	&te_defaultgateway[0],
	&te_defaultgateway[1],
	&te_defaultgateway[2],
	&te_defaultgateway[3],
	&te_dnsserver[0],
	&te_dnsserver[1],
	&te_dnsserver[2],
	&te_dnsserver[3],
	0
};

#define BTN_ID_SET	12
#define BTN_ID_CANCEL	11

static struct st_ui_button ui_btn_set = {
	.id	= BTN_ID_SET,
	.view_area = {
		.pos.x	= BTN_POS_X_SET,
		.pos.y	= BTN_POS_Y_SET,
		.sur.width	= SET_BUTTON_WIDTH,
		.sur.height	= BUTTON_HEIGHT,
	},
	.name	= "SET",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_cancel = {
	.id	= BTN_ID_CANCEL,
	.view_area = {
		.pos.x	= BTN_POS_X_CANCEL,
		.pos.y	= BTN_POS_Y_CANCEL,
		.sur.width	= CANCEL_BUTTON_WIDTH,
		.sur.height	= BUTTON_HEIGHT,
	},
	.name	= "CANCEL",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button *ui_tenbutton_view[] = {
	&ui_btn_set,
	&ui_btn_cancel,
	0
};

static struct st_rect setting_view_area = {
	0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT
};

static struct st_ui_edittext *last_te = 0;

static int		flg_enable_dhcp;
static ip_addr_t	ipaddress;
static ip_addr_t	subnetmask;
static ip_addr_t	default_gateway;
static ip_addr_t	dns_server[DNS_MAX_SERVERS];

extern struct netif netif;

static void get_network_setting(void)
{
#ifdef GSC_DEV_ENABLE_ETHER
	int i;
	const ip4_addr_t *addr;

	if(dhcp_status() != NET_DHCP_STAT_DISABLE) {
		flg_enable_dhcp = 1;
	} else {
		flg_enable_dhcp = 0;
	}

	addr = netif_ip4_addr(&netif);
	ipaddress.addr = addr->addr;

	addr = netif_ip4_netmask(&netif);
	subnetmask.addr = addr->addr;

	addr = netif_ip4_gw(&netif);
	default_gateway.addr = addr->addr;

	for(i=0; i<DNS_MAX_SERVERS; i++) {
		addr = dns_getserver(i);
		dns_server[i].addr = addr->addr;
		//tprintf("DNS%d %08X\n", i+1, dns_server[i].addr);
	}
#endif
}

static void set_network_setting(void)
{
	if(flg_enable_dhcp != 0) {
		enable_dhcp();
	} else {
		int i;
		disable_dhcp();
		netif_set_addr(&netif,
			       &ipaddress,
			       &subnetmask,
			       &default_gateway);
		for(i=0; i<DNS_MAX_SERVERS; i++) {
			dns_setserver(i, &dns_server[i]);
		}
	}
}

#ifdef GSC_COMP_ENABLE_FATFS
#include "appsetting.h"
#ifndef GSC_UI_NETWORK_CONFFILE
#define GSC_UI_NETWORK_CONFFILE	"0:network.cfg"	/// $gsc ネットワークコンフィグ設定ファイル名
#endif

static struct st_conf_header network_conf[] = {
	{ "DHCP", CFGTYPE_INT, &flg_enable_dhcp },
	{ "IPADDRESS", CFGTYPE_IPADDRESS, &ipaddress },
	{ "SUBNETMASK", CFGTYPE_IPADDRESS, &subnetmask },
	{ "DEFAULTGATEWAY", CFGTYPE_IPADDRESS, &default_gateway },
	{ "DNSSERVER1", CFGTYPE_IPADDRESS, &dns_server[0] },
	{ "DNSSERVER2", CFGTYPE_IPADDRESS, &dns_server[1] },
	{ 0, 0, 0 }
};

int load_network_setting(void)
{
	int rt = 0;

	rt = load_appsetting((uchar *)GSC_UI_NETWORK_CONFFILE, network_conf);
	if(rt < 0) {
		save_network_setting();
	} else {
		set_network_setting();
	}

	return rt;
}

void save_network_setting(void)
{
	save_appsetting((uchar *)GSC_UI_NETWORK_CONFFILE, network_conf);
}
#endif // GSC_COMP_ENABLE_FATFS

void prepare_netset(void)
{
	int i;

	get_network_setting();

	uisw_dhcp.value = flg_enable_dhcp;
	for(i=0; i<4; i++) {
		te_ipaddress[i].flg_uneditable = uisw_dhcp.value;
		te_subnetmask[i].flg_uneditable = uisw_dhcp.value;
		te_defaultgateway[i].flg_uneditable = uisw_dhcp.value;
		te_dnsserver[i].flg_uneditable = uisw_dhcp.value;
	}

	for(i=0; i<4; i++) {
		tsprintf((char *)ipaddress_str[i], "%d", ip4_addr_get_byte_val(ipaddress, i));
		tsprintf((char *)subnetmask_str[i], "%d", ip4_addr_get_byte_val(subnetmask, i));
		tsprintf((char *)defaultgateway_str[i], "%d", ip4_addr_get_byte_val(default_gateway, i));
		tsprintf((char *)dnsserver_str[i], "%d", ip4_addr_get_byte_val(dns_server[0], i));
	}
	//tprintf("DNS1 = %s.%s.%s.%s\n", dnsserver_str[0], dnsserver_str[1], dnsserver_str[2], dnsserver_str[3]);
}

void draw_netset(void)
{
	int i;

	set_mode_tenkey(1);
	set_forecolor(UI_BACK_COLOR);
	draw_fill_rect(&setting_view_area);

	set_backcolor(UI_BACK_COLOR);
	set_forecolor(UI_NORMAL_FORE_COLOR);
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	set_font_by_name(GSC_FONTS_DEFAULT_FONT);
	draw_str(PANEL_X + IPTEXT_X, PANEL_Y + IP_TEXT_Y - DEFCHAR_HEIGHT, (uchar *)"IP address", 256);
	draw_str(PANEL_X + IPTEXT_X, PANEL_Y + IP_SUBNETMASK_Y - DEFCHAR_HEIGHT, (uchar *)"Subnet mask", 256);
	draw_str(PANEL_X + IPTEXT_X, PANEL_Y + IP_DEFAULTGATEWAY_Y - DEFCHAR_HEIGHT, (uchar *)"Default gateway", 256);
	draw_str(PANEL_X + IPTEXT_X, PANEL_Y + IP_DNSSERVER_Y - DEFCHAR_HEIGHT, (uchar *)"DNS server", 256);

	set_font_by_name(NETSET_FONT);
	for(i=0; i<3; i++) {
		draw_char(te_ipaddress[i].view_area.pos.x + EDITCHAR_WIDTH*4,
			  te_ipaddress[i].view_area.pos.y + TEXT_AREA_MARGIN, '.');
		draw_char(te_subnetmask[i].view_area.pos.x + EDITCHAR_WIDTH*4,
			  te_subnetmask[i].view_area.pos.y + TEXT_AREA_MARGIN, '.');
		draw_char(te_defaultgateway[i].view_area.pos.x + EDITCHAR_WIDTH*4,
			  te_defaultgateway[i].view_area.pos.y + TEXT_AREA_MARGIN, '.');
		draw_char(te_dnsserver[i].view_area.pos.x + EDITCHAR_WIDTH*4,
			  te_dnsserver[i].view_area.pos.y + TEXT_AREA_MARGIN, '.');
	}

	draw_ui_switch(&uisw_dhcp);
	draw_ui_edittext_list(setip_textedit);
	draw_ui_button_list(ui_tenbutton_view);

	draw_tenkey();
}

static int str2addr(ip4_addr_t *addr, uchar str[4][4])
{
	char addrstr[16];
	int rt = 0;

	tsprintf(addrstr, "%s.%s.%s.%s", (char *)str[0], (char *)str[1], (char *)str[2], (char *)str[3]);
	rt = ip4addr_aton(addrstr, addr);

	return rt;
}

static int check_addrstr(uchar *str)
{
	int i = dstoi(str);

	if(i > 255) {
		return -1;
	} else {
		return 0;
	}
}

static int check_addr(struct st_ui_edittext *te)
{
	int i;

	for(i=0; i<4; i++) {
		if(check_addrstr(te->text) != 0) {
			select_all_ui_edittext(te);
			return -1;
		}
		te ++;
	}

	return 0;
}

int proc_netset(struct st_sysevent *event)
{
	struct st_button_event obj_evt;
	int rt = 0;

	rt = proc_ui_switch(&uisw_dhcp, event);
	switch(rt) {
	case UI_SWITCH_EVT_ON:
		uneditable_ui_edittext_list(setip_textedit);
		break;

	case UI_SWITCH_EVT_OFF:
		editable_ui_edittext_list(setip_textedit);
		inactivate_ui_edittext_list(setip_textedit);
		select_all_ui_edittext(&te_ipaddress[0]);
		break;

	case UI_SWITCH_EVT_NULL:
	default:
		break;
	}

	if(proc_ui_edittext_list(setip_textedit, event, &last_te) != 0) {
		// [TODO]
	}

	if(proc_ui_button_list(&obj_evt, ui_tenbutton_view, event) != 0) {
		switch(obj_evt.id) {
		case BTN_ID_SET:
			if(obj_evt.what == UI_BUTTON_EVT_PULL) {
				unactive_ui_edittext_list(setip_textedit);

				if(check_addr(te_ipaddress) != 0) {
					return 0;
				}
				if(check_addr(te_subnetmask) != 0) {
					return 0;
				}
				if(check_addr(te_defaultgateway) != 0) {
					return 0;
				}
				if(check_addr(te_dnsserver) != 0) {
					return 0;
				}

				if(str2addr(&ipaddress, ipaddress_str) == 0) {
					return 0;
				}
				if(str2addr(&subnetmask, subnetmask_str) == 0) {
					return 0;
				}
				if(str2addr(&default_gateway, defaultgateway_str) == 0) {
					return 0;
				}
				if(str2addr(&dns_server[0], dnsserver_str) == 0) {
					return 0;
				}

				flg_enable_dhcp = uisw_dhcp.value;
				return 1;
			}
			break;

		case BTN_ID_CANCEL:
			if(obj_evt.what == UI_BUTTON_EVT_PULL) {
				return -1;
			}
			break;

		default:
			break;
		}
	}

	if(proc_tenkey(&obj_evt, event) != 0) {
		switch(obj_evt.id) {
		case BTN_ID_0:
		case BTN_ID_1:
		case BTN_ID_2:
		case BTN_ID_3:
		case BTN_ID_4:
		case BTN_ID_5:
		case BTN_ID_6:
		case BTN_ID_7:
		case BTN_ID_8:
		case BTN_ID_9:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				set_char_ui_edittext_list(setip_textedit, '0' + obj_evt.id - BTN_ID_0, 1);
			}
			break;

		case BTN_ID_BACK:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				move_cursor_ui_edittext_list(setip_textedit, -1, 1);
			}
			break;

		case BTN_ID_FORWARD:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				move_cursor_ui_edittext_list(setip_textedit, 1, 1);
			}
			break;

		case BTN_ID_BACKSPACE:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				backspace_char_ui_edittext_list(setip_textedit);
			}
			break;

		case BTN_ID_DELETE:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				delete_char_ui_edittext_list(setip_textedit);
			}
			break;

		default:
			break;
		}
	}

	return 0;
}

int do_netset(timeset_proc proc)
{
	while(1) {
		struct st_sysevent event;
		int rt = 0;

		get_event(&event, 50);

		rt = proc_netset(&event);
		if(rt != 0) {
			return rt;
		}
		if(proc !=0) {
			rt = proc(&event);
		}
		if(rt != 0) {
			return rt;
		}
	}

	return 0;
}

int open_netset_dialog(timeset_proc proc)
{
	int rt = 0;

	prepare_netset();
	draw_netset();
	inactivate_ui_edittext_list(setip_textedit);
	select_all_ui_edittext(&te_ipaddress[0]);

	rt = do_netset(proc);

	if(rt == 1) {
		set_network_setting();
		save_network_setting();
	}

	return rt;
}
