#include "main.h"
#include "function.h"

    fat32_BS_t BS;
    fat32_FSInfo_t FSInfo;
    FILE *p_file;
    FILE *p_output_file;

    char filename[30];

int main(int argc, char *argv[])
{
    preambula_len = 0;

    if (argc ==1)
    {
        printf("Not enough parameters!\n");
        printf("Give file name like:    %s disk_name \n", argv[0]);
        printf("or:                     %s disk_name.img \n", argv[0]);

        return 1;
    }
    else
    {
       p_file = open_file (argv[1]);
    }

    p_output_file = fopen("output.txt", "w+");

    if (p_file)
        {
            get_BS_info(&BS); //get information about boot sector of FAT
            get_FS_info(&FSInfo);

            if(Get_FAT_information(&BS)) // get main information of sectors starts position
            {   //some kind of error. was printf in function Get_FAT_information()
                return 1;
            }

            scan_root();

            fclose(p_file);
            fclose(p_output_file);
        }
        else
        {
            printf("Error open file!\n");
            return 1;
        }
    return 0;
}

FILE *open_file (const char * file_name)
{
    size_t len;
    char *pstr = NULL;
    FILE *pfile;

    len = strlen(file_name);
    pstr = strstr(file_name, ".img");
    if (pstr)
    {
        pstr = (char*) malloc(len*sizeof(size_t));
        if (pstr == NULL)
            return NULL;
        strcpy(pstr, file_name);
    }
    else
    {//extension do not exist
        len += FILE_EXTENTION_LEN + 1;
        pstr = (char*) malloc(len*sizeof(size_t));

        if (pstr == NULL)
           return NULL;

        strcpy(pstr, file_name);
        strcat(pstr, ".img");
    }

    strcpy(filename, pstr);
    pfile = fopen(pstr, "r");

    free(pstr);
    return pfile;
}


