#!/bin/bash
e="\x1b[";c=$e"39;49;00m";y=$e"93;01m";cy=$e"96;01m";r=$e"1;91m";g=$e"92;01m";m=$e"95;01m";
##########################################################
#                    Based on:                                    #
#           Carliv Image Kitchen for Android             #
#     boot+recovery images copyright-2015 carliv@xda     #
#    including support for MTK powered phones images     #
#                                                        #
##########################################################
bin=.bin;
chmod -R 755 "$bin";
chmod -R 755 scripts;
cd "";
mkdir output
###########################################################
bfolder="/boot"
boot_repack()
{
if [ -d "$bfolder" ]; 
	then
	scripts/repack_img "$bfolder";
	else
	echo -e "The folder for repack $g $bfolder$c doesn't exists. Are you sure you didn't delete it?";
fi;
}

boot_repack;
