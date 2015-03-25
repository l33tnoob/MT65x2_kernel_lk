#!/system/bin/sh

#change partition permission
/system/bin/chown root:system /mtd@preloader
/system/bin/chmod 0640 /mtd@preloader
/system/bin/chown root:system /mtd@pro_info
/system/bin/chmod 0660 /mtd@pro_info
/system/bin/chown root:system /mtd@bootimg
/system/bin/chmod 0640 /mtd@bootimg
/system/bin/chown root:system /mtd@recovery
/system/bin/chmod 0640 /mtd@recovery
/system/bin/chown root:system /mtd@sec_ro
/system/bin/chmod 0640 /mtd@sec_ro

/system/bin/chown root:system /mtd@nvram
/system/bin/chmod 0660 /mtd@nvram
/system/bin/chown root:system /mtd@seccfg
/system/bin/chmod 0660 /mtd@seccfg
/system/bin/chown root:system /mtd@misc
/system/bin/chmod 0660 /mtd@misc


