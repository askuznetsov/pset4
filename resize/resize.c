#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

const int SIZE_OF_HEADER=54;
const int SIZE_OF_BMP_INFO=40;
const int BITS_PER_PIXEL=24;
const int NO_COMPRESSION=0;
const int PARAGRAPH_WIDTH=4;
const int NEED_ARGUMENTS=4;
const int MORE_PRECISION=100;
const WORD START_BMP=0x4d42;
const BYTE MAX_COLOR=0xff;
const BYTE MIN_COLOR=0x00;


CHECKARGUMENTS checkArguments(int argc, char* argv[])
{       
    CHECKARGUMENTS answer;
    answer.is_bad=0;
    answer.factor=0;
    
    if (argc !=  NEED_ARGUMENTS)
    {
        fprintf(stderr, "Usage: ./copy n infile outfile\n");
        answer.is_bad=1;
    }


    if (sscanf(argv[1],"%f",&answer.factor)==0)
    {
        fprintf(stderr, "Usage: ./copy n infile outfile\n");
        answer.is_bad=1;   
    }
   
    if(answer.factor<0 || answer.factor>100)
    {
        fprintf(stderr, "Factor is must be in range of 0.0 to 100.0\n");
        answer.is_bad=1;   
    }
    
    return answer;
    
}

void printPadding(int out_padding, FILE *outptr)
{
    for (int p = 0; p < out_padding; p++)
    {
        fputc(0x00, outptr);
    }
    
}

void printRaw(float factor,RGBTRIPLE *buffer,LONG image_width, FILE *outptr, int padding)
{
    for (int r = 0; r < factor; r++)
    {
        fwrite(buffer, sizeof(RGBTRIPLE), image_width, outptr); 
        printPadding(padding,outptr);

    } 
}
int flowBuffer(float factor, int raw_element, RGBTRIPLE *buffer, RGBTRIPLE triple)
{   
    for (int k = 0; k < factor; k++) 
    {   
        buffer[raw_element++] = triple; 
    }
    return raw_element;
}

int main(int argc, char *argv[])
{   
    CHECKARGUMENTS argument=checkArguments(argc,argv);
    if(argument.is_bad==1)
    {
        return 1;
    }

    int precision=argument.factor*MORE_PRECISION;
    
    char *infile = argv[2];
    char *outfile = argv[3];

    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    if (bf.bfType != START_BMP || bf.bfOffBits != SIZE_OF_HEADER || bi.biSize != SIZE_OF_BMP_INFO || 
        bi.biBitCount != BITS_PER_PIXEL || bi.biCompression != NO_COMPRESSION)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    int in_padding =  (PARAGRAPH_WIDTH - (bi.biWidth * sizeof(RGBTRIPLE)) % PARAGRAPH_WIDTH) % PARAGRAPH_WIDTH;
    
    LONG in_width=bi.biWidth;
    LONG in_height=bi.biHeight;
    
    bi.biWidth=argument.factor*precision*bi.biWidth/precision;
    bi.biHeight=argument.factor*precision*bi.biHeight/precision;
    
    int out_padding =  (PARAGRAPH_WIDTH - (bi.biWidth * sizeof(RGBTRIPLE)) % PARAGRAPH_WIDTH) % PARAGRAPH_WIDTH;
    
    bi.biSizeImage=(sizeof(RGBTRIPLE)*bi.biWidth+out_padding)*abs(bi.biHeight);
    bf.bfSize=bi.biSizeImage+sizeof(bf)+sizeof(bi);
    
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    RGBTRIPLE triple;
    RGBTRIPLE *buffer = malloc(sizeof(RGBTRIPLE) * bi.biWidth); 
    
    int raw_element;
    
    for (int i = 0, biHeight = abs(in_height); i < biHeight; i++)
    {
        raw_element=0;

        for (int j = 0; j < in_width; j++)
        {
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            
            if(j*precision%(int)(precision/argument.factor)==0)
            {
                raw_element=flowBuffer(argument.factor, raw_element, buffer, triple);
            }
        }
       
        fseek(inptr, in_padding, SEEK_CUR);
        
        if(i*precision%(int)(precision/argument.factor)==0)
        {
            printRaw(argument.factor, buffer, bi.biWidth, outptr, out_padding);
        }
    }
    
    free(buffer);

    fclose(inptr);

    fclose(outptr);

    return 0;
}
