#!/bin/bash
# Automated secure off-site backup
RED="$(tput setaf 1)"
GREEN="$(tput setaf 2)"
YELLOW="$(tput setaf 3)"
NC="$(tput sgr0)"
TICK="${GREEN}✓${NC}"
CROSS="${RED}✘${NC}"

# Inputs
if [ -z "$2" ]; then
    echo "${RED}USAGE${NC}: ./backup.sh <path2fake> <files...>"
    exit
fi
if [ ! -f "$1" ]; then
    echo "${RED}ERROR${NC}: Target backup file [$1] does not exist"
    exit
fi
if [ ! -f "$2" ] && [ ! -d "$2" ]; then
    echo "${RED}ERROR${NC}: Source file [$2] does not exist"
    exit
fi
path2fake=$1 # e.g. /usr/bin/<binary-file>
files=$2 $3 $4 $5 $6 $7 $8 $9 # e.g. ./my-code-directory/
file2fake=`basename $path2fake`
first_source=$2

# Infrastructure
if [ -z "$(which srm)" ]; then
    echo "${GREEN}INFO${NC}: Installing srm secure-delete system"
    sudo apt install secure-delete
fi

# Remove object files
objects=`find "$first_source" | grep -E "\\.o$"`
if [ -n "$objects" ]; then
    echo "Remove object files"
    find "$first_source" | grep -E "\\.o$" | xargs rm
fi

# TAR GZ
tar_file="Backup.tar.gz"
echo "Build ${YELLOW}$tar_file${NC} from ${YELLOW}$files${NC}"
if [ -f $tar_file ]; then rm $tar_file; fi
tar -czf $tar_file $files
if [ $? != 0 ]; then exit; fi

# GPG
backup_file="$tar_file.gpg"
echo "Encrypt to ${YELLOW}$backup_file${NC}"
if [ -f $backup_file ]; then rm $backup_file; fi
gpg -c $tar_file
if [ $? != 0 ]; then exit; fi
if [ -f $tar_file ]; then rm $tar_file; fi

# Obscure date
system_date=`date -Is`
target_date=`stat -c "%y" $path2fake | cut -d " " -f 1`
echo "Obscure system date to ${YELLOW}$target_date${NC}"
if [ -z "$(which timedatectl)" ]; then echo "timedatectl required"; exit; fi
sudo timedatectl set-ntp no
if [ $? != 0 ]; then exit; fi
sudo timedatectl set-time "$target_date"
if [ $? != 0 ]; then exit; fi

# It is possible to detect encrypted files
# file <file>
# <file>: GPG symmetrically encrypted data (AES256 cipher)
# So we place a lib.so header at the front
bytes=10000
echo "Dsiguise as ${YELLOW}$file2fake${NC} with ${YELLOW}$bytes${NC} prefix bytes"
if [ -f $file2fake ]; then rm $file2fake; fi
head -c $bytes $path2fake > $file2fake # Final file creation
if [ $? != 0 ]; then exit; fi
cat $backup_file >> $file2fake
tail -c $bytes $path2fake >> $file2fake
if [ $? != 0 ]; then exit; fi
if [ -f $backup_file ]; then rm $backup_file; fi

# Un-Obscure date
# sudo timedatectl set-time "$system_date"
sudo timedatectl set-ntp yes
actual_date=`date`
echo "Returned date to ${YELLOW}$actual_date${NC}"

# Disguise owner
owner=`stat -c "%U:%G" $path2fake`
echo "Setting owner to ${YELLOW}$owner${NC}"
sudo chmod ogu+x,g-w $file2fake
sudo chown $owner $file2fake

# Restore test
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

# Updates change last access time
sudo touch -d "$target_date" $file2fake

# No action has been taken, only a file created to be moved
# maybe to a different server
read -p "${GREEN}QUESTION${NC}: Overwrite the target backup file on this machine ${YELLOW}(requires sudo)${NC}? [Y/n] (n) " yn
case $yn in
    [Yy]* )
        sudo mv $file2fake $path2fake
        # Updates change last access time
        sudo touch -d "$target_date" $path2fake
        stat $path2fake
        ;;
    * )
        ;;
esac

# Remove source
read -p "${GREEN}QUESTION${NC}: Remove the source files ${YELLOW}$files${NC} with srm? [Y/n] (n) " yn
case $yn in
    [Yy]* )
        echo "${GREEN}INFO${NC}: srm -l shredding ${YELLOW}$files${NC}"
        # -l 2 passes instead of 38
        srm -lr $files
        ;;
    * )
        ;;
esac

echo "Finished"
