################################################################################
#### PROTECTION LOGIC - DO NOT EDIT ############################################

SRS_PROTECT_TYPE := license

# LICENSE - license location & auth id - without this file, SRS processing will be disabled
# with this file, SRS processing will be in 'demo' mode (tone every ~30 seconds) until a 
# new license file (and auth id) is provided by SRS
SRS_PROTECT_PATH := /system/data/srsmodels.lic
SRS_PROTECT_AUTHID := 0xA845D752
