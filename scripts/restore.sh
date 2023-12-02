#!/bin/bash
# Automated secure off-site restore
# For GS source code
RED="$(tput setaf 1)"
GREEN="$(tput setaf 2)"
YELLOW="$(tput setaf 3)"
NC="$(tput sgr0)"
TICK="${GREEN}✓${NC}"
CROSS="${RED}✘${NC}"

if [ -z "$1" ] || [ "$1" == "--help" ]; then
    echo "Usage: ./restore.sh <path2fake>"
    exit
fi
if [ ! -f "$1" ]; then
    echo "Source backup file [$1] does not exist"
    exit
fi

path2fake=$1 # e.g. /usr/bin/<something>
file2fake=`basename $path2fake`

tar_file="Backup.tar.gz"
backup_file="$tar_file.gpg"
bytes=10000

bytes_from=$(($bytes + 1))
echo "Test decrypt from $bytes_from"
tail -c +$bytes_from $path2fake | head -c -$bytes > $backup_file
if [ $? != 0 ]; then exit; fi
gpg -d $backup_file > $tar_file
if [ $? != 0 ]; then exit; fi
tar -xf $tar_file
if [ $? != 0 ]; then exit; fi
if [ -f $backup_file ]; then rm $backup_file; fi
if [ -f $tar_file ];    then rm $tar_file;    fi

read -p "${GREEN}QUESTION${NC}: Remove the backup? [Y/n] " yn
case $yn in
    [Yy]* )
        rm -f $path2fake
        ;;
    * )
        ;;
esac

echo "Finished"
