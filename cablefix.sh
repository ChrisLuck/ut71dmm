#!/bin/bash

# 2014 Christian Lück
# Suspend the datacable (otherwise it won't work properly)
# Thanks to Ralf Burger for figuring this out.
# Further information about this: http://erste.de/UT61/index.html

for dat in /sys/bus/usb/devices/*;
        do 
        if ([ -e $dat/idVendor ] && [ -e $dat/idProduct ]); then 
		grep "1a86" $dat/idVendor>/dev/null&&
		grep "e008" $dat/idProduct>/dev/null&&
		echo auto>${dat}/power/level&&
		echo 0>${dat}/power/autosuspend&&
		echo "success! ("$dat")"
        fi      
done
