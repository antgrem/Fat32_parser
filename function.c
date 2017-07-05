#include "function.h"

void scan_root(void);
void scan_directory (uint32_t cluster);
uint32_t get_next_cluster (uint32_t cluster);
uint32_t get_FAT_table_value (uint32_t cluster);
file_attr_t get_next_element(void);
file_attr_t get_name(void);
void read_dir_strct(fat32_Dir_t * Dir);

uint16_t Get_FAT_information(void);
void get_BS_info(fat32_BS_t *BS);
void get_FS_info(fat32_FSInfo_t *FSInfo);

    static FILE *p_output_file;
    static FILE *p_file;

    static uint32_t FAT_start_pos, Cluster2_start_pos;
    static uint32_t Cluster_size, Root_dir_first_cluster;
    static uint32_t global_file_pos;

    static uint32_t BS_Info_start_pos;
    static uint8_t cluster_count;



uint8_t Parse_file(FILE *file_name, char *str_file_name)
{
    p_output_file = fopen("output.txt", "w+");

    p_file = file_name;

    printf("%s\n", str_file_name);// print image file name
    fprintf(p_output_file, "%s\n", str_file_name);

    // check type of FAT and
    // get main information of sectors starts position
    if(Get_FAT_information())
    {   //some kind of error. was printf in function Get_FAT_information()
        return 1;
    }


    scan_root();

    fclose(p_output_file);
    return 0;
}



void scan_root(void)
{
    scan_directory(Root_dir_first_cluster);
}


void scan_directory (uint32_t cluster)
{
    uint32_t data;
    file_attr_t temp_file;

    data = cluster;
    while ((data < LAST_CLUSTER) && (data != BAD_CLUSTER))
        {

            cluster_count = 0;
            global_file_pos = Cluster2_start_pos + (data - 2)*Cluster_size; //set start position of sector

            do
            {
                temp_file = get_next_element();
            }
            while((temp_file.Status != S_EMPTY) && (cluster_count < MAX_CLUSTER_FILES));

            data = get_next_cluster(data);
        }

}



uint32_t get_next_cluster (uint32_t cluster)
{

    return get_FAT_table_value(cluster);

}




uint32_t get_FAT_table_value (uint32_t cluster)
{
    uint32_t res;
    size_t cnt;
    char *str, i;

    fseek(p_file, FAT_start_pos + cluster*sizeof(uint32_t), SEEK_SET);
    cnt = fread(&res, 1, sizeof(uint32_t), p_file);
    if (cnt != sizeof(uint32_t))
    {// correction of reading file error
        fseek(p_file, FAT_start_pos + cluster*sizeof(uint32_t) + cnt + 1, SEEK_SET);

        str = (char *) &res;
        str += (char) cnt;

        *str++ = 0x1a;
        for (i= 0; i < sizeof(uint32_t) - cnt; i++)
            {
                *str++ = (char) fgetc(p_file);
            }
    }

    fseek(p_file, global_file_pos, SEEK_SET);

    return res;

}



file_attr_t get_next_element(void)
{
    static const wchar_t space_str[] = L"    ";
    static const wchar_t dir_str[] = L"L___";
    static uint16_t preambula_len = 0;

    file_attr_t element;
    uint32_t pos_file;
    uint16_t i;

    element = get_name();

    if ((element.Status != S_EMPTY) &&(element.Status != S_DELETED))
        switch (element.DIR_Attr)
        {
        case VOLUME_ID:

            preambula_len = 1;
            break;

        case ARCHIVE:
            for (i = 0; i < preambula_len; i++)
            {
                printf("%ls", space_str);
                fprintf(p_output_file, "%ls", space_str);
            }

            if (element.FlagSL_name == DLONG_NAME)
            {
                printf ("%ls\n", element.Long_Name);
                fprintf (p_output_file, "%ls\n", element.Long_Name);
            }
            else
            {
                printf("%s\n", element.ShortName);
                fprintf(p_output_file, "%s\n", element.ShortName);
            }
            break;

        case DIRECTORY:
            if ((element.ShortName[0] != '.'))
            {
                for (i = 0; i < preambula_len - 1; i++)
                {
                    printf("%ls", space_str);
                    fprintf(p_output_file, "%ls", space_str);
                }
                printf("%ls", dir_str);
                fprintf(p_output_file, "%ls", dir_str);

                if (element.FlagSL_name)
                {
                    printf ("%ls\n", element.Long_Name);
                    fprintf (p_output_file, "%ls\n", element.Long_Name);
                }
                else
                {
                    printf("%s\n", element.ShortName);
                    fprintf(p_output_file, "%s\n", element.ShortName);
                }

                preambula_len += 1;
                pos_file = global_file_pos;
                scan_directory(element.Cluster);
                global_file_pos = pos_file;
                preambula_len -= 1;

            }
            break;

        default:
            break;

        }

    return element;
}



//get information in cluster and transform it in string
file_attr_t get_name(void)
{
    uint8_t i, j;
    uint8_t LNF_count;
    uint8_t count_str;
    fat32_Dir_t dir;
    fat32_LFN_t long_name[20];
    file_attr_t res;

    //read first folder block
    read_dir_strct(&dir);

    //check
    if (dir.DIR_Name[0] == 0x00)
    {// empty folder
        //printf("Empty or deleted folder");
        res.Status = S_EMPTY;
        return res;
    }else if (dir.DIR_Name[0] == 0xE5)
    {// deleted folder
        res.Status = S_DELETED;
        return res;
    }
    else if ((dir.DIR_Attr & LNF) != LNF)
    {//short name
        res.DIR_Attr = dir.DIR_Attr;
        res.Cluster = dir.DIR_FstClusLO | (dir.DIR_FstClusHI<<16);
        res.FileSize = dir.DIR_FileSize;
        res.Status = (res.DIR_Attr == DIRECTORY) ? S_DIRECTORY : S_FILE;
        res.FlagSL_name = DSHORT_NAME;

        i = 0;
        j = 0;
        while ((j < 8) && (dir.DIR_Name[j] != ' '))
            res.ShortName[i++] = dir.DIR_Name[j++]; // copy name
        if(res.DIR_Attr != DIRECTORY)
            res.ShortName[i++] = '.';
        while ((j < 11) && (dir.DIR_Name[j] != ' '))
            res.ShortName[i++] = dir.DIR_Name[j++]; // copy extension
        res.ShortName[i] = '\0';

        res.Long_Name[0] = 0;// clean long string
    }
    else
    {  // long name
        LNF_count = 0;
        // save read data
        memcpy(&long_name[LNF_count], &dir, sizeof(fat32_LFN_t));

       do
        {//take all long name parts
           LNF_count++;
           read_dir_strct((fat32_Dir_t*)&long_name[LNF_count]);
        }while((long_name[LNF_count].LDIR_Attr & LNF) == LNF);

        //copy already read short name part of file
        memcpy(&dir, &long_name[LNF_count], sizeof(fat32_Dir_t));

        LNF_count--;
        count_str = 0;
        do
        { //connect parts of long name in one
            j = 0;
            while((j < 5) && (long_name[LNF_count].LDIR_Name1[j] != 0x0000) && (long_name[LNF_count].LDIR_Name1[j] != 0xFFFF))
                {
                    res.Long_Name[count_str] = long_name[LNF_count].LDIR_Name1[j];
                    j++;
                    count_str++;
                }

            j = 0;
            while((j < 6) && (long_name[LNF_count].LDIR_Name2[j] != 0x0000) && (long_name[LNF_count].LDIR_Name2[j] != 0xFFFF))
                {
                    res.Long_Name[count_str++] = long_name[LNF_count].LDIR_Name2[j];
                    j++;
                }

            j = 0;
            while((j < 2) && (long_name[LNF_count].LDIR_Name3[j] != 0x0000) && (long_name[LNF_count].LDIR_Name3[j] != 0xFFFF))
                {
                    res.Long_Name[count_str++] = long_name[LNF_count].LDIR_Name3[j];
                    j++;
                }
        }while(LNF_count--);

        res.Long_Name[count_str] = 0x0000;// zero end of string

        res.DIR_Attr = dir.DIR_Attr;
        res.Cluster = dir.DIR_FstClusLO | (dir.DIR_FstClusHI<<16);
        res.FileSize = dir.DIR_FileSize;
        res.Status = (res.DIR_Attr == DIRECTORY) ? S_DIRECTORY : S_FILE;
        res.FlagSL_name = DLONG_NAME;

        i = 0;
        j = 0;
        while ((j < 8) && (dir.DIR_Name[j] != ' '))
            res.ShortName[i++] = dir.DIR_Name[j++]; // copy name
        if(res.DIR_Attr != DIRECTORY)
            res.ShortName[i++] = '.';
        while ((j < 11) && (dir.DIR_Name[j] != ' '))
            res.ShortName[i++] = dir.DIR_Name[j++]; // copy extension
        res.ShortName[i] = '\0';
    }//end else long name

        return res;
}


// read one dir structure in cluster
void read_dir_strct(fat32_Dir_t * Dir)
{
    uint8_t i;
    char *str;
    size_t cnt;

    fseek(p_file, global_file_pos, SEEK_SET);

    cnt = fread(Dir, 1, sizeof(fat32_Dir_t), p_file);
    if(cnt != sizeof(fat32_Dir_t))
    {// corrected unexpected EOF
        fseek(p_file, global_file_pos + cnt + 1, SEEK_SET);

        str = (char *) Dir;
        str += (char) cnt;

        *str++ = 0x1a;
        for (i= 0; i < sizeof(fat32_Dir_t) - cnt; i++)
            {
                *str++ = (char) fgetc(p_file);
            }
    }

    cluster_count++; // count already read dir structure in cluster
    global_file_pos += sizeof(fat32_Dir_t); // prevent errors of reading

    //return 0;
}


// Check FAT32 system and calculate main parameters
uint16_t Get_FAT_information(void)
{
    fat32_BS_t BS;
    uint32_t TotSec, DataSec, CountofClusters;
    uint16_t FATSz;

    get_BS_info(&BS); //get information about boot sector of FAT

    FATSz = (BS.BS_FATSz16 != 0) ? BS.BS_FATSz16 : BS.BS_FATSz32;
    TotSec = (BS.BS_TotSec16 != 0) ? BS.BS_TotSec16 : BS.BS_TotSec32;

    DataSec = TotSec - (BS.BS_RsvdSecCnt + (BS.BS_NumFATs * FATSz));
    CountofClusters = DataSec / BS.BS_SecPerClus;

    //check correction fat
    if(CountofClusters < 4085) {
    printf("FAT12. Not FAT32!\n");
    return 1;
    } else if(CountofClusters < 65525) {
        printf("FAT16. Not Fat32!\n");
        return 1;
    } else {
        //printf("FAT32\n");
    }

    //calculated starts positions
    BS_Info_start_pos = BS.BS_FSInfo * BS.BS_BytsPerSec;
    FAT_start_pos = BS.BS_RsvdSecCnt * BS.BS_BytsPerSec; //bytes
    Cluster2_start_pos = FAT_start_pos + BS.BS_NumFATs * FATSz * BS.BS_BytsPerSec;//bytes
    Cluster_size = BS.BS_SecPerClus * BS.BS_BytsPerSec;
    Root_dir_first_cluster = BS.BS_RootClus;

    return 0;
}


void get_BS_info(fat32_BS_t *BS)
{
    // set position Boot sector structure in file
    fseek(p_file, 0, SEEK_SET);

    fread(BS, 1, sizeof(fat32_BS_t), p_file);

}



void get_FS_info(fat32_FSInfo_t *FSInfo)
{
    // set position FSinfo structure in file
    fseek(p_file, BS_Info_start_pos, SEEK_SET);

    fread(FSInfo, 1, sizeof(fat32_FSInfo_t), p_file);

}



