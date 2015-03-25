PL_MAX_RAM_SIZE=43008
PL_MAX_ROM_SIZE=128000

ROM_RPT_SIZE=500
RAM_RPT_SIZE=500
    
size `ls ../../${MTK_ROOT_OUT}/PRELOADER_OBJ/obj/*.o` > ../../${MTK_ROOT_OUT}/PRELOADER_OBJ/report-objsize.txt

# check image rom size
PL_ROM_SIZE=$(stat -c%s "${PL_IMAGE}")    

if [ $PL_ROM_SIZE -gt $PL_MAX_ROM_SIZE ]; then
    echo "===================== Building Fail ==========================="
    echo " IMG ROM Size: ${PL_ROM_SIZE} bytes > ${PL_MAX_ROM_SIZE} bytes!! "
    echo " WARNING: Reduce your code size first then compile again!! "
    echo "==============================================================="
    echo "                    ROM SIZE REPORT (>${ROM_RPT_SIZE} bytes)   "
    echo "---------------------------------------------------------------"
    cat report-codesize.txt | awk -F' ' '{if ($3=="FUNC" && $2>sz) print $0}' sz=${ROM_RPT_SIZE}
    echo "---------------------------------------------------------------"
    rm ${PL_IMAGE}
    echo "BUILD FAIL."
    exit 1;
fi

# check image ram size
PL_RAM_SIZE=$(awk '{if($3=="_bss_start") {BSS_START= strtonum("0x" $1)} ;
             if($3=="_bss_end") {BSS_END=strtonum("0x" $1)}}
             END{printf("%d\n",BSS_END-BSS_START)}' ${PL_FUN_MAP})
if [ $PL_RAM_SIZE -gt $PL_MAX_RAM_SIZE ]; then
    echo "===================== Building Fail ==========================="
    echo " IMG RAM Size: ${PL_RAM_SIZE} bytes > ${PL_MAX_RAM_SIZE} bytes!!"
    echo " WARNING: Reduce your code size first then compile again!! "
    echo "==============================================================="
    echo "                    RAM SIZE REPORT (>${RAM_RPT_SIZE} bytes)   "
    echo "---------------------------------------------------------------"
    cat report-codesize.txt | awk -F' ' '{if ($3=="OBJECT" && $2>sz) print $0}' sz=${RAM_RPT_SIZE}
    echo "---------------------------------------------------------------"
    rm -f ${PL_IMAGE}
    echo "BUILD FAIL."
    exit 1;
fi

# check image dram size
PL_DRAM_SIZE=$(awk '{if($3=="_dram_start") {DRAM_START= strtonum("0x" $1)} ;
             if($3=="_dram_end") {DRAM_END=strtonum("0x" $1)}}
             END{printf("%d\n",DRAM_END-DRAM_START)}' ${PL_FUN_MAP})

PL_SIZE=`expr $PL_RAM_SIZE + $PL_ROM_SIZE`
echo "===================== Building Success ==========================="
echo " PL ROM size:  ${PL_ROM_SIZE} bytes"
echo " PL RAM size:  ${PL_RAM_SIZE} bytes"
echo " PL DRAM size: ${PL_DRAM_SIZE} bytes"
echo " PL ROM+RAM size: ${PL_SIZE} bytes"
echo "=================================================================="


