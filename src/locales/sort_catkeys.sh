#!/bin/sh
tempfile=$1_orig
mv $1 $tempfile
head -1 $tempfile > $1
tail -n +2 $tempfile | sort -k 2 -t$'\t' >> $1
settype -t locale/x-vnd.Be.locale-catalog.plaintext $1
