#
# GeoCoding Query database file
# 
# Include this file in a product makefile to include these audio files
#
# 

LOCAL_PATH:= mediatek/external/GeoCoding

PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/geocoding.db:system/etc/geocoding.db
        
