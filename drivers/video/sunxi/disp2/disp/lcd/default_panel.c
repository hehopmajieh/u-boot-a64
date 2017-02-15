#include "default_panel.h"
#include <i2c.h>
#include "anx6345.h"

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);
//static disp_vdevice_source_ops tv_source_ops;

static s32 anx_i2c_write0(u8 sub_addr, u8 data);
static s32 anx_i2c_read0(u8 sub_addr, u8 *data);
static s32 anx_i2c_write1(u8 sub_addr, u8 data);
static s32 anx_i2c_read1(u8 sub_addr, u8 *data);

//`:anx_i2c_write(u8 sub_addr, u8 data);
//s32 tv_i2c_read(u8 sub_addr, u8 *data);
s32 anx_i2c_exit(void);


static unsigned short normal_i2c[] = {0x38,0x39};
int tv_i2c_id = 0;
int tv_i2c_used = 1;

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);
int edp_i2c_id = 0;

//0x70 = 0x38  port0
//	0x39   port1

static int  anx_i2c_init(void)
{

            i2c_init( 400000,CONFIG_SYS_I2C_SLAVE);         //twi1 for cvbs
    return 0;
}

int tv_i2c_exit(void)
{
    if(tv_i2c_used){
           //i2c_exit();
           return 0;
    }
        return -1;
}

static s32 anx_i2c_write0(__u8 sub_addr, __u8 data)
{
    if(tv_i2c_used){
           __s8 ret= -10;
       //    ret = i2c_write(tv_i2c_id, normal_i2c[0],sub_addr,1,&data,1);
	 ret =  i2c_write(tv_i2c_id, normal_i2c[0], sub_addr, &data, 1);

           return ret;
    }
        return -1;
}

static s32 anx_i2c_read0(__u8 sub_addr, __u8 *data)
{
    if(tv_i2c_used){
           __s8 ret= -10;
//       ret = i2c_read(tv_i2c_id, normal_i2c[0],sub_addr,1,data,1);
	 ret = i2c_read(tv_i2c_id, normal_i2c[0],sub_addr,data,1);

           return ret;
    }
        return -1;
}

static s32 anx_i2c_write1(__u8 sub_addr, __u8 data)
{
    if(tv_i2c_used){
           __s8 ret= -10;
            ret =  i2c_write(tv_i2c_id, normal_i2c[1], sub_addr, &data, 1);
           return ret;
    }
        return -1;
}

static s32 anx_i2c_read1(__u8 sub_addr, __u8 *data)
{
    if(tv_i2c_used){
           __s8 ret= -10;
       //et = i2c_read(tv_i2c_id, normal_i2c[1],sub_addr,1,data,1);
	 ret = i2c_read(tv_i2c_id, normal_i2c[1],sub_addr,data,1);
           return ret;
    }
        return -1;
}



 void anx6345_init(unsigned int i2c_bus)
{

  __u8 val = 0x00;
        __s32 i = 0;
        __u8 cnt = 50;
         val = 0x30;
	anx_i2c_init();
        val = 0x00;
        anx_i2c_write1(0x05,val);
        anx_i2c_read1(0x01,&val);
        val = 00;
        anx_i2c_write1( SP_POWERD_CTRL_REG, val);
        val = SP_TX_RST_HW_RST;
        anx_i2c_write1( SP_TX_RST_CTRL_REG, val);
        udelay(1000);
        val = 0x00;
        anx_i2c_write1( SP_TX_RST_CTRL_REG, val);
        //reset i2c registers
        val = SP_TX_I2C_REG_RST;
        anx_i2c_write1(  SP_TX_RST_CTRL2_REG, val);
        val = 0x40;
        udelay(5000);
        anx_i2c_write1(  SP_TX_RST_CTRL2_REG, val);
        val = 0x01;
        anx_i2c_write1(  0xd9, val);
        anx_i2c_read1( SP_POWERD_CTRL_REG,&val);
        val = 0x00;
        anx_i2c_write1( SP_POWERD_CTRL_REG, val);
        //get chip ID. Make sure I2C is OK
        anx_i2c_read1( SP_TX_DEV_IDH_REG,&val );
        if (val==0x63)
                printf("Chip found\n");

        //for clocl detect
        for(i=0;i<100;i++)
                {
                anx_i2c_read0( SP_TX_SYS_CTRL1_REG,&val);
                anx_i2c_write0( SP_TX_SYS_CTRL1_REG, val);
                anx_i2c_read0( SP_TX_SYS_CTRL1_REG,&val);
                if((val&SP_TX_SYS_CTRL1_DET_STA)!=0)
                {
                        printf("clock is detected.\n");
                        break;
                }

                udelay(10000);
        }
for(i=0;i<50;i++)
        {
                anx_i2c_read0( SP_TX_SYS_CTRL2_REG,&val);
                anx_i2c_write0( SP_TX_SYS_CTRL2_REG, val);
                anx_i2c_read0( SP_TX_SYS_CTRL2_REG,&val);
                if((val&SP_TX_SYS_CTRL2_CHA_STA)==0)
                {
                        printf("clock is stable. \n");
                        break;
                }
                udelay(100000);
        }
        //VESA range, 8bits BPC, RGB 
        val = 0x00;//0x10
        anx_i2c_write1( SP_TX_VID_CTRL2_REG, val);
        //RK_EDP chip analog setting
        val = 0x07;
        anx_i2c_write0( SP_TX_PLL_CTRL_REG, val);
        val = 0x19;
        anx_i2c_write1( PLL_FILTER_CTRL3, val);
        val = 0xd9;
        anx_i2c_write1( 0xE6, val);  // not sure ? DP_TX_PLL_CTRL3

        //Select AC mode
        val = 0x40; //0x40;
        anx_i2c_write1( SP_TX_RST_CTRL2_REG, val);
	//Set bist format 1366 768
        val = 0x20;//0x58; //0x20;
        anx_i2c_write1( SP_TX_TOTAL_LINEL_REG, val);
        val = 0x03;//0x04;//0x03;
        anx_i2c_write1(  SP_TX_TOTAL_LINEH_REG, val);

        val = 0x00;//0x38;//0x00;
        anx_i2c_write1(  SP_TX_ACT_LINEL_REG, val);
        val = 0x03;//0x04;//0x03;
        anx_i2c_write1(  SP_TX_ACT_LINEH_REG,val);
        val = 0x14;
        anx_i2c_write1(  SP_TX_VF_PORCH_REG, val);
        val = 0x04;
        anx_i2c_write1(  SP_TX_VSYNC_CFG_REG,val);
        val = 0x10;
        anx_i2c_write1(  SP_TX_VB_PORCH_REG, val);
        val = 0x38;//0x62;//0x38;
        anx_i2c_write1(  SP_TX_TOTAL_PIXELL_REG, val);
        val = 0x06;//0x08;//0x06;
        anx_i2c_write1(  SP_TX_TOTAL_PIXELH_REG, val);
        val = 0x56;//0x80;//0x56;
        anx_i2c_write1(  SP_TX_ACT_PIXELL_REG, val);
        val = 0x05;//0x07;//0x05;
        anx_i2c_write1(  SP_TX_ACT_PIXELH_REG, val);

        val = 0x31;
        anx_i2c_write1(  SP_TX_HF_PORCHL_REG, val);
        val = 0x00;
        anx_i2c_write1(  SP_TX_HF_PORCHH_REG, val);

        val = 0x10;
        anx_i2c_write1(  SP_TX_HSYNC_CFGL_REG,val);
        val = 0x00;
        anx_i2c_write1( SP_TX_HSYNC_CFGH_REG,val);
        val = 0xa1;
        anx_i2c_write1(  SP_TX_HB_PORCHL_REG, val);
        val = 0x00;
        anx_i2c_write1(  SP_TX_HB_PORCHH_REG, val);


        val = 0x00; //0x03;
        anx_i2c_write1(  SP_TX_VID_CTRL10_REG, val);

	//RK_EDP chip analog setting
        val = 0xf0; //0xf0;
        anx_i2c_write1( ANALOG_DEBUG_REG1, val);
        val = 0x99; //0x99;
        anx_i2c_write1( ANALOG_DEBUG_REG3, val);//maybe lane level
        val = 0x7b; //0x7b;
        anx_i2c_write1( PLL_FILTER_CTRL1, val);
        val = 0x70;//0x30;
        anx_i2c_write1( SP_TX_LINK_DEBUG_REG,val);
        val = 0x06;//0x06;
        anx_i2c_write1(0xE2, val);
        val = 0x00;//0x06;
        anx_i2c_write0(0xdd, val);

        //force HPD
        val = 0x30;
        anx_i2c_write0( SP_TX_SYS_CTRL3_REG, val);
        //power on 4 lanes
        val = 0xce;
        anx_i2c_write0(  0xc8, val);
        //lanes setting
        val = 0xfe;
        anx_i2c_write0(  0xa3, val);
        val = 0xff;
        anx_i2c_write0(  0xa4, val);
        anx_i2c_write0(  0xa5, val);
        anx_i2c_write0(  0xa6, val);
        //reset AUX CH
        val = 0x44;
        anx_i2c_write1(  SP_TX_RST_CTRL2_REG, val);
        val = 0x40;
        udelay(50000);
        anx_i2c_write1(  SP_TX_RST_CTRL2_REG, val);
        //Select 1.62G
        val = 0x0a;
        anx_i2c_write0( SP_TX_LINK_BW_SET_REG, val);
        //Select 4 lanes
        val = 0x01;
        anx_i2c_write0( SP_TX_LANE_COUNT_SET_REG, val);
	#if 1
        val = SP_TX_LINK_TRAINING_CTRL_EN;
        anx_i2c_write0( SP_TX_LINK_TRAINING_CTRL_REG, val);
        udelay(50000);
        anx_i2c_write0(SP_TX_LINK_TRAINING_CTRL_REG, val);
        while((val&0x01)&&(cnt++ < 10))
        {
                printf("Waiting...\n");
                udelay(5000);
                anx_i2c_read0(SP_TX_LINK_TRAINING_CTRL_REG, &val );
        }

        if(cnt >= 10)
        {
                printf("HW LT fail\n");
        }
        else
        {
                printf( "HW LT success ...cnt:%d\n",cnt);
        }

#endif

        //enable video input
        val = 0x81;
        anx_i2c_write1(  SP_TX_VID_CTRL1_REG, val);

        //enable BIST
        val = 0x00; //SP_TX_VID_CTRL4_BIST;
        anx_i2c_write1(  SP_TX_VID_CTRL4_REG, val);

        //force HPD and stream valid
        val = 0x03;
        anx_i2c_write0(  0x82, val);


}

static void LCD_cfg_panel_info(panel_extend_para * info)
{
	u32 i = 0, j=0;
	u32 items;
	u8 lcd_gamma_tbl[][2] =
	{
		//{input value, corrected value}
		{0, 0},
		{15, 15},
		{30, 30},
		{45, 45},
		{60, 60},
		{75, 75},
		{90, 90},
		{105, 105},
		{120, 120},
		{135, 135},
		{150, 150},
		{165, 165},
		{180, 180},
		{195, 195},
		{210, 210},
		{225, 225},
		{240, 240},
		{255, 255},
	};

	u8 lcd_bright_curve_tbl[][2] =
	{
		//{input value, corrected value}
		{0    ,0  },//0
		{15   ,3  },//0
		{30   ,6  },//0
		{45   ,9  },// 1
		{60   ,12  },// 2
		{75   ,16  },// 5
		{90   ,22  },//9
		{105   ,28 }, //15
		{120  ,36 },//23
		{135  ,44 },//33
		{150  ,54 },
		{165  ,67 },
		{180  ,84 },
		{195  ,108},
		{210  ,137},
		{225 ,171},
		{240 ,210},
		{255 ,255},
	};

	u32 lcd_cmap_tbl[2][3][4] = {
	{
		{LCD_CMAP_G0,LCD_CMAP_B1,LCD_CMAP_G2,LCD_CMAP_B3},
		{LCD_CMAP_B0,LCD_CMAP_R1,LCD_CMAP_B2,LCD_CMAP_R3},
		{LCD_CMAP_R0,LCD_CMAP_G1,LCD_CMAP_R2,LCD_CMAP_G3},
		},
		{
		{LCD_CMAP_B3,LCD_CMAP_G2,LCD_CMAP_B1,LCD_CMAP_G0},
		{LCD_CMAP_R3,LCD_CMAP_B2,LCD_CMAP_R1,LCD_CMAP_B0},
		{LCD_CMAP_G3,LCD_CMAP_R2,LCD_CMAP_G1,LCD_CMAP_R0},
		},
	};

	memset(info,0,sizeof(panel_extend_para));

	items = sizeof(lcd_gamma_tbl)/2;
	for(i=0; i<items-1; i++) {
		u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

		for(j=0; j<num; j++) {
			u32 value = 0;

			value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];

	items = sizeof(lcd_bright_curve_tbl)/2;
	for(i=0; i<items-1; i++) {
		u32 num = lcd_bright_curve_tbl[i+1][0] - lcd_bright_curve_tbl[i][0];

		for(j=0; j<num; j++) {
			u32 value = 0;

			value = lcd_bright_curve_tbl[i][1] + ((lcd_bright_curve_tbl[i+1][1] - lcd_bright_curve_tbl[i][1]) * j)/num;
			info->lcd_bright_curve_tbl[lcd_bright_curve_tbl[i][0] + j] = value;
		}
	}
	info->lcd_bright_curve_tbl[255] = lcd_bright_curve_tbl[items-1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static s32 LCD_open_flow(u32 sel)
{
	LCD_OPEN_FUNC(sel, LCD_power_on, 30);   //open lcd power, and delay 50ms
	LCD_OPEN_FUNC(sel, LCD_panel_init, 50);   //open lcd power, than delay 200ms
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 100);     //open lcd controller, and delay 100ms
	LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

	return 0;
}

static s32 LCD_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, LCD_bl_close, 0);       //close lcd backlight, and delay 0ms
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);         //close lcd controller, and delay 0ms
	LCD_CLOSE_FUNC(sel, LCD_panel_exit,	200);   //open lcd power, than delay 200ms
	LCD_CLOSE_FUNC(sel, LCD_power_off, 500);   //close lcd power, and delay 500ms

	return 0;
}

static void LCD_power_on(u32 sel)
{
	sunxi_lcd_power_enable(sel, 0);//config lcd_power pin to open lcd power0
	sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_power_disable(sel, 0);//config lcd_power pin to close lcd power0
}

static void LCD_bl_open(u32 sel)
{
	sunxi_lcd_pwm_enable(sel);
	sunxi_lcd_backlight_enable(sel);//config lcd_bl_en pin to open lcd backlight
}

static void LCD_bl_close(u32 sel)
{
	sunxi_lcd_backlight_disable(sel);//config lcd_bl_en pin to close lcd backlight
	sunxi_lcd_pwm_disable(sel);
}

static void LCD_panel_init(u32 sel)
{
anx6345_init(0);
	return;
}

static void LCD_panel_exit(u32 sel)
{
	return ;
}

//sel: 0:lcd0; 1:lcd1
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

__lcd_panel_t default_panel = {
	/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
	.name = "default_lcd",
	.func = {
		.cfg_panel_info = LCD_cfg_panel_info,
		.cfg_open_flow = LCD_open_flow,
		.cfg_close_flow = LCD_close_flow,
		.lcd_user_defined_func = LCD_user_defined_func,
	},
};
