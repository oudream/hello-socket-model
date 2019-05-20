#!/bin/sh
TESTDEV=/home/stathis/eleutheria/cdev
NETBSDDISK=/usr/nbsd-disk
SRCDIR=/usr/src
DOMUNAME=nbsd

# Copy source code files to netbsd source tree
cp -v mydev.c $SRCDIR/sys/dev
cp -v mydev.h $SRCDIR/sys/sys

# Recompile kernel
cd $SRCDIR
./build.sh  -O ../obj -T ../tools -u kernel=XEN3_DOMU || exit

# Build testdev userland program
cd $TESTDEV
echo "Building testdev..."
echo "Current directory:" `pwd`
gcc -Wall -W testdev.c -o testdev -lprop -I /usr/src/sys -Wall || exit

# Shutdown domain if it's already running
DOMUID=`xm list | grep "$DOMUNAME" | awk {'print $2'}`
if [ "$DOMUID" ];
then
    xm shutdown $DOMUID
fi

# Copy testdev in domain's virtual disk
sleep 2
echo "Configuring vnode pseudo disk device..."
vnconfig vnd0 $NETBSDDISK || exit

echo "Mounting netbsd disk image to device..."
mount /dev/vnd0a /mnt || exit

echo "Copying testdev to /root"
cp /home/stathis/eleutheria/cdev/testdev /mnt/root

echo "Unmounting netbsd disk image..."
umount /dev/vnd0a || exit

"Unconfiguring vnode pseudo disk device..."
vnconfig -u vnd0 || exit

echo "DONE"
