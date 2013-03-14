#!/bin/bash

sudo mount -o loop pm.img /mnt/floppy
sudo cp mycom.com /mnt/floppy/.
sudo umount /mnt/floppy