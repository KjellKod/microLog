#!/bin/sh
# 
# File:   viewlog.sh
# Author: Pietro Mele <pietrom16@gmail.com>
#
# Created on 29-May-2012, 09:36:02
#

if [ -z "$1" ]
then
	LOGFILE="addrun.log"
else
	LOGFILE=$1
fi

echo "**********************************************************"
echo Log file: $LOGFILE

tail --lines=50 -F $LOGFILE
