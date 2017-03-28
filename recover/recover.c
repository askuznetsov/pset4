#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint8_t  BYTE;

const int NEED_ARGUMENT=2;
const int BLOCK=512;
const BYTE BEGIN_IMAGE[5]={0xff,0xd8,0xff,0xe0,0xef};
const int FILE_NAME_SIZE=7;


int checkBeginImage(BYTE *file_block)
{
    if(file_block[0]==BEGIN_IMAGE[0] &&
       file_block[1]==BEGIN_IMAGE[1] &&
       file_block[2]==BEGIN_IMAGE[2] &&
       file_block[3]>=BEGIN_IMAGE[3] &&
       file_block[3]<=BEGIN_IMAGE[4])
       {
           return 1;
    }else
    {
        return 0;      
    }
}

int main (int argc, char* argv[])
{
    if(argc!=NEED_ARGUMENT)
    {
        fprintf(stderr, "Usage: ./recover infile\n");
        return 1;
    }

    char *infile = argv[1];
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }
    
    int outfile_indicator=0;
    FILE *outptr=NULL;
    char filename[FILE_NAME_SIZE];
    memset(&filename,0,FILE_NAME_SIZE*sizeof(char));
    BYTE buffer[BLOCK];
    
    while(fread(buffer,BLOCK,1,inptr) == 1)
    {
        if(checkBeginImage(buffer) == 1)
        {
            if(outptr != NULL)
            {
                    fclose(outptr);
            }
            sprintf(filename,"%03i.jpg",outfile_indicator++);
            outptr = fopen(filename, "w");
            if (outptr == NULL)
            {
                fclose(inptr);
                fprintf(stderr, "Could not create %s.\n", filename);
                return 3;
            }
        }else
        {
            if(outptr == NULL)
            {
                continue;
            }
        }
        fwrite(buffer,BLOCK,1,outptr);
    }
    
    free(outptr);
    free(inptr);
    
    return 0;
}
