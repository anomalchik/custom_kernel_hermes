#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/slab.h>
#include <linux/xlog.h>

#define PFX "S5K3M2_PDAFOTP"
#define S5K3M_USE_WB_OTP
#define LOG_INF(format, args...)	xlog_printk(ANDROID_LOG_INFO   , PFX, "[%s] " format, __FUNCTION__, ##args)

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
extern void kdSetI2CSpeed(u16 i2cSpeed);
//extern int iBurstWriteReg_multi(u8 *pData, u32 bytes, u16 i2cId, u16 transfer_length);
extern int iMultiReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId, u8 number);


#define USHORT             unsigned short
#define BYTE               unsigned char
#define Sleep(ms) mdelay(ms)

#define S5K3M2_EEPROM_READ_ID  0xA0
#define S5K3M2_EEPROM_WRITE_ID   0xA3
#define S5K3M2_I2C_SPEED        100  
#define S5K3M2_MAX_OFFSET		0xFFFF

#define DATA_SIZE 2048
BYTE s5k3m2_eeprom_pdaf_data[DATA_SIZE]= {0};
static bool get_done = false;

static bool selective_read_eeprom(kal_uint16 addr, BYTE* data)
{
	char pu_send_cmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    if(addr > S5K3M2_MAX_OFFSET)
        return false;
	kdSetI2CSpeed(S5K3M2_I2C_SPEED);

	if(iReadRegI2C(pu_send_cmd, 2, (u8*)data, 1, S5K3M2_EEPROM_READ_ID)<0)
		return false;
    return true;
}

static bool _read_3m2_eeprom(kal_uint16 addr, BYTE* data, kal_uint32 size ){
	int i = 0;
	int offset = addr;
	for(i = 0; i < size; i++) {
		if(!selective_read_eeprom(offset, &data[i])){
			return false;
		}
		LOG_INF("read_eeprom 0x%0x %d\n",offset, data[i]);
		offset++;
	}
    return true;
}

bool read_3m2_eeprom( kal_uint16 addr, BYTE* data, kal_uint32 size){
	int i;
	addr = 0x0791;
	size = 496;
	
	LOG_INF("read 3m2 eeprom, size = %d\n", size);

	if (false == get_done)
	{
		//read proc 1
		if(!_read_3m2_eeprom(addr, s5k3m2_eeprom_pdaf_data, size))
		{
			return false;
		}

		//read proc 2
		addr = 0x0983;
		size = 806;
		if(!_read_3m2_eeprom(addr, &s5k3m2_eeprom_pdaf_data[496], size))
		{
			return false;
		}

		//read proc 3
		addr = 0x0CAB;
		size = 102;
		if(!_read_3m2_eeprom(addr, &s5k3m2_eeprom_pdaf_data[496+806], size))
		{
			return false;
		}

		get_done = true;
	}

	memcpy(data, s5k3m2_eeprom_pdaf_data, 496+806+102);
    return true;
}

#if defined(S5K3M_USE_WB_OTP)
#define SENSORDB(fmt, arg...) printk( "[S5K3M_OTP] "  fmt, ##arg)

// R/G and B/G of typical camera module is defined here
static uint32_t RG_Ratio_typical=0x237;
static uint32_t BG_Ratio_typical=0x26D;
static kal_uint16 r_current=0,g_current=0,b_current=0;

//extern function
extern void write_cmos_sensor_8(kal_uint16 addr, kal_uint8 para);
extern void write_cmos_sensor(kal_uint16 addr, kal_uint16 para);

static kal_uint16 s5k3m_read_eeprom(kal_uint16 addr)
{
    kal_uint16 get_byte=0;
    char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
	
    iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,S5K3M2_EEPROM_READ_ID);
    return get_byte;
}

// return:  0: OTP value not valid, 1: OTP value valid
static int check_otp_wb()
{
    int flag_Pro,flag_AWB;
    int index;
    int bank, address;
	
	flag_AWB= s5k3m_read_eeprom(0x1C);//awb valid flag
    
    SENSORDB("check_otp_wb,  flag_AWB = %x\n",  flag_AWB);

	if(flag_AWB == 0x01)
		return 1;
	else 
		return 0;
}
// For HW

// otp_ptr: pointer of otp_struct
// return:  0,
static int read_otp_wb()
{
   
    kal_uint16 otp_data[12]= {0};
    kal_uint16 address=0x001C;
	int otp_awb_size = 0x0022 - 0x001C;
    int i=0;
    for(i; i<= otp_awb_size+1; i++)
    {
        otp_data[i] = s5k3m_read_eeprom(address+i);
		#ifdef S5K3M_OTP_DEBUG
			SENSORDB("read_otp_wb i=0x%x value=0x%x \n",i,otp_data[i]);
		#endif
    }

    r_current = (otp_data[1]<<8)|otp_data[2];
    b_current = (otp_data[3]<<8)|otp_data[4];
    g_current = (otp_data[5]<<8)|otp_data[6];

	SENSORDB("%s,otp_data[0]=0x%x otp_data[1]=0x%x otp_data[2]=0x%x \n",__func__,otp_data[0],otp_data[1],otp_data[2]);
	SENSORDB("%s,r_gain=0x%x b_gain=0x%x g_gain=0x%x \n",__func__,r_current,b_current,g_current);
    return 0;
}



static void AWBfors5k3m2(kal_uint16 RoverG_dec,kal_uint16 BoverG_dec,kal_uint16 GboverGr_dec)
{
	kal_uint16 RoverG_dec_base,BoverG_dec_base,GboverGr_dec_base;
	kal_uint16 R_test,B_test,G_test;
	kal_uint16 R_test_H3,R_test_L8,B_test_H3,B_test_L8,G_test_H3,G_test_L8;
	
	RoverG_dec_base = RG_Ratio_typical;//the typcical value
	BoverG_dec_base = BG_Ratio_typical;//the typcical value
	GboverGr_dec_base = 0;//the typcical value
	
	kal_uint32 G_test_R, G_test_B;
	
	if(BoverG_dec < BoverG_dec_base)
	{
		if (RoverG_dec < RoverG_dec_base)
		{
			G_test = 0x100;
			B_test = 0x100 * BoverG_dec_base / BoverG_dec;
			R_test = 0x100 * RoverG_dec_base / RoverG_dec;
		}
		else
		{
			R_test = 0x100;
			G_test = 0x100 * RoverG_dec / RoverG_dec_base;
			B_test = G_test * BoverG_dec_base / BoverG_dec;
		}
	}
	else
	{
		if (RoverG_dec < RoverG_dec_base)
		{
			B_test = 0x100;
			G_test = 0x100 * BoverG_dec / BoverG_dec_base;
			R_test = G_test * RoverG_dec_base / RoverG_dec;
		}
		else
		{
			G_test_B = BoverG_dec * 0x100 / BoverG_dec_base;
			G_test_R = RoverG_dec * 0x100 / RoverG_dec_base;
			if(G_test_B > G_test_R )
			{
				B_test = 0x100;
				G_test = G_test_B;
				R_test = G_test_B * RoverG_dec_base / RoverG_dec;
			}
			else
			{
				R_test = 0x100;
				G_test = G_test_R;
				B_test = G_test_R * BoverG_dec_base / BoverG_dec;
			}
		}
	}
	if(R_test < 0x100)
	{
		R_test = 0x100;
	}
	if(G_test < 0x100)
	{
		G_test = 0x100;
	}
	if(B_test < 0x100)
	{
		B_test = 0x100;
	}
	
	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x602A, 0x020E);
	write_cmos_sensor(0x6F12,G_test);
	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x602A, 0x0210);
	write_cmos_sensor(0x6F12, R_test);//R_test
	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x602A, 0x0212);
	write_cmos_sensor(0x6F12,B_test);
	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x602A, 0x0214);	
	write_cmos_sensor(0x6F12,G_test);
	
	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x602A, 0x3056);
	write_cmos_sensor_8(0x6F12, 0x01);
	SENSORDB("R_test=0x%x,G_test=0x%x,B_test=0x%x",R_test,G_test,B_test);
}

// call this function after S5K3M initialization
// return value: 0 update success
//      1, no OTP
int s5k3m_update_otp_wb()
{
   
    int otp_valid;
   
    SENSORDB("s5k3m_update_otp_wb,\n");
    otp_valid = check_otp_wb();
    if(otp_valid==0)
    {
        // no valid wb OTP data
        SENSORDB("s5k3m_update_otp_wb,no valid wb OTP data\n");
        return 1;
    }
	read_otp_wb();
    AWBfors5k3m2(r_current,b_current,g_current);

    return 0;
}
#endif /*S5K3M_USE_WB_OTP*/

