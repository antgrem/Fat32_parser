#ifndef __FUNCTION_H_
#define __FUNCTION_H_

#include "main.h"


 //extern   uint32_t Fat_copy[];
 extern   char filename[];
 extern   FILE *p_file;
 extern   FILE *p_output_file;


#define S_EMPTY         0
#define S_DIRECTORY     1
#define S_FILE          2
#define S_DELETED       3

#define DSHORT_NAME     0
#define DLONG_NAME      1

#define MAX_CLUSTER_FILES       128
#define MAX_LNF_LENGHT          256
#define MAX_SHORT_NAME_LENGHT   12


/* Attributes of file/folder */
typedef struct {
    uint32_t    Cluster;                                // Address cluster
	uint32_t    FileSize;                               // File size;
	wchar_t     Long_Name[MAX_LNF_LENGHT];              // long unicode name
    uint16_t    Status;                                 // FILE, DIRECTORY, ENPTY or DELETED
    uint8_t     flagSL_name;                            // flag short (0) or long(1) file name
	uint8_t     DIR_Attr;                               // Attribute
    uint8_t     ShortName[MAX_SHORT_NAME_LENGHT];       // Short file name

}  file_attr_t;

file_attr_t get_name(void);
uint32_t get_next_cluster (uint32_t cluster);
void scan_directory (uint32_t cluster);

void scan_root(void);
file_attr_t get_next_element(void);
void read_dir_strct(fat32_Dir_t * Dir);

uint16_t Get_FAT_information(void);
void get_BS_info(fat32_BS_t *BS);
void get_FS_info(fat32_FSInfo_t *FSInfo);

uint32_t get_FAT_table_value (uint32_t cluster);

#endif // __FUNCTION_H_
