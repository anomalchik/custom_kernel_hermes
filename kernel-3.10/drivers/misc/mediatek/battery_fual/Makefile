include $(srctree)/drivers/misc/mediatek/Makefile.custom

# In case the platform does NOT support this type of sensors

ifeq ($(CONFIG_FG_CW2015),y)
obj-$(CONFIG_MTK_SMART_BATTERY) += cw2015_battery.o charging_hw_pmic.o
endif

