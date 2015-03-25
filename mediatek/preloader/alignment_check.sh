#!/bin/bash

FILE_PATH=$1
ALIGNMENT=$2

FILE_SIZE=$(stat -c%s "${FILE_PATH}")
REMAINDER=$((${FILE_SIZE} % ${ALIGNMENT}))
if [ ${REMAINDER} -ne 0 ]; then
	echo
	echo ${FILE_PATH} is not ${ALIGNMENT} byte alignment
	exit 1;
fi