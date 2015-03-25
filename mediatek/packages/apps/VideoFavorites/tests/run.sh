#!/bin/sh

local TOPFILE=build/core/envsetup.mk
# We redirect cd to /dev/null in case it's aliased to
# a command that prints something as a side-effect
# (like pushd)
local HERE=`/bin/pwd`
T=
while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
	cd .. > /dev/null
	T=`PWD= /bin/pwd`
done
if [ -f "$T/$TOPFILE" ]; then
	SRC_ROOT=$T
else
	echo "Error: source tree was not found."
	exit
fi

PRODUCT=`cat $SRC_ROOT/.product`
PRODUCT_OUT=$SRC_ROOT/out/target/product/$PRODUCT
INTERMEDIATES=$SRC_ROOT/out/target/common/obj/APPS/VideoFavorites_intermediates
TEST_RUNNER=com.zutubi.android.junitreport.JUnitReportTestRunner

if adb remount 
then
  # Prevent insufficient storage space
  adb shell rm -r /data/core

  # Copy binary to device
  adb uninstall com.mediatek.videofavorites
  adb install -r $PRODUCT_OUT/system/app/VideoFavorites.apk
  adb uninstall com.mediatek.videofavorites.tests
  adb install -r $PRODUCT_OUT/data/app/VideoFavoritesTests.apk

  # fire the activity to dismiss keyguard
  adb shell am start -a android.ntent.action.MAIN -n com.mediatek.videofavorites.tests/.DismissKeyguardActivity

  # Run instrumentation test
  PACKAGE=com.mediatek.videofavorites
  adb shell am instrument -e coverage true -w $PACKAGE.tests/$TEST_RUNNER
  adb pull /data/data/$PACKAGE/files/junit-report.xml $SRC_ROOT/junit-report-videofavorites.xml

  adb shell am force-stop com.mediatek.videofavorites.tests

  # Pull performance test data
  adb pull /data/data/$PACKAGE/app_perf $SRC_ROOT/perf-$PACKAGE

  # Generate emma code coverage report
  cd $INTERMEDIATES
  adb pull /data/data/$PACKAGE/files/coverage.ec
  java -cp ~/local/emma/lib/emma.jar emma report -r xml -in coverage.ec -in coverage.em
fi
