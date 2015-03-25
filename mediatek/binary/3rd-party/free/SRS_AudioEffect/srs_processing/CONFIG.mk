################################################################################
#### BUILD CONFIG - EDIT AS REQUIRED ###########################################

# trumedia
SRS_PRODUCT := trumedia

# customer tag (variant) - feel free to edit this variant (or comment it out) to enable identification of your tuning/cfg files easier.
SRS_VARIANT := "Generic"

# TAGS - unstructured data tags (readable by getParams - srs_processing_tags)
# these tags are used by various sources (including customer software)
# the default listed cause a 'gaming' icon to appear in our demo UI, as well as control
# slider behavior
SRS_TAGS += gaming
SRS_TAGS += mirror_sliders

# FEATURE - user-facing TruEQ instance (in addition to tuning PEQ)
#SRS_FEATURES += userpeq

# FEATURE - user-facing MobileEQ instance - Enable if a build with the GraphicEQ is desired.
#SRS_FEATURES += usergeq

# FEATURE - extended MobileEQ - DEPENDS ON usergeq - Enables EQ preset settings for each device/media-preset.
#SRS_FEATURES += extendgeq

# FEATURE - logging (text-based logging mostly via ALOG)
SRS_FEATURES += logging

# FEATURE - audio logging (saves audio pre/post to paths specified)
# Using SRS Toolkit, if enabled, a user can toggle audio logging (very useful when debugging)
#SRS_FEATURES += audiolog
SRS_AUDIOLOG_PREPATH := /data/system/srs_prelog.pcm
SRS_AUDIOLOG_POSTPATH := /data/system/srs_postlog.pcm

# SUB-FEATURE - if audio logging is enabled, insert a 'marker' into the logs to show when audio was 'slept' for more than 2 seconds
SRS_FEATURES += audiosesslog

# FEATURE - forced output silence (for debugging/etc)
#SRS_FEATURES += forcesilence

# FEATURE - performance metrics (process-loop MIPS calculation)
#SRS_FEATURES += perftrack

################################################################################
#### PATHS & PERMISSIONS - EDIT AS REQUIRED ####################################

# What path is the .cfg read from - 'base' meaning _full_ tunings and default settings?
SRS_BASECFG_READPATH := /system/data/srs_processing.cfg

# Force the 'base' file into the built Image (edit to match the same filename as SRS_BASECFG_READPATH, notice no leading slash after the colon...
#DISABLED BY DEFAULT - Android doesn't like this variable changed except during 100% clean builds.
#NOTE - These paths need to be created in the image with proper permissions
#       Base is read-only - Generally this path is fine 'as-is'
#       User should be read-write - Vendors should consider a path that works best for them.
#       During development, it is often ideal to create the user path manually before using the Toolkit.
#PRODUCT_COPY_FILES += $(LOCAL_PATH)/srs_base.cfg:system/data/srs_processing.cfg
#PRODUCT_COPY_FILES += $(LOCAL_PATH)/srs_user.cfg:data/system/srs_processing.cfg

# What path is the .cfg read/write from - 'user' meaning only user-preferences?
SRS_USERCFG_PATH := /data/misc/srs_processing.cfg

# Is the save/load of user-preferences allowed?
# ENABLED for Debugging and Tuning Tools
SRS_USERCFG_ALLOW := true

# Will the user-preferences file also store tuning/config values (instead of just user-preferences)?
# ENABLED for Debugging and Tuning Tools
SRS_USERCFG_UNLOCKED := true

# Will enable or disable get/set Params calls for certain parameter types 
# DISABLED for Debugging and Tuning Tools
# Uncomment to ENABLE prevention of a specific type (PREF is user-preferences, block with caution)
#SRS_PARAMWRITE_CFG := false
#SRS_PARAMWRITE_PREF := false
#SRS_PARAMREAD_INFO := false
#SRS_PARAMREAD_DEBUG := false
#SRS_PARAMREAD_CFG := false
#SRS_PARAMREAD_PREF := false
