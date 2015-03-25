/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <termios.h>
#include<pthread.h>
#include <getopt.h>
#define LOG_TAG "U3DG"
#include <cutils/log.h>
#include <cutils/properties.h>
#include <usbhost/usbhost.h>
#define USB_HS_DIR "/dev/bus/usb/001"
#define COMPORT_SRCOFF_DESOFF   0x00
#define COMPORT_SRCON_DESOFF   0x01
#define COMPORT_SRCOFF_DESON   0x02
#define COMPORT_SRCON_DESON    0x03
#define COMPORT_CHNAGE         0x04
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <pwd.h>
#include <grp.h>

#define BUF_LEN 1024
#define Printf(x...) {if(u3dg_debug&1) printf(x); if(u3dg_debug&2) SLOGD(x);}
#define U3DG_SRC 0
#define U3DG_DES 1
int u3dg_debug = 0;
int u3dg_thread = 1;
int    loop=1;    /* loop while TRUE */ 
int u3dg_raw_flag=0;
pthread_mutex_t lock;

static int u3dg_comport_state = COMPORT_SRCOFF_DESOFF; 
struct U3DG_SETTING {
    char *fileName;
    fd_set nfds;
    int infd;
    int iwd;
    int istatus;
} u3dg_setting[2] =
{
	{.fileName="/dev/ttyGS0", .istatus=0},
	{.fileName="/dev/ttyACM0", .istatus=0},
};
#define MAX(a, b) ((a) > (b) ? (a) : (b))


static int verbose = 0;
static char str_buff[4096];
static char *modem_name=NULL;
static unsigned int modem_idx=0;
static struct usb_endpoint_descriptor modem_ep_in;
static struct usb_endpoint_descriptor modem_ep_out;
static struct usb_interface_descriptor modem_interface;

static struct USB_MODEM_SUPPORT_LIST
{
	uint16_t vid; 
	uint16_t pid;
	__u8  bInf;
	__u8  bOut;
	int idelay;
	unsigned char switchcmd[31];
} modem_support_list[] =
{
	{.vid=0x0E8D, .pid=0x0002, .bInf=0, .bOut=1, .idelay=2,
	 .switchcmd={0x55, 0x53, 0x42, 0x43, 0x12, 0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xf0, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{.vid=0x12D1, .pid=0x1446, .bInf=0, .bOut=1, .idelay=0, 
	 .switchcmd={0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
};
	

#define EXT_MD_IOC_MAGIC		'E'
#define EXT_MD_IOCTL_LET_MD_GO		_IO(EXT_MD_IOC_MAGIC, 1)
#define EXT_MD_IOCTL_REQUEST_RESET	_IO(EXT_MD_IOC_MAGIC, 2)
#define EXT_MD_IOCTL_POWER_ON_HOLD	_IO(EXT_MD_IOC_MAGIC, 3)
#define EXT_MD_IOCTL_POWER_ON   	_IO(EXT_MD_IOC_MAGIC, 100)
#define EXT_MD_IOCTL_POWER_OFF   	_IO(EXT_MD_IOC_MAGIC, 102)
#define EXT_MD_IOCTL_RESET      	_IO(EXT_MD_IOC_MAGIC, 103)
#define EXT_MD_IOCTL_R8_TO_PC   	_IO(EXT_MD_IOC_MAGIC, 104)
#define EXT_MD_IOCTL_R8_TO_AP   	_IO(EXT_MD_IOC_MAGIC, 105)
#define EXT_MD_IOCTL_R8_DOWNLOAD   	_IO(EXT_MD_IOC_MAGIC, 106)
#define EXT_MD_IOCTL_R8_ASSERTLOG    _IO(EXT_MD_IOC_MAGIC, 107)
#define EXT_MD_IOCTL_R8_ASSERTLOG_STATUS    _IO(EXT_MD_IOC_MAGIC, 108)
#define EXT_MD_IOCTL_R8_WAKEUPEN    _IO(EXT_MD_IOC_MAGIC, 109)
#define EXT_MD_MONITOR_DEV "/dev/ext_md_ctl0"
#define CAT_BUFSIZ (4096)

static int bflag, eflag, fflag, lflag, nflag, sflag, tflag, vflag;
static int rval;
static const char *filename;



static void
raw_cat(int rfd)
{
	static char *buf;
	static char fb_buf[CAT_BUFSIZ];
	static size_t bsize;

	struct stat sbuf;
	ssize_t nr, nw, off;
	int wfd;
#if 0
	wfd = fileno(stdout);
	if (buf == NULL) {
		if (fstat(wfd, &sbuf) == 0) {
			bsize = sbuf.st_blksize > CAT_BUFSIZ ?
			    sbuf.st_blksize : CAT_BUFSIZ;
			buf = malloc(bsize);
		}
		if (buf == NULL) {
			buf = fb_buf;
			bsize = CAT_BUFSIZ;
		}
	}
#endif	
	memset(fb_buf, 0x00, sizeof(fb_buf));
    nr = read(rfd, fb_buf, CAT_BUFSIZ);
    printf("%d--%s--\n",nr,fb_buf);
#if 0    
		for (off = 0; nr; nr -= nw, off += nw)
			if ((nw = write(wfd, buf + off, (size_t)nr)) < 0)
			{
				perror("write");
				exit(EXIT_FAILURE);
			}
#endif			
//	if (nr < 0) {
//		fprintf(stderr,"%s: invalid length\n", filename);
//		rval = 1;
//	}
}

static void
raw_args(char **argv)
{
    char buf[CAT_BUFSIZ];
	int fd, ilen;
    ssize_t nw;
//	fd = fileno(stdin);
	filename = "stdin";
//	do {
		if (*argv) {
#if 0			
			if (!strcmp(*argv, "-"))
				fd = fileno(stdin);
			else if (fflag) {
				struct stat st;
				fd = open(*argv, O_RDONLY|O_NONBLOCK, 0);
				if (fd < 0)
					goto skip;

				if (fstat(fd, &st) == -1) {
					close(fd);
					goto skip;
				}
				if (!S_ISREG(st.st_mode)) {
					close(fd);
					errno = EINVAL;
					goto skipnomsg;
				}
			}
			else 
#endif				
			if ((fd = open(*argv,  O_RDWR | O_NONBLOCK, 0)) < 0) {
skip:
				perror(*argv);
skipnomsg:
				rval = 1;
				++argv;
				goto Exit;
			}


  struct termios options;   
  tcgetattr( fd, &options );     
  cfmakeraw(&options);
  /* SEt Baud Rate */  
  cfsetispeed( &options, B115200 );   
  cfsetospeed( &options, B115200 ); 

//  options.c_cflag |= (CRTSCTS | CS8 | CLOCAL | CREAD);

  
  if ( tcsetattr( fd, TCSANOW, &options ) == -1 )     
      printf ("Error with tcsetattr = %s\n", strerror ( errno ) );   
  else    
      printf ( "%s\n", "tcsetattr succeed" );     


			
			filename = *argv++;
		}
		if(argv)
		{
		    
		    ilen = strlen(*argv);
			strcpy(buf, *argv);
			buf[ilen++] = 0x0D;
//			buf[ilen++] = 0x0A;
			buf[ilen] = 0x00;
			printf("%d:%02X %02X %02x %02X %02X %02X %02X %02X\n", ilen,
				buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
			if ((nw = write(fd, buf, ilen)) < 0)
			{
				perror("write");
				exit(0);//exit(EXIT_FAILURE);
			}
		}
		sleep(2);
		printf("::\n");
		raw_cat(fd);
		printf("::\n");
Exit:
//		if (fd != fileno(stdin))
			close(fd);
//	} while (*argv);
}

int
switchusb_main(int argc, char *argv[])
{
	int ch, fd, ret=0, val=0, i=0;
	struct flock stdout_lock;

	while ((ch = getopt(argc, argv, "anfgprsdw")) != -1)
		switch (ch) {
		case 'n':
			printf("power on r8\n");
			fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
			if (fd < 0) {
				printf("fail to open %s: ", EXT_MD_MONITOR_DEV);
				perror("");
				close(fd);
				return -1;
			}
			ret = ioctl(fd, EXT_MD_IOCTL_POWER_ON, NULL);
			if (ret < 0) {
				printf("power on modem failed!\n");
				close(fd);
				return ret;
			}	
			close(fd);
			return 0;
		case 'f':
			printf("power off r8\n");
			fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
			if (fd < 0) {
				printf("fail to open %s: ", EXT_MD_MONITOR_DEV);
				perror("");
				close(fd);
				return -1;
			}
			ret = ioctl(fd, EXT_MD_IOCTL_POWER_OFF, NULL);
			if (ret < 0) {
				printf("power off modem failed!\n");
				close(fd);
				return ret;
			}	
			close(fd);
			return 0;
		case 'r':
			printf("reset r8\n");
			fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
			if (fd < 0) {
				printf("fail to open %s: ", EXT_MD_MONITOR_DEV);
				perror("");
				close(fd);
				return -1;
			}
			ret = ioctl(fd, EXT_MD_IOCTL_RESET, NULL);
			if (ret < 0) {
				printf("reset r8 failed!\n");
				close(fd);
				return ret;
			}	
			close(fd);
			return 0;
		case 'p':
			printf("switch r8 to pc\n");
			fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
			if (fd < 0) {
				printf("fail to open %s: ", EXT_MD_MONITOR_DEV);
				perror("");
				close(fd);
				return -1;
			}
			ret = ioctl(fd, EXT_MD_IOCTL_R8_TO_PC, NULL);
			if (ret < 0) {
				printf("switch r8 to pc failed!\n");
				close(fd);
				return ret;
			}	
			close(fd);
			return 0;
		case 'a':
			printf("switch r8 to ap\n");
			fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
			if (fd < 0) {
				printf("fail to open %s: ", EXT_MD_MONITOR_DEV);
				perror("");
				close(fd);
				return -1;
			}
			ret = ioctl(fd, EXT_MD_IOCTL_R8_TO_AP, NULL);
			if (ret < 0) {
				printf("switch r8 to ap failed!\n");
				close(fd);
				return ret;
			}	
			close(fd);
			return 0;
		case 'd':
			printf("download r8\n");
			fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
			if (fd < 0) {
				printf("fail to open %s: ", EXT_MD_MONITOR_DEV);
				perror("");
				close(fd);
				return -1;
			}
			ret = ioctl(fd, EXT_MD_IOCTL_R8_DOWNLOAD, NULL);
			if (ret < 0) {
				printf("download r8 failed!\n");
				close(fd);
				return ret;
			}	
			close(fd);
			return 0;
		case 's':
			printf("sleep wait for md assert log\n");
			fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
			if (fd < 0) {
				printf("fail to open %s: ", EXT_MD_MONITOR_DEV);
				perror("");
				close(fd);
				return -1;
			}
			ret = ioctl(fd, EXT_MD_IOCTL_R8_ASSERTLOG, NULL);
			if (ret < 0) {
				printf("sleep failed!\n");
				close(fd);
				return ret;
			}	
			close(fd);
			return 0;
		case 'g':
			printf("get md assert log status\n");
			fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
			if (fd < 0) {
				printf("fail to open %s: ", EXT_MD_MONITOR_DEV);
				perror("");
				close(fd);
				return -1;
			}
			for(i = 0; i < 10; i++)
			{
				ret = ioctl(fd, EXT_MD_IOCTL_R8_ASSERTLOG_STATUS, &val);
				printf("assert log %d\n", val);
				sleep(1);
			}
			if (ret < 0) {
				printf("sleep failed!\n");
				close(fd);
				return ret;
			}	
			close(fd);
			return 0;			
		case 'w':
			printf("disab modem wake up\n");
			fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
			if (fd < 0) {
				printf("fail to open %s: ", EXT_MD_MONITOR_DEV);
				perror("");
				close(fd);
				return -1;
			}
			for(i = 0; i < 10; i++)
			{
				ret = ioctl(fd, EXT_MD_IOCTL_R8_WAKEUPEN, &val);
				printf("assert log %d\n", val);
				sleep(1);
			}
			if (ret < 0) {
				printf("sleep failed!\n");
				close(fd);
				return ret;
			}	
			close(fd);
			return 0;			
		default:
			break;
		}
	argv += optind;
    if(argc > 3)
	raw_args(argv);
	return 0;
}

	
static const char *get_str(struct usb_device *dev, int id)
{
    char *str = usb_device_get_string(dev, id);

    if (id && str) {
        strlcpy(str_buff, str, sizeof(str_buff));
        free(str);
    } else {
        snprintf(str_buff, sizeof(str_buff), "%02x", id);
    }

    return str_buff;
}


static void lsusb_parse_device_descriptor(struct usb_device *dev,
                                          struct usb_device_descriptor *desc)
{
    printf("  Device Descriptor\n");
    printf("\tbcdUSB: %04x\n", letoh16(desc->bcdUSB));
    printf("\tbDeviceClass: %02x\n", desc->bDeviceClass);
    printf("\tbDeviceSubClass: %02x\n", desc->bDeviceSubClass);
    printf("\tbDeviceProtocol: %02x\n", desc->bDeviceProtocol);
    printf("\tbMaxPacketSize0: %02x\n", desc->bMaxPacketSize0);
    printf("\tidVendor: %04x\n", letoh16(desc->idVendor));
    printf("\tidProduct: %04x\n", letoh16(desc->idProduct));
    printf("\tbcdDevice: %04x\n", letoh16(desc->bcdDevice));
    printf("\tiManufacturer: %s\n", get_str(dev, desc->iManufacturer));
    printf("\tiProduct: %s\n", get_str(dev, desc->iProduct));
    printf("\tiSerialNumber: %s\n", get_str(dev,desc->iSerialNumber));
    printf("\tbNumConfiguration: %02x\n", desc->bNumConfigurations);
    printf("\n");
}

static void lsusb_parse_config_descriptor(struct usb_device *dev,
                                          struct usb_config_descriptor *desc)
{
    printf("  Config Descriptor\n");
    printf("\twTotalLength: %04x\n", letoh16(desc->wTotalLength));
    printf("\tbNumInterfaces: %02x\n", desc->bNumInterfaces);
    printf("\tbConfigurationValue: %02x\n", desc->bConfigurationValue);
    printf("\tiConfiguration: %s\n", get_str(dev, desc->iConfiguration));
    printf("\tbmAttributes: %02x\n", desc->bmAttributes);
    printf("\tbMaxPower: %d mA\n", desc->bMaxPower * 2);
    printf("\n");
}

static void lsusb_parse_interface_descriptor(struct usb_device *dev,
                                             struct usb_interface_descriptor *desc)
{
    printf("  Interface Descriptor\n");
    printf("\tbInterfaceNumber: %02x\n", desc->bInterfaceNumber);
    printf("\tbAlternateSetting: %02x\n", desc->bAlternateSetting);
    printf("\tbNumEndpoints: %02x\n", desc->bNumEndpoints);
    printf("\tbInterfaceClass: %02x\n", desc->bInterfaceClass);
    printf("\tbInterfaceSubClass: %02x\n", desc->bInterfaceSubClass);
    printf("\tbInterfaceProtocol: %02x\n", desc->bInterfaceProtocol);
    printf("\tiInterface: %s\n", get_str(dev, desc->iInterface));
    printf("\n");
}

static void lsusb_parse_endpoint_descriptor(struct usb_device *dev,
                                            struct usb_endpoint_descriptor *desc)
{
    printf("  Endpoint Descriptor\n");
    printf("\tbEndpointAddress: %02x\n", desc->bEndpointAddress);
    printf("\tbmAttributes: %02x\n", desc->bmAttributes);
    printf("\twMaxPacketSize: %02x\n", letoh16(desc->wMaxPacketSize));
    printf("\tbInterval: %02x\n", desc->bInterval);
    printf("\tbRefresh: %02x\n", desc->bRefresh);
    printf("\tbSynchAddress: %02x\n", desc->bSynchAddress);
    printf("\n");
}

static void lsusb_dump_descriptor(struct usb_device *dev,
                                  struct usb_descriptor_header *desc)
{
    int i;
    printf("  Descriptor type %02x\n", desc->bDescriptorType);

    for (i = 0; i < desc->bLength; i++ ) {
        if ((i % 16) == 0)
            printf("\t%02x:", i);
        printf(" %02x", ((uint8_t *)desc)[i]);
        if ((i % 16) == 15)
            printf("\n");
    }

    if ((i % 16) != 0)
        printf("\n");
    printf("\n");
}

static void lsusb_parse_descriptor(struct usb_device *dev,
                                   struct usb_descriptor_header *desc)
{
    switch (desc->bDescriptorType) {
    case USB_DT_DEVICE:
        lsusb_parse_device_descriptor(dev, (struct usb_device_descriptor *) desc);
        break;

    case USB_DT_CONFIG:
        lsusb_parse_config_descriptor(dev, (struct usb_config_descriptor *) desc);
        break;

    case USB_DT_INTERFACE:
        lsusb_parse_interface_descriptor(dev, (struct usb_interface_descriptor *) desc);
        break;

    case USB_DT_ENDPOINT:
        lsusb_parse_endpoint_descriptor(dev, (struct usb_endpoint_descriptor *) desc);
        break;

    default:
        lsusb_dump_descriptor(dev, desc);

        break;
    }
}

static int lsusb_device_added(const char *dev_name, void *client_data)
{
unsigned int i;
    struct usb_device *dev = usb_device_open(dev_name);

    if (!dev) {
        fprintf(stderr, "can't open device %s: %s\n", dev_name, strerror(errno));
        return 0;
    }

    if (verbose) {
        struct usb_descriptor_iter iter;
        struct usb_descriptor_header *desc;

        printf("%s:\n", dev_name);

        usb_descriptor_iter_init(dev, &iter);

        while ((desc = usb_descriptor_iter_next(&iter)) != NULL)
            lsusb_parse_descriptor(dev, desc);

    } else {
        uint16_t vid, pid;
        char *mfg_name, *product_name, *serial;

        vid = usb_device_get_vendor_id(dev);
        pid = usb_device_get_product_id(dev);
        mfg_name = usb_device_get_manufacturer_name(dev);
        product_name = usb_device_get_product_name(dev);
        serial = usb_device_get_serial(dev);

        printf("%s: %04x:%04x %s %s %s\n", dev_name, vid, pid,
               mfg_name, product_name, serial);
        for(i=0; i < sizeof(modem_support_list)/sizeof(struct USB_MODEM_SUPPORT_LIST); i++)
        {
			if((vid == modem_support_list[i].vid)&&(pid == modem_support_list[i].pid))
			{
			struct usb_descriptor_iter iter;
			struct usb_descriptor_header *desc;
			struct usb_endpoint_descriptor *ep;
			    printf("found %02X %02X\n",vid,pid);
				if(modem_name)
					free(modem_name);
				modem_name = strdup(dev_name);
			    usb_descriptor_iter_init(dev, &iter);
			    while ((desc = usb_descriptor_iter_next(&iter)) != NULL)
			    {
				    switch(desc->bDescriptorType)
				    {
				    case USB_DT_INTERFACE:		
				    	memcpy(&modem_interface, desc, sizeof(modem_interface));
				    	break;
				    case USB_DT_ENDPOINT:
				    	ep = (struct usb_endpoint_descriptor *)desc;
				    	if(ep->bEndpointAddress&USB_ENDPOINT_DIR_MASK)
				    	{
				    	    memcpy(&modem_ep_in, ep, sizeof(modem_ep_in));
				    	}
				    	else
				    	{
				    	    memcpy(&modem_ep_out, ep, sizeof(modem_ep_out));
				    	}
				    	break;
				    default:
					    break;
				    }		
			    }
			    modem_idx = i;
			    break;
			}
        }
        free(mfg_name);
        free(product_name);
        free(serial);
    }

    usb_device_close(dev);

    return 0;
}

static int lsusb_device_removed(const char *dev_name, void *client_data)
{
    return 0;
}


static int lsusb_discovery_done(void *client_data)
{
    return 1;
}
#if 1
static unsigned char switchcmd[]={
0x55, 0x53, 0x42, 0x43, 0x12, 0x34, 0x56, 0x78, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xf0, 
0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#else
static unsigned char switchcmd[]={
0x55, 0x53, 0x42, 0x43, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11,
0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif
int r8_data_mode(int argc, char **argv)
{
    struct usb_host_context *ctx;
    struct usb_device *dev;
int ret;
int i;
struct USB_MODEM_SUPPORT_LIST *pml;
    if (argc == 2 && !strcmp(argv[1], "-v"))
        verbose = 1;
    ctx = usb_host_init();
    if (!ctx) {
        perror("usb_host_init:");
        return 1;
    }
    modem_name = NULL;
    usb_host_run(ctx,
                 lsusb_device_added,
                 lsusb_device_removed,
                 lsusb_discovery_done,
                 NULL);
    usb_host_cleanup(ctx);
    
    if(modem_name)
    {
    	if(modem_idx < sizeof(modem_support_list)/sizeof(struct USB_MODEM_SUPPORT_LIST))
   		{
    		pml = &modem_support_list[modem_idx];
    	}
    	else
    	{
   		    Printf("Invalid idx %d\n", modem_idx);
			return 0;			
    	}
	    //modem_interface.bInterfaceNumber = 0;
    	//modem_ep_out.bEndpointAddress = 1;
        //modem_name="/dev/bus/usb/001/001";
        if(pml->idelay)
	    	sleep(pml->idelay);        
   		Printf("%d modem_name %s \n",modem_idx, modem_name);
    	dev = usb_device_open(modem_name);
//    	usb_device_claim_interface(dev, modem_interface.bInterfaceNumber);
		 usb_device_connect_kernel_driver(dev, pml->bInf, 0);
        ret = usb_device_claim_interface(dev, pml->bInf);

   		Printf("interface %d endp %d %d\n", pml->bInf, pml->bOut, ret);
		ret = usb_device_bulk_transfer(dev, pml->bOut, pml->switchcmd, sizeof(pml->switchcmd), 0);
		if(ret < 0)
		{
			Printf("usb_device_bulk_transfer error %d\n", ret);
		}
    	free(modem_name);
        usb_device_release_interface(dev, pml->bInf);
		usb_device_connect_kernel_driver(dev, pml->bInf, 1);

    	usb_device_close(dev);
    }
    return 0;
}
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void handle_input_from_source1(int ifd1, int ifd2)
{
#if 1
int ilen;
unsigned char buffer[BUF_LEN];
    ilen = read(ifd1, buffer, BUF_LEN);
    Printf("%d: %02X %02X %02X %02X\n", ilen, buffer[0], buffer[1], buffer[2], buffer[3]);
    if(ilen <= 0)
    {
        pthread_mutex_lock(&lock);
        u3dg_comport_state = COMPORT_SRCOFF_DESOFF;
		pthread_mutex_unlock(&lock);        
		Printf("Read comport error %d %d", ifd1, ilen);
    }
    ilen = write(ifd2, buffer, ilen);
	if (ilen <= 0)
	{
        pthread_mutex_lock(&lock);	
		u3dg_comport_state = COMPORT_SRCOFF_DESOFF;
		pthread_mutex_unlock(&lock);		
		Printf("Write comport error %d %d\n", ifd2, ilen);
	}
#endif
};
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void handle_input_from_source2()
{};
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
int open_input_source(char *comport_name)
{
int ifd;
    ifd = open(comport_name,  O_RDWR | O_NONBLOCK, 0);
	if ( ifd <= 0) 
	{
	  Printf("Open %s error\n", comport_name);
	}
	return ifd;
}
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
int u3dg_is_comport_exist(struct U3DG_SETTING *setting)
{
int ifd, iret = 1; 
	ifd = open(setting->fileName,  O_RDONLY, 0);
	if (ifd <= 0) 
	{
	    Printf("Open RD %s error\n", setting->fileName);
		iret = 0;
	}
	close(ifd);
    return iret;
}
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
int u3dg_is_exist(void)
{
    u3dg_comport_state = COMPORT_SRCOFF_DESOFF;
    if(u3dg_is_comport_exist(&u3dg_setting[U3DG_SRC]))
    {
        pthread_mutex_lock(&lock);
		u3dg_comport_state |= COMPORT_SRCON_DESOFF;
		pthread_mutex_unlock(&lock);
		Printf("Comport %s exist\n", u3dg_setting[U3DG_SRC].fileName);
    }
    if(u3dg_is_comport_exist(&u3dg_setting[U3DG_DES]))
    {
        pthread_mutex_lock(&lock);
		u3dg_comport_state |= COMPORT_SRCOFF_DESON;
        pthread_mutex_unlock(&lock);
		Printf("Comport %s exist\n", u3dg_setting[U3DG_DES].fileName);
    }
    Printf("Checking com port status %d\n", u3dg_comport_state);
	return u3dg_comport_state;
}
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
int u3dg_open(int *fd1, int *fd2)
{
  struct termios options;   
	*fd1 = open_input_source(u3dg_setting[U3DG_SRC].fileName);   /* COM2 */
	if (*fd1 <= 0) 
	{
        pthread_mutex_lock(&lock);	
		u3dg_comport_state = COMPORT_SRCOFF_DESOFF;
		pthread_mutex_unlock(&lock);			
		Printf ("open %s fail\n", u3dg_setting[U3DG_SRC].fileName);
	//		exit(0);
	}
	if(u3dg_raw_flag & 1)
	{
	  tcgetattr( *fd1, &options );     
	  cfmakeraw(&options);
	  /* SEt Baud Rate */  
	  cfsetispeed( &options, B115200 );   
	  cfsetospeed( &options, B115200 ); 
	//  options.c_cflag |= (CRTSCTS | CS8 | CLOCAL | CREAD);
	  if(tcsetattr( *fd1, TCSANOW, &options ) == -1 ) 
	  {
	      Printf ("Error1 with tcsetattr \n");
	  }
	  else    
	  {
	      Printf ("tcsetattr1 succeed\n");
	  }
	}	
	*fd2 = open_input_source(u3dg_setting[U3DG_DES].fileName);   /* COM3 */
	if (*fd2 <=0) 
	{
        pthread_mutex_lock(&lock);	
		u3dg_comport_state = COMPORT_SRCOFF_DESOFF;
		pthread_mutex_unlock(&lock);			
		Printf ("open %s fail\n", u3dg_setting[U3DG_DES].fileName);	
	//		exit(0);
	}
	if(u3dg_raw_flag & 2)
	{
	  tcgetattr( *fd2, &options );     
	  cfmakeraw(&options);
	  /* SEt Baud Rate */  
	  cfsetispeed( &options, B115200 );   
	  cfsetospeed( &options, B115200 ); 
	//  options.c_cflag |= (CRTSCTS | CS8 | CLOCAL | CREAD);
	  if(tcsetattr( *fd2, TCSANOW, &options ) == -1 ) 
	  {
	      Printf ("Error2 with tcsetattr \n");
	  }
	  else    
	  {
	      Printf ("tcsetattr2 succeed\n");
	  }
	}	
	Printf("======== u3dg connect successfully ======= fd1=%d, fd2=%d\n", *fd1, *fd2);
	return 0;
}
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
int u3dg_close(int *fd1, int *fd2)
{
	close(*fd1);
	close(*fd2);	
	return 0;
}
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
int u3dg_process_inotify_events(struct U3DG_SETTING *setting, struct U3DG_SETTING *setting2)
{
int i;
char buffer[BUF_LEN];
int length ;
int iret=0;
	/* Process the inotify events */
	  length = read( setting->infd, buffer, BUF_LEN );  
	  if ( length < 0 ) {
	    Printf( "infd %s read error\n", setting->fileName);
	  }  
	  i =0;
	  while ( i < length ) {
	    struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
	    if ( event->len ) {
	      if ( event->mask & IN_CREATE ) {
	        if ( event->mask & IN_ISDIR ) {
	          Printf( "The directory %s was created.\n", event->name );       
	        }
	        else {
	          Printf( "The file %s was created.\n", event->name );
	        }
	        if(strcmp(setting->fileName+5, event->name) == 0)
 	           iret = COMPORT_CHNAGE;
	        if(strcmp(setting2->fileName+5, event->name) == 0)
 	           iret = COMPORT_CHNAGE;
	      }
	      else if ( event->mask & IN_DELETE ) {
	        if ( event->mask & IN_ISDIR ) {
	          Printf( "The directory %s was deleted.\n", event->name );       
	        }
	        else {
	          Printf( "The file %s was deleted.\n", event->name );
	        }
	        if(strcmp(setting->fileName+5, event->name) == 0)
 	           iret = COMPORT_CHNAGE;
	        if(strcmp(setting2->fileName+5, event->name) == 0)
 	           iret = COMPORT_CHNAGE;
	      }
	    }
	    i += sizeof(struct inotify_event) + event->len;
	  }
	  Printf("iret %d\n", iret);
   	return iret;
}
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
int u3dg_is_comport_changed(struct U3DG_SETTING *setting, struct U3DG_SETTING *setting2)
{
int return_value;
int iret = 0;
int maxfd;     /* maximum file desciptor used */

    maxfd = MAX (setting->infd, setting->infd)+1;
//    Printf("select 1\n");
    return_value = select(maxfd, &setting->nfds, NULL, NULL, NULL);
//    Printf("select 2\n");    
	if (return_value < 0)
	{
	    Printf("select error %d %s\n", return_value, setting->fileName);
		/* Error */
	}
	else if (!return_value) 
	{
	    Printf("select error %d %s\n", return_value, setting->fileName);
		/* Timeout */
	}
	else if(FD_ISSET(setting->infd, &setting->nfds))
    {
        	/* Process the inotify events */
	   		iret = u3dg_process_inotify_events(setting, setting2);
    }
  return iret;
};
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void *u3dg_is_changed(void *arg)
{
	while(loop)
	{
	    if(u3dg_is_comport_changed(&u3dg_setting[U3DG_SRC], &u3dg_setting[U3DG_DES]))
	    {
			sleep(1);
	    	u3dg_is_exist();
//	    	return COMPORT_CHNAGE;
	    }
	//    if(u3dg_is_comport_changed(&u3dg_setting[U3DG_DES]))
	    {
	//    	return COMPORT_CHNAGE;
	    }
//        Printf("u3dg_is_changed =%d\n", u3dg_comport_state);
//		sleep(1);
	}
	return 0;
}
#if 1
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void u3dg_chmodown(char *arg)
{
struct passwd *pw;
struct group *grp = NULL;
    uid_t uid;
    gid_t gid = -1; // passing -1 to chown preserves current group

    pw = getpwnam("radio");
    if (pw != NULL) 
    {
        uid = pw->pw_uid;
    } 
    else 
    {
          Printf("No such user '%s'\n", "radio");
          return 10;
    }
    grp = getgrnam("radio");
    if (grp != NULL) 
    {
        gid = grp->gr_gid;
    } 
    else 
    {
        Printf("No such group '%s'\n", "radio");
        return 11;
    }
    if (chown(arg, uid, gid) < 0) 
    {
        Printf("Unable to chown %s\n", arg);
        return 12;
    }
    if (chmod(arg, 0x1B0) < 0) 
    {
        Printf("Unable to chmod %s\n", arg);
        return 13;
    }
}
#endif
int u3dg_set_power_state(int state)
{
int fd, ret=0;

	printf("wake up state %d\n", state);
	fd = open(EXT_MD_MONITOR_DEV, O_RDWR);
	if (fd < 0) {
		printf("fail to open %s: ", EXT_MD_MONITOR_DEV);
		perror("");
		close(fd);
		return -1;
	}
	if(state == 3)
	{
	  ret = ioctl(fd, EXT_MD_IOCTL_RESET, NULL);
	}
	else
	{
	   ret = ioctl(fd, EXT_MD_IOCTL_R8_WAKEUPEN, &state);
	}
	if (ret < 0) {
		printf("wake up state failed!\n");
		close(fd);
		return ret;
	}	
	close(fd);
	return 0;			
}
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void *u3dg_switch(void *arg)
{
char *argv[]={"u3dg_switch",NULL};
char *fileName = USB_HS_DIR;
fd_set nfds;
int infd;
int iwd;
int i,j;
char buffer[BUF_LEN];
int length ;
int iret=0;
int return_value;
int maxfd;     /* maximum file desciptor used */
int mdrecovery = 0;
   	for(i = 0; i<0x10; i++)
   	{
   		if(access("/dev/ttyUSB1", F_OK)==0)
  		{
    		u3dg_chmodown("/dev/ttyUSB0");
    		u3dg_chmodown("/dev/ttyUSB1");
    		u3dg_chmodown("/dev/ttyUSB2");
    		u3dg_chmodown("/dev/ttyUSB3");  		
   			Printf("/dev/ttyUSB1 exist\n");
    		property_set("ctl.start", "gsm0710muxd");
    		u3dg_set_power_state(1);
    		break;
   		}
   		sleep(1);
   	}			        	
	r8_data_mode(1, argv);
 	infd= inotify_init();
 	if(infd <= 0)
 	{
     	Printf(" %s nofity init fail\n", fileName);
     	return 0;
 	}
 	//fcntl(u3dg_setting[U3DG_SRC].infd, F_SETFL, fcntl(u3dg_setting[U3DG_SRC].infd, F_GETFL) | O_NONBLOCK);
    FD_ZERO(&nfds);
    FD_SET(infd,&nfds);
	iwd = inotify_add_watch(infd, fileName, IN_CREATE | IN_DELETE );
 	if(iwd <= 0)
 	{
     	Printf(" %s nofity watch add fail\n", fileName);
     	return 0;
 	}	
	while(loop)
	{

	    maxfd = MAX (infd, infd)+1;
	//    Printf("select 1\n");
	    return_value = select(maxfd, &nfds, NULL, NULL, NULL);
		if (return_value < 0)
		{
		    Printf("select error %d %s\n", return_value, fileName);
			/* Error */
		}
		else if (!return_value) 
		{
		    Printf("select error %d %s\n", return_value, fileName);
			/* Timeout */
		}
		else if(FD_ISSET(infd, &nfds))
	    {
	        	/* Process the inotify events */
		   		//iret = u3dg_process_inotify_events(setting, setting2);
			/* Process the inotify events */
			  length = read( infd, buffer, BUF_LEN );  
			  if ( length < 0 ) {
			    Printf( "infd %s read error\n", fileName);
			  }  
			  i =0;
			  while ( i < length ) {
			    struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
			    if ( event->len ) {
			      if ( event->mask & IN_CREATE ) {
			        if ( event->mask & IN_ISDIR ) {
			          Printf( "The directory %s was created.\n", event->name );       
			        }
			        else {
			          Printf( "The file %s was created.\n", event->name );
			        }
			        r8_data_mode(1, argv);
			    	for(i = 0; i<0x10; i++)
			    	{
			    		if(access("/dev/ttyUSB4", F_OK)==0)
			    		{
			    		    if(mdrecovery > 8)
			    		    	break;
			    		    mdrecovery++;
   			                property_set("ctl.stop", "extmdlogger");
			                property_set("ctl.stop", "ril-daemon");
   			                property_set("ctl.stop", "gsm0710muxd");
    				        sleep(2);
                            u3dg_set_power_state(3);
   			                property_set("ctl.start", "extmdlogger");
                            break;
			    		}
			    		else if(access("/dev/ttyUSB2", F_OK)==0)
			    		{
			                property_set("ctl.stop", "ril-daemon");
   			                property_set("ctl.stop", "gsm0710muxd");
    				        sleep(5);
				    		//property_set("sys.usb.u3dg", "radio");
				    		//usleep(300000);
				    		u3dg_chmodown("/dev/ttyUSB0");
				    		u3dg_chmodown("/dev/ttyUSB1");
				    		u3dg_chmodown("/dev/ttyUSB2");
				    		u3dg_chmodown("/dev/ttyUSB3");
			    			Printf("/dev/ttyUSB2 exist\n");
				    		property_set("ctl.start", "gsm0710muxd");
                            u3dg_set_power_state(1);
				    		break;
			    		}
			    		sleep(1);
			    	}			        
			      }
			      else if ( event->mask & IN_DELETE ) {
			        if ( event->mask & IN_ISDIR ) {
			          Printf( "The directory %s was deleted.\n", event->name );       
			        }
			        else {
			          Printf( "The file %s was deleted.\n", event->name );
			          for(j = 0; j < 0x3; j++)
			          {
				          sleep(1);
				          if(access("/dev/ttyUSB1", F_OK)!=0)
			    		  {
			    		     Printf("/dev/ttyUSB1 removedt\n");
    		                 u3dg_set_power_state(0);
			    		     break;
				          }
			          }
			        }
			      }
			    }
			    i += sizeof(struct inotify_event) + event->len;
			  }
		}
		sleep(1);
	}
	return 0;
}
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
int u3dg_init(char *src)
{
 	u3dg_setting[U3DG_SRC].infd= inotify_init();
 	if(u3dg_setting[U3DG_SRC].infd <= 0)
 	{
     	Printf(" %s nofity init fail\n", src);
     	return 1;
 	}
 	//fcntl(u3dg_setting[U3DG_SRC].infd, F_SETFL, fcntl(u3dg_setting[U3DG_SRC].infd, F_GETFL) | O_NONBLOCK);
    FD_ZERO(&u3dg_setting[U3DG_SRC].nfds);
    FD_SET(u3dg_setting[U3DG_SRC].infd,&u3dg_setting[U3DG_SRC].nfds);
	u3dg_setting[U3DG_SRC].iwd = inotify_add_watch( u3dg_setting[U3DG_SRC].infd, src, IN_CREATE | IN_DELETE );
 	if(u3dg_setting[U3DG_SRC].iwd <= 0)
 	{
     	Printf(" %s nofity watch add fail\n", src);
     	return 2;
 	}	
#if 0 	
	u3dg_setting[U3DG_DES].infd = inotify_init();
 	if(u3dg_setting[U3DG_DES].infd <= 0)
 	{
     	Printf(" %s nofity init fail\n", des);
     	return 3;
 	}
// 	fcntl(u3dg_setting[U3DG_DES].infd, F_SETFL, fcntl(u3dg_setting[U3DG_DES].infd, F_GETFL) | O_NONBLOCK);
//    FD_ZERO(&u3dg_setting[U3DG_DES].nfds);
    FD_SET(u3dg_setting[U3DG_DES].infd,&u3dg_setting[U3DG_SRC].nfds);
	u3dg_setting[U3DG_DES].iwd = inotify_add_watch( u3dg_setting[U3DG_DES].infd, des, IN_CREATE | IN_DELETE );
 	if(u3dg_setting[U3DG_DES].iwd <= 0)
 	{
     	Printf(" %s nofity watch add fail\n", des);
     	return 4;
 	}	

#endif 	
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 5;
    }
    return 0;
};
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void u3dg_exit(void)
{
   inotify_rm_watch(u3dg_setting[U3DG_SRC].infd, u3dg_setting[U3DG_SRC].iwd);
   inotify_rm_watch(u3dg_setting[U3DG_DES].infd, u3dg_setting[U3DG_DES].iwd);
//   return 0;
};
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
int u3dg_process_cfg(int argc, char **argv)
{
    int c;

    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"r1", 0, &u3dg_raw_flag, 1},
            {"r2", 0, &u3dg_raw_flag, 2},
            {"r3", 0, &u3dg_raw_flag, 3},
            {"src", 1, 0, 's'},
            {"des", 1, 0, 'd'},
            {"d1", 0, &u3dg_debug, 1},
            {"d2", 0, &u3dg_debug, 2},
            {"d3", 0, &u3dg_debug, 3},
            {"t0", 0, &u3dg_thread, 0},
            {"t1", 0, &u3dg_thread, 1},
            {"t2", 0, &u3dg_thread, 2},
            {"t3", 0, &u3dg_thread, 3},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "bd:s:r",
                 long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'b':
            Printf ("option bb\n");
            u3dg_raw_flag |=2; 
            break;

        case 'r':
            Printf ("option r with value\n");
            u3dg_raw_flag |=1; 
            break;
        case 's':
        	u3dg_setting[U3DG_SRC].fileName = optarg;
        	break;
        case 'd':
        	u3dg_setting[U3DG_DES].fileName = optarg;
        	break;
        case '?':
            break;

        default:
            Printf ("?? getopt returned character code 0%x ??\n", c);
        }
    }


   return 0;
};

#define SIG_TEST 44 /* we define our own signal, hard coded since SIGRTMIN is different in user and in kernel space */ 

void receiveEmdCtlData(int n, siginfo_t *info, void *unused) {
	printf("received value %i\n", info->si_int);
	if (info->si_int == 1111) {
		property_set("ctl.stop", "ril-daemon");
		usleep(200*1000);
		property_set("ctl.stop", "gsm0710muxd");
		usleep(200*1000);
	}
}

int connectEmdCtl()
{
	int configfd;
	char buf[10];
	/* setup the signal handler for SIG_TEST 
 	 * SA_SIGINFO -> we want the signal handler function with 3 arguments
 	 */
	struct sigaction sig;
	sig.sa_sigaction = receiveEmdCtlData;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIG_TEST, &sig, NULL);

	/* kernel needs to know our pid to be able to send us a signal ->
 	 * we use debugfs for this -> do not forget to mount the debugfs!
 	 */
	configfd = open("/sys/kernel/debug/signalconfpid", O_WRONLY);
	if(configfd < 0) {
		perror("open");
		return -1;
	}
	sprintf(buf, "%i", getpid());
	if (write(configfd, buf, strlen(buf) + 1) < 0) {
		perror("fwrite"); 
		return -1;
	}
	return 0;
}


///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	int    fd1, fd2;  /* input sources 1 and 2 */
	fd_set readfs;    /* file descriptor set */
	int    maxfd;     /* maximum file desciptor used */
    int    iret=0;
    char ch=0;
	/* open_input_source opens a device, sets the port correctly, and
	 returns a file descriptor */
    pthread_t tid1,tid2;
	int	isConnectedEmdCtl = 0;

	if (connectEmdCtl() == 0)
		isConnectedEmdCtl = 1;
	switchusb_main(argc, argv);
	u3dg_process_cfg(argc,argv);
    if(u3dg_thread&1)
    {
		Printf("Start t1\n");
    	iret = pthread_create(&(tid1), NULL, &u3dg_switch, NULL);
    	if(u3dg_thread == 1)
    	{
			while(loop)
			{
				if (isConnectedEmdCtl == 0)
				{
					if (connectEmdCtl() == 0)
						isConnectedEmdCtl = 1;
				}
				sleep(2);
			};
			return iret;
    	}
    }
    else if(u3dg_thread&2)
    {
		Printf("Start t2\n");
    }
	else
	{
		return iret;
	}
	Printf("SRC %s\n", u3dg_setting[U3DG_SRC].fileName);
	Printf("DES %s\n", u3dg_setting[U3DG_DES].fileName);
	FD_ZERO(&readfs);                
	//if(u3dg_init(u3dg_setting[U3DG_SRC].fileName, u3dg_setting[U3DG_SRC].fileName))
	if(u3dg_init("/dev"))
	{
	    Printf("nofity init fail\n");
	   	return 1;
	}
	if(u3dg_is_exist() == COMPORT_SRCON_DESON)
	{
	    Printf("com port already exist\n");
	}
	iret = pthread_create(&(tid2), NULL, &u3dg_is_changed, NULL);
    if (iret != 0)
    {
        Printf("\ncan't create thread :[%d]", iret);
    }
    else
    {
        Printf("\n Thread created successfully\n");
		while(loop)
		{
		    Printf("u3dg_comport_state =%d\n", u3dg_comport_state);
			if(u3dg_comport_state == COMPORT_SRCON_DESON)
		    {
    		    //sleep(5);
		        u3dg_open(&fd1, &fd2);
				maxfd = MAX (fd1, fd2)+1;  /* maximum bit entry (fd) to test */
				/* loop for input */
				while (u3dg_comport_state == COMPORT_SRCON_DESON) 
				{
					FD_SET(fd1, &readfs);  /* set testing for source 1 */
					FD_SET(fd2, &readfs);  /* set testing for source 2 */
					/* block until input becomes available */
					select(maxfd, &readfs, NULL, NULL, NULL);
					if (FD_ISSET(fd1, &readfs))         /* input from source 1 available */
					{
						Printf("send ");
						handle_input_from_source1(fd1, fd2);
					}
					if (FD_ISSET(fd2, &readfs))         /* input from source 2 available */
					{
						Printf("rec ");
						handle_input_from_source1(fd2, fd1);
					}
	//	            if(u3dg_is_changed())
		            {
	//					break;
		            }
				}
	            u3dg_close(&fd1, &fd2);			
		    }
			
	//	    if(u3dg_is_changed())
		    {
	//	    	u3dg_is_exist();
		    }
		    sleep(2);
		}
   	}
	u3dg_exit();
	return iret;
}

