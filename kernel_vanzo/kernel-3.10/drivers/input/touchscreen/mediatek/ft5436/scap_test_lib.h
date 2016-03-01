#ifndef _FT_RAWDATA_H
#define _FT_RAWDATA_H

#define MAX_CHANNEL 64
#define boolean unsigned char
#define false 0
#define true  1


typedef int (*FTS_I2c_Read_Function)(unsigned char * , int , unsigned char *, int);
typedef int (*FTS_I2c_Write_Function)(unsigned char * , int );

int Init_I2C_Read_Func(FTS_I2c_Read_Function fpI2C_Read);
int Init_I2C_Write_Func(FTS_I2c_Write_Function fpI2C_Write);

int SetParamData(char * TestParamData);
int focal_save_scap_sample(char * databuf, int databuflen);
void FreeTestParamData(void);
boolean StartTestTP(void);		//return true pass,else ng

#endif
