#!/bin/bash

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 FILENAME"
    exit 1
fi
file=$1

if [[ ! -f "$file" ]]; then
    echo "File not found: $file"
    exit 1
fi

FILESIZE=$(stat -c%s "$file")

cname=$file
cname=${cname//-/_}
cname=${cname//./_}

echo "/*static*/ unsigned int _data_len = $FILESIZE;"
#echo "/*static*/ unsigned char $cname[] = {"
echo "/*static*/ unsigned char _data[] = {"
hexdump -v -e '" " 16/1 "  0x%02x, " "\n"' $file | \
   sed -e '$s/0x  ,//g'
echo "};"
