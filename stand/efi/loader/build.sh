#!/bin/sh

set -e

echo "Cleaning files..."

sudo make clean cleandir cleandepend

echo "Making files..."

sudo make -j4

echo "Installing files..."

sudo make install

echo "Copying loader.efi over..."

sudo cp /boot/loader.efi /boot/efi/efi/freebsd/

echo "Copying sym file over..."

ls -l /usr/obj/usr/src/amd64.amd64/stand/efi/loader/

sleep 30s

sudo cp /usr/obj/usr/src/amd64.amd64/stand/efi/loader/loader_lua.sym.debug /usr/src/stand/efi/loader/

echo "Adding sym file to git..."

git add loader_lua.sym.debug

git commit -m "sym file"

git push origin sym_file

echo "Successfully added to git. Shutting down..."

sudo poweroff
