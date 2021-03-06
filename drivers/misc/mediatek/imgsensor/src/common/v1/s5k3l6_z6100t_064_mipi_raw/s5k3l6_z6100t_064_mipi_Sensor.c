/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/*****************************************************************************
 *
 * Filename:
 * ---------
 *     s5k3l6_z6100t_064_mipi_sensor.c
 *
 * Project:
 * --------
 *     ALPS
 *
 * Description:
 * ------------
 *     Source code of Sensor driver
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "s5k3l6_z6100t_064_mipi_Sensor.h"

/****************************Modify Following Strings for Debug****************************/
#define PFX "s5k3l6_z6100t_064_camera_sensor"
#define LOG_1 LOG_INF("S5K3L6_Z6100T_064,MIPI 4LANE\n")
#define OFF_READY_DELAY 25
/* #define LOG_2 LOG_INF("preview 2304*1728@30fps,102Mbps/lane; capture 16M@30fps,1500Mbps/lane\n") */
/****************************   Modify end    *******************************************/

#define LOG_INF(format, args...)	pr_err(PFX "[%s] " format, __func__, ##args)

static DEFINE_SPINLOCK(imgsensor_drv_lock);


static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = S5K3L6_Z6100T_064_SENSOR_ID,

	.checksum_value = 0x143d0c73,	/*checksum value for Camera Auto Test */

	.pre = {
		.pclk = 480000000,				/*  record different mode's pclk */
		.linelength  = 4896,				/*  record different mode's linelength */
		.framelength = 3260,			/*  record different mode's framelength */
		.startx = 0,					/*  record different mode's startx of grabwindow */
		.starty = 0,					/*  record different mode's starty of grabwindow */
		.grabwindow_width  = 2080,		/*  record different mode's width of grabwindow */
		.grabwindow_height = 1560,		/*  record different mode's height of grabwindow */
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 243200000,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,
	},
	.cap = {
		.pclk = 480000000,				/*  record different mode's pclk */
		.linelength  = 4896,            /*  record different mode's linelength */
		.framelength = 3260,			/*  record different mode's framelength */
		.startx = 0,					/*  record different mode's startx of grabwindow */
		.starty = 0,					/*  record different mode's starty of grabwindow */
		.grabwindow_width  = 4160,		/*  record different mode's width of grabwindow */
		.grabwindow_height = 3120,		/*  record different mode's height of grabwindow */
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 448000000,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,
	},
	.normal_video = {
		.pclk = 480000000,				/*  record different mode's pclk */
		.linelength  = 4896,            /*  record different mode's linelength */
		.framelength = 3260,			/*  record different mode's framelength */
		.startx = 0,					/*  record different mode's startx of grabwindow */
		.starty = 0,					/*  record different mode's starty of grabwindow */
		.grabwindow_width  = 2080,		/*  record different mode's width of grabwindow */
		.grabwindow_height = 1560,		/*  record different mode's height of grabwindow */
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 243200000,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,
	},
	.hs_video = {
		.pclk = 480000000,				/*  record different mode's pclk */
		.linelength  = 4720,				/*  record different mode's linelength */
		.framelength = 848,			/*  record different mode's framelength */
		.startx = 0,					/*  record different mode's startx of grabwindow */
		.starty = 0,					/*  record different mode's starty of grabwindow */
		.grabwindow_width  = 1052,		/*  record different mode's width of grabwindow */
		.grabwindow_height = 780,		/*  record different mode's height of grabwindow */
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 30800000,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 1200,
	},
	.slim_video = {
		.pclk = 480000000,				/*  record different mode's pclk */
		.linelength  = 4896,				/*  record different mode's linelength */
		.framelength = 3260,			/*  record different mode's framelength */
		.startx = 0,					/*  record different mode's startx of grabwindow */
		.starty = 0,					/*  record different mode's starty of grabwindow */
		.grabwindow_width  = 2080,		/*  record different mode's width of grabwindow */
		.grabwindow_height = 1560,		/*  record different mode's height of grabwindow */
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 240000000,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,
	},
	.custom1 = {
		.pclk = 480000000,				/*  record different mode's pclk */
		.linelength  = 4896,            /*  record different mode's linelength */
		.framelength = 3260,			/*  record different mode's framelength */
		.startx = 0,					/*  record different mode's startx of grabwindow */
		.starty = 0,					/*  record different mode's starty of grabwindow */
		.grabwindow_width  = 4160,		/*  record different mode's width of grabwindow */
		.grabwindow_height = 3120,		/*  record different mode's height of grabwindow */
		/*	 following for MIPIDataLowPwr2HighSpeedSettleDelayCount by different scenario	*/
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 448000000,
		/*	 following for GetDefaultFramerateByScenario()	*/
		.max_framerate = 300,
	},

	.margin = 5,		/* sensor framelength & shutter margin */
	.min_shutter = 4,	/* min shutter */
	.max_frame_length = 0xFFFF,	/* max framelength by sensor register's limitation */
	.ae_shut_delay_frame = 0,
	/* shutter delay frame for AE cycle, 2 frame with ispGain_delay-shut_delay=2-0=2 */
	.ae_sensor_gain_delay_frame = 0,
	/* sensor gain delay frame for AE cycle,2 frame with ispGain_delay-sensor_gain_delay=2-0=2 */
	.ae_ispGain_delay_frame = 2,	/* isp gain delay frame for AE cycle */
	.ihdr_support = 0,	/* 1, support; 0,not support */
	.ihdr_le_firstline = 0,	/* 1,le first ; 0, se first */
	.sensor_mode_num = 6,	/* support sensor mode num */

	.cap_delay_frame = 3,
	.pre_delay_frame = 3,
	.video_delay_frame = 3,
	.hs_video_delay_frame = 3,
	.slim_video_delay_frame = 3,
	.custom1_delay_frame = 2,

	.isp_driving_current = ISP_DRIVING_4MA,	/* mclk driving current */
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,	/* sensor_interface_type */
	.mipi_sensor_type = MIPI_OPHY_NCSI2,	/* 0,MIPI_OPHY_NCSI2;  1,MIPI_OPHY_CSI2 */
	.mipi_settle_delay_mode = MIPI_SETTLEDELAY_AUTO,
	/* 0,MIPI_SETTLEDELAY_AUTO; 1,MIPI_SETTLEDELAY_MANNUAL */
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_Gr,	/* sensor output first pixel color */
	.mclk = 24,		/* mclk value, suggest 24 or 26 for 24Mhz or 26Mhz */
	.mipi_lane_num = SENSOR_MIPI_4_LANE,	/* mipi lane num */
	.i2c_addr_table = {0x5A, 0xff},
};

static struct imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,	/* mirrorflip information */
	.sensor_mode = IMGSENSOR_MODE_INIT,
	/* IMGSENSOR_MODE enum value,record current sensor mode,such as: INIT, Preview, Capture, Video */
	.shutter = 0x200,	/* current shutter */
	.gain = 0x200,		/* current gain */
	.dummy_pixel = 0,	/* current dummypixel */
	.dummy_line = 0,	/* current dummyline */
	.current_fps = 300,	/* full size current fps : 24fps for PIP, 30fps for Normal or ZSD */
	.autoflicker_en = KAL_FALSE,
	/* auto flicker enable: KAL_FALSE for disable auto flicker, KAL_TRUE for enable auto flicker */
	.test_pattern = KAL_FALSE,
	/* test pattern mode or not. KAL_FALSE for in test pattern mode, KAL_TRUE for normal output */
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,	/* current scenario id */
	.ihdr_en = 0,		/* sensor need support LE, SE with HDR feature */
	.i2c_write_id = 0x5A,	/* record current sensor's i2c write id */
};



/* Sensor output window information */
static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[6] = {
	{ 4208, 3120,  24,      0, 4160, 3120, 2080, 1560,   0, 0, 2080, 1560,  0, 0, 2080, 1560}, /*   Preview */
	{ 4208, 3120,  24,      0, 4160, 3120, 4160, 3120,   0, 0, 4160, 3120,  0, 0, 4160, 3120}, /*   capture */
	{ 4208, 3120,  24,      0, 4160, 3120, 2080, 1560,   0, 0, 2080, 1560,  0, 0, 2080, 1560}, /*   video */
	{ 4208, 3120,  0,  0, 4208, 3120,  1052,  780,   0, 0,  1052,  780,  0, 0,  1052,  780}, /* high video */
	{ 4208, 3120,  24,  0, 4160, 3120, 2080, 1560,   0, 0, 2080, 1560,  0, 0, 2080, 1560},/*   slim video */
	{ 4208, 3120,  24,      0, 4160, 3120, 4160, 3120,   0, 0, 4160, 3120,  0, 0, 4160, 3120},/* Custom1 */
};


static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info = {
	.i4OffsetX = 24,
	.i4OffsetY = 24,
	.i4PitchX = 64,
	.i4PitchY = 64,
	.i4PairNum = 16,
	.i4SubBlkW = 16,
	.i4SubBlkH = 16,		/* need check xb.pang */
	.i4BlockNumX = 65,
	.i4BlockNumY = 48,
	.i4PosR = {{28, 35}, {80, 35}, {44, 39}, {64, 39}, {32, 47}, {76, 47}, {48, 51},
	{60, 51}, {48, 67}, {60, 67}, {32, 71}, {76, 71}, {44, 79}, {64, 79}, {28, 83}, {80, 83},},
	.i4PosL = {{28, 31}, {80, 31}, {44, 35}, {64, 35}, {32, 51}, {76, 51}, {48, 55},
	{60, 55}, {48, 63}, {60, 63}, {32, 67}, {76, 67}, {44, 83}, {64, 83}, {28, 87}, {80, 87},},

	.iMirrorFlip = 0,
};

#if 0
static SET_PD_BLOCK_INFO_T imgsensor_pd_info = {
	.i4OffsetX = 0,
	.i4OffsetY = 0,
	.i4PitchX = 16,
	.i4PitchY = 16,
	.i4PairNum = 4,
	.i4SubBlkW = 8,
	.i4SubBlkH = 8,		/* need check xb.pang */
	.i4BlockNumX = 288,
	.i4BlockNumY = 216,
	.i4PosR = {
	    {5, 2}, {13, 2}, {1, 10}, {9, 10},},
	.i4PosL = {
	    {6, 2}, {14, 2}, {2, 10}, {10, 10},},
	.iMirrorFlip = 0,
};
#endif

static kal_uint16 read_cmos_sensor_byte(kal_uint16 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pu_send_cmd, 2, (u8 *)&get_byte, 1, imgsensor.i2c_write_id);

	return get_byte;
}
static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;

	char pu_send_cmd[2] = { (char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pu_send_cmd, 2, (u8 *) &get_byte, 1, imgsensor.i2c_write_id);

	return get_byte;
}
#if 0
static void write_cmos_sensor_byte(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};

	iWriteRegI2C(pu_send_cmd, 3, imgsensor.i2c_write_id);
}
#endif
static void write_cmos_sensor(kal_uint16 addr, kal_uint16 para)
{
	char pusendcmd[4] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para >> 8), (char)(para & 0xFF)};

	iWriteRegI2C(pusendcmd, 4, imgsensor.i2c_write_id);
}

static void set_dummy(void)
{
	LOG_INF("dummyline = %d, dummypixels = %d\n", imgsensor.dummy_line, imgsensor.dummy_pixel);
	/* you can set dummy by imgsensor.dummy_line and imgsensor.dummy_pixel,
	or you can set dummy by imgsensor.frame_length and imgsensor.line_length */
	write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
	write_cmos_sensor(0x0342, imgsensor.line_length & 0xFFFF);

}				/*    set_dummy  */

static kal_uint32 return_sensor_id(void)
{
	return ((read_cmos_sensor_byte(0x0000) << 8) | read_cmos_sensor_byte(0x0001));
}

static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{
	kal_uint32 frame_length = imgsensor.frame_length;

	LOG_INF("framerate = %d, min_framelength_en = %d\n", framerate, min_framelength_en);

	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;

	spin_lock(&imgsensor_drv_lock);
	imgsensor.frame_length =
	    (frame_length > imgsensor.min_frame_length) ? frame_length : imgsensor.min_frame_length;
	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length) {
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);

	set_dummy();
}				/*    set_max_framerate  */

static void write_shutter(kal_uint16 shutter)
{
	kal_uint16 realtime_fps = 0;

	LOG_INF("shutter = %d\n", shutter);
	/* 0x3500, 0x3501, 0x3502 will increase VBLANK to get exposure larger than frame exposure */
	/* AE doesn't update sensor gain at capture mode, thus extra exposure lines must be updated here. */

	/* OV Recommend Solution */
	/* if shutter bigger than frame_length, should extend frame length first */
	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;
	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter =
	    (shutter >
	     (imgsensor_info.max_frame_length -
	      imgsensor_info.margin)) ? (imgsensor_info.max_frame_length -
					 imgsensor_info.margin) : shutter;

	/* Framelength should be an even number */
	shutter = (shutter >> 1) << 1;
	imgsensor.frame_length = (imgsensor.frame_length >> 1) << 1;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;

		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else {
		/* Extend frame length */
		write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
		}
	} else {
		/* Extend frame length */
		write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
	}

	/* Update Shutter */
	write_cmos_sensor(0X0202, shutter & 0xFFFF);
	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter, imgsensor.frame_length);
}


/*************************************************************************
* FUNCTION
*    set_shutter
*
* DESCRIPTION
*    This function set e-shutter of sensor to change exposure time.
*
* PARAMETERS
*    iShutter : exposured lines
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void set_shutter(kal_uint16 shutter)
{
	unsigned long flags;

	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

	write_shutter(shutter);
}
static void set_shutter_frame_length(kal_uint16 shutter, kal_uint16 frame_length)
{

	unsigned long flags;
	kal_uint16 realtime_fps = 0;
	kal_int32 dummy_line = 0;

	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);
	LOG_INF("Enter shutter =%d\n", shutter);

	spin_lock(&imgsensor_drv_lock);
	/* Change frame time */
	if (frame_length > 1)
		dummy_line = frame_length - imgsensor.frame_length;
	imgsensor.frame_length = imgsensor.frame_length + dummy_line;

	if (shutter > imgsensor.frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;

	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin))
		? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else {
			/* Extend frame length */
		write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
		}
	} else {
		/* Extend frame length */
		write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
	}
	write_cmos_sensor(0X0202, shutter & 0xFFFF);

	LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter, imgsensor.frame_length);

}

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = 0x0000;

	/* gain = 64 = 1x real gain; */
	reg_gain = gain/2;
	/* reg_gain = reg_gain & 0xFFFF; */

	return (kal_uint16) reg_gain;
}

/*************************************************************************
* FUNCTION
*    set_gain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    iGain : sensor global gain(base: 0x40)
*
* RETURNS
*    the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;

	LOG_INF("gain is %d\n", gain);
	if (gain < BASEGAIN || gain > 16 * BASEGAIN) {
		LOG_INF("Error gain setting");
		if (gain < BASEGAIN)
			gain = BASEGAIN;
		else if (gain > 16 * BASEGAIN)
			gain = 16 * BASEGAIN;
	}

	reg_gain = gain2reg(gain);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain;
	spin_unlock(&imgsensor_drv_lock);
	LOG_INF("gain = %d , reg_gain = 0x%x\n ", gain, reg_gain);

	write_cmos_sensor(0x0204, (reg_gain&0xFFFF));

	return gain;
}				/*    set_gain  */

static void ihdr_write_shutter_gain(kal_uint16 le, kal_uint16 se, kal_uint16 gain)
{
	LOG_INF("le:0x%x, se:0x%x, gain:0x%x\n", le, se, gain);
	if (imgsensor.ihdr_en) {

		spin_lock(&imgsensor_drv_lock);
			if (le > imgsensor.min_frame_length - imgsensor_info.margin)
				imgsensor.frame_length = le + imgsensor_info.margin;
			else
				imgsensor.frame_length = imgsensor.min_frame_length;
			if (imgsensor.frame_length > imgsensor_info.max_frame_length)
				imgsensor.frame_length = imgsensor_info.max_frame_length;
			spin_unlock(&imgsensor_drv_lock);
			if (le < imgsensor_info.min_shutter)
				le = imgsensor_info.min_shutter;
			if (se < imgsensor_info.min_shutter)
				se = imgsensor_info.min_shutter;


		/* Extend frame length first */

		set_gain(gain);
	}
}

#if 0
static void set_mirror_flip(kal_uint8 image_mirror)
{
	LOG_INF("image_mirror = %d\n", image_mirror);

    /********************************************************
       *
       *   0x3820[2] ISP Vertical flip
       *   0x3820[1] Sensor Vertical flip
       *
       *   0x3821[2] ISP Horizontal mirror
       *   0x3821[1] Sensor Horizontal mirror
       *
       *   ISP and Sensor flip or mirror register bit should be the same!!
       *
       ********************************************************/
	spin_lock(&imgsensor_drv_lock);
	imgsensor.mirror = image_mirror;
	spin_unlock(&imgsensor_drv_lock);
	switch (image_mirror) {
	case IMAGE_NORMAL:
			write_cmos_sensor(0x0101, 0X00); /*  GR */
		break;
	case IMAGE_H_MIRROR:
			write_cmos_sensor(0x0101, 0X01); /*  R */
		break;
	case IMAGE_V_MIRROR:
			write_cmos_sensor(0x0101, 0X02); /*  B */
		break;
	case IMAGE_HV_MIRROR:
			write_cmos_sensor(0x0101, 0X03); /*  GB */
		break;
	default:
		LOG_INF("Error image_mirror setting\n");
	}

}
#endif

/*************************************************************************
* FUNCTION
*    night_mode
*
* DESCRIPTION
*    This function night mode of sensor.
*
* PARAMETERS
*    bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static void night_mode(kal_bool enable)
{
/*No Need to implement this function*/
}				/*    night_mode    */

static void sensor_init(void)
{
	LOG_INF("E\n");
	write_cmos_sensor(0x0100, 0x0000);
	mdelay(3);

	write_cmos_sensor(0x3084, 0x1314);
	write_cmos_sensor(0x3266, 0x0001);
	write_cmos_sensor(0x3242, 0x2020);
	write_cmos_sensor(0x306A, 0x2F4C);
	write_cmos_sensor(0x306C, 0xCA01);
	write_cmos_sensor(0x307A, 0x0D20);
	write_cmos_sensor(0x309E, 0x002D);
	write_cmos_sensor(0x3072, 0x0013);
	write_cmos_sensor(0x3074, 0x0977);
	write_cmos_sensor(0x3076, 0x9411);
	write_cmos_sensor(0x3024, 0x0016);
	write_cmos_sensor(0x3070, 0x3D00);
	write_cmos_sensor(0x3002, 0x0E00);
	write_cmos_sensor(0x3006, 0x1000);
	write_cmos_sensor(0x300A, 0x0C00);
	write_cmos_sensor(0x3010, 0x0400);
	write_cmos_sensor(0x3018, 0xC500);
	write_cmos_sensor(0x303A, 0x0204);
	write_cmos_sensor(0x3452, 0x0001);
	write_cmos_sensor(0x3454, 0x0001);
	write_cmos_sensor(0x3456, 0x0001);
	write_cmos_sensor(0x3458, 0x0001);
	write_cmos_sensor(0x345a, 0x0002);
	write_cmos_sensor(0x345C, 0x0014);
	write_cmos_sensor(0x345E, 0x0002);
	write_cmos_sensor(0x3460, 0x0014);
	write_cmos_sensor(0x3464, 0x0006);
	write_cmos_sensor(0x3466, 0x0012);
	write_cmos_sensor(0x3468, 0x0012);
	write_cmos_sensor(0x346A, 0x0012);
	write_cmos_sensor(0x346C, 0x0012);
	write_cmos_sensor(0x346E, 0x0012);
	write_cmos_sensor(0x3470, 0x0012);
	write_cmos_sensor(0x3472, 0x0008);
	write_cmos_sensor(0x3474, 0x0004);
	write_cmos_sensor(0x3476, 0x0044);
	write_cmos_sensor(0x3478, 0x0004);
	write_cmos_sensor(0x347A, 0x0044);
	write_cmos_sensor(0x347E, 0x0006);
	write_cmos_sensor(0x3480, 0x0010);
	write_cmos_sensor(0x3482, 0x0010);
	write_cmos_sensor(0x3484, 0x0010);
	write_cmos_sensor(0x3486, 0x0010);
	write_cmos_sensor(0x3488, 0x0010);
	write_cmos_sensor(0x348A, 0x0010);
	write_cmos_sensor(0x348E, 0x000C);
	write_cmos_sensor(0x3490, 0x004C);
	write_cmos_sensor(0x3492, 0x000C);
	write_cmos_sensor(0x3494, 0x004C);
	write_cmos_sensor(0x3496, 0x0020);
	write_cmos_sensor(0x3498, 0x0006);
	write_cmos_sensor(0x349A, 0x0008);
	write_cmos_sensor(0x349C, 0x0008);
	write_cmos_sensor(0x349E, 0x0008);
	write_cmos_sensor(0x34A0, 0x0008);
	write_cmos_sensor(0x34A2, 0x0008);
	write_cmos_sensor(0x34A4, 0x0008);
	write_cmos_sensor(0x34A8, 0x001A);
	write_cmos_sensor(0x34AA, 0x002A);
	write_cmos_sensor(0x34AC, 0x001A);
	write_cmos_sensor(0x34AE, 0x002A);
	write_cmos_sensor(0x34B0, 0x0080);
	write_cmos_sensor(0x34B2, 0x0006);
	write_cmos_sensor(0x32A2, 0x0000);
	write_cmos_sensor(0x32A4, 0x0000);
	write_cmos_sensor(0x32A6, 0x0000);
	write_cmos_sensor(0x32A8, 0x0000);
	write_cmos_sensor(0x0100, 0x0000);

}				/*    MIPI_sensor_Init  */

static void preview_setting(void)
{
	LOG_INF("E\n");
	write_cmos_sensor(0x0100, 0x0000);

	write_cmos_sensor(0x0344, 0x0020);
	write_cmos_sensor(0x0346, 0x0008);
	write_cmos_sensor(0x0348, 0x105F);
	write_cmos_sensor(0x034A, 0x0C37);
	write_cmos_sensor(0x034C, 0x0820);
	write_cmos_sensor(0x034E, 0x0618);
	write_cmos_sensor(0x0900, 0x0122);
	write_cmos_sensor(0x0380, 0x0001);
	write_cmos_sensor(0x0382, 0x0001);
	write_cmos_sensor(0x0384, 0x0001);
	write_cmos_sensor(0x0386, 0x0003);
	write_cmos_sensor(0x0114, 0x0330);
	write_cmos_sensor(0x0110, 0x0002);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0078);
	write_cmos_sensor(0x3C1E, 0x0000);
	write_cmos_sensor(0x030C, 0x0003);
	write_cmos_sensor(0x030E, 0x004C);
	write_cmos_sensor(0x3C16, 0x0001);
	write_cmos_sensor(0x0300, 0x0006);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CBC);
	write_cmos_sensor(0x38C4, 0x0004);
	write_cmos_sensor(0x38D8, 0x0012);
	write_cmos_sensor(0x38DA, 0x0005);
	write_cmos_sensor(0x38DC, 0x0006);
	write_cmos_sensor(0x38C2, 0x0005);
	write_cmos_sensor(0x38C0, 0x0005);
	write_cmos_sensor(0x38D6, 0x0005);
	write_cmos_sensor(0x38D4, 0x0004);
	write_cmos_sensor(0x38B0, 0x0007);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x3938, 0x000C);
	write_cmos_sensor(0x0820, 0x0258);
	write_cmos_sensor(0x380C, 0x0049);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x309C, 0x0600);
	write_cmos_sensor(0x3090, 0x8000);
	write_cmos_sensor(0x3238, 0x000B);
	write_cmos_sensor(0x314A, 0x5F02);
	write_cmos_sensor(0x3300, 0x0000);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x32B2, 0x0008);
	write_cmos_sensor(0x32B4, 0x0008);
	write_cmos_sensor(0x32B6, 0x0008);
	write_cmos_sensor(0x32B8, 0x0008);
	write_cmos_sensor(0x3C34, 0x0008);
	write_cmos_sensor(0x3C36, 0x3000);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
}				/*    preview_setting  */


static void capture_setting(kal_uint16 currefps)
{

LOG_INF(" E\n");

		write_cmos_sensor(0x0100, 0x0000);

		write_cmos_sensor(0x0344, 0x0020);
		write_cmos_sensor(0x0346, 0x0008);
		write_cmos_sensor(0x0348, 0x105F);
		write_cmos_sensor(0x034A, 0x0C37);
		write_cmos_sensor(0x034C, 0x1040);
		write_cmos_sensor(0x034E, 0x0C30);
		write_cmos_sensor(0x0900, 0x0000);
		write_cmos_sensor(0x0380, 0x0001);
		write_cmos_sensor(0x0382, 0x0001);
		write_cmos_sensor(0x0384, 0x0001);
		write_cmos_sensor(0x0386, 0x0001);
		write_cmos_sensor(0x0114, 0x0330);
		write_cmos_sensor(0x0110, 0x0002);
		write_cmos_sensor(0x0136, 0x1800);
		write_cmos_sensor(0x0304, 0x0004);
		write_cmos_sensor(0x0306, 0x0078);
		write_cmos_sensor(0x3C1E, 0x0000);
		write_cmos_sensor(0x030C, 0x0003);
		write_cmos_sensor(0x030E, 0x0046);
		write_cmos_sensor(0x3C16, 0x0000);
		write_cmos_sensor(0x0300, 0x0006);
		write_cmos_sensor(0x0342, 0x1320);
		write_cmos_sensor(0x0340, 0x0CBC);
		write_cmos_sensor(0x38C4, 0x0009);
		write_cmos_sensor(0x38D8, 0x0027);
		write_cmos_sensor(0x38DA, 0x0009);
		write_cmos_sensor(0x38DC, 0x000A);
		write_cmos_sensor(0x38C2, 0x0009);
		write_cmos_sensor(0x38C0, 0x000D);
		write_cmos_sensor(0x38D6, 0x0009);
		write_cmos_sensor(0x38D4, 0x0008);
		write_cmos_sensor(0x38B0, 0x000E);
		write_cmos_sensor(0x3932, 0x1000);
		write_cmos_sensor(0x3938, 0x000C);
		write_cmos_sensor(0x0820, 0x0460);
		write_cmos_sensor(0x380C, 0x0090);
		write_cmos_sensor(0x3064, 0xFFCF);
		write_cmos_sensor(0x309C, 0x0640);
		write_cmos_sensor(0x3090, 0x8800);
		write_cmos_sensor(0x3238, 0x000C);
		write_cmos_sensor(0x314A, 0x5F00);
		write_cmos_sensor(0x3300, 0x0000);
		write_cmos_sensor(0x3400, 0x0000);
		write_cmos_sensor(0x3402, 0x4E42);
		write_cmos_sensor(0x32B2, 0x0000);
		write_cmos_sensor(0x32B4, 0x0000);
		write_cmos_sensor(0x32B6, 0x0000);
		write_cmos_sensor(0x32B8, 0x0000);
		write_cmos_sensor(0x3C34, 0x0008);
		write_cmos_sensor(0x3C36, 0x3000);
		write_cmos_sensor(0x3C38, 0x0020);
		write_cmos_sensor(0x393E, 0x4000);
		write_cmos_sensor(0x303A, 0x0204);
		write_cmos_sensor(0x3034, 0x4B01);
		write_cmos_sensor(0x3036, 0x0029);
		write_cmos_sensor(0x3032, 0x4800);
		write_cmos_sensor(0x320E, 0x049E);
		write_cmos_sensor(0x3C1E, 0x0100);
		write_cmos_sensor(0x0100, 0x0103);
		write_cmos_sensor(0x3C1E, 0x0000);
    /* dual cam sync */
	/* write_cmos_sensor_byte(0x3C67, 0x10); */
}				/*    capture_setting  */

static void normal_video_setting(kal_uint16 currefps)
{
	LOG_INF("E! currefps:%d\n", currefps);

	/* preview_setting(); */
	LOG_INF("E! currefps:%d\n", currefps);
		write_cmos_sensor(0x0100, 0x0000);

		write_cmos_sensor(0x0344, 0x0020);
		write_cmos_sensor(0x0346, 0x0008);
		write_cmos_sensor(0x0348, 0x105F);
		write_cmos_sensor(0x034A, 0x0C37);
		write_cmos_sensor(0x034C, 0x0820);
		write_cmos_sensor(0x034E, 0x0618);
		write_cmos_sensor(0x0900, 0x0122);
		write_cmos_sensor(0x0380, 0x0001);
		write_cmos_sensor(0x0382, 0x0001);
		write_cmos_sensor(0x0384, 0x0001);
		write_cmos_sensor(0x0386, 0x0003);
		write_cmos_sensor(0x0114, 0x0330);
		write_cmos_sensor(0x0110, 0x0002);
		write_cmos_sensor(0x0136, 0x1800);
		write_cmos_sensor(0x0304, 0x0004);
		write_cmos_sensor(0x0306, 0x0078);
		write_cmos_sensor(0x3C1E, 0x0000);
		write_cmos_sensor(0x030C, 0x0003);
		write_cmos_sensor(0x030E, 0x004C);
		write_cmos_sensor(0x3C16, 0x0001);
		write_cmos_sensor(0x0300, 0x0006);
		write_cmos_sensor(0x0342, 0x1320);
		write_cmos_sensor(0x0340, 0x0CBC);
		write_cmos_sensor(0x38C4, 0x0004);
		write_cmos_sensor(0x38D8, 0x0012);
		write_cmos_sensor(0x38DA, 0x0005);
		write_cmos_sensor(0x38DC, 0x0006);
		write_cmos_sensor(0x38C2, 0x0005);
		write_cmos_sensor(0x38C0, 0x0005);
		write_cmos_sensor(0x38D6, 0x0005);
		write_cmos_sensor(0x38D4, 0x0004);
		write_cmos_sensor(0x38B0, 0x0007);
		write_cmos_sensor(0x3932, 0x1000);
		write_cmos_sensor(0x3938, 0x000C);
		write_cmos_sensor(0x0820, 0x0258);
		write_cmos_sensor(0x380C, 0x0049);
		write_cmos_sensor(0x3064, 0xFFCF);
		write_cmos_sensor(0x309C, 0x0600);
		write_cmos_sensor(0x3090, 0x8000);
		write_cmos_sensor(0x3238, 0x000B);
		write_cmos_sensor(0x314A, 0x5F02);
		write_cmos_sensor(0x3300, 0x0000);
		write_cmos_sensor(0x3400, 0x0000);
		write_cmos_sensor(0x3402, 0x4E46);
		write_cmos_sensor(0x32B2, 0x0008);
		write_cmos_sensor(0x32B4, 0x0008);
		write_cmos_sensor(0x32B6, 0x0008);
		write_cmos_sensor(0x32B8, 0x0008);
		write_cmos_sensor(0x3C34, 0x0008);
		write_cmos_sensor(0x3C36, 0x3000);
		write_cmos_sensor(0x3C38, 0x0020);
		write_cmos_sensor(0x393E, 0x4000);
}				/*    preview_setting  */

static void hs_video_setting(void)
{
	LOG_INF("123E\n");
	LOG_INF("E\n");
	write_cmos_sensor(0x0100, 0x0000);

	write_cmos_sensor(0x0344, 0x0008);
	write_cmos_sensor(0x0346, 0x0008);
	write_cmos_sensor(0x0348, 0x1077);
	write_cmos_sensor(0x034A, 0x0C37);
	write_cmos_sensor(0x034C, 0x041C);
	write_cmos_sensor(0x034E, 0x030C);
	write_cmos_sensor(0x0900, 0x0144);
	write_cmos_sensor(0x0380, 0x0001);
	write_cmos_sensor(0x0382, 0x0001);
	write_cmos_sensor(0x0384, 0x0001);
	write_cmos_sensor(0x0386, 0x0007);
	write_cmos_sensor(0x0114, 0x0330);
	write_cmos_sensor(0x0110, 0x0002);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0078);
	write_cmos_sensor(0x3C1E, 0x0000);
	write_cmos_sensor(0x030C, 0x0003);
	write_cmos_sensor(0x030E, 0x004D);
	write_cmos_sensor(0x3C16, 0x0002);
	write_cmos_sensor(0x0300, 0x0006);
	write_cmos_sensor(0x0342, 0x1270);
	write_cmos_sensor(0x0340, 0x0350);
	write_cmos_sensor(0x38C4, 0x0002);
	write_cmos_sensor(0x38D8, 0x0006);
	write_cmos_sensor(0x38DA, 0x0003);
	write_cmos_sensor(0x38DC, 0x0003);
	write_cmos_sensor(0x38C2, 0x0003);
	write_cmos_sensor(0x38C0, 0x0000);
	write_cmos_sensor(0x38D6, 0x0002);
	write_cmos_sensor(0x38D4, 0x0002);
	write_cmos_sensor(0x38B0, 0x0002);
	write_cmos_sensor(0x3932, 0x1800);
	write_cmos_sensor(0x3938, 0x000C);
	write_cmos_sensor(0x0820, 0x0132);
	write_cmos_sensor(0x380C, 0x0045);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x3090, 0x8000);
	write_cmos_sensor(0x3238, 0x000A);
	write_cmos_sensor(0x314A, 0x5F00);
	write_cmos_sensor(0x3300, 0x0000);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x32B2, 0x000A);
	write_cmos_sensor(0x32B4, 0x000A);
	write_cmos_sensor(0x32B6, 0x000A);
	write_cmos_sensor(0x32B8, 0x000A);
	write_cmos_sensor(0x3C34, 0x0048);
	write_cmos_sensor(0x3C36, 0x3000);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x303A, 0x0202);
	write_cmos_sensor(0x3034, 0x4B00);
	write_cmos_sensor(0x3036, 0xF729);
	write_cmos_sensor(0x3032, 0x3500);
	write_cmos_sensor(0x320E, 0x0480);

}

static void slim_video_setting(void)
{
	LOG_INF("E\n");
	LOG_INF("E\n");
	write_cmos_sensor(0x0100, 0x0000);

	write_cmos_sensor(0x0344, 0x0020);
	write_cmos_sensor(0x0346, 0x0008);
	write_cmos_sensor(0x0348, 0x105F);
	write_cmos_sensor(0x034A, 0x0C37);
	write_cmos_sensor(0x034C, 0x0820);
	write_cmos_sensor(0x034E, 0x0618);
	write_cmos_sensor(0x0900, 0x0122);
	write_cmos_sensor(0x0380, 0x0001);
	write_cmos_sensor(0x0382, 0x0001);
	write_cmos_sensor(0x0384, 0x0001);
	write_cmos_sensor(0x0386, 0x0003);
	write_cmos_sensor(0x0114, 0x0330);
	write_cmos_sensor(0x0110, 0x0002);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0078);
	write_cmos_sensor(0x3C1E, 0x0000);
	write_cmos_sensor(0x030C, 0x0003);
	write_cmos_sensor(0x030E, 0x004B);
	write_cmos_sensor(0x3C16, 0x0001);
	write_cmos_sensor(0x0300, 0x0006);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CBC);
	write_cmos_sensor(0x38C4, 0x0004);
	write_cmos_sensor(0x38D8, 0x0012);
	write_cmos_sensor(0x38DA, 0x0005);
	write_cmos_sensor(0x38DC, 0x0006);
	write_cmos_sensor(0x38C2, 0x0005);
	write_cmos_sensor(0x38C0, 0x0005);
	write_cmos_sensor(0x38D6, 0x0005);
	write_cmos_sensor(0x38D4, 0x0004);
	write_cmos_sensor(0x38B0, 0x0007);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x3938, 0x000C);
	write_cmos_sensor(0x0820, 0x0258);
	write_cmos_sensor(0x380C, 0x0049);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x309C, 0x0600);
	write_cmos_sensor(0x3090, 0x8000);
	write_cmos_sensor(0x3238, 0x000B);
	write_cmos_sensor(0x314A, 0x5F02);
	write_cmos_sensor(0x3300, 0x0000);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E46);
	write_cmos_sensor(0x32B2, 0x0008);
	write_cmos_sensor(0x32B4, 0x0008);
	write_cmos_sensor(0x32B6, 0x0008);
	write_cmos_sensor(0x32B8, 0x0008);
	write_cmos_sensor(0x3C34, 0x0008);
	write_cmos_sensor(0x3C36, 0x3000);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
}


static void custom1_setting(void)
{

	LOG_INF(" E\n");
	write_cmos_sensor(0x0100, 0x0000);

	write_cmos_sensor(0x0344, 0x0020);
	write_cmos_sensor(0x0346, 0x0008);
	write_cmos_sensor(0x0348, 0x105F);
	write_cmos_sensor(0x034A, 0x0C37);
	write_cmos_sensor(0x034C, 0x1040);
	write_cmos_sensor(0x034E, 0x0C30);
	write_cmos_sensor(0x0900, 0x0000);
	write_cmos_sensor(0x0380, 0x0001);
	write_cmos_sensor(0x0382, 0x0001);
	write_cmos_sensor(0x0384, 0x0001);
	write_cmos_sensor(0x0386, 0x0001);
	write_cmos_sensor(0x0114, 0x0330);
	write_cmos_sensor(0x0110, 0x0002);
	write_cmos_sensor(0x0136, 0x1800);
	write_cmos_sensor(0x0304, 0x0004);
	write_cmos_sensor(0x0306, 0x0078);
	write_cmos_sensor(0x3C1E, 0x0000);
	write_cmos_sensor(0x030C, 0x0003);
	write_cmos_sensor(0x030E, 0x0046);
	write_cmos_sensor(0x3C16, 0x0000);
	write_cmos_sensor(0x0300, 0x0006);
	write_cmos_sensor(0x0342, 0x1320);
	write_cmos_sensor(0x0340, 0x0CBC);
	write_cmos_sensor(0x38C4, 0x0009);
	write_cmos_sensor(0x38D8, 0x0027);
	write_cmos_sensor(0x38DA, 0x0009);
	write_cmos_sensor(0x38DC, 0x000A);
	write_cmos_sensor(0x38C2, 0x0009);
	write_cmos_sensor(0x38C0, 0x000D);
	write_cmos_sensor(0x38D6, 0x0009);
	write_cmos_sensor(0x38D4, 0x0008);
	write_cmos_sensor(0x38B0, 0x000E);
	write_cmos_sensor(0x3932, 0x1000);
	write_cmos_sensor(0x3938, 0x000C);
	write_cmos_sensor(0x0820, 0x0460);
	write_cmos_sensor(0x380C, 0x0090);
	write_cmos_sensor(0x3064, 0xFFCF);
	write_cmos_sensor(0x309C, 0x0640);
	write_cmos_sensor(0x3090, 0x8800);
	write_cmos_sensor(0x3238, 0x000C);
	write_cmos_sensor(0x314A, 0x5F00);
	write_cmos_sensor(0x3300, 0x0000);
	write_cmos_sensor(0x3400, 0x0000);
	write_cmos_sensor(0x3402, 0x4E42);
	write_cmos_sensor(0x32B2, 0x0000);
	write_cmos_sensor(0x32B4, 0x0000);
	write_cmos_sensor(0x32B6, 0x0000);
	write_cmos_sensor(0x32B8, 0x0000);
	write_cmos_sensor(0x3C34, 0x0008);
	write_cmos_sensor(0x3C36, 0x3000);
	write_cmos_sensor(0x3C38, 0x0020);
	write_cmos_sensor(0x393E, 0x4000);
	write_cmos_sensor(0x303A, 0x0204);
	write_cmos_sensor(0x3034, 0x4B01);
	write_cmos_sensor(0x3036, 0x0029);
	write_cmos_sensor(0x3032, 0x4800);
	write_cmos_sensor(0x320E, 0x049E);
	write_cmos_sensor(0x3C1E, 0x0100);
	write_cmos_sensor(0x0100, 0x0103);
	write_cmos_sensor(0x3C1E, 0x0000);
	write_cmos_sensor(0x3C66, 0x0110);
	write_cmos_sensor(0x3C70, 0x0083);


}
/*************************************************************************
* FUNCTION
*    get_imgsensor_id
*
* DESCRIPTION
*    This function get the sensor ID
*
* PARAMETERS
*    *sensorID : return the sensor ID
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	/* sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address */
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		LOG_INF("Read sensor addr, addr: 0x%x\n", imgsensor.i2c_write_id);
		spin_unlock(&imgsensor_drv_lock);
		do {
			*sensor_id = return_sensor_id();
			if (*sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n",
					imgsensor.i2c_write_id, *sensor_id);
				return ERROR_NONE;
			}
			LOG_INF("Read sensor id fail, id: 0x%x\n", *sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		retry = 2;
	}
	if (*sensor_id != imgsensor_info.sensor_id) {
		/* if Sensor ID is not correct, Must set *sensor_id to 0xFFFFFFFF */
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}
	return ERROR_NONE;
}


/*************************************************************************
* FUNCTION
*    open
*
* DESCRIPTION
*    This function initialize the registers of CMOS sensor
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 open(void)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint32 sensor_id = 0;

	LOG_1;
	/* LOG_2; */

	/* sensor have two i2c address 0x6c 0x6d & 0x21 0x20, we should detect the module used i2c address */
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		LOG_INF("Read sensor i2c addr id: 0x%x\n", imgsensor.i2c_write_id);
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = return_sensor_id();
			if (sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n",
					imgsensor.i2c_write_id, sensor_id);
				break;
			}
			LOG_INF("Read sensor id fail, id: 0x%x\n", sensor_id);
			retry--;
		} while (retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}
	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;

	/* initail sequence write in  */
	sensor_init();

	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en = KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_en = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}				/*    open  */



/*************************************************************************
* FUNCTION
*    close
*
* DESCRIPTION
*
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("E\n");

	/*No Need to implement this function */

	return ERROR_NONE;
}				/*    close  */


/*************************************************************************
* FUNCTION
* preview
*
* DESCRIPTION
*    This function start the sensor preview.
*
* PARAMETERS
*    *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	/* imgsensor.video_mode = KAL_FALSE; */
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	preview_setting();
	/* set_mirror_flip(sensor_config_data->SensorImageMirror); */
	mdelay(2);
	return ERROR_NONE;
}				/*    preview   */

/*************************************************************************
* FUNCTION
*    capture
*
* DESCRIPTION
*    This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*    None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	imgsensor.pclk = imgsensor_info.cap.pclk;
	imgsensor.line_length = imgsensor_info.cap.linelength;
	imgsensor.frame_length = imgsensor_info.cap.framelength;
	imgsensor.min_frame_length = imgsensor_info.cap.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;

	spin_unlock(&imgsensor_drv_lock);
	capture_setting(imgsensor.current_fps);
	/* set_mirror_flip(sensor_config_data->SensorImageMirror); */
	mdelay(2);
	return ERROR_NONE;
}				/* capture() */

static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			       MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	/* imgsensor.current_fps = 300; */
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting(imgsensor.current_fps);
	/* set_mirror_flip(sensor_config_data->SensorImageMirror); */
	return ERROR_NONE;
}				/*    normal_video   */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			   MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	/* imgsensor.video_mode = KAL_TRUE; */
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	/* set_mirror_flip(sensor_config_data->SensorImageMirror); */
	return ERROR_NONE;
}				/*    hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			     MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	/* set_mirror_flip(sensor_config_data->SensorImageMirror); */

	return ERROR_NONE;
}				/*    slim_video     */

static kal_uint32 custom1(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM1;
	imgsensor.pclk = imgsensor_info.custom1.pclk;
	imgsensor.line_length = imgsensor_info.custom1.linelength;
	imgsensor.frame_length = imgsensor_info.custom1.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom1.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom1_setting();

	mdelay(2);
	return ERROR_NONE;
}   /*  Custom1   */




static kal_uint32 get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	LOG_INF("E\n");
	sensor_resolution->SensorFullWidth = imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight = imgsensor_info.cap.grabwindow_height;

	sensor_resolution->SensorPreviewWidth = imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight = imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth = imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight = imgsensor_info.normal_video.grabwindow_height;


	sensor_resolution->SensorHighSpeedVideoWidth = imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight = imgsensor_info.hs_video.grabwindow_height;

	sensor_resolution->SensorSlimVideoWidth = imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight = imgsensor_info.slim_video.grabwindow_height;

	sensor_resolution->SensorCustom1Width  = imgsensor_info.custom1.grabwindow_width;
	sensor_resolution->SensorCustom1Height     = imgsensor_info.custom1.grabwindow_height;
	return ERROR_NONE;
}				/*    get_resolution    */

static kal_uint32 get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			   MSDK_SENSOR_INFO_STRUCT *sensor_info,
			   MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);


	/* sensor_info->SensorVideoFrameRate = imgsensor_info.normal_video.max_framerate/10;  not use */
	/* sensor_info->SensorStillCaptureFrameRate= imgsensor_info.cap.max_framerate/10;  not use */
	/* imgsensor_info->SensorWebCamCaptureFrameRate= imgsensor_info.v.max_framerate;  not use */

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;	/* not use */
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;	/* inverse with datasheet */
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4;	/* not use */
	sensor_info->SensorResetActiveHigh = FALSE;	/* not use */
	sensor_info->SensorResetDelayCount = 5;	/* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat = imgsensor_info.sensor_output_dataformat;

	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;
	sensor_info->Custom1DelayFrame = imgsensor_info.custom1_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0;	/* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;

	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;
	/* The frame of setting shutter default 0 for TG int */
	sensor_info->AESensorGainDelayFrame = imgsensor_info.ae_sensor_gain_delay_frame;
	/* The frame of setting sensor gain */
	sensor_info->AEISPGainDelayFrame = imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;
	sensor_info->PDAF_Support = PDAF_SUPPORT_RAW_LEGACY;

	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3;	/* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2;	/* not use */
	sensor_info->SensorPixelClockCount = 3;	/* not use */
	sensor_info->SensorDataLatchCount = 2;	/* not use */

	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;	/* 0 is default 1x */
	sensor_info->SensorHightSampling = 0;	/* 0 is default 1x */
	sensor_info->SensorPacketECCOrder = 1;

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
		    imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
		    imgsensor_info.cap.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:

		sensor_info->SensorGrabStartX = imgsensor_info.normal_video.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.normal_video.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
		    imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
		    imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		sensor_info->SensorGrabStartX = imgsensor_info.slim_video.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.slim_video.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
		    imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;

		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		sensor_info->SensorGrabStartX = imgsensor_info.custom1.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom1.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
		    imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;

		break;
	default:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
		    imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
		break;
	}

	return ERROR_NONE;
}				/*    get_info  */


static kal_uint32 control(enum MSDK_SCENARIO_ID_ENUM scenario_id,
				MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		preview(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		capture(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		normal_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		hs_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		slim_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		custom1(image_window, sensor_config_data);
		break;
	default:
		LOG_INF("Error ScenarioId setting");
		preview(image_window, sensor_config_data);
		return ERROR_INVALID_SCENARIO_ID;
	}
	return ERROR_NONE;
}				/* control() */



static kal_uint32 set_video_mode(UINT16 framerate)
{
	LOG_INF("framerate = %d\n ", framerate);
	/* SetVideoMode Function should fix framerate */
	if (framerate == 0)
		/* Dynamic frame rate */
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;
	else
		imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps, 1);

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	LOG_INF("enable = %d, framerate = %d\n", enable, framerate);
	spin_lock(&imgsensor_drv_lock);
	if (enable)		/* enable auto flicker */
		imgsensor.autoflicker_en = KAL_TRUE;
	else			/* Cancel Auto flick */
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}


static kal_uint32 set_max_framerate_by_scenario(
					enum MSDK_SCENARIO_ID_ENUM scenario_id,
							MUINT32 framerate)
{
	kal_uint32 frame_length;

	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		frame_length =
		    imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
		    (frame_length >
		     imgsensor_info.pre.framelength) ? (frame_length -
							imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		if (framerate == 0)
			return ERROR_NONE;
		frame_length =
		    imgsensor_info.normal_video.pclk / framerate * 10 /
		    imgsensor_info.normal_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
		    (frame_length >
		     imgsensor_info.normal_video.framelength) ? (frame_length -
								 imgsensor_info.normal_video.
								 framelength) : 0;
		imgsensor.frame_length =
		    imgsensor_info.normal_video.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		frame_length =
		imgsensor_info.cap.pclk / framerate * 10 / imgsensor_info.cap.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
		(frame_length >
		imgsensor_info.cap.framelength) ? (frame_length -
		imgsensor_info.cap.framelength) : 0;
		imgsensor.frame_length = imgsensor_info.cap.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
	break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		frame_length =
		    imgsensor_info.hs_video.pclk / framerate * 10 /
		    imgsensor_info.hs_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
		    (frame_length >
		     imgsensor_info.hs_video.framelength) ? (frame_length -
							     imgsensor_info.hs_video.
							     framelength) : 0;
		imgsensor.frame_length = imgsensor_info.hs_video.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
		    set_dummy();
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		frame_length =
		    imgsensor_info.slim_video.pclk / framerate * 10 /
		    imgsensor_info.slim_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
		    (frame_length >
		     imgsensor_info.slim_video.framelength) ? (frame_length -
							       imgsensor_info.slim_video.
							       framelength) : 0;
		imgsensor.frame_length =
		    imgsensor_info.slim_video.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		frame_length =
		    imgsensor_info.custom1.pclk / framerate * 10 / imgsensor_info.custom1.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
		    (frame_length >
		     imgsensor_info.custom1.framelength) ? (frame_length -
							imgsensor_info.custom1.framelength) : 0;
		imgsensor.frame_length = imgsensor_info.custom1.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	default:		/* coding with  preview scenario by default */
		frame_length =
		    imgsensor_info.pre.pclk / framerate * 10 / imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
		    (frame_length >
		     imgsensor_info.pre.framelength) ? (frame_length -
							imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length = imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		LOG_INF("error scenario_id = %d, we use preview scenario\n", scenario_id);
		break;
	}
	return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(
				enum MSDK_SCENARIO_ID_ENUM scenario_id,
				MUINT32 *framerate)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		*framerate = imgsensor_info.pre.max_framerate;
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		*framerate = imgsensor_info.normal_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		*framerate = imgsensor_info.cap.max_framerate;
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		*framerate = imgsensor_info.hs_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		*framerate = imgsensor_info.slim_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		*framerate = imgsensor_info.custom1.max_framerate;
		break;
	default:
		break;
	}

	return ERROR_NONE;
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);

	if (enable) {
    /*  0x5E00[8]: 1 enable,  0 disable
     0x5E00[1:0]; 00 Color bar, 01 Random Data, 10 Square, 11 BLACK */
		write_cmos_sensor(0x3202, 0x0080);
		write_cmos_sensor(0x3204, 0x0080);
		write_cmos_sensor(0x3206, 0x0080);
		write_cmos_sensor(0x3208, 0x0080);
		write_cmos_sensor(0x3232, 0x0000);
		write_cmos_sensor(0x3234, 0x0000);
		write_cmos_sensor(0x32a0, 0x0100);
		write_cmos_sensor(0x3300, 0x0001);
		write_cmos_sensor(0x3400, 0x0001);
		write_cmos_sensor(0x3402, 0x4e00);
		write_cmos_sensor(0x3268, 0x0000);
		write_cmos_sensor(0x0600, 0x0002);
	} else {
    /*  0x5E00[8]: 1 enable,  0 disable
     0x5E00[1:0]; 00 Color bar, 01 Random Data, 10 Square, 11 BLACK */
		write_cmos_sensor(0x3202, 0x0000);
		write_cmos_sensor(0x3204, 0x0000);
		write_cmos_sensor(0x3206, 0x0000);
		write_cmos_sensor(0x3208, 0x0000);
		write_cmos_sensor(0x3232, 0x0000);
		write_cmos_sensor(0x3234, 0x0000);
		write_cmos_sensor(0x32a0, 0x0000);
		write_cmos_sensor(0x3300, 0x0000);
		write_cmos_sensor(0x3400, 0x0000);
		write_cmos_sensor(0x3402, 0x0000);
		write_cmos_sensor(0x3268, 0x0000);
		write_cmos_sensor(0x0600, 0x0000);
	}

	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}

static kal_uint32 streaming_control(kal_bool enable)
{
	kal_uint16 val = 0;
	int index = 0;
	LOG_INF("streaming_enable(0=Sw Standby,1=streaming): %d\n", enable);

	if (enable)
		write_cmos_sensor(0x0100, 0x0100);
	else {
		write_cmos_sensor(0x0100, 0x0000);
		mdelay(3);
		for (index = 0; index < OFF_READY_DELAY; index++) {
			val = read_cmos_sensor(0x0005);
			LOG_INF("mtk_test: 0x0005 = 0x%x\n", val);
			if (val == 0xff) {
				LOG_INF("mtk_test: index = %d\n", index);
				break;
			}
			mdelay(5);
		}
	}
	return ERROR_NONE;
}

static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
				  UINT8 *feature_para, UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16 = (UINT16 *) feature_para;
	UINT16 *feature_data_16 = (UINT16 *) feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *) feature_para;
	UINT32 *feature_data_32 = (UINT32 *) feature_para;
	unsigned long long *feature_data = (unsigned long long *)feature_para;
	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	struct SET_PD_BLOCK_INFO_T *PDAFinfo;
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data = (MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

	LOG_INF("feature_id = %d\n", feature_id);
	switch (feature_id) {
	case SENSOR_FEATURE_GET_PERIOD:
		*feature_return_para_16++ = imgsensor.line_length;
		*feature_return_para_16 = imgsensor.frame_length;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
		*feature_return_para_32 = imgsensor.pclk;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_ESHUTTER:
		set_shutter(*feature_data);
		break;
	case SENSOR_FEATURE_SET_NIGHTMODE:
		night_mode((BOOL) * feature_data);
		break;
	case SENSOR_FEATURE_SET_GAIN:
		set_gain((UINT16) *feature_data);
		break;
	case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
		break;
	case SENSOR_FEATURE_SET_REGISTER:
		write_cmos_sensor(sensor_reg_data->RegAddr, sensor_reg_data->RegData);
		break;
	case SENSOR_FEATURE_GET_REGISTER:
		sensor_reg_data->RegData = read_cmos_sensor(sensor_reg_data->RegAddr);
		break;
	case SENSOR_FEATURE_SET_SHUTTER_FRAME_TIME:
		set_shutter_frame_length((UINT16) (*feature_data), (UINT16) (*(feature_data + 1)));
		break;
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
		/* get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE */
		/* if EEPROM does not exist in camera module. */
		*feature_return_para_32 = LENS_DRIVER_ID_DO_NOT_CARE;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_VIDEO_MODE:
		set_video_mode(*feature_data);
		break;
	case SENSOR_FEATURE_CHECK_SENSOR_ID:
		get_imgsensor_id(feature_return_para_32);
		break;
	case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
		set_auto_flicker_mode((BOOL) * feature_data_16, *(feature_data_16 + 1));
		break;
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
			set_max_framerate_by_scenario(
				(enum MSDK_SCENARIO_ID_ENUM) *feature_data, *(feature_data+1));
			break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
			get_default_framerate_by_scenario(
				(enum MSDK_SCENARIO_ID_ENUM) *(feature_data),
				(MUINT32 *)(uintptr_t)(*(feature_data+1)));
			break;
	case SENSOR_FEATURE_SET_TEST_PATTERN:
		set_test_pattern_mode((BOOL) * feature_data);
		break;
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:	/* for factory mode auto testing */
		*feature_return_para_32 = imgsensor_info.checksum_value;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_FRAMERATE:
		LOG_INF("current fps :%d\n", *feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.current_fps = (UINT16) *feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_SET_HDR:
		LOG_INF("ihdr enable :%d\n", *feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.ihdr_en = (BOOL) * feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_GET_CROP_INFO:
		/* For pointer address in user space is 32-bit, need use compat_ptr to */
		/* convert it to 64-bit in kernel space */
			wininfo = (struct SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));

		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[1],
						sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[2],
						sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[3],
						sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[4],
						sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[5],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy((void *)wininfo, (void *)&imgsensor_winsize_info[0],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		}
		break;
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
		LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n", *feature_data_32,
			*(feature_data_32 + 1), *(feature_data_32 + 2));
		ihdr_write_shutter_gain(*feature_data_32, *(feature_data_32 + 1),
					*(feature_data_32 + 2));
		break;
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
		{
			kal_uint32 rate;

			switch (*feature_data) {
			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
				rate = imgsensor_info.cap.mipi_pixel_rate;
				break;
			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
				rate = imgsensor_info.normal_video.mipi_pixel_rate;
				break;
			case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
				rate = imgsensor_info.hs_video.mipi_pixel_rate;
				break;
			case MSDK_SCENARIO_ID_SLIM_VIDEO:
				rate = imgsensor_info.slim_video.mipi_pixel_rate;
				break;
			case MSDK_SCENARIO_ID_CUSTOM1:
				rate = imgsensor_info.custom1.mipi_pixel_rate;
				break;
			case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			default:
				rate = imgsensor_info.pre.mipi_pixel_rate;
				break;
			}
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = rate;
		}
		break;
	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_SUSPEND\n");
		streaming_control(KAL_FALSE);
		break;
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_RESUME, shutter:%llu\n", *feature_data);
		if (*feature_data != 0)
			set_shutter(*feature_data);
		streaming_control(KAL_TRUE);
		break;
	case SENSOR_FEATURE_GET_PDAF_INFO:
		LOG_INF("mtk_test SENSOR_FEATURE_GET_PDAF_INFO scenarioId:%lld\n", *feature_data);
		PDAFinfo = (struct SET_PD_BLOCK_INFO_T *) (uintptr_t) (*(feature_data + 1));
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)PDAFinfo, (void *)&imgsensor_pd_info,
			       sizeof(struct SET_PD_BLOCK_INFO_T));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_CUSTOM1:
		default:
			break;
		}
		break;
	case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
		LOG_INF("mtk_test SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY scenarioId:%lld\n", *feature_data);
		/* PDAF capacity enable or not, S5K3L6 only full size support PDAF */
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = 1;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = 0;	/* video & capture use same setting */
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = 0;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = 0;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = 0;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = 0;
			break;
		default:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) = 0;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_PIXEL_RATE:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.cap.pclk /
			(imgsensor_info.cap.linelength - 80))*
			imgsensor_info.cap.grabwindow_width;
			break;

		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.normal_video.pclk /
			(imgsensor_info.normal_video.linelength - 80))*
			imgsensor_info.normal_video.grabwindow_width;
			break;

		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.hs_video.pclk /
			(imgsensor_info.hs_video.linelength - 80))*
			imgsensor_info.hs_video.grabwindow_width;
			break;

		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.slim_video.pclk /
			(imgsensor_info.slim_video.linelength - 80))*
			imgsensor_info.slim_video.grabwindow_width;
			break;

		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) =
			(imgsensor_info.pre.pclk /
			(imgsensor_info.pre.linelength - 80))*
			imgsensor_info.pre.grabwindow_width;
			break;
		}
		break;
	default:
		break;
	}

	return ERROR_NONE;
}				/*    feature_control()  */

static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};


UINT32 S5K3L6_Z6100T_064_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc = &sensor_func;
	return ERROR_NONE;
}				/*    S5K3L6_Z6100T_064MIPISensorInit    */
