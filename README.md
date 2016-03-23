# Custom kernel for Xiaomi Hermes (Redmi Note 2)
# Kernel version 3.10.61 LCSH and Vanzo Hybrid
Works in rom(tested 7.0.8.0) and recovery

* Works:
	* LCM(nt35596 tianma)
	* tps65132
	* Sdcard
	* Wi-fi
	* IOCTL (fixed hwcomposer and surfaceflinger)
	* da9210 (charger driver)
	* Bt
	* Button-backlight
	* Brightness
	* Leds indication only (red,green,blu)
	* MD1 and MD2(sim1 and sim2)

* Partitialy works:
	* Touch(focaltech)
	* Alsps (ps bugged)
	* Accel
	* Mag
	* Giro
	* Battery 3000mah(sw battery driver)

* Don't work:
	* tps6128x (mt-i2c: transfer error)
	* LCM (nt35532_boe, nt35596_auo)
	* Touch(atmel)
	* Imgsensor(all img sensors)
	* Lens
	* CW2015 (hw battery driver)
	* Other

=================================================
# BUILD
Сonfigure shell script "build.sh" and RUN!

# OTHER
Dts forked from stock

# I2c

* I2C0
	* tps65132              (003e)
	* kd_camera_hw          (007f)
	* DF9761BAF             (0018) - LENS
	* CAM_CAL_DRV           (0036)

* I2C1
	* da9210                (0068)
	* tps6128x              (0075)

* I2C2
	* atmel                 (004a)
	* kd_camera_hw_bus 2    (007f)
	* FT			(0038)

* I2C3
	* akm0991               (000c)
	* yas537                (002e)
	* LSM6DS3_ACCEL         (006a)
	* LTR_559ALS		(0023)
	* LSM6DS3_GYRO		(0034)
	* stk3x1x               (0048)
	* bmi160_gyro		(0066)
	* bmi160_acc		(0068)

* I2C4
	* CW2015 		(0062)

# AUTORS
* nofearnohappy
* LazyC0DEr
* Anomalchik

# Thanks to
* ariafan
