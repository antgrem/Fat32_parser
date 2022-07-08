#ifndef __FUNCTION_H_
#define __FUNCTION_H_

#include "main.h"
#include <stdint.h>

#define S_EMPTY         0
#define S_DIRECTORY     1
#define S_FILE          2
#define S_DELETED       3

#define DSHORT_NAME     0
#define DLONG_NAME      1

#define MAX_CLUSTER_FILES       128
#define MAX_LNF_LENGHT          256
#define MAX_SHORT_NAME_LENGHT   12

#define FILE_EXTENTION_LEN 3

#define FREE_CLUSTER    0x00000000
#define BAD_CLUSTER     0x0FFFFFF7
#define LAST_CLUSTER    0x0FFFFFF8

//Attributes of the file
#define READ_ONLY   0x01
#define HIDDEN      0x02
#define SYSTEM      0x04
#define VOLUME_ID   0x08
#define DIRECTORY   0x10
#define ARCHIVE     0x20
#define LNF         0x0F
//#define LNF         (READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID)

#define DIR_NAME            0         /* Short file name (11-byte) */
#define DIR_ATTR            11        /* Attribute (BYTE) */
#define DIR_NTRES           12        /* Lower case flag (BYTE) */
#define DIR_CRTTIME10       13        /* Created time sub-second (BYTE) */
#define DIR_CRTTIME         14        /* Created time (DWORD) */
#define DIR_LSTACCDATE      18        /* Last accessed date (WORD) */
#define DIR_FSTCLUSHI       20        /* Higher 16-bit of first cluster (WORD) */
#define DIR_MODTIME         22        /* Modified time (DWORD) */
#define DIR_FSTCLUSLO       26        /* Lower 16-bit of first cluster (WORD) */
#define DIR_FILESIZE        28        /* File size (DWORD) */
#define LDIR_ORD            0         /* LFN: LFN order and LLE flag (BYTE) */
#define LDIR_ATTR           11        /* LFN: LFN attribute (BYTE) */
#define LDIR_TYPE           12        /* LFN: Entry type (BYTE) */
#define LDIR_CHKSUM         13        /* LFN: Checksum of the SFN (BYTE) */
#define LDIR_FSTCLUSLO      26        /* LFN: MBZ field (WORD) */

#pragma pack(push)
#pragma pack(1)

/* Boot sector structure */
typedef struct {
    uint8_t   BS_jmpBoot[3];      // Jump Command
    uint8_t   BS_OEMName[8];      // OEM name
    uint16_t  BS_BytsPerSec;      // Number of bytes per sector
    uint8_t   BS_SecPerClus;      // Number of sectors per cluster
    uint16_t  BS_RsvdSecCnt;      // Number of reserved sectors at the beginning of the partition
    uint8_t   BS_NumFATs;         // Number of FATs on the partition
    uint16_t  BS_RootEntCnt;      // Number of root directory entries
    uint16_t  BS_TotSec16;        // Total number of sectors
    uint8_t   BS_Media;           // Media descriptor
    uint16_t  BS_FATSz16;         // Number of sectors per FAT
    uint16_t  BS_SecPerTrk;       // Number of sectors per track
    uint16_t  BS_NumHeads;        // Number of heads
    uint32_t  BS_HiddSec;         // Number of hidden sectors
    uint32_t  BS_TotSec32;        // Total sector count (32 bits)
    uint32_t  BS_FATSz32;         // Sectors per FAT (32 bits)
    uint16_t  BS_ExtFlags;        // Presently active FAT. Defined by bits 0-3 if bit 7 is 1.
    uint16_t  BS_FSVers;          // FAT32 filesystem version.  Should be 0:0
    uint32_t  BS_RootClus;        // Start cluster of the root directory (should be 2)
    uint16_t  BS_FSInfo;          // File system information
    uint16_t  BS_BkBootSec;       // Backup boot sector address.
    uint8_t   BS_Reserved[12];    // Reserved space
    uint8_t   BS_DrvNum;          // Drive number
    uint8_t   BS_Reserved1;       // Reserved space
    uint8_t   BS_BootSig;         // Boot signature - 0x29
    uint8_t   BS_VolID[4];        // Volume ID
    uint8_t   BS_VolLab[11];      // Volume Label
    uint8_t   BS_FilSysType[8];   // File system type in ASCII.  Not used for determination
} fat32_BS_t;

typedef struct {

    uint32_t    FSI_LeadSig;        //lead signature used to validate that this is in fact an FSInfo sector. Should be 0x41615252
    uint8_t     FSI_Reservd1[480];  //reserved/ zero.
    uint32_t    FSI_StrucSig;       //0x61417272. Another signature that is more localized in the sector to the location of the fields that are used.
    uint32_t    FSI_FreeCnt;        //
    uint32_t    FSI_NxtFree;        //
    uint8_t     FSI_Reservd2[12];   //reserved/ Zero.
    uint32_t    TrailSig;           //0xAA550000
} fat32_FSInfo_t;
//} __attribute__((packed)) fat32_FSInfo_t;

/* Directory object structure (DIR) */
typedef struct {
    uint8_t     DIR_Name[8];        // Short file name (SFN)
    uint8_t     DIR_Extension[3];   // Short file extension (SFN)
    uint8_t     DIR_Attr;           // Attribute
    uint8_t     DIR_NTres;          // Optional flags that indicates case information of the SFN
    uint8_t     DIR_CrtTime10;      // sub-second information corresponds to DIR_CrtTime
    uint16_t    DIR_CrtTime;        // Optional file creation time
    uint16_t    DIR_Date;           // Optional file creation date
    uint16_t    DIR_LstAccDate;     // Optional last access date
    uint16_t    DIR_FstClusHI;      // Upper part of cluster number. Always zero on the FAT12/16 volume
    uint16_t    DIR_WrtTime;        // Last time when any change is made to the file (typically on closeing)
    uint16_t    DIR_WrtDate;        // Last data when any change is made to the file (typically on closeing).
    uint16_t    DIR_FstClusLO;      // Lower part of cluster number. Always zero if the file size is zero
    uint32_t    DIR_FileSize;       // Size of the file in unit of byte. Not used when it is a directroy and the value must be always zero
} fat32_Dir_t;

/* Directory LFN structure (DIR) */
typedef struct {
    uint8_t         LDIR_Ord;           // Flag of LNF
    uint16_t        LDIR_Name1[5];      // Part of LFN from 1st character to 5th character
    uint8_t         LDIR_Attr;          // LFN attribute. Always ATTR_LONG_NAME and it indicates this is an LFN entry
    uint8_t         LDIR_Type;          // Must be zero
    uint8_t         LDIR_ChkSum;        // Checksum of the SFN entry associated with this entry
    uint16_t        LDIR_Name2[6];      // Part of LFN from 6th character to 11th character
    uint16_t        LDIR_FstClusLO;     // Must be zero to avoid any wrong repair by old disk utility
    uint16_t        LDIR_Name3[2];      // Part of LFN from 12th character to 13th character
} fat32_LFN_t;

#pragma pack(pop)

/* Attributes of file/folder */
typedef struct {
    uint32_t    Cluster;                                // Address cluster
    uint32_t    FileSize;                               // File size;
    wchar_t     Long_Name[MAX_LNF_LENGHT];              // long unicode name
    uint16_t    Status;                                 // FILE, DIRECTORY, ENPTY or DELETED
    uint8_t     FlagSL_name;                            // flag short (0) or long(1) file name
    uint8_t     DIR_Attr;                               // Attribute
    uint8_t     ShortName[MAX_SHORT_NAME_LENGHT];       // Short file name
} file_attr_t;

uint8_t parse_file(FILE *file_name, char *str_file_name);

#endif // __FUNCTION_H_
