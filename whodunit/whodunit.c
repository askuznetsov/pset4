#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

const int SIZE_OF_HEADER=54;
const int SIZE_OF_BMP_INFO=40;
const int BITS_PER_PIXEL=24;
const int NO_COMPRESSION=0;
const int PARAGRAPH_WIDTH=4;
const int NEED_ARGUMENTS=3;
const WORD START_BMP=0x4d42;
const BYTE MAX_COLOR=0xff;
const BYTE MIN_COLOR=0x00;


RGBTRIPLE changeColor(RGBTRIPLE color){
    if(color.rgbtRed==MAX_COLOR && color.rgbtGreen==MIN_COLOR && color.rgbtBlue==MIN_COLOR)
    {
        color.rgbtGreen=MAX_COLOR;
        color.rgbtBlue=MAX_COLOR;
    }
    else if(color.rgbtRed!=MAX_COLOR || color.rgbtGreen!=MAX_COLOR || color.rgbtBlue !=MAX_COLOR)
    {
        color.rgbtGreen=MIN_COLOR;
        color.rgbtBlue=MAX_COLOR;
        color.rgbtRed=MIN_COLOR;
    }    
return color;    
}

int checkArguments(int argc)
{
    if (argc !=  NEED_ARGUMENTS)
        {
            fprintf(stderr, "Usage: ./copy infile outfile\n");
            return 1;
        }
    else
    {
        return 0;
    }
        
}

int main(int argc, char *argv[])
{
    if(checkArguments(argc)!=0)
    {
        return 1;
    }

    char *infile = argv[1];
    char *outfile = argv[2];

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
    
    bf.bfSize=bi.biSizeImage+sizeof(bf)+sizeof(bi);
    
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    int padding =  (PARAGRAPH_WIDTH - (bi.biWidth * sizeof(RGBTRIPLE)) % PARAGRAPH_WIDTH) % PARAGRAPH_WIDTH;
    
    RGBTRIPLE triple;
    
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        for (int j = 0; j < bi.biWidth; j++)
        {
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            
            triple=changeColor(triple);
            
            fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
        }

        fseek(inptr, padding, SEEK_CUR);

        for (int k = 0; k < padding; k++)
        {
            fputc(0x00, outptr);
        }
    }

    fclose(inptr);

    fclose(outptr);

    return 0;
}
