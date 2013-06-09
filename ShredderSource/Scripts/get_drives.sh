#!/bin/bash

if [ $# -ne 1 ] 
then
	echo "Usage: ./get_drives output_file"
else
	# Clearing the log file.
	rm -f $1

	for curr_drive in `cat /proc/partitions  | awk '{print $4}' | grep -w ...` 
	do
		# Getting the current drive label & writing it to the output file.
		curr_drive=`echo $curr_drive | head -c 8`
		curr_long_drive=`echo /dev/$curr_drive`
                echo "$curr_long_drive" >> $1

		# Writing the current drive's partitions to the output file.
                cat /proc/partitions | grep $curr_drive. | awk '{print $4,$3}' >> $1

		# Getting the current drive's capacity & writing it to the output file.
		curr_capacity=`fdisk -l $curr_long_drive | head -2 | tail -1 | awk '{print $3, $4}'`
		tmp_capacity_length=`echo $curr_capacity | wc -c`
		tmp_capacity_length=`expr $tmp_capacity_length - 2`
		curr_capacity=`echo $curr_capacity | head -c $tmp_capacity_length`
		echo "$curr_capacity" >> $1

		# Writing the current drive's heads to the output file.
		fdisk -l $curr_long_drive | head -3 | tail -1 | awk '{print $1}' >> $1

		# Writing the current drive's sectors/track to the output file.
		fdisk -l $curr_long_drive | head -3 | tail -1 | awk '{print $3}' >> $1

		# Writing the current drive's cylinders to the output file.
		fdisk -l $curr_long_drive | head -3 | tail -1 | awk '{print $5}' >> $1

		# Writing the current drive's size in bytes to the output file.
#                fdisk -l $curr_drive | head -2 | tail -1 | awk '{print $5}' >>
	done
fi
