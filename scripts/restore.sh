#!/bin/bash
# Automated secure off-site restore
RED="$(tput setaf 1)"
GREEN="$(tput setaf 2)"
YELLOW="$(tput setaf 3)"
NC="$(tput sgr0)"
TICK="${GREEN}✓${NC}"
CROSS="${RED}✘${NC}"

# Inputs
if [ -z "$1" ] || [ "$1" == "--help" ]; then
    echo "Usage: ./restore.sh <path2fake>"
    exit
fi
if [ ! -f "$1" ]; then
    echo "Source backup file ${YELLOW}$1${NC} does not exist"
    exit
fi
path2fake=$1 # e.g. /usr/bin/<something>
file2fake=`basename $path2fake`
tar_file="Backup.tar.gz"
backup_file="$tar_file.gpg"
bytes=10000

# Infrastructure
if [ -z "$(which srm)" ]; then
    echo "${GREEN}INFO${NC}: Installing srm secure-delete system"
    sudo apt -y install secure-delete
fi

# Un-disguise
echo "Un-disguise from ${YELLOW}$bytes_from${NC}"
bytes_from=$(($bytes + 1))
tail -c +$bytes_from $path2fake | head -c -$bytes > $backup_file
if [ $? != 0 ]; then exit; fi

# Decrypt
echo "Decrypt"
gpg -d $backup_file > $tar_file
if [ $? != 0 ]; then exit; fi

# Un-tar-gz
echo "Un-tar.gz"
tar -xf $tar_file
if [ $? != 0 ]; then exit; fi

echo "Remove working files"
if [ -f $backup_file ]; then rm $backup_file; fi
if [ -f $tar_file ];    then rm $tar_file;    fi

read -p "${GREEN}QUESTION${NC}: Remove the backup? [Y/n] (n) " yn
case $yn in
    [Yy]* )
        # -l = 2 passes only, not 38
        srm -l $path2fake
        ;;
    * )
        ;;
esac

echo "Finished"
