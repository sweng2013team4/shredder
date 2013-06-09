#!/bin/bash

if [ $# -ne 1 ]
then
    echo "Error - wrong script usage."
    echo "Usage: get_boot_drive <result_file>"
else
    # Delete the file
    rm -f $1

    df -k /boot | grep -v Filesystem | awk '{print $1}' > $1
fi
