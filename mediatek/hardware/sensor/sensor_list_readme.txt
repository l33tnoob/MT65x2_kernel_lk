This document descript sensor attribute. It define the detail informations of sensor, as 
sensor_t struct define. You can use this information to compele your sensor hal customizaton file.
alps/mtk/src/custom/&(project)/hal/sensors/sensor/hwmsen_custom.c file. You can add your project
support sensor information to this file.
struct sensor_t {
    /* name of this sensors */
    const char*     name;
    /* vendor of the hardware part */
    const char*     vendor;
    /* version of the hardware part + driver. The value of this field is
     * left to the implementation and doesn't have to be monotonicaly
     * increasing.
     */    
    int             version;
    /* handle that identifies this sensors. This handle is used to activate
     * and deactivate this sensor. The value of the handle must be 8 bits
     * in this version of the API. 
     */
    int             handle;
    /* this sensor's type. */
    int             type;
    /* maximaum range of this sensor's value in SI units */
    float           maxRange;
    /* smallest difference between two values reported by this sensor */
    float           resolution;
    /* rough estimate of this sensor's power consumption in mA */
    float           power;
    /* reserved fields, must be zero */
    void*           reserved[9];
};


Now we support sensor type as follow (M-sensor will caluate the orientation sensor with G-sensor data):
G-sensor: ADXL345,
M-sensor: AMI304, Yamaha529
ALS: CM3623
PS: CM3623

struct sensor_t AMI304_M
	{ 
		.name       = "AMI304 3-axis Magnetic Field sensor",
		.vendor     = "Aichi Steel",
		.version    = 1,
		.handle     = ID_MAGNETIC,
		.type       = SENSOR_TYPE_MAGNETIC_FIELD,
		.maxRange   = 600.0f,
		.resolution = 0.16667f,
		.power      = 0.25f,
		.reserved   = {}
	};
	
struct sensor_t AMI304_O
{ 
		.name       = "AMI304 Orientation sensor",
		.vendor     = "Aichi Steel",
		.version    = 1,
		.handle     = ID_ORIENTATION,
		.type       = SENSOR_TYPE_ORIENTATION,
		.maxRange   = 360.0f,
		.resolution = 0.9f,
		.power      = 0,
		.reserved   = {}
	};
	
struct sensor_t YAMAHA529_M
	{ 
		.name       = "YAMAHA529 3-axis Magnetic Field sensor",
		.vendor     = "Yamaha",
		.version    = 1,
		.handle     = ID_MAGNETIC,
		.type       = SENSOR_TYPE_MAGNETIC_FIELD,
		.maxRange   = 300.0f,
		.resolution = 0.6f,
		.power      = 4f,
		.reserved   = {}
	};
	
struct sensor_t YAMAHA529_O
{ 
		.name       = "YAMAHA529 Orientation sensor",
		.vendor     = "Yamaha",
		.version    = 1,
		.handle     = ID_ORIENTATION,
		.type       = SENSOR_TYPE_ORIENTATION,
		.maxRange   = 360.0f,
		.resolution = 1.0f,
		.power      = 0,
		.reserved   = {}
	};
	
struct sensor_t AKM8975_M
	{ 
		.name       = "AKM8975 3-axis Magnetic Field sensor",
		.vendor     = "AKM",
		.version    = 1,
		.handle     = ID_MAGNETIC,
		.type       = SENSOR_TYPE_MAGNETIC_FIELD,
		.maxRange   = 1200.0f,
		.resolution = 0.3f,
		.power      = 2f,
		.reserved   = {}
	};
	
struct sensor_t AKM8975_O
{ 
		.name       = "AKM8975 Orientation sensor",
		.vendor     = "AKM",
		.version    = 1,
		.handle     = ID_ORIENTATION,
		.type       = SENSOR_TYPE_ORIENTATION,
		.maxRange   = 360.0f,
		.resolution = 1.0f,
		.power      = 0,
		.reserved   = {}
	};
	
struct sensor_t MMC314X_M
	{ 
		.name       = "MMC314X 3-axis Magnetic Field sensor",
		.vendor     = "Memsic",
		.version    = 1,
		.handle     = ID_MAGNETIC,
		.type       = SENSOR_TYPE_MAGNETIC_FIELD,
		.maxRange   = 400.0f,
		.resolution = 0.195f,
		.power      = 0.55f,
		.reserved   = {}
	};
	
struct sensor_t MMC314X_O
{ 
		.name       = "MMC314X Orientation sensor",
		.vendor     = "Memsic",
		.version    = 1,
		.handle     = ID_ORIENTATION,
		.type       = SENSOR_TYPE_ORIENTATION,
		.maxRange   = 360.0f,
		.resolution = 1.0f,
		.power      = 0,
		.reserved   = {}
	};			



struct sensor_t ADXL345_G
	{  
		.name       = "ADXL345 3-axis Accelerometer",
		.vendor     = "ADI",
		.version    = 1,
		.handle     = ID_ACCELEROMETER,
		.type       = SENSOR_TYPE_ACCELEROMETER,
		.maxRange   = 32.0f,
		.resolution = 4.0f/1024.0f,
		.power      =130.0f/1000.0f,
		.reserved   = {}
	};

struct sensor_t BMA150_G
	{  
		.name       = "BMA150 3-axis Accelerometer",
		.vendor     = "BOSCH",
		.version    = 1,
		.handle     = ID_ACCELEROMETER,
		.type       = SENSOR_TYPE_ACCELEROMETER,
		.maxRange   = 39.22f,
		.resolution = 4.0f/1024.0f,
		.power      =200.0f/1000.0f,
		.reserved   = {}
	};

struct sensor_t KXTF9_G
	{  
		.name       = "KXTF9 3-axis Accelerometer",
		.vendor     = "KIONIX",
		.version    = 1,
		.handle     = ID_ACCELEROMETER,
		.type       = SENSOR_TYPE_ACCELEROMETER,
		.maxRange   = 39.22f,
		.resolution = 4.0f/4096.0f,
		.power      =750.0f/1000.0f,
		.reserved   = {}
	};

struct sensor_t CM3623_PS
	{ 
		.name       = "CM3623 Proximity Sensor",
		.vendor     = "Capella",
		.version    = 1,
		.handle     = ID_PROXIMITY,
		.type       = SENSOR_TYPE_PROXIMITY,
		.maxRange   = 1.00f,
		.resolution = 1.0f,
		.power      = 0.13f,
		.reserved   = {}
	};         

struct sensor_t CM3623_ALS
	{ 
		.name       = "CM3623 Light Sensor",
		.vendor     = "Capella",
		.version    = 1,
		.handle     = ID_LIGHT,
		.type       = SENSOR_TYPE_LIGHT,
		.maxRange   = 10240.0f,
		.resolution = 1.0f,
		.power      = 0.13f,
		.reserved   = {}
	},