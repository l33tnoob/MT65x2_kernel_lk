#
# AudioPackage MTK
# 
# Include this file in a product makefile to include these audio files
#
# This is a larger package of sounds than the 1.0 release for devices
# that have larger internal flash.
# 

LOCAL_MTK_PATH:= mediatek/frameworks-ext/base/data/sounds

PRODUCT_COPY_FILES += \
	$(LOCAL_MTK_PATH)/effects/camera_click.ogg:system/media/audio/ui/camera_click.ogg \
	$(LOCAL_MTK_PATH)/effects/camera_shutter.ogg:system/media/audio/ui/camera_shutter.ogg \
