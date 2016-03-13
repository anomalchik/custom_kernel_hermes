# Custom kernel for Xiaomi Hermes (Redmi Note 2)
# Kernel version 3.10.61 LCSH and Vanzo Hybrid
Works in rom(tested 7.0.8.0) and recovery

* Works:
	* LCM(nt35596 tianma)
	* Sdcard
	* Wi-fi
	* Bt
	* IOCTL (fixed hwcomposer and surfaceflinger)

* Partitialy works:
	* Touch(focaltech) - menu button don't work
	* Alsps (ps bugged)
	* Accel
	* Mag
	* Giro
	* Battery 3000mah(sw battery driver)

* Don't work:
	* LCM (nt35532_boe, nt35596_auo)
	* MD1 and MD2
	* Touch(atmel)
	* Imgsensor(all img sensors)
	* Lens
	* CW2015 (hw battery driver)
	* Other
=================================================

