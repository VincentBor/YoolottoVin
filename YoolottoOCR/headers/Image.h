/*
 * Image.h
 *
 *  Created on: Jul 26, 2012
 *      Author: Wicek
 */

#ifndef IMAGE_H_
#define IMAGE_H_
#include  "../headers/wintypes.h"
#define MAX_WIDTH 3500
#define MAX_HEIGHT 3500
//#define HEADER_LEN 54
#define LEFT_BYTE 255
#define RIGHT_BYTE 255
#define LEFT_BIT 0x80
#define BLUE8 0xE0
#define GREEN8 0x1C
#define RED8 0x7

#define BFTYPEOFF 0
#define BFSIZEOFF 2
#define BFRESERVED1OFF 6
#define BFRESERVED2OFF 8
#define BFOOFBITSOFF 10

#define BISIZEOFF 0
#define BIWIDTHOFF 4
#define BIHEIGHTOFF 8
#define BIPLANESOFF 12
#define BIBITCOUNTOFF 14
#define BICOMPRESSIONOFF 16
#define BISIZEIMAGEOFF 20
#define BIXPELSPERMETEROFF 24
#define BIYPELSPERMETEROFF 28
#define BICLRUSEDOFF 32
#define BICLRIMPORTANTOFF 36




typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD, *LPRGBQUAD;

typedef struct tagBITMAPFILEHEADERW {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADERW, *LPBITMAPFILEHEADERW;

typedef struct tagBITMAPFILEHEADER {
	BYTE	bfType[2];
	BYTE	bfSize[4];
	BYTE	bfReserved1[2];
	BYTE	bfReserved2[2];
	BYTE	bfOffBits[4];
} BITMAPFILEHEADER,*LPBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADERW {
  DWORD biSize;
  DWORD  biWidth;
  DWORD  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  DWORD  biXPelsPerMeter;
  DWORD  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADERW, *LPBITMAPINFOHEADERW;

typedef struct tagBITMAPINFOHEADER{
	BYTE	biSize[4];
	BYTE	biWidth[4];
	BYTE	biHeight[4];
	BYTE	biPlanes[2];
	BYTE	biBitCount[2];
	BYTE	biCompression[4];
	BYTE	biSizeImage[4];
	BYTE	biXPelsPerMeter[4];
	BYTE	biYPelsPerMeter[4];
	BYTE	biClrUsed[4];
	BYTE	biClrImportant[4];
} BITMAPINFOHEADER,*LPBITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER_NUM {
	int	    bfType;
	unsigned long	bfSize;
	int	    bfReserved1;
	int	    bfReserved2;
	unsigned long	bfOffBits;
} BITMAPFILEHEADER_NUM ,*LPBITMAPFILEHEADER_NUM ;

typedef struct tagBITMAPINFOHEADER_NUM {
	unsigned long biSize;
	int	    biWidth;
	int	    biWidthPad;
	int  	biHeight;
	int  	biPlanes;
	int 	biBitCount;
	int 	biCompression;
	unsigned long	biSizeImage;
	int 	biXPelsPerMeter;
	int 	biYPelsPerMeter;
	int 	biClrUsed;
	int 	biClrImportant;
} BITMAPINFOHEADER_NUM ,*LPBITMAPINFOHEADER_NUM ;


typedef struct tagIMGDATA {

    char        szIMGPath[200];    // path to image files
    char        szFilePrototypePath[200];     // path to Prototype files
    char        szLibPath[200];     // path to OCR Lib files
    char        szInpPath[200];    // path to input directory
    char        szOutPath[200];     // path to output files
    char        szWorkPath[200];
    BITMAPFILEHEADER_NUM bf;
    BITMAPINFOHEADER_NUM bi;
    char        error[200];


    int        dpiX;                 //  Img's DPI on X axis
    int        dpiY;                 //  Img's DPI on Y axis
	int	       width;               // Img's width
	int  	   height;              // Img's height



    LPBYTE      ptr;                // pointer to bmp
    LPBYTE      pixPtr;              // pointer to pixels
    LPBYTE      colorPtr;              // pointer to colors

    int         iAngle;             // global Img's angle
    int        wFlip;              // actual orient.(1=180,2=270,3=90)

    char        szIMGName[50];     // current image file name
//    char        szFDFName[13];      // extracted FDF-name
    int        wConfidence[2];     // Img's confidences
} IMGDATA, *LPIMGDATA;

typedef struct tagRGBA {
	BYTE	   R;
	BYTE	   G;
	BYTE	   B;
	BYTE	   A;
} RGBA ,*LPRGBA;

#endif /* IMAGE_H_ */
