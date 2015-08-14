#!/bin/bash

if grep -qs '/dev/loop2' /proc/mounts; then
    echo "ArcanOS disk image already mounted"
else
    sudo losetup /dev/loop1 boot.img
    sudo losetup /dev/loop2 -o 1048576 /dev/loop1
    sudo mount /dev/loop2 mnt
fi
    sudo cp -v obj/kern/kernel mnt/kernel
    sync
    sudo umount /dev/loop2
    sudo losetup -d /dev/loop2
    sudo losetup -d /dev/loop1
