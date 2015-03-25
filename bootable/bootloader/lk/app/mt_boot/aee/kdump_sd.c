#include <string.h>
#include <malloc.h>
#include <mt_partition.h>
#include <stdint.h>
#include <string.h>
#include <video.h>
#include <platform/mtk_key.h>
#include <platform/mtk_wdt.h>
#include <target/cust_key.h>

#include "aee.h"
#include "kdump.h"
#include "kdump_elf.h"
#include "kdump_sdhc.h"

#define PAGE_SIZE 4096

#define DEVICE_SECTOR_BYTES 512

FAT_Para m_bFATInfo;
uint32_t m_bLastFATPage; 
struct aee_timer total_time;
static struct mrdump_dev *dumpdev;

LfnEntry g_LfnEntry = {
    0x41,								// sequence number
    {'C','\0','E','\0','D','\0','u','\0','m','\0'},                     // name characters (five UTF-16 characters)
    0x0F,								// attributes (always 0x0F)
    0x00,								// reserved (alwyas 0x00)
    0xDF,								// checksum of DOS file name
    {'p','\0','.','\0','k','\0','d','\0','m','\0','p','\0'},            // name characters (six UTF-16 characters)
    0x0000,								// word of first cluster (always 0x0000)
    {'\0','\0',0xFF,0xFF}
};

DirEntry g_DirEntry = {
    {'C','E','D','U','M','P','~','1','K','D','M'},              // name
    0x20,							// attr
    0x00,							// NTflags
    0x00,							// createdTimeMsec
    0x63E8,							// createdTime
    0x2E21,							// createdDate
    0x2E21,							// lastAccessDate
    0x0000,							// clusFirstHigh
    0x6490,							// time
    0x2E21,							// date
    0x0000,							// clusFirst
    0x00000000						// size
};

unsigned int OALGetTickCount(void)
{
    return 0 ;
}

static unsigned char ToLower(unsigned char c)
{
    if ((c >= 'A') && (c <= 'Z'))
        return c+ 'a' - 'A';
    else 
        return c;
}

static bool Compare_sd(const uint8_t *a, const uint8_t *b, int length)
{
    while (length--) {
        if (ToLower(*a++) != ToLower(*b++)) 
            return false;
    }
    return true;
}

static uint32_t BytesToNum_sd(uint8_t *b, int bytes)
{
    uint32_t result = 0;
    int i;

    for (i = 0; i < bytes; i++) {
        result |= b[i] << (i << 3);
    }
    return result;
}

static void fatfs_commit_fat_entry(FileHandler *pFileHandler, int fat_offset, uint32_t value)
{
    if (m_bFATInfo.FileSysType == FAT_32) {
        pFileHandler->FATBuffer[fat_offset * 4] = (value) & 0xff;
        pFileHandler->FATBuffer[fat_offset * 4 + 1] = (value >> 8) & 0xff;
        pFileHandler->FATBuffer[fat_offset * 4 + 2] = (value >> 16) & 0xff;
        pFileHandler->FATBuffer[fat_offset * 4 + 3] = (value >> 24) & 0xff;
    }
    else {
        pFileHandler->FATBuffer[fat_offset * 2] = (value) & 0xff;
        pFileHandler->FATBuffer[fat_offset * 2 + 1] = (value >> 8) & 0xff;
    }
}

uint32_t FindBootPartition_sd(uint8_t *SectorBuffer)
{
    uint32_t PartitionStart;
    uint32_t PartitionTpye;
    PartitionTpye = BytesToNum_sd(SectorBuffer + 0x1c2, 1);
    PartitionStart = BytesToNum_sd(SectorBuffer + 0x1c6, 4);

    voprintf_debug("SDCard: PartitionStart=0x%x ,PartitionTpye=0x%x\n", PartitionStart, PartitionTpye);
    if (PartitionTpye == 5) {
        if (!dumpdev->read(dumpdev, PartitionStart,SectorBuffer,1)) {
            voprintf_debug("SDCard: can not find BootPosition! \n"); 
            return 0;
        }
        PartitionStart += BytesToNum_sd(SectorBuffer + 0x1c6, 4);
        voprintf_debug("SDCard: PartitionStart=0x%x\n", PartitionStart);
    }
    return PartitionStart;
}

bool ReadBootPartition_sd(uint8_t *SectorBuffer)
{ 
    m_bFATInfo.BPB_BytsPerSec = BytesToNum_sd(SectorBuffer+11, 2);
    m_bFATInfo.BPB_SecPerClus = BytesToNum_sd(SectorBuffer+13, 1);
    m_bFATInfo.BPB_RsvdSecCnt = BytesToNum_sd(SectorBuffer+14, 2);
    m_bFATInfo.BPB_NumFATs = BytesToNum_sd(SectorBuffer+16, 1);
    m_bFATInfo.BPB_FATSz = BytesToNum_sd(SectorBuffer+22, 2);
    if (m_bFATInfo.BPB_FATSz) {
        m_bFATInfo.FileSysType = FAT_16;		
        m_bFATInfo.BPB_RootEntCnt = BytesToNum_sd(SectorBuffer+17, 2);
        m_bFATInfo.BPB_TotSec = BytesToNum_sd(SectorBuffer+19, 2);
        m_bFATInfo.BPB_RootClus = 0;
        voprintf_debug("SDCard: FilSysType = FAT16\n");
    }
    else {
        m_bFATInfo.FileSysType = FAT_32;
        m_bFATInfo.BPB_TotSec = BytesToNum_sd(SectorBuffer+32, 4);
        m_bFATInfo.BPB_FATSz = BytesToNum_sd(SectorBuffer+36, 4);
        m_bFATInfo.BPB_RootEntCnt = 0;
        m_bFATInfo.BPB_RootClus = BytesToNum_sd(SectorBuffer+44, 4);
        voprintf_debug("SDCard: FilSysType = FAT32\n");
    }
	
    voprintf_debug("SDCard: BPB_BytsPerSec = 0x%04x\n", m_bFATInfo.BPB_BytsPerSec);
    voprintf_debug("SDCard: BPB_SecPerClus = 0x%02x\n", m_bFATInfo.BPB_SecPerClus);
    voprintf_debug("SDCard: BPB_RsvdSecCnt = 0x%04x\n", m_bFATInfo.BPB_RsvdSecCnt);
    voprintf_debug("SDCard: BPB_NumFATs = 0x%02x\n", m_bFATInfo.BPB_NumFATs);
    voprintf_debug("SDCard: BPB_FATSz = 0x%08x\n", m_bFATInfo.BPB_FATSz);
    voprintf_debug("SDCard: BPB_RootClus = 0x%08x\n", m_bFATInfo.BPB_RootClus);	
    voprintf_debug("SDCard: BPB_TotSec = 0x%08x\n", m_bFATInfo.BPB_TotSec);

    if ((PAGE_SIZE > m_bFATInfo.BPB_SecPerClus * m_bFATInfo.BPB_BytsPerSec) ||
        ((m_bFATInfo.BPB_SecPerClus * m_bFATInfo.BPB_BytsPerSec) % PAGE_SIZE != 0)) {
        voprintf_error("Can't support SDCard cluster bytes %d\n", m_bFATInfo.BPB_SecPerClus * m_bFATInfo.BPB_BytsPerSec);
        return false;
    }
    return true;
}

static bool Block0_is_BootSector(uint8_t *Ptr)
{
    uint32_t BytesPerSec = 0;
    uint32_t SecPerClus = 0;
    uint32_t BPB_Media = 0;

    BytesPerSec = BytesToNum_sd(Ptr + 11, 2);
    if (!((BytesPerSec == 512)||(BytesPerSec == 1024)||(BytesPerSec == 2048)||(BytesPerSec == 4096))) {
        voprintf_error("Unsupport sector size %d\n", BytesPerSec);
        return false;
    }
    SecPerClus = BytesToNum_sd(Ptr + 13, 1);
    if(!((SecPerClus == 1)||(SecPerClus == 2)||(SecPerClus == 4)||
         (SecPerClus == 8)||(SecPerClus == 16)||(SecPerClus == 32)||
         (SecPerClus == 64)||(SecPerClus == 128)) && 
       (SecPerClus * BytesPerSec <= 0x10000)) {
        voprintf_error("Unsupport cluster size %d\n", SecPerClus);
        return false;
    }
    BPB_Media = BytesToNum_sd(Ptr + 21, 1);
    if(!((BPB_Media == 0xF8)||(BPB_Media == 0xF0)||(BPB_Media == 0xF9)||
         (BPB_Media == 0xFA)||(BPB_Media == 0xFB)||(BPB_Media == 0xFC)||
         (BPB_Media == 0xFD)||(BPB_Media == 0xFE)||(BPB_Media == 0xFF))) {
        voprintf_error("Unsupport media descriptor %d\n", BPB_Media);
        return false;
    }
	
    return true;
}

bool GetBPBInfo_sd(uint8_t *Ptr)
{   
    m_bFATInfo.BPB_BytsPerSec = 512;
    m_bFATInfo.BootStartSec = 0;

    //*pSectorPosition
    if (!dumpdev->read(dumpdev, m_bFATInfo.BootStartSec, Ptr, 1)) {
        voprintf_error("SDCard: can not find MBR\n"); 
        return false;
    }

    // Add support block0 is bootPartition
    if (Block0_is_BootSector(Ptr)) {
        m_bFATInfo.BootStartSec = 0;
    }
    else {
        m_bFATInfo.BootStartSec  = FindBootPartition_sd(Ptr);
        if (!dumpdev->read(dumpdev, m_bFATInfo.BootStartSec, Ptr, 1)) {
            voprintf_error("SDCard: can't find BootPosition\n"); 
            mrdump_status_error("SDCard: can't find BootPosition\n"); 
            return false;
        }
        if (!Block0_is_BootSector(Ptr)) {
            voprintf_error("SDCard: BPB sector dismatch FAT Spec\n");
            mrdump_status_error("SDCard: BPB sector dismatch FAT Spec\n");
            return false;
        }
    }
    if (!ReadBootPartition_sd(Ptr)) {
        voprintf_error("SDCard: can not Read BootPartition\n");
        mrdump_status_error("SDCard: can not Read BootPartition\n");
        return false;
    }

    m_bFATInfo.FATStartSec = m_bFATInfo.BootStartSec + m_bFATInfo.BPB_RsvdSecCnt;

    if (m_bFATInfo.FileSysType == FAT_32) {
        m_bFATInfo.ClusStartSec = m_bFATInfo.FATStartSec+(m_bFATInfo.BPB_NumFATs)*(m_bFATInfo.BPB_FATSz);
        m_bFATInfo.RootDirStartSec = m_bFATInfo.ClusStartSec + (m_bFATInfo.BPB_RootClus-2)*(m_bFATInfo.BPB_SecPerClus);
    }
    else {
        m_bFATInfo.RootDirStartSec = m_bFATInfo.FATStartSec+(m_bFATInfo.BPB_NumFATs)*(m_bFATInfo.BPB_FATSz);
        m_bFATInfo.ClusStartSec = m_bFATInfo.RootDirStartSec+32*m_bFATInfo.BPB_RootEntCnt/m_bFATInfo.BPB_BytsPerSec;
    }	

    return true;
}

uint32_t FindFirstClusInFAT_sd(uint32_t StartClusNum, uint8_t *Ptr)
{
    uint32_t SectorNum;
    uint32_t NextClusterPosition;
    uint32_t BytsPerFAT;
    uint32_t BytsPerAdd;
    if(m_bFATInfo.FileSysType == FAT_32)
        BytsPerAdd = 4;//FAT32	
    else
        BytsPerAdd = 2;//FAT16
	
    BytsPerFAT = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;
	
    m_bLastFATPage = StartClusNum/BytsPerFAT;
    SectorNum = m_bFATInfo.FATStartSec+m_bLastFATPage;
#ifdef SD_DATA_PRINT
    voprintf_verbose("SDCard: now FATSec = %d \n",SectorNum);
#endif
    if (!dumpdev->read(dumpdev, SectorNum, Ptr, 1)) {
        voprintf_error("SDCard: can not FindFirstClusInFAT_sd\n"); 
        // FIXME: this is not right, why return 0?
	return 0;
    }

    NextClusterPosition = BytesToNum_sd(Ptr+BytsPerAdd * (StartClusNum%BytsPerFAT), BytsPerAdd);
#ifdef SD_DATA_PRINT
    voprintf_verbose("SDCard: NextClusterPosition = %08x\n", NextClusterPosition);
#endif
    return NextClusterPosition;
}

uint32_t FindNextClusInFAT_sd(uint32_t StartClusNum, uint8_t *Ptr)
{
    uint32_t SectorNum;
    uint32_t NextClusterPosition;
    uint32_t BytsPerFAT;
    uint32_t BytsPerAdd;
    uint32_t TempPage;
    if(m_bFATInfo.FileSysType==FAT_32)
        BytsPerAdd = 4;//FAT32	
    else
        BytsPerAdd = 2;//FAT16
    
    BytsPerFAT = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;	
    TempPage = StartClusNum/BytsPerFAT;
    if(TempPage!=m_bLastFATPage) {	    
        SectorNum = m_bFATInfo.FATStartSec+TempPage;
#ifdef SD_DATA_PRINT
        DBGKDUMP_PRINTK("SDCard: now FATSec = %d \n",SectorNum);
#endif
        if(!dumpdev->read(dumpdev, SectorNum,Ptr,1)) {
            voprintf_error("SDCard: can not FindNextClusInFAT_sd! \n"); 
            // FIXME: this is not right, why return 0?
            return 0;
        }
        m_bLastFATPage=TempPage;
#if 0
        for(j=0;j<(int)m_bFATInfo.BPB_BytsPerSec;j++) {
            if((j%16)==0) DBGKDUMP_PRINTK("i= 0x%04x    ",j);
            DBGKDUMP_PRINTK(" %02x ", Ptr[j]);
            if(((j+1)%16)==0) DBGKDUMP_PRINTK("     i= %d\n",j);
        }
#endif
    }
    NextClusterPosition=BytesToNum_sd(Ptr+BytsPerAdd*(StartClusNum%BytsPerFAT),BytsPerAdd);
#ifdef SD_DATA_PRINT
    DBGKDUMP_PRINTK("SDCard: NextClusterPosition = %08x \n",NextClusterPosition);
#endif
    return NextClusterPosition;
}

uint32_t FindFirstFreeClusInFAT_sd(FileHandler *pFileHandler)
{
    uint32_t i;
    uint32_t SectorNum;
    uint32_t FreeClusterNum;
    uint32_t value;
    uint32_t FATSector, FATOffset;
    uint32_t EntryPerSector;	
    uint32_t BytsPerAdd;
	
    if(m_bFATInfo.FileSysType==FAT_32)
        BytsPerAdd = 4;//FAT32	
    else
        BytsPerAdd = 2;//FAT16

    FreeClusterNum = 0;
    EntryPerSector = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;
    FATSector = pFileHandler->CurrClusterNum/EntryPerSector;
    FATOffset = pFileHandler->CurrClusterNum%EntryPerSector + 1;	 // start search frome next cluster
	
    // for safty, we don't use all FAT entries, just reserve the last FAT sector
    while (FATSector < (m_bFATInfo.BPB_FATSz-1)) {
        // Read new FAT sector to cache
        SectorNum = m_bFATInfo.FATStartSec+FATSector;
        if (!dumpdev->read(dumpdev, SectorNum, pFileHandler->FATBuffer, 1)) {
            voprintf_error("SDCard: %s read failed\n", __func__); 
            return 0;
        }

        for (i=FATOffset; i<EntryPerSector; i++) {
            value = BytesToNum_sd(pFileHandler->FATBuffer+i*BytsPerAdd, BytsPerAdd);
            if (value == 0) {
                FreeClusterNum = FATSector*EntryPerSector + i;	// found free entry in FAT
                pFileHandler->FATSector = FATSector;
                return FreeClusterNum;
            }
        }
        // try next FAT sector
        FATSector++;
        FATOffset = 0;
    }

    return 0;
}

uint32_t ChainFreeClusInFAT_sd(FileHandler *pFileHandler)
{
    uint32_t i;
    uint32_t SectorNum;
    uint32_t FreeClusterNum;
    uint32_t value;
    uint32_t CurrFATSector, CurrFATOffset;
    uint32_t NextFATSector, NextFATOffset;
    uint32_t EntryPerSector;	
    uint32_t BytsPerAdd;
    uint8_t  TempFAT[512];
    uint8_t  *pBuf;
	
    if(m_bFATInfo.FileSysType==FAT_32)
        BytsPerAdd = 4;//FAT32	
    else
        BytsPerAdd = 2;//FAT16

    FreeClusterNum = 0;
    EntryPerSector = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;
    CurrFATSector = pFileHandler->CurrClusterNum/EntryPerSector;
    CurrFATOffset = pFileHandler->CurrClusterNum%EntryPerSector;
    NextFATSector = CurrFATSector;
    NextFATOffset = CurrFATOffset+1;	// start search frome next cluster
    memset(TempFAT,0,512);

    pBuf = pFileHandler->FATBuffer;
    // for safty, we don't use all FAT entries, just reserve the last FAT sector
    while (NextFATSector < (m_bFATInfo.BPB_FATSz-1)) {
        if (NextFATSector != pFileHandler->FATSector) {
            // FAT sector changed, read new FAT sector to temp buffer
            pBuf = TempFAT;
            SectorNum = m_bFATInfo.FATStartSec+NextFATSector;
            if (!dumpdev->read(dumpdev, SectorNum, pBuf, 1)) {
                voprintf_error("SDCard: %sread failed\n", __func__); 
                return 0;
            }
        }
        // find free cluster in FAT cache
        for (i=NextFATOffset; i<EntryPerSector; i++) {
            value = BytesToNum_sd(pBuf+i*BytsPerAdd, BytsPerAdd);
            if (value == 0) {
                FreeClusterNum = NextFATSector*EntryPerSector + i;	// found free entry in FAT
                // commit FAT entry
                fatfs_commit_fat_entry(pFileHandler, CurrFATOffset, FreeClusterNum);
                break;
            }
        }

        if (FreeClusterNum!=0) {
            break;
        }
		
        // try next FAT sector
        NextFATSector++;
        NextFATOffset = 0;
    }

    // check if need to refresh cache
    if ((FreeClusterNum != 0) && (NextFATSector != pFileHandler->FATSector)) {
        // FAT sector changed, flush cache to SD
        SectorNum = m_bFATInfo.FATStartSec+CurrFATSector;
        dumpdev->write(dumpdev, SectorNum, pFileHandler->FATBuffer, 1);
		
        // copy temp buffer to cache
        memcpy(pFileHandler->FATBuffer, TempFAT, 512);
        pFileHandler->FATSector = NextFATSector;
    }

    return FreeClusterNum;
}

bool MarkEndClusInFAT_sd(FileHandler *pFileHandler)
{
    uint32_t SectorNum;
    uint32_t BytsPerAdd;
    uint32_t EntryPerSector;
    uint32_t FATSector;
    uint32_t FATOffset;

    if(m_bFATInfo.FileSysType==FAT_32)
        BytsPerAdd = 4;//FAT32	
    else
        BytsPerAdd = 2;//FAT16	

    EntryPerSector = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;
    FATSector = pFileHandler->CurrClusterNum/EntryPerSector;
    FATOffset = pFileHandler->CurrClusterNum%EntryPerSector;
    SectorNum = m_bFATInfo.FATStartSec+FATSector;
	
    if (FATSector != pFileHandler->FATSector) {
        if (!dumpdev->read(dumpdev, SectorNum, pFileHandler->FATBuffer, 1)) {
            voprintf_error("SDCard: MarkEndClusInFAT_sd dumpdev->read failed\n"); 
            return false;
        }
    }

    fatfs_commit_fat_entry(pFileHandler, FATOffset, 0xffffffff);

    // flush FAT cache to SD
    if (!dumpdev->write(dumpdev, SectorNum, pFileHandler->FATBuffer, 1)) {
        voprintf_error("SDCard: MarkEndClusInFAT_sd dumpdev->write failed\n"); 
        return false;
    }
    return true;
}

bool DeleteFileInFAT_sd(FileHandler *pFileHandler)
{
    uint32_t StartClusNum;
    uint32_t SectorNum;
    uint32_t NextClusNum;
    uint32_t FATSector, FATOffset;
    uint32_t EntryPerSector;
    uint32_t BytsPerAdd;
    bool NewSector = true;
    bool LastEntry = false;

    if(m_bFATInfo.FileSysType == FAT_32) {
        BytsPerAdd = 4;//FAT32	
    }
    else {
        BytsPerAdd = 2;//FAT16
    }

    EntryPerSector = m_bFATInfo.BPB_BytsPerSec/BytsPerAdd;
    StartClusNum = pFileHandler->CurrClusterNum;
    SectorNum = m_bFATInfo.FATStartSec+StartClusNum/EntryPerSector;

    while (!LastEntry)
	{
            FATSector = StartClusNum/EntryPerSector;
            FATOffset = StartClusNum%EntryPerSector;

            if (NewSector) {
                SectorNum = m_bFATInfo.FATStartSec+FATSector;
#ifdef SD_DATA_PRINT
                DBGKDUMP_PRINTK("SDCard: now FATSec = %d \n",SectorNum);
#endif
                if(!dumpdev->read(dumpdev, SectorNum, pFileHandler->FATBuffer, 1)) {
                    voprintf_error("SDCard: DeleteFile_sd read failed\n"); 
                    return false;
                }
                NewSector = false;
            }
		
            NextClusNum = BytesToNum_sd(pFileHandler->FATBuffer+FATOffset*BytsPerAdd, BytsPerAdd);
	
            fatfs_commit_fat_entry(pFileHandler, FATOffset, 0x0);
            // Release FAT entry
            if(m_bFATInfo.FileSysType==FAT_32) {

                if ((NextClusNum >= 0xFFFFFF8) || (NextClusNum == 0)) {
                    LastEntry = true;
                }
            }
            else {
                if ((NextClusNum >= 0xFFF8) || (NextClusNum == 0)) {
                    LastEntry = true;
                }
            }

            if (NextClusNum/EntryPerSector != FATSector) {
                NewSector = true;	// next cluster is not at current FAT Sector, write current FAT sector back
            }

            if (NewSector || LastEntry)	{
                if (!dumpdev->write(dumpdev, SectorNum, pFileHandler->FATBuffer, 1))
                    {
                        voprintf_error("SDCard: DeleteFile_sd write failed\n"); 
                        return false;
                    }
            }

            StartClusNum = NextClusNum;
	}
	
#ifdef SD_DATA_PRINT
    DBGKDUMP_PRINTK("SDCard: NextClusterPosition = %08x \n",NextClusterPosition);
#endif

    return true;
}

static bool OpenDumpFile_sd(FileHandler *pFileHandler)
{
    int i, j, times;
    uint32_t SectorNum;
    uint32_t Temp;
    uint32_t dwStartTick;
    uint32_t FindFile_TIMEOUT = 60000;
    int SecLen;
    uint32_t NextRootFAT = 0;
    uint8_t RootDirFAT[512];
    bool foundLfn = false;

    // init File Handler
    memset(pFileHandler, 0, sizeof(FileHandler));
    if(!GetBPBInfo_sd(pFileHandler->FileBuffer)) {
        return false;
    }
	
    dwStartTick = OALGetTickCount(); 

    voprintf_debug("SDCard: FATStartSec=%d RootdirStartSec=%d\n", m_bFATInfo.FATStartSec, m_bFATInfo.RootDirStartSec);
    SectorNum = m_bFATInfo.RootDirStartSec;
    if (m_bFATInfo.FileSysType == FAT_16) {
        SecLen=32;
        times=1;
    }
    else {
        SecLen = m_bFATInfo.BPB_SecPerClus;
        times=MaxFindFileClusNum;
    }

    while(times) {
        for (i = 0; i < SecLen; i++) {
            if(!dumpdev->read(dumpdev, SectorNum, pFileHandler->FileBuffer, 1)) {
                voprintf_error("SDCard: can not read RootDir!\n"); 
                return false;
            }
                    
            for(j=0; j<(int)m_bFATInfo.BPB_BytsPerSec; j+=32) {
                if (foundLfn) {
                    Temp = BytesToNum_sd(pFileHandler->FileBuffer+j+20, 2);
                    pFileHandler->CurrClusterNum |= (Temp<<16);
                    Temp = BytesToNum_sd(pFileHandler->FileBuffer+j+26, 2);
                    pFileHandler->CurrClusterNum |= (Temp&0xFFFF);
					
                    // delete file DIR entries
                    memset(pFileHandler->FileBuffer+j, 0, 32);
                    if(!dumpdev->write(dumpdev, SectorNum, pFileHandler->FileBuffer, 1)) {
                        voprintf_error("SDCard: can not write directory entry!\n"); 
                        return false;
                    }
                    // delete file in FAT entries
                    if(!DeleteFileInFAT_sd(pFileHandler)) {
                        voprintf_error("SDCard: can not delete file in FAT entries!\n"); 
                        return false;
                    }
                    goto Done;
                }
                else if (pFileHandler->FileBuffer[j] == 0x41 &&
                         Compare_sd(pFileHandler->FileBuffer+j+1, g_LfnEntry.name1, 10) &&
                         Compare_sd(pFileHandler->FileBuffer+j+14, g_LfnEntry.name2, 12))
                    {
#if 0
                        // delete LFN entries
                        memset(pFileHandler->FileBuffer+j, 0, 32);
                        if(!dumpdev->write(dumpdev, SectorNum, pFileHandler->FileBuffer, 1)) 
                            {
                                voprintf_error("SDCard: can not write LFN entry!\n"); 
                                return false;
                            }
                        foundLfn = true;
#else
			voprintf_error("SDCard: dump file exist, skip dumping\n");
			mrdump_status_error("File exist at SDCARD, skip dumping\n");
			return false;
#endif
                    }
            }
            SectorNum+=1;
        }
        if((OALGetTickCount() - dwStartTick) >= FindFile_TIMEOUT)
            {
                voprintf_error("SDCard: Find File timeout\n");
                return false;
            }
        if(m_bFATInfo.FileSysType==FAT_32)
            {    
                if(times==MaxFindFileClusNum)
                    { 
                        NextRootFAT=FindFirstClusInFAT_sd(m_bFATInfo.BPB_RootClus, RootDirFAT);
                        if(NextRootFAT>=0xFFFFFF8)
                            {
                                voprintf_error("SDCard: Dump file not exsited.\n");
                                break;
                            }
                    }
                else
                    {    
                        NextRootFAT=FindNextClusInFAT_sd(NextRootFAT, RootDirFAT);
                        if(NextRootFAT>=0xFFFFFF8)
                            {
                                voprintf_error("SDCard: Dump file not exsited.\n");
                                break;
                            }   
                    }       
                voprintf_info("SDCard: NextRootFAT=0x%08x  \n", NextRootFAT);
                SectorNum=m_bFATInfo.ClusStartSec + (NextRootFAT-2)*(m_bFATInfo.BPB_SecPerClus);  
            }	
        else {
            // FAT16
            break;
        }
        times--;
    }
	
  Done:
    // return a free cluster to create new file
    pFileHandler->CurrClusterNum = 2;
    pFileHandler->CurrClusterNum = FindFirstFreeClusInFAT_sd(pFileHandler);
    pFileHandler->PrevClusterNum = pFileHandler->CurrClusterNum;
    g_DirEntry.clusFirstHigh = (uint16_t)((pFileHandler->CurrClusterNum >> 16) & 0xFFFF);
    g_DirEntry.clusFirst = (uint16_t)(pFileHandler->CurrClusterNum & 0xFFFF);
    if(!pFileHandler->CurrClusterNum)
	{
            pFileHandler->DiskFull = true;
            voprintf_error("SDCard full, not free space available at create file\n");
            mrdump_status_error("SDCard full, not free space available at create file\n");
            return false;
	}
	
    voprintf_debug("%s: ok\n", __func__);
    return true;
}

bool UpdateDirectoryEntry_sd(FileHandler *pFileHandler)
{
    int i, j, times;
    uint32_t SectorNum;
    uint32_t dwStartTick;
    uint32_t FindFile_TIMEOUT = 60000;
    int SecLen;
    uint32_t NextRootFAT = 0;
    uint8_t RootDirFAT[512];

    dwStartTick = OALGetTickCount(); 
    voprintf_info("SDCard: FATStartSec = %d \n",m_bFATInfo.FATStartSec);
    voprintf_info("SDCard: RootDirStartSec = %d \n", m_bFATInfo.RootDirStartSec);
    SectorNum = m_bFATInfo.RootDirStartSec;

    if(m_bFATInfo.FileSysType==FAT_16) {
        SecLen=32;
        times=1;
    }
    else {
        SecLen = m_bFATInfo.BPB_SecPerClus;
        times=MaxFindFileClusNum;
    }

    while(times) {
        for (i = 0; i < SecLen; i++) {
            if (!dumpdev->read(dumpdev, SectorNum, pFileHandler->FileBuffer, 1)) {
                voprintf_error("SDCard: can not read RootDir!\n"); 
                return false;
            }

            for (j = 0; j < (int)(m_bFATInfo.BPB_BytsPerSec - 32); j += 32) {
                if ((pFileHandler->FileBuffer[j] == 0x0 || pFileHandler->FileBuffer[j] == 0xE5) &&
                    (pFileHandler->FileBuffer[j+32] == 0x0 || pFileHandler->FileBuffer[j+32] == 0xE5)) {
                    memcpy(pFileHandler->FileBuffer+j, &g_LfnEntry, sizeof(LfnEntry));
                    memcpy(pFileHandler->FileBuffer+j+32, &g_DirEntry, sizeof(DirEntry));

                    if (!dumpdev->write(dumpdev, SectorNum, pFileHandler->FileBuffer, 1)) {
                        voprintf_error("SDCard: can not read RootDir! \n"); 
                        return false;
                    }
                    return true;
                }
            }
            SectorNum += 1;
        }
        if ((OALGetTickCount() - dwStartTick) >= FindFile_TIMEOUT) {
            voprintf_error("SDCard: Find File Error timeout\n");
            return false;
        }
        if (m_bFATInfo.FileSysType==FAT_32) {    
            if (times == MaxFindFileClusNum) { 
                NextRootFAT=FindFirstClusInFAT_sd(m_bFATInfo.BPB_RootClus, RootDirFAT);
                if(NextRootFAT>=0xFFFFFF8) {
                    voprintf_error("SDCard: Dump file not exsited.\n");
                    break;
                }
            }
            else {    
                NextRootFAT=FindNextClusInFAT_sd(NextRootFAT, RootDirFAT);
                if(NextRootFAT>=0xFFFFFF8) {
                    voprintf_error("SDCard: Dump file not exsited.\n");
                    break;
                }   
            }       
            voprintf_error("SDCard: NextRootFAT=0x%08x  \n", NextRootFAT);
            SectorNum=m_bFATInfo.ClusStartSec + (NextRootFAT-2)*(m_bFATInfo.BPB_SecPerClus);  
        }	
        else {
            // FAT16
            break;
        }
        times--;
    }
	
    return false;
}

static bool WriteDumpFile_sd(FileHandler *pFileHandler, uint8_t *Ptr, uint32_t Length, uint32_t Total)
{
    uint32_t i;
    uint32_t SectorNum;
    uint32_t FreeClusterNum;
    uint8_t val;

    uint32_t ClusterSize = m_bFATInfo.BPB_SecPerClus * m_bFATInfo.BPB_BytsPerSec;
	
    while (Length > 0) {
        // for every cluster boundary, check disk free space
        if (pFileHandler->DiskFull) {
            return false;
        }

        for(i = pFileHandler->BufferLen; (i < ClusterSize) && (Length > 0); i++) {
            val = *Ptr++;
            pFileHandler->FileBuffer[pFileHandler->BufferLen++] = val;
            pFileHandler->CheckSum += val;
            pFileHandler->TotalLen++;
            Length--;
        }

        // a cluster collected, flush to SD
        if (pFileHandler->BufferLen == ClusterSize) {
            pFileHandler->BufferLen = 0;
            SectorNum=m_bFATInfo.ClusStartSec + (pFileHandler->CurrClusterNum-2)*(m_bFATInfo.BPB_SecPerClus); 
            //DBGKDUMP_PRINTK("SDCard: WriteDumpFile_sd() write sd card from %d blocks!\n",SectorNum);
            
	    if(!dumpdev->write(dumpdev, SectorNum, pFileHandler->FileBuffer, m_bFATInfo.BPB_SecPerClus)) {
                    voprintf_error("SDCard: WriteDumpFile_sd() write file content from %d blocks failed!!!!\n",SectorNum); 
                    return false;
                }

            pFileHandler->PrevClusterNum = pFileHandler->CurrClusterNum;
            FreeClusterNum =  ChainFreeClusInFAT_sd(pFileHandler);
            if (FreeClusterNum == 0) {
                pFileHandler->DiskFull = true;
		voprintf_error("SDcard full, no free space available\n");
		mrdump_status_error("SDcard full, no free space available\n");
                return false;
            }
            pFileHandler->CurrClusterNum =FreeClusterNum;
        }
        else if (pFileHandler->BufferLen > ClusterSize) {
            voprintf_error("SDCard: WriteDumpFile_sd() BufferLen error!\n"); 
        }
    }
	
    return true;
}

static bool CloseDumpFile_sd(FileHandler *pFileHandler)
{
    uint32_t SectorNum;

    g_DirEntry.size = pFileHandler->TotalLen;

#if 0
    uint16_t year;
    g_DirEntry.createdTime = g_DirEntry.time = (((INREG16(&m_pRTCRegs->RTC_TC_HOU) << 11) & 0xF800) |
                                                ((INREG16(&m_pRTCRegs->RTC_TC_MIN) << 5) & 0x7E0) |
                                                ((INREG16(&m_pRTCRegs->RTC_TC_SEC)>>1) & 0x1F));
    year = INREG16(&m_pRTCRegs->RTC_TC_YEA);
    year = (year > 20) ? (year-20) : 0;
    g_DirEntry.createdDate = g_DirEntry.date = (((year<<9) & 0xFE00) |
                                                ((INREG16(&m_pRTCRegs->RTC_TC_MTH)<<5) & 0x1E0) |
                                                (INREG16(&m_pRTCRegs->RTC_TC_DOM) & 0x1F));
#endif											
    g_DirEntry.createdTime = g_DirEntry.time = 0;
    g_DirEntry.createdDate = g_DirEntry.date = 0;
    
    if (pFileHandler->BufferLen == 0) {
        // the free cluster is not used, so that the last cluster should be the previous one
        pFileHandler->CurrClusterNum = pFileHandler->PrevClusterNum;
    }
    else if (!pFileHandler->DiskFull) {
        // flush the reset data
        SectorNum=m_bFATInfo.ClusStartSec + (pFileHandler->CurrClusterNum-2)*(m_bFATInfo.BPB_SecPerClus); 
        if(!dumpdev->write(dumpdev, SectorNum, pFileHandler->FileBuffer, m_bFATInfo.BPB_SecPerClus)) {
            voprintf_error("SDCard: CloseFile_sd() write file content failed!\n"); 
            return false;
        }
    }

    if(!MarkEndClusInFAT_sd(pFileHandler)) {
        voprintf_error("SDCard: CloseFile_sd() MarkEndClusInFAT_sd failed!\n"); 
        return false;
    }	

    if(!UpdateDirectoryEntry_sd(pFileHandler)) {
        voprintf_error("SDCard: CloseFile_sd() UpdateDirectoryEntry_sd failed!\n"); 
        return false;
    }	
    
    return true;
}

static int sd_write_cb(void *handle, void *buf, int size)
{
    if (WriteDumpFile_sd(handle, buf, size, 0)) {
	return size;
    }
    else {
	return 0;
    }
}

static int kdump_file_output(const struct mrdump_control_block *mrdump_cb, uint32_t total_dump_size, struct mrdump_dev *mrdump_dev)
{
    const struct mrdump_machdesc *kparams = &mrdump_cb->machdesc;
    if (mrdump_dev == NULL) {
        return -1;
    }

    total_dump_size = memory_size() - ((uint32_t)kparams->phys_offset - (uint32_t)DRAM_PHY_ADDR);
    dumpdev = mrdump_dev;
    voprintf_info("%s dumping(address %p, size:%dM)\n", dumpdev->name, kparams->phys_offset, total_dump_size / 0x100000UL);

    mtk_wdt_restart();
    aee_timer_init(&total_time);

    aee_timer_start(&total_time);
    FileHandler *file_handle = calloc(1, sizeof(FileHandler));
    if (OpenDumpFile_sd(file_handle)) {
	bool ok = true;
        void *bufp = kdump_core_header_init(mrdump_cb, (uint32_t)kparams->phys_offset, total_dump_size);
	if (bufp != NULL) {
            mtk_wdt_restart();
            struct kzip_file *zf = kzip_open(file_handle, sd_write_cb);
            if (zf != NULL) {
                struct kzip_memlist memlist[3];
                memlist[0].address = bufp;
                memlist[0].size = KDUMP_CORE_SIZE;
                memlist[1].address = kparams->phys_offset;
                memlist[1].size = total_dump_size;
                memlist[2].address = NULL;
                memlist[2].size = 0;
                if (!kzip_add_file(zf, memlist, "SYS_COREDUMP")) {
		    ok = false;
		}
                kzip_close(zf);
                zf = NULL;
            }
            else {
                ok = false;
            }
         free(bufp);
        }

	mtk_wdt_restart();
	CloseDumpFile_sd(file_handle);
	free(file_handle);

	if (ok) {
            aee_timer_stop(&total_time);
	    voprintf_info("Dump finished.(%d sec)\n", total_time.acc_ms / 1000);
	    mrdump_status_ok("SDCARD-OUTPUT\nFILENAME:%s\nCHECKSUM:%x\n", "CEDump.kdmp", 0);
	}
    }

    return 0;
}

int kdump_emmc_output(const struct mrdump_control_block *kparams, uint32_t total_dump_size)
{
    kdump_file_output(kparams, total_dump_size, mrdump_dev_emmc());
    return 0;
}

int kdump_sdcard_output(const struct mrdump_control_block *kparams, uint32_t total_dump_size)
{

    kdump_file_output(kparams, total_dump_size, mrdump_dev_sdcard());
    return 0;
}

