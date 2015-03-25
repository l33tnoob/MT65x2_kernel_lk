@echo off

if "%1" == "-h" goto show_help
if "%1" == "help" goto show_help
if "%1" == "clean" goto clean_all
if "%1" == "--set_env" goto set_env
if "%1" == "slim" (
	call build_slim_swip.bat
	goto end
)

:: ***********************************************
:: ****** 1. modify input config here ************
:: ***********************************************
set in1=arm9rvct3.1
set in2=arm11rvct3.1
set in3=arm7rvct3.1
set in4=pc
set in5=arm9rvct2.2

:: ***********************************************
:: ****** 2.modify config for make all ***********
:: ***********************************************
set all_config=%in1% %in2% %in3% %in4% %in5%

set target=""
:: ***********************************************
:: ****** 3.modify makefile config mapping *******
:: ***********************************************
if "%1" == "%in1%" set target=arm
if "%1" == "%in2%" set target=arm
if "%1" == "%in3%" set target=arm
if "%1" == "%in4%" set target=pc
if "%1" == "%in5%" set target=arm

if "%1" == "all"   set target=all
if %target%=="" goto show_help

:set_env
if "%env_already_set%" == "1" goto end_set_env
:: *************** set environment variables ***************
set env_already_set=1
:end_set_env

:: ****** set temp_file name for output makeall result ******
if "%1" == "all" set temp_file_name=_result.temp

:: ===============================================
:: ==== Do not need to modify after this line ====
:: ===============================================
if "%1" == "--set_env" goto end
if "%1" == "all" goto make_all

:make_one
    set makeswip_result=success (CONFIG=%1)
    make PLATFORM=%target% CONFIG=%1
    if %errorlevel% neq 0 set makeswip_result=fail    (CONFIG=%1)
    set error_msg=Make result: %makeswip_result%
    if "%2" == "__internal__" goto save_result_2file
    echo.
    echo %error_msg%
    goto end
:save_result_2file
    echo %error_msg% >> %temp_file_name%
    goto end

:make_all
    if exist %temp_file_name% del %temp_file_name%
    for %%a in (%all_config%) do call makeswip.bat %%a __internal__
    echo .
    echo ******* make swip results *******
    if exist %temp_file_name% more %temp_file_name%
    if exist %temp_file_name% del %temp_file_name%
    goto end

:clean_all
    make clean
    goto end

:show_help
    echo.
    echo Usage : makeswip.bat [CONFIG]
    echo [CONFIG]:
    for %%a in (%all_config%) do echo    %%a
    echo    all   ( make all of the above configs )
    echo    clean ( make clean )
    echo    slim  ( make slim SWIP )
    goto end

:end
