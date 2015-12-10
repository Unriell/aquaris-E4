/*
 * (c) MediaTek Inc. 2010
 */

#ifndef BUILD_LK
#include <linux/string.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#ifdef BUILD_LK
#define LCD_DBG printf	
#else
#define LCD_DBG printk	
#endif

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(800)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0x00   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define   TRUE     1
#endif
 
#ifndef FALSE
    #define   FALSE    0
#endif

#define LCM_ID1_PIN GPIO5|0x80000000
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

//static kal_bool IsFirstBoot = KAL_TRUE;

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[120];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

	//NT35512_14.05.05
	//{0xFF,	4,	{0xAA,0x55,0xA5,0x80}},
	//{0xF4,	15,	{0x80,0x00,0x55,0x00,0x04,0x31,0x20,0x10,0x45,0x32,0x31,0x73,0xFF,0x75,0x0A}},
/*	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x02}},
	{0xB7,	1,	{0x01}},
	{0xB8,	1,	{0x07}},

	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x01}},
	{0xB0,	1,	{0x0D}},
	{0xB6,	1,	{0x44}},
	{0xB1,	1,	{0x0D}},
	{0xB7,	1,	{0x34}},
	{0xB2,	1,	{0x00}},
	{0xB8,	1,	{0x24}},
	{0xBF,	1,	{0x01}},
	{0xB3,	3,	{0x0F,0x0F,0x0F}},
	{0xB9,	1,	{0x34}},
	{0xB5,	1,	{0x08}},
	{0xC2,	1,	{0x03}},
	{0xBA,	1,	{0x34}},
	{0xBC,	3,	{0x00,0x78,0x00}},
	{0xBD,	3,	{0x00,0x78,0x00}},
	{0xBE,	4,	{0x00, 0x90, 0x00, 0x90}},

	{0xD1, 52, {0x00,0x00,0x00,0x03,0x00,0x09,0x00,0x14,0x00,0x25,0x00,0x4B,0x00,0x6F,0x00,0xAC,0x00,0xE0,0x01,0x2C,0x01,0x61,0x01,0xAA,0x01,0xE1,0x01,0xE2,0x02,0x16,0x02,0x49,0x02,0x63,0x02,0x82,0x02,0x96,0x02,0xAE,0x02,0xBE,0x02,0xD0,0x02,0xDE,0x02,0xF1,0x03,0x1B,0x03,0xEE}},

	{0xF0,	5,	{0x55,0xAA,0x52,0x08,0x00}},
	{0xB1,	2,	{0xFC,0x00}},
	{0xB5,	1,	{0x50}},
	{0xB6,	1,	{0x05}},
	{0xB7,	2,	{0x70,0x70}},
	{0xB8,	4,	{0x01,0x03,0x03,0x03}},
	{0xBC,	3,	{0x02,0x00,0x00}},
	{0xC9,	5,	{0xC0,0x3C,0x50,0x50,0x50}},
	{0x35,	1,	{0x00}},

	{0xFF,	4,	{0xAA,0x55,0x69,0x10}},
	{0x6F,	1,	{0x20}},
	{0xFA,	1,	{0x0F}},
	
	{0x11,	0,	{0x00}},
	{REGFLAG_DELAY, 120, {}},
	
	{0x29,	0,	{0x00}},
	//{REGFLAG_DELAY, 20, {}},

	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
*/
//pengchengwei  F4013  hsd  3.97
//LV2 Page 1 enable
{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
 
//AVDD Set AVDD 5.2V
{0xB0,3,{0x0A,0x0A,0x0A}},  //0a
 
//AVDD ratio
{0xB6,3,{0x34,0x34,0x34}},
 
//AVEE  -5.2V
{0xB1,3,{0x0A,0x0A,0x0A}},  //08
 
//AVEE ratio
{0xB7,3,{0x35,0x35,0x35}},
 
//VCL  -2.5V
{0xB2,3,{0x00,0x00,0x00}},
 
//VCL ratio
{0xB8,3,{0x24,0x24,24}},   //24 
 
//VGH 15V  
// {0xBF,1,{0x00}},
{0xB3,3,{0x08,0x08,0x08}},//08  
 
//VGH ratio
{0xB9,3,{0x34,0x34,0x34}},  //24  37
 
//VGLX ratio
{0xBA,3,{0x24,0x24,0x24}},   //34
 
//VGMP/VGSP 4.7V/0V
{0xBC,3,{0x00,0x78,0x00}},    //A8
 
//VGMN/VGSN -4.7V/0V
{0xBD,3,{0x00,0x78,0x00}},
 
//VCOM -2.0375V 
{0xBE,2,{0x00,0x8C}},  //6D//76
 
//Gamma Setting
{0xD1,52,{0x00,0x06,0x00,0x07,0x00,0x0E,0x00,0x22,0x00,0x3B,0x00,0x71,0x00,0x9F,0x00,0xE2,0x01,0x12,0x01,0x57,0x01,0x88,0x01,0xCE,0x02,0x07,0x02,0x08,0x02,0x39,0x02,0x6C,0x02,0x87,0x02,0xA6,0x02,0xBA,0x02,0xD2,0x02,0xE2,0x02,0xF7,0x03,0x06,0x03,0x1E,0x03,0x55,0x03,0xFF}},
{0xD2,52,{0x00,0x06,0x00,0x07,0x00,0x0E,0x00,0x22,0x00,0x3B,0x00,0x71,0x00,0x9F,0x00,0xE2,0x01,0x12,0x01,0x57,0x01,0x88,0x01,0xCE,0x02,0x07,0x02,0x08,0x02,0x39,0x02,0x6C,0x02,0x87,0x02,0xA6,0x02,0xBA,0x02,0xD2,0x02,0xE2,0x02,0xF7,0x03,0x06,0x03,0x1E,0x03,0x55,0x03,0xFF}},
{0xD3,52,{0x00,0x06,0x00,0x07,0x00,0x0E,0x00,0x22,0x00,0x3B,0x00,0x71,0x00,0x9F,0x00,0xE2,0x01,0x12,0x01,0x57,0x01,0x88,0x01,0xCE,0x02,0x07,0x02,0x08,0x02,0x39,0x02,0x6C,0x02,0x87,0x02,0xA6,0x02,0xBA,0x02,0xD2,0x02,0xE2,0x02,0xF7,0x03,0x06,0x03,0x1E,0x03,0x55,0x03,0xFF}},
{0xD4,52,{0x00,0x06,0x00,0x07,0x00,0x0E,0x00,0x22,0x00,0x3B,0x00,0x71,0x00,0x9F,0x00,0xE2,0x01,0x12,0x01,0x57,0x01,0x88,0x01,0xCE,0x02,0x07,0x02,0x08,0x02,0x39,0x02,0x6C,0x02,0x87,0x02,0xA6,0x02,0xBA,0x02,0xD2,0x02,0xE2,0x02,0xF7,0x03,0x06,0x03,0x1E,0x03,0x55,0x03,0xFF}},
{0xD5,52,{0x00,0x06,0x00,0x07,0x00,0x0E,0x00,0x22,0x00,0x3B,0x00,0x71,0x00,0x9F,0x00,0xE2,0x01,0x12,0x01,0x57,0x01,0x88,0x01,0xCE,0x02,0x07,0x02,0x08,0x02,0x39,0x02,0x6C,0x02,0x87,0x02,0xA6,0x02,0xBA,0x02,0xD2,0x02,0xE2,0x02,0xF7,0x03,0x06,0x03,0x1E,0x03,0x55,0x03,0xFF}},
{0xD6,52,{0x00,0x06,0x00,0x07,0x00,0x0E,0x00,0x22,0x00,0x3B,0x00,0x71,0x00,0x9F,0x00,0xE2,0x01,0x12,0x01,0x57,0x01,0x88,0x01,0xCE,0x02,0x07,0x02,0x08,0x02,0x39,0x02,0x6C,0x02,0x87,0x02,0xA6,0x02,0xBA,0x02,0xD2,0x02,0xE2,0x02,0xF7,0x03,0x06,0x03,0x1E,0x03,0x55,0x03,0xFF}},
 
//LV2 Page 0 enable
{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
 
//Display control
{0xB1,2,{0xFC,0x00}},
{0xB2,1,{ 0x71}},//00
//480x800
{0xB5,1,{ 0x50}},
 
//Source hold time
{0xB6,1,{0x03}},//05
 
//Gate EQ control
{0xB7,2,{0x70,0x70}},//74
 
//Source EQ control (Mode 2)
{0xB8,4,{0x01,0x03,0x03,0x03}},
 
//Inversion mode  (Column)
{0xBC,3,{0x00,0x00,0x00}}, //02
 
//Timing control 8phase dual side/4H/4delay/RST_EN
{0xC9,5,{0xD0,0x02,0x50,0x50,0x50}},
//{0x3A,1,{0x55}},
{0x35,1,{0x00}},
 
 //**********bist mode*************/*
/*
{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
{0xe2,2,{0x00,0x04}},
{0xe4,4,{0x87,0x78,0x02,0x40}},*/




{0x11, 1 ,{0x00}}, 
{REGFLAG_DELAY, 120, {}},
 
{0x29, 1 ,{0x00}}, 
{REGFLAG_DELAY, 50, {}},
//{0x2C, 1 ,{0x00}}, 

       
{REGFLAG_END_OF_TABLE, 0x00, {}}
};



static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
                  {REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},
                 {REGFLAG_DELAY, 10, {}},
    // Sleep Mode On
	{0x10, 0, {0x00}},
                   {REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};





static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				//MDELAY(10);//soso add or it will fail to send register
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{

		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE; 
        #endif
#ifdef SLT_DEVINFO_LCM
		params->module="yushun";
		params->vendor="yushun";
		params->ic="NT35512";
		params->info="800*480";
#endif		
		// DSI
		// Command mode setting
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
		params->dsi.vertical_sync_active				= 4;// 3    2
		params->dsi.vertical_backporch					= 15;// 20   1
		params->dsi.vertical_frontporch					= 15; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 5;// 50  2
		params->dsi.horizontal_backporch				= 0x2a;//0x1d;
		params->dsi.horizontal_frontporch				= 0x2a;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		params->dsi.PLL_CLOCK=180;
}

static void lcm_init(void)
{
	unsigned int data_array[64];
    //LCD_DBG("[NT35512] enter the %s\n",__func__);
    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);//Must > 10ms
    SET_RESET_PIN(1);
    MDELAY(120);//Must > 120ms
    
	//IsFirstBoot = KAL_TRUE;

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	//LCD_DBG("[NT35512]enter the %s\n", __func__);
   unsigned int array[8];
/*    array[0] = 0x00002200;
    dsi_set_cmdq(array, 1, 1);
    MDELAY(10);
*/
   push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(10);//Must > 10ms
    SET_RESET_PIN(1);
    MDELAY(120);//Must > 120ms

}


static void lcm_resume(void)
{
	LCD_DBG("[NT35512]enter the %s\n", __func__);
	lcm_init();
        //push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);

}
static unsigned int lcm_compare_id(void)
{
	int id = -1;
	int buf = 0;
	unsigned char buffer[3];
	unsigned int array[16];
	
	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

	array[0] = 0x00063902;
	array[1] = 0x52aa55f0;
	array[2] = 0x00000108;
	dsi_set_cmdq(array, 3, 1);
	MDELAY(10);
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xc5, buffer, 2);
	buf = buffer[0]<<8|buffer[1];
/*
	array[0] = 0x00033700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0x04, buffer, 3);
	id = buffer[1]; //we only need ID
*/
	if(buf == 0x5512)
	{
		mt_set_gpio_mode(LCM_ID1_PIN,GPIO_MODE_00);
		mt_set_gpio_dir(LCM_ID1_PIN,GPIO_DIR_IN);
		id = mt_get_gpio_in(LCM_ID1_PIN);
	}
#if defined(BUILD_LK)
	printf("\n\n\n\n[soso]%s, buf = 0x%x,id = 0x%x\n", __func__, buf,id);
#endif
    return (id == 1)?1:0;

}

static unsigned int lcm_esd_check(void)
{
	unsigned char buffer[4];
	unsigned int array[4];
    	//LCD_DBG("lcm_esd_check enter!\n");
	array[0] = 0x00013700;
	dsi_set_cmdq(array,1,1);

	read_reg_v2(0x0A,buffer,1);
        LCD_DBG("buffer[0]:%x\n", buffer[0]);

	if(buffer[0] == 0x9c)
		{return FALSE;}
	else
		{return TRUE;}
}

static unsigned int lcm_esd_recover(void) 
{ 
	lcm_init();
	
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
	
	return TRUE; 
} 

LCM_DRIVER nt35512_wvga_dsi_vdo_yushun2_lcm_drv = 
{
    .name			= "nt35512_wvga_dsi_vdo_yushun2",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
	.esd_check     = lcm_esd_check,
	.esd_recover	= lcm_esd_recover,
};





