#include "function.h"

    const wchar_t space_str[] = L"    ";
    const wchar_t dir_str[] = L"\xC3---";
    uint32_t BS_Info_start_pos;
    uint8_t cluster_count;

int read_dir_strct(fat32_Dir_t * Dir)
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
    global_file_pos += 0x20; // prevent errors of reading

    return 0;
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
        res.flagSL_name = DSHORT_NAME;

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

#ifdef DEBIG_INFO
        printf("Name = %s\nAttr = 0x%x\nCluster = 0x%x\nSize = 0X%x\n", res.ShortName, res.DIR_Attr, res.Cluster, res.FileSize);
        printf("Name = %s\n\n", res.ShortName);
#endif // DEBIG_INFO
    }
    else
    {  // long name
        LNF_count = 0;
        // save read data
        memcpy(&long_name[LNF_count], &dir, sizeof(fat32_LFN_t));

       do
        {
           LNF_count++;
        //   pos_file = ftell(p_file);
           read_dir_strct((fat32_Dir_t*)&long_name[LNF_count]);
        }while((long_name[LNF_count].LDIR_Attr & 0x0F) == 0x0F);

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
            while((j < 6) && (long_name[LNF_count].LDIR_Name2[j] != 0x00) && (long_name[LNF_count].LDIR_Name2[j] != 0xFFFF))
                {
                    res.Long_Name[count_str++] = long_name[LNF_count].LDIR_Name2[j];
                    j++;
                }

            j = 0;
            while((j < 2) && (long_name[LNF_count].LDIR_Name3[j] != 0x00) && (long_name[LNF_count].LDIR_Name3[j] != 0xFFFF))
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
        res.flagSL_name = DLONG_NAME;

        i = 0;
        j = 0;
        while ((j < 8) && (dir.DIR_Name[j] != ' '))
            res.ShortName[i++] = dir.DIR_Name[j++]; // copy name
        if(res.DIR_Attr != DIRECTORY)
            res.ShortName[i++] = '.';
        while ((j < 11) && (dir.DIR_Name[j] != ' '))
            res.ShortName[i++] = dir.DIR_Name[j++]; // copy extension
        res.ShortName[i] = '\0';

#ifdef DEBIG_INFO
        printf("Short Name = %s\nAttr = 0x%x\nCluster = 0x%x\nSize = 0x%x\n", res.ShortName, res.DIR_Attr, res.Cluster, res.FileSize);
        wprintf (L"Name = %ls \n\n", res.Long_Name);
#endif // DEBIG_INFO

    }

        return res;
}


void scan_cluster (uint32_t Cluster)
{
    uint8_t count=0;
    file_attr_t temp_file;

#ifdef DEBIG_INFO
    printf ("------------------\n");
    printf("%d \t 0x%x\n", Cluster2_start_pos + (Cluster - 2)*Cluster_size,Cluster2_start_pos + (Cluster - 2)*Cluster_size);
    fprintf(p_output_file, "%d \t 0x%x\n", Cluster2_start_pos + (Cluster - 2)*Cluster_size,Cluster2_start_pos + (Cluster - 2)*Cluster_size);
    printf("------------------\n");
#endif

    cluster_count = 0;
    global_file_pos = Cluster2_start_pos + (Cluster - 2)*Cluster_size;

#ifdef DEBIG_INFO
    fprintf(p_output_file, "\n0x%x, pos = 0x%x, fat = 0x%x\n", Cluster, global_file_pos, Fat_copy[Cluster]);
#endif // DEBIG_INFO

    temp_file = get_next_element();

    while((temp_file.Status != S_EMPTY) && (cluster_count < MAX_CLUSTER_FILES))
        {
            count++;
            temp_file = get_next_element();
        }

}


void scan_directory (uint32_t cluster)
{
    uint32_t data;

    scan_cluster(cluster);

    data = get_FAT_table_value(cluster);
    while ((data < LAST_CLUSTER) && (data != BAD_CLUSTER))
        {
            //next_cluster = data;
            scan_cluster(data);
            data = get_FAT_table_value(data);
        }

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

void scan_root(void)
{
    scan_directory(Root_dir_first_cluster);
}



file_attr_t get_next_element(void)
{
    file_attr_t element;
    uint32_t pos_file;
    uint16_t i;

    element = get_name();

    if ((element.Status != S_EMPTY) &&(element.Status != S_DELETED))
        switch (element.DIR_Attr)
        {
        case VOLUME_ID:
            printf("%s\n", filename);
            fprintf(p_output_file, "%s\n", filename);

            preambula_len = 1;
            break;

        case ARCHIVE:
            for (i = 0; i < preambula_len; i++)
            {
                printf("%ls", space_str);
                fprintf(p_output_file, "%ls", space_str);
            }

            if (element.flagSL_name == DLONG_NAME)
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

                if (element.flagSL_name)
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

void get_FS_info(fat32_FSInfo_t *FSInfo)
{
    int res;
    res = fseek(p_file, BS_Info_start_pos, SEEK_SET);
    if (res == 0)
    {
        fread(FSInfo, 1, sizeof(fat32_FSInfo_t), p_file);
    }
}

void get_BS_info(fat32_BS_t *BS)
{
    int res;
    res = fseek(p_file, 0, SEEK_SET);
    if (res == 0)
    {
        fread(BS, 1, sizeof(fat32_BS_t), p_file);
    }
}

void get_FAT(uint32_t *FAT)
{
    int res;
    res = fseek(p_file, FAT_start_pos, SEEK_SET);
    if (res == 0)
    {
        fread(FAT, 1, FAT_len, p_file);
    }
}

// Check FAT32 system and calculate main parameters
uint16_t Get_FAT_information(const fat32_BS_t *BS)
{
    uint32_t TotSec, DataSec, CountofClusters;
    uint16_t FATSz;

    FATSz = (BS->BS_FATSz16 != 0) ? BS->BS_FATSz16 : BS->BS_FATSz32;
    TotSec = (BS->BS_TotSec16 != 0) ? BS->BS_TotSec16 : BS->BS_TotSec32;

    DataSec = TotSec - (BS->BS_RsvdSecCnt + (BS->BS_NumFATs * FATSz));
    CountofClusters = DataSec / BS->BS_SecPerClus;

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
    BS_Info_start_pos = BS->BS_FSInfo * BS->BS_BytsPerSec;
    FAT_start_pos = BS->BS_RsvdSecCnt * BS->BS_BytsPerSec; //bytes
    FAT_len = FATSz * BS->BS_BytsPerSec;
    Cluster2_start_pos = FAT_start_pos + BS->BS_NumFATs * FATSz * BS->BS_BytsPerSec;//bytes
    Cluster_size = BS->BS_SecPerClus * BS->BS_BytsPerSec;
    Root_dir_first_cluster = BS->BS_RootClus;

#ifdef DEBIG_INFO
    printf("FAT_start_pos = 0X%x, \nCluster2_start_pos = 0X%x\n", FAT_start_pos, Cluster2_start_pos);
    printf("Cluster size = 0x%x\nRoot dir first cluster = %d\n", Cluster_size, Root_dir_first_cluster);
#endif

    return 0;
}


void Print_BS_info(fat32_BS_t BS)
{
    uint32_t FirstDataSector;
    uint16_t FATSz;
    uint32_t TotSec, DataSec, CountofClusters;

            printf("Number of bytes per sector: BPB_BytsPerSec = %d   OK\n", BS.BS_BytsPerSec);//must be 512
            printf("Number of sectors per cluster: BPB_BS_SecPerClus = %d   OK\n", BS.BS_SecPerClus);
            printf("Number of reserved sectors at the beginning of the partition: BS_RsvdSecCnt = %d   OK\n", BS.BS_RsvdSecCnt);
            printf("Number of FATs on the partition: BS_NumFATs = %d   \n", BS.BS_NumFATs);
            printf("Number of root directory entries BS_RootEntCnt = %d\n", BS.BS_RootEntCnt);
            printf("Total sector count (32 bits) BS_TotSec32 = %d\n", BS.BS_TotSec32);
            printf("Sectors per FAT (32 bits) BS_FATSz32 = %d\n", BS.BS_FATSz32);
            printf("BS_FSInfo = %d\n", BS.BS_FSInfo);
            printf("BS_BootSig = %d\n", BS.BS_BootSig);

            printf("BS_NumFATs = %d\n", BS.BS_NumFATs);

            FirstDataSector = BS.BS_RsvdSecCnt + (BS.BS_NumFATs * BS.BS_FATSz32);//do not use RootDir section -> FAT23 do not have RootDir
            printf("FirstDataSector = %d\n", FirstDataSector);

            FATSz = (BS.BS_FATSz16 != 0) ? BS.BS_FATSz16 : BS.BS_FATSz32;
            TotSec = (BS.BS_TotSec16 != 0) ? BS.BS_TotSec16 : BS.BS_TotSec32;

            DataSec = TotSec - (BS.BS_RsvdSecCnt + (BS.BS_NumFATs * FATSz));
            printf("DataSec = %d\n", DataSec);
            CountofClusters = DataSec / BS.BS_SecPerClus;

            if(CountofClusters < 4085) {
            printf("FAT12. Not FAT32!\n");
            return ;
            } else if(CountofClusters < 65525) {
                printf("FAT16. Not Fat32!\n");
                return ;
            } else {
                printf("FAT32\n");
            }

}

