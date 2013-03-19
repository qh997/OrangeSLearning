#!/bin/bash

sudo mount -o loop images/pm.img /mnt/floppy
sudo cp execute.com /mnt/floppy/.
sudo umount /mnt/floppy