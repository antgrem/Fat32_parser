#include "main.h"
#include "function.h"


int main(int argc, char *argv[])
{
    FILE *img_file;

char filename[MAX_LNF_LENGHT];

    // take name of image file
    if (argc ==1)
    {
        printf("Not enough parameters!\n");
        printf("Give file name like:    %s disk_name \n", argv[0]);
        printf("or:                     %s disk_name.img \n", argv[0]);

        return 1;
    }
    else
    {
       img_file = open_file (argv[1], filename);
    }

    if (img_file != NULL)
        {
            if(Parse_file(img_file, filename))
            {//error in function
                return 1;
            }
            fclose(img_file);
        }
        else
        {
            printf("Error open file!\n");
            return 1;
        }
    return 0;
}

FILE *open_file (const char * file_name, char *str_file_name)
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

    strcpy(str_file_name, pstr);
    pfile = fopen(pstr, "r");

    free(pstr);
    return pfile;
}


