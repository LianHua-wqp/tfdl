#make clean
make
#mv Debug/FWT0311 /tftpboot/
scp tfdl root@192.168.1.230:/home/root/tfdl
#scp root@192.168.6.127:/home/root/tmp.txt ./