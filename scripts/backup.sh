#!/bin/bash
# Automated secure off-site backup
# For GS source code
RED="$(tput setaf 1)"
GREEN="$(tput setaf 2)"
YELLOW="$(tput setaf 3)"
NC="$(tput sgr0)"
TICK="${GREEN}✓${NC}"
CROSS="${RED}✘${NC}"

if [ -z "$2" ]; then
    echo "${RED}USAGE${NC}: ./backup.sh <path2fake> <files...>"
    exit
fi
if [ ! -f "$1" ]; then
    echo "${RED}ERROR${NC}: Target backup file [$1] does not exist"
    exit
fi
if [ ! -f "$2" ]; then
    echo "${RED}ERROR${NC}: Source file [$2] does not exist"
    exit
fi

path2fake=$1 # e.g. /usr/bin/<something>
files=$2 $3 $4 $5 $6 $7 $8 $9 # e.g. ./general_server

file2fake=`basename $path2fake`

tar_file="Backup.tar.gz"
echo "Build ${YELLOW}$tar_file${NC} from ${YELLOW}$files${NC}"
if [ -f $tar_file ]; then rm $tar_file; fi
tar -czf $tar_file $files
if [ $? != 0 ]; then exit; fi

backup_file="$tar_file.gpg"
echo "Encrypt to ${YELLOW}$backup_file${NC}"
if [ -f $backup_file ]; then rm $backup_file; fi
gpg -c $tar_file
if [ $? != 0 ]; then exit; fi
if [ -f $tar_file ]; then rm $tar_file; fi

# It is possible to detect encrypted files
# file <file>
# <file>: GPG symmetrically encrypted data (AES256 cipher)
# So we place a lib.so header at the front
bytes=10000
echo "Dsiguise as ${YELLOW}$file2fake${NC} with ${YELLOW}$bytes${NC} prefix bytes"
if [ -f $file2fake ]; then rm $file2fake; fi
head -c $bytes $path2fake > $file2fake
if [ $? != 0 ]; then exit; fi
cat $backup_file >> $file2fake
tail -c $bytes $path2fake >> $file2fake
if [ $? != 0 ]; then exit; fi
if [ -f $backup_file ]; then rm $backup_file; fi

# Obscure date
date=`stat -c "%y" $path2fake | cut -d " " -f 1`
echo "Obscure date to ${YELLOW}$date${NC}"
touch -d "$date" $file2fake
if [ $? != 0 ]; then exit; fi

# Disguise owner
owner=`stat -c "%U:%G" $path2fake`
echo "Setting owner to ${YELLOW}$owner${NC}"
sudo chmod ogu+x,g-w $file2fake
sudo chown $owner $file2fake

bytes_from=$(($bytes + 1))
echo "Test decrypt from ${YELLOW}$bytes_from${NC}"
tail -c +$bytes_from $file2fake | head -c -$bytes > $backup_file
gpg -d $backup_file > $tar_file
if [ $? != 0 ]; then exit; fi
echo "${GREEN}INFO${NC}: TAR contents for verification:"
tar -ztf $tar_file
if [ $? != 0 ]; then exit; fi
if [ -f $backup_file ]; then rm $backup_file; fi
if [ -f $tar_file ];    then rm $tar_file;    fi

# No action has been taken, only a file created to be moved
# maybe to a different server
read -p "${GREEN}QUESTION${NC}: Overwrite the target backup file on this machine ${YELLOW}(requires sudo)${NC}? [Y/n] " yn
case $yn in
    [Yy]* )
        sudo mv $file2fake $path2fake
        sudo touch -d \"$date\" $path2fake
        ls -la $path2fake
        ;;
    * )
        ;;
esac

read -p "${GREEN}QUESTION${NC}: Remove the source files ${YELLOW}$files${NC}? [Y/n] " yn
case $yn in
    [Yy]* )
        rm -r $files
        ;;
    * )
        ;;
esac

echo "Finished"
