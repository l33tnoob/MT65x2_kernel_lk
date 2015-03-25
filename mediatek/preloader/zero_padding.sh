#!/bin/bash

FILE_PATH=$1
ALIGNMENT=$2
PADDING_SIZE=0

FILE_SIZE=$(stat -c%s "${FILE_PATH}")
REMAINDER=$((${FILE_SIZE} % ${ALIGNMENT}))
if [ ${REMAINDER} -ne 0 ]; then
    PADDING_SIZE=$((${ALIGNMENT} - ${REMAINDER}))
    dd if=/dev/zero of=padding.txt bs=$PADDING_SIZE count=1
    cat padding.txt>>${FILE_PATH}
    rm padding.txt
fi
