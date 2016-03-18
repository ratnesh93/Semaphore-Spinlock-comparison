dmesg --clear
make
insmod prod_cons_spinlock_cs12b1030.ko
rmmod prod_cons_spinlock_cs12b1030
dmesg >> output