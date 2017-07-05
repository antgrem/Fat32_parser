#ifndef __FUNCTION_H_
#define __FUNCTION_H_

#include "main.h"

    uint32_t FAT_start_pos, Cluster2_start_pos;
    uint32_t Cluster_size, Root_dir_first_cluster;
    uint32_t FAT_len;
    uint32_t global_file_pos;
    uint16_t preambula_len;

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

#define MAX_CLUSTER_FILES   128


/* Attributes of file/folder */
typedef struct {
    uint32_t    Cluster;            // Address cluster
	uint32_t    FileSize;           // File size;
	wchar_t     Long_Name[256];     // long unicode name
    uint16_t    Status;
    uint8_t     flagSL_name;        // flag short (0) or long(1) file name
	uint8_t     DIR_Attr;           // Attribute
    uint8_t     ShortName[12];      // Short file name

}  file_attr_t;

file_attr_t get_name(void);
void scan_cluster (uint32_t Cluster);
void scan_directory (uint32_t cluster);

void scan_root(void);
file_attr_t get_next_element(void);
int read_dir_strct(fat32_Dir_t * Dir);

void Print_BS_info(fat32_BS_t BS);
uint16_t Get_FAT_information(const fat32_BS_t *BS);
void get_BS_info(fat32_BS_t *BS);
void get_FS_info(fat32_FSInfo_t *FSInfo);

uint32_t get_FAT_table_value (uint32_t cluster);

#endif // __FUNCTION_H_
