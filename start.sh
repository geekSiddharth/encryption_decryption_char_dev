#! /bin/bash

# remove dev files if already present
sudo rm /dev/decdev
sudo rm /dev/encdev

# makes device file
sudo mknod /dev/decdev c 601 0
sudo mknod /dev/encdev c 600 0

# transfer access to the user
sudo chown $USER /dev/decdev
sudo chown $USER /dev/encdev

# gives read and write access
sudo chmod +rw /dev/decdev
sudo chmod +rw /dev/encdev
