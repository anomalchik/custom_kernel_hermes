/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#include <mach/mt_gpio.h>

#include <linux/delay.h>
#include <asm/delay.h>

#include <cust_leds.h>
#include <cust_leds_def.h>

#include <mach/mt_pwm.h>

#include <linux/kernel.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>

#define GPIO_LCD_BIAS_ENP_PIN (GPIO131 | 0x80000000)

#define BACKLIGHT_LEVEL_PWM_MODE_CONFIG BACKLIGHT_LEVEL_PWM_256_SUPPORT

// Only support 64 levels of backlight (when lcd-backlight = MT65XX_LED_MODE_PWM)
#define BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT 64 
// Support 256 levels of backlight (when lcd-backlight = MT65XX_LED_MODE_PWM)
#define BACKLIGHT_LEVEL_PWM_256_SUPPORT 256 

// Configure the support type "BACKLIGHT_LEVEL_PWM_256_SUPPORT" or "BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT" !!
#define BACKLIGHT_LEVEL_PWM_MODE_CONFIG BACKLIGHT_LEVEL_PWM_256_SUPPORT

unsigned int Cust_GetBacklightLevelSupport_byPWM(void)
{
    return BACKLIGHT_LEVEL_PWM_MODE_CONFIG;
}

unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;

    mapped_level = level;

    return mapped_level;
}


static unsigned int back_level = 1023 >> 5;
#define T_LONG 7
#define T_SHORT 3
unsigned int disp_set_backlight(int level)
{
    int now_level,addr = 0x72;
    int i;

    mt_set_gpio_mode(GPIO_LCD_BIAS_ENP_PIN, GPIO_MODE_GPIO);
    mt_set_gpio_dir(GPIO_LCD_BIAS_ENP_PIN, GPIO_DIR_OUT);
    mt_set_gpio_pull_enable(GPIO_LCD_BIAS_ENP_PIN, GPIO_PULL_DISABLE);

    now_level = (level >> 5);
    if(now_level == back_level)
        return 0;
	printk("%s line %d level %d now_level %d back_level %d ",__func__,__LINE__,level, now_level,back_level);
    if(level == 0) {
        mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
    } else {
            if(back_level == 0) {
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
                mdelay(3);
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
                udelay(150);
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
                udelay(450);
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
                udelay(580);
            }
       mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
//                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
//                udelay(150);
//                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
//                udelay(450);
//                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
//                udelay(200);
        udelay(4);
        for(i=0 ;i < 8;i++)
        {
            if(addr&0x80)
            {
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
                udelay(T_SHORT);
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
                udelay(T_LONG);
            }
            else
            {
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
                udelay(T_LONG);
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
                udelay(T_SHORT);
            }
            addr <<= 1;
        }
        mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
        udelay(4);
        mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
        udelay(4);
        for(i=0 ;i < 8;i++)
        {
            if(now_level&0x80)
            {
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
                udelay(T_SHORT);
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
                udelay(T_LONG);
            }
            else
            {
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
                udelay(T_LONG);
                mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
                udelay(T_SHORT);
            }
            now_level <<= 1;
        }
        mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
        udelay(4);
        mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
        udelay(4);
    }
        back_level = (level >> 5) ;
    return 0;
}
#if 0
#define KEYPAD_GPIO_PIN (GPIO146 | 0x80000000)

unsigned int l901_set_keylight(int level)
{
  mt_set_gpio_mode(KEYPAD_GPIO_PIN, 0); /* GPIO mode */
  mt_set_gpio_pull_enable(KEYPAD_GPIO_PIN,GPIO_PULL_ENABLE);
  mt_set_gpio_pull_select(KEYPAD_GPIO_PIN,GPIO_PULL_DOWN);

  if(level)
    mt_set_gpio_out(KEYPAD_GPIO_PIN, GPIO_OUT_ONE);
  else
    mt_set_gpio_out(KEYPAD_GPIO_PIN, GPIO_OUT_ZERO);

}
#endif

static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK0,{0}},
	{"green",             MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"blue",              MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK2,{0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK3,{0}},
        {"lcd-backlight",     MT65XX_LED_MODE_CUST_BLS_PWM, (long)disp_set_backlight,{0}},
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
    return cust_led_list;
}
