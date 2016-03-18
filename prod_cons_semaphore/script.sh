dmesg --clear
make
insmod prodConsSemaphoreCs12b1030.ko
rmmod prodConsSemaphoreCs12b1030
dmesg >> output
