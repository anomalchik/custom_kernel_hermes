# Custom kernel for Xiaomi Hermes (Redmi Note 2)
# Kernel version 3.10.61 LCSH and Vanzo Hybrid
Works in all roms(libsrv_init.so is include to buildable zip) and recovery

* Works:
	* LCM(nt35596_tianma, nt35596_auo)
	* tps65132
	* Sdcard
	* Wi-fi
	* IOCTL (fixed hwcomposer and surfaceflinger)
	* da9210 (charger driver)
	* Bt
	* Button-backlight
	* Brightness
	* Leds indication (only red,green,blue)
	* MD1 and MD2(sim1 and sim2)
	* Touch(focaltech and atmel)
	* Sound

* Partitialy works:
	* Alsps (ps bugged)
	* Accel
	* Mag
	* Giro
	* Battery 3000mah(sw battery driver)

* Don't work:
	* tps6128x (mt-i2c: transfer error)
	* LCM (nt35532_boe)
	* Imgsensor(all img sensors)
	* Lens
	* CW2015 (hw battery driver)
	* Other

=================================================
# BUILD
Сonfigure shell script "build.sh" and RUN!

# CLEAN
Сonfigure shell script "clean.sh" and RUN!

# OTHER
Dts forked from stock

# I2c

* I2C0
	* tps65132              (003e) - lcm
	* kd_camera_hw          (007f) - kd_sensor_list
	* DF9761BAF             (0018) - LENS
	* CAM_CAL_DRV           (0036) - S5K3M2 cam cal driver

* I2C1
	* da9210                (0068) - charger
	* tps6128x              (0075) - charger

* I2C2
	* atmel                 (004a) - atmel touch
	* kd_camera_hw_bus 2    (007f) - kd_sensor_list
	* FT			(0038) - focaltech touch

* I2C3
	* akm0991               (000c)
	* yas537                (002e)
	* LSM6DS3_ACCEL         (006a) - accelerometer
	* LTR_559ALS		(0023) - AutomaticLightSensor/ProximitySensor
	* LSM6DS3_GYRO		(0034) - gyroscope
	* stk3x1x               (0048)
	* bmi160_gyro		(0066) - gyroscope
	* bmi160_acc		(0068) - accelerometer 

* I2C4
	* CW2015 		(0062) - hw charger driver

# AUTORS
* nofearnohappy
* LazyC0DEr
* Anomalchik

# Thanks to
* ariafan
