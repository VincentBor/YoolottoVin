/*
 * PreProcessing.c
 *
 *  Created on: Sep 28, 2012
 *      Author: Wicek
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include  "../headers/Functions.h"
void histVPtrPix(LPBYTE ptrPix, int widthImg, int hightImg, int MaxSize, int x1, int x2, int y1, int y2, int *vHist);
void getMapValues_24(LPBYTE imgPtr, int biWidth, int biWidthPad, int biHeight, int *lpImgMap_m, int *lpImgMap_s, int grid);
void cleanTicket(LPIMGDATA img);
void smearing(LPIMGDATA imgInp, LPIMGDATA imgOut);
void sharpenImage(LPIMGDATA img,  int channels, int passes, float strength);
void testImage(LPIMGDATA img);
void createEdge(LPIMGDATA img,  int channels);
void microcontrast(LPIMGDATA img, float strength);
BYTE getPixelCompress(LPBYTE ptr, int xn, int yn, int wPad, int height);
void setPixelDecompress(LPBYTE ptr,  int wPad, int height, int x, int y, int color);
void createEdgeMonochrom(LPIMGDATA imgInp, LPIMGDATA img);
void addEdgeMonochrom(LPIMGDATA img, LPIMGDATA imgEdge);

#define H_SIZE_PREPROC  2000
int hHistPreProc[H_SIZE_PREPROC];
int vHistPreProc[H_SIZE_PREPROC];

//void saveLog(char *str1, char *str2);

void pre_ProcessColorImageBMP(LPIMGDATA img, LPIMGDATA imgOut){
    LPBYTE imgPtr = img->pixPtr;
    int    biWidthPad, biSizeImage;
    int *  lpImgMap_m;
    int *  lpImgMap_s;
    int    grid = 50;
    int    mapSize;
    IMGDATA imgOut2;
//    char   buff[500];

	biWidthPad = ((img->bi.biWidth*img->bi.biBitCount+31)/32)*4;
	biSizeImage = biWidthPad*img->bi.biHeight;
	mapSize = ((img->bi.biWidth/grid)+1)*((img->bi.biHeight/grid)+1);


//	sprintf(buff," biWidthPad= %d, img->bi.biBitCount= %d\n",biWidthPad, img->bi.biBitCount);
//	saveLog("-------", buff);



	switch(img->bi.biBitCount) {
	case 1:break;
	case 4:break;
	case 8:break;
	case 16:break;
	case 24:

/*
//        createEdge(img,  3);
		microcontrast(img, 0.05);
//		sharpenImage(img,  3, 1, 0.05);

//		testImage(img);
		{char imgNameTmp[500];
		   strcpy(imgNameTmp, img->szIMGName);
		   strcpy(&img->szIMGName[strlen(img->szIMGName)-5], "_2");
		   strcat(img->szIMGName, ".bmp");
		    saveBMP (img);
		    strcpy(img->szIMGName, imgNameTmp);
		}
*/
		lpImgMap_m = calloc(mapSize, sizeof(int));
	    if(lpImgMap_m == NULL) {
    	   sprintf(img->error,"pre_ProcessColorImageBMP ( \"%s\" ) calloc ( %d ) failed: %s\n",img->szIMGName, mapSize, strerror(errno));
    	   return;
        }
		lpImgMap_s = calloc(mapSize, sizeof(int));
	    if(lpImgMap_s == NULL) {
    	   sprintf(img->error,"pre_ProcessColorImageBMP ( \"%s\" ) calloc ( %d ) failed: %s\n",img->szIMGName, mapSize, strerror(errno));
    	   return;
        }
		getMapValues_24(imgPtr, img->bi.biWidth, biWidthPad, img->bi.biHeight, lpImgMap_m, lpImgMap_s, grid);
		convert24BMPtoMonochromeGrid(img, imgOut, lpImgMap_m, lpImgMap_s, grid);
		free(lpImgMap_m);
		free(lpImgMap_s);
		if(imgOut->bi.biWidth > imgOut->bi.biHeight)
		   rotate(imgOut, 90);

		cleanTicket(imgOut);
		smearing(imgOut, &imgOut2);
		createEdgeMonochrom(imgOut, &imgOut2);
//saveBMPbuffer (imgOut2.ptr, "gradient.bmp", imgOut->szOutPath);
		addEdgeMonochrom(imgOut, &imgOut2);

 //saveBMPbuffer (imgOut->ptr, "addEdge.bmp", imgOut->szOutPath);

		break;
	case 32:break;
	}
}

int getMeanFromGrid_24(LPBYTE imgPtr, int idxW, int idxH, int biWidth, int biWidthPad, int biHeight, int grid, int gridW, int gridH){
   int mean = 0;
   int xna, yna;
   int iW, iH, xn, yn;
   long nY;
   long nX;
   int sh = 3;
   LPBYTE ptr;
   int max;
//  char buff[500];

    yna = idxH*grid;
    xna = idxW*grid;
//if(yn >= 2200){
//sprintf(buff," xn= %d, yn= %d, grid= %d, gridW= %d, gridH= %d\n",xna, yna, grid, gridW, gridH);
//saveLog("-----------------------------------", buff);

//}
	for(iH=0;iH<gridH;iH++){
		yn =yna+iH;
		nY=(biHeight-1-yn)*biWidthPad;
		for(iW=0;iW<gridW;iW++){
			xn = xna+iW;
			nX = xn*sh;
			ptr = imgPtr+nY+nX;
//if(yn >= 2400){
//	sprintf(buff," xn= %d yn= %d\n",xn, yn);
//	saveLog("--", buff);
//  sprintf(buff," iW= %d, iH= %d, nX= %ld nY= %ld\n",iW, iH, nX, nY);
//  saveLog("-- ", buff);
//}
//		sprintf(buff," %ld\n",nY+nX);
//		saveLog("nY+nX= ", buff);
            max = max(*ptr, *(ptr+1));
            max = max(max, *(ptr+2));
//	        mean += ((*ptr+*(ptr+1)+*(ptr+2))/3);
            mean += max;
		}
	}
	mean /= (gridH*gridW);
   return mean;
}
int computeLuminosity(int r, int g, int b){
	return (int)((0.299 * (float)r) + (0.587 * (float)g) + (0.114 * (float)b)+0.5);
}

int getSigmaFromGrid_24(LPBYTE imgPtr, int idxW, int idxH, int biWidth, int biWidthPad, int biHeight, int grid, int gridW, int gridH, int mean){
   int sigma = 0;
   int xna, yna;
   int iW, iH, xn, yn, a;
   long nY;
   long nX;
   int sh = 3;
   LPBYTE ptr;
   int max;

    yna = idxH*grid;
    xna = idxW*grid;
	for(iH=0;iH<gridH;iH++){
		yn =yna+iH;
		nY=(biHeight-1-yn)*biWidthPad;
		for(iW=0;iW<gridW;iW++){
			xn = xna+iW;
			nX = xn*sh;
			ptr = imgPtr+nY+nX;
            max = max(*ptr, *(ptr+1));
            max = max(max, *(ptr+2));
//			a = ((*ptr+*(ptr+1)+*(ptr+2))/3)-mean;
			a = max-mean;
	        sigma += (a*a);
		}
	}
	sigma /= (gridH*gridW);
   return sqrt(sigma);
}
int getMeanLumFromGrid_24(LPBYTE imgPtr, int idxW, int idxH, int biWidth, int biWidthPad, int biHeight, int grid, int gridW, int gridH){
   int mean = 0;
   int xna, yna;
   int iW, iH, xn, yn;
   long nY;
   long nX;
   int sh = 3;
   int r, g, b;
   LPBYTE ptr;
//  char buff[500];

    yna = idxH*grid;
    xna = idxW*grid;
	for(iH=0;iH<gridH;iH++){
		yn =yna+iH;
		nY=(biHeight-1-yn)*biWidthPad;
		for(iW=0;iW<gridW;iW++){
			xn = xna+iW;
			nX = xn*sh;
			ptr = imgPtr+nY+nX;
//	        mean += ((*ptr+*(ptr+1)+*(ptr+2))/3);
	        b =*ptr;
	        g =*(ptr+1);
	        r =*(ptr+2);

            mean += computeLuminosity(r, g, b);
		}
	}
	mean /= (gridH*gridW);
   return mean;
}
int getMeanLumFromGridCenter_24(LPBYTE imgPtr, int idxW, int idxH, int biWidth, int biWidthPad, int biHeight, int grid, int gridW, int gridH){
   int mean = 0;
   int xna, yna;
   int iW, iH, xn, yn;
   long nY;
   long nX;
   int sh = 3;
   int r, g, b;
   LPBYTE ptr;
//  char buff[500];

    yna = idxH*grid;
    xna = idxW*grid;

	for(iH=-gridH/2;iH<gridH/2;iH++){
		yn =yna+iH;
		if(yn < 0 || yn >= biHeight) continue;
		nY=(biHeight-1-yn)*biWidthPad;
		for(iW=-gridW/2;iW<gridW/2;iW++){
			xn = xna+iW;
			if(xn < 0 || xn >= biWidth) continue;
			nX = xn*sh;
			ptr = imgPtr+nY+nX;
//	        mean += ((*ptr+*(ptr+1)+*(ptr+2))/3);
	        b =*ptr;
	        g =*(ptr+1);
	        r =*(ptr+2);

            mean += computeLuminosity(r, g, b);
		}
	}
	mean /= (gridH*gridW);
   return mean;
}

int getSigmaLumFromGrid_24(LPBYTE imgPtr, int idxW, int idxH, int biWidth, int biWidthPad, int biHeight, int grid, int gridW, int gridH, int mean){
   int sigma = 0;
   int xna, yna;
   int iW, iH, xn, yn, a;
   long nY;
   long nX;
   int sh = 3;
   LPBYTE ptr;
   int r, g, b;

    yna = idxH*grid;
    xna = idxW*grid;
	for(iH=0;iH<gridH;iH++){
		yn =yna+iH;
		nY=(biHeight-1-yn)*biWidthPad;
		for(iW=0;iW<gridW;iW++){
			xn = xna+iW;
			nX = xn*sh;
			ptr = imgPtr+nY+nX;
	        b =*ptr;
	        g =*(ptr+1);
	        r =*(ptr+2);
            a = computeLuminosity(r, g, b)-mean;
	        sigma += (a*a);
		}
	}
	sigma /= (gridH*gridW);
   return sqrt(sigma);
}
int getSigmaLumFromGridCenter_24(LPBYTE imgPtr, int idxW, int idxH, int biWidth, int biWidthPad, int biHeight, int grid, int gridW, int gridH, int mean){
   int sigma = 0;
   int xna, yna;
   int iW, iH, xn, yn, a;
   long nY;
   long nX;
   int sh = 3;
   LPBYTE ptr;
   int r, g, b;

    yna = idxH*grid;
    xna = idxW*grid;
	for(iH=-gridH/2;iH<gridH/2;iH++){
		yn =yna+iH;
		if(yn < 0 || yn >= biHeight) continue;
		nY=(biHeight-1-yn)*biWidthPad;
		for(iW=-gridW/2;iW<gridW/2;iW++){
			xn = xna+iW;
			if(xn < 0 || xn >= biWidth) continue;
			nX = xn*sh;
			ptr = imgPtr+nY+nX;
	        b =*ptr;
	        g =*(ptr+1);
	        r =*(ptr+2);
            a = computeLuminosity(r, g, b)-mean;
	        sigma += (a*a);
		}
	}
	sigma /= (gridH*gridW);
   return sqrt(sigma);
}

void getMapValues_24(LPBYTE imgPtr, int biWidth, int biWidthPad, int biHeight, int *lpImgMap_m, int *lpImgMap_s, int grid){
	int sizeW, sizeH, lastW, lastH;
	int idxW, idxH;
	int gridW, gridH;
	int m;
//	char buff[500];

	sizeW = biWidth/grid;
	lastW = biWidth - sizeW*grid;
	if(lastW > 0)
	   sizeW++;
	sizeH = biHeight/grid;
	lastH = biHeight - sizeH*grid;
	if(lastH > 0)
	   sizeH++;


//sprintf(buff," %d\n",biWidthPad*biHeight);
//saveLog("max img point= ", buff);
//sprintf(buff," sizeW= %d, lastW= %d, sizeH= %d lastH= %d\n",sizeW, lastW, sizeH, lastH);
//saveLog("-----------------------------------", buff);
//sprintf(buff," biWidth= %d, biWidthPad= %d, biHeight= %d\n",biWidth, biWidthPad, biHeight);
//	saveLog("-----------------------------------", buff);

	for(idxH=0;idxH<sizeH;idxH++){
		gridH = grid;
		if(idxH == sizeH-1) gridH = lastH;
	    for(idxW=0;idxW<sizeW;idxW++){
	    	gridW = grid;
	    	if(idxW == sizeW-1) gridW = lastW;
//			 m = getMeanFromGrid_24(imgPtr, idxW, idxH, biWidth, biWidthPad, biHeight, grid, gridW, gridH);
			 m = getMeanLumFromGrid_24(imgPtr, idxW, idxH, biWidth, biWidthPad, biHeight, grid, gridW, gridH);
//			 m = getMeanLumFromGridCenter_24(imgPtr, idxW, idxH, biWidth, biWidthPad, biHeight, grid, gridW, gridH);

			 *(lpImgMap_m+idxH*sizeW+ idxW) = m;
//			 *(lpImgMap_s+idxH*sizeW+ idxW) = getSigmaFromGrid_24(imgPtr, idxW, idxH, biWidth, biWidthPad, biHeight, grid, gridW, gridH, m);
			 *(lpImgMap_s+idxH*sizeW+ idxW) = getSigmaLumFromGrid_24(imgPtr, idxW, idxH, biWidth, biWidthPad, biHeight, grid, gridW, gridH, m);
//			 *(lpImgMap_s+idxH*sizeW+ idxW) = getSigmaLumFromGridCenter_24(imgPtr, idxW, idxH, biWidth, biWidthPad, biHeight, grid, gridW, gridH, m);

		}
	}
/*
	for(idxH=0;idxH<sizeH;idxH++){
		sprintf(buff,"\n");
    	saveLog("", buff);
	    for(idxW=0;idxW<sizeW;idxW++){
	    	sprintf(buff,"%d (%d), ", *(lpImgMap_m+idxH*sizeW+ idxW), *(lpImgMap_s+idxH*sizeW+ idxW));
	    	saveLog("", buff);

//	    	printf(" iW= %d, iH= %d, m= %d, s= %d \n", idxW, idxH, *(lpImgMap_m+idxH*sizeW+ idxW), *(lpImgMap_s+idxH*sizeW+ idxW));
		}
	}
*/
}

void convertBMPTo_mono_BMP_init(LPIMGDATA img, LPIMGDATA imgOutp){
int hSize = 54;
int bufforSize;
LPBYTE ptr ;
LPBITMAPFILEHEADER bf;
LPBITMAPINFOHEADER bi;
int  biSizeImage, biWidthPad;

biWidthPad = ((img->bi.biWidth+31)/32)*4;


biSizeImage = biWidthPad*img->bi.biHeight;
bufforSize= biSizeImage+hSize+8;

	    ptr = calloc(bufforSize, sizeof(BYTE));
	    if(ptr == NULL) {
	    	sprintf(img->error,"convertRGBAToBMP ( \"%s\" ) calloc ( %d ) failed: %s\n",img->szIMGName, bufforSize, strerror(errno));
	    	return;
	    }
	    bf = (LPBITMAPFILEHEADER)(ptr);
	    bi = (LPBITMAPINFOHEADER)(ptr + sizeof (BITMAPFILEHEADER));

	    bf->bfType[0]= 0x42;
	    bf->bfType[1]= 0x4d;
	    set4Bytes(bf->bfSize, bufforSize);

	    bf->bfReserved1[0] = 0;
	    bf->bfReserved1[1] = 0;
	    bf->bfReserved2[0] = 0;
	    bf->bfReserved2[1] = 0;
	    set4Bytes(bf->bfOffBits, hSize+8);

	    imgOutp->ptr = ptr;
	    imgOutp->bf.bfType = getLeftBytes((int*)bf->bfType, 2);
	    imgOutp->bf.bfSize = getLeftBytes((int*)bf->bfSize, 4);
	    imgOutp->bf.bfReserved1 =  getLeftBytes((int*)bf->bfReserved1, 2);
	    imgOutp->bf.bfReserved2 =  getLeftBytes((int*)bf->bfReserved2, 2);
	    imgOutp->bf.bfOffBits =   getLeftBytes((int*)bf->bfOffBits, 4);
	    imgOutp->pixPtr = ptr +imgOutp->bf.bfOffBits;



	    set4Bytes(bi->biSize, hSize-14);
        set4Bytes(bi->biWidth, img->width);
	    set4Bytes(bi->biHeight, img->height);
		*bi->biPlanes = (short)1;
		*bi->biBitCount= (short)1;


	    set4Bytes(bi->biCompression, 0);
		set4Bytes(bi->biSizeImage, biSizeImage);
		set4Bytes(bi->biXPelsPerMeter, 512);
	    set4Bytes(bi->biYPelsPerMeter, 512);
		set4Bytes(bi->biClrUsed, 0);
		set4Bytes(bi->biClrImportant, 0);


	     imgOutp->bi.biSize = getLeftBytes((int*)bi->biSize, 4);
	     imgOutp->bi.biWidth = getLeftBytes((int*)bi->biWidth, 4);
	     imgOutp->bi.biHeight = getLeftBytes((int*)bi->biHeight, 4);
	     imgOutp->bi.biPlanes = getLeftBytes((int*)bi->biPlanes, 2);
	     imgOutp->bi.biBitCount = getLeftBytes((int*)bi->biBitCount, 2);
	     imgOutp->bi.biCompression = getLeftBytes((int*)bi->biCompression, 4);
	     imgOutp->bi.biSizeImage = getLeftBytes((int*)bi->biSizeImage, 4);
	     imgOutp->bi.biXPelsPerMeter = getLeftBytes((int*)bi->biXPelsPerMeter, 4);
	     imgOutp->bi.biYPelsPerMeter = getLeftBytes((int*)bi->biYPelsPerMeter, 4);
	     imgOutp->bi.biClrUsed = getLeftBytes((int*)bi->biClrUsed, 4);
	     imgOutp->bi.biClrImportant = getLeftBytes((int*)bi->biClrImportant, 4);


	     imgOutp->bi.biWidthPad = ((imgOutp->bi.biWidth*imgOutp->bi.biBitCount+31)/32)*4;
	     imgOutp->colorPtr = ptr + 14 + imgOutp->bi.biSize;
	     *(ptr+hSize) = 0;
	     *(ptr+hSize+1) = 0;
	     *(ptr+hSize+2) = 0;
	     *(ptr+hSize+3) = 0;
	     *(ptr+hSize+4) = 255;
	     *(ptr+hSize+5) = 255;
	     *(ptr+hSize+6) = 255;
	     *(ptr+hSize+7) = 0;


//      setPixelArrayFromBMPTomonoWB_tH3(imgOutp, img, tH1, tH2, tH3);
}

void convertBMPTo_mono_BMP_Next(LPBYTE ptrInp, int biWidthInp, int biWidthPadInp, int biHeightInp,
		                   LPBYTE ptrOut, int biWidthOut, int biWidthPadOut, int biHeightOut,
		                   int idxW, int idxH, int grid, int gridW, int gridH, int tH){

	   int iW, iH;
	   int xna, yna;
	   int xn, yn;
	   long nYInp, nXInp;
	   long nYOut, nXOut;

	   int sh = 3;
	   LPBYTE ptrI, ptrO;
	   int v;
	   BYTE mask;
	   int  max;

	    yna = idxH*grid;
	    xna = idxW*grid;

		for(iH=0;iH<gridH;iH++){
			yn =yna+iH;
			nYInp=(biHeightInp-1-yn)*biWidthPadInp;
			nYOut=(biHeightOut-1-yn)*biWidthPadOut;
			for(iW=0;iW<gridW;iW++){
				xn = xna+iW;
				nXInp = xn*sh;
				nXOut = xn/8;
				ptrI = ptrInp+nYInp+nXInp;
//		        v = (*ptrI+*(ptrI+1)+*(ptrI+2))/3;
	            max = max(*ptrI, *(ptrI+1));
	            max = max(max, *(ptrI+2));
	            v = max;
		        if(v < tH) v= 0; else v = 255;

				ptrO = ptrOut+nYOut+nXOut;
		        mask=LEFT_BIT;
		        mask >>=(xn%8);
		        if(v!=0){
		        	 *ptrO = *ptrO | mask;
		         }
		         else {
		        	 *ptrO = *ptrO & (~mask);
		         }

			}
		}

}
void convertBMPTo_mono_BMPLum_Next(LPBYTE ptrInp, int biWidthInp, int biWidthPadInp, int biHeightInp,
		                   LPBYTE ptrOut, int biWidthOut, int biWidthPadOut, int biHeightOut,
		                   int idxW, int idxH, int grid, int gridW, int gridH, int tH){

	   int iW, iH;
	   int xna, yna;
	   int xn, yn;
	   long nYInp, nXInp;
	   long nYOut, nXOut;

	   int sh = 3;
	   LPBYTE ptrI, ptrO;
	   int v;
	   BYTE mask;
//	   int  max;
	   int r, g, b;

	    yna = idxH*grid;
	    xna = idxW*grid;

		for(iH=0;iH<gridH;iH++){
			yn =yna+iH;
			nYInp=(biHeightInp-1-yn)*biWidthPadInp;
			nYOut=(biHeightOut-1-yn)*biWidthPadOut;
			for(iW=0;iW<gridW;iW++){
				xn = xna+iW;
				nXInp = xn*sh;
				nXOut = xn/8;
				ptrI = ptrInp+nYInp+nXInp;
//	            max = max(*ptrI, *(ptrI+1));
//	            max = max(max, *(ptrI+2));
		        b =*ptrI;
		        g =*(ptrI+1);
		        r =*(ptrI+2);
	            v = computeLuminosity(r, g, b);
		        if(v < tH) v= 0; else v = 255;

				ptrO = ptrOut+nYOut+nXOut;
		        mask=LEFT_BIT;
		        mask >>=(xn%8);
		        if(v!=0){
		        	 *ptrO = *ptrO | mask;
		         }
		         else {
		        	 *ptrO = *ptrO & (~mask);
		         }

			}
		}

}

void convert24BMPtoMonochromeGrid(LPIMGDATA img, LPIMGDATA imgOut, int *lpImgMap_m, int *lpImgMap_s, int grid){
		LPBITMAPINFOHEADER bi;

		int sizeW, sizeH, lastW, lastH;
		int idxW, idxH;
		int gridW, gridH;
		int biWidthInp, biWidthPadInp, biHeightInp;
		int biWidthOut, biWidthPadOut, biHeightOut;
		int m, s, tH;
//	    int sh = 3;
//	    float k = 0.07;
//	    float k = 0.1;
	    float k = 0.15;
	    int   R = 128;
//	    int   R = 500;
//   	    char buff[500];


		biWidthInp=img->bi.biWidth;
		biWidthPadInp = ((img->bi.biWidth*img->bi.biBitCount+31)/32)*4;
		biHeightInp = img->bi.biHeight;

		convertBMPTo_mono_BMP_init(img, imgOut);
		biWidthOut=imgOut->bi.biWidth;
		biWidthPadOut = ((imgOut->bi.biWidth+31)/32)*4;
		biHeightOut = imgOut->bi.biHeight;


		sizeW = img->bi.biWidth/grid;
		lastW = img->bi.biWidth - sizeW*grid;
		if(lastW > 0)
		   sizeW++;
		sizeH = img->bi.biHeight/grid;
		lastH = img->bi.biHeight - sizeH*grid;
		if(lastH > 0)
		   sizeH++;

		for(idxH=0;idxH<sizeH;idxH++){
			gridH = grid;
			if(idxH == sizeH-1) gridH = lastH;
		    for(idxW=0;idxW<sizeW;idxW++){
		    	gridW = grid;
		    	if(idxW == sizeW-1) gridW = lastW;
		    	m = *(lpImgMap_m+idxH*sizeW+ idxW);
		    	s = *(lpImgMap_s+idxH*sizeW+ idxW);
                tH = m*(1 + k*(s/R-1));
		    	 convertBMPTo_mono_BMPLum_Next(img->pixPtr, biWidthInp, biWidthPadInp, biHeightInp,
		    			                    imgOut->pixPtr, biWidthOut, biWidthPadOut, biHeightOut,
		    			                    idxW, idxH, grid, gridW, gridH, tH);
			}
		}
//horizontal histogram to check if it is too dark
		{
	    	 int x1, x2, y1, y2;
	    	 int xC, yC;
	    	 int i, zeroCount, lowCount;
	    	 int maxZero, zeroFlag;
	    	 int *ptrH;


//	  	      sprintf(buff,"---------imgOut->bi.biWidth= %d, imgOut->bi.biHeight= %d\n",imgOut->bi.biWidth, imgOut->bi.biHeight);
//	   	      saveLog("", buff);

//	 		 if(imgOut->bi.biWidth > imgOut->bi.biHeight)
//	 		    rotate(imgOut, 90);
//	 		   biWidthOut=imgOut->bi.biWidth;
	 		   biWidthPadOut = ((imgOut->bi.biWidth+31)/32)*4;
	 		   biHeightOut = imgOut->bi.biHeight;


//	  	      sprintf(buff,"2---------imgOut->bi.biWidth= %d, imgOut->bi.biHeight= %d\n",imgOut->bi.biWidth, imgOut->bi.biHeight);
//	   	      saveLog("", buff);

	         xC = (imgOut->bi.biWidth*imgOut->bi.biBitCount)/2;
	         yC = imgOut->bi.biHeight/2;

 		     x1 = xC-xC/2;
 		     x2 = xC+xC/2;
 		     y1 = yC-(2*yC)/3;
 		     y2 = yC;
  	         histHTest_X1_X1(imgOut->pixPtr, biWidthPadOut, biHeightOut,  x1, x2, y1, y2, hHistPreProc);
  	         histVPtrPix(imgOut->pixPtr, biWidthPadOut, biHeightOut, H_SIZE_PREPROC, x1, x2, y1, y2, vHistPreProc);
  	         ptrH = hHistPreProc;
  	       	 if(imgOut->bi.biWidth > imgOut->bi.biHeight)
  	       	   ptrH = vHistPreProc;

  	         zeroCount = 0;
  	         lowCount = 0;
  	         zeroFlag = 0;
  	         maxZero = 0;
	         for(i=1;i<=ptrH[0];i++) {
//   	  	      sprintf(buff,"h---i= %d, ptrH[i]= %d\n",i, ptrH[i]);
//   	   	      saveLog("", buff);
	        	 if(ptrH[i] == 0){
	        		 zeroCount++;
	        		 zeroFlag = 1;
	        	 }
	        	 else{
	        		 if(zeroFlag == 1) {
	        		    zeroFlag = 0;
	        		    if(zeroCount > maxZero)
	        			    maxZero = zeroCount;
	        		    zeroCount = 0;
	        		 }
		        	 if(ptrH[i] < 10)
		        		 lowCount++;
	        	 }
	         }
//    	  	      sprintf(buff,"---------maxZero= %d, lowCount= %d\n",maxZero, lowCount);
//    	   	      saveLog("", buff);

//	   	  	      sprintf(buff,"biWidthPadOut= %d, imgOut->bi.biHeight= %d, x2-x1-1 = %d\n",biWidthPadOut, imgOut->bi.biHeight, x2-x1-1);
//	   	   	      saveLog("", buff);

//	   	  	      sprintf(buff,"x1= %d, x2= %d, y1 = %d, y2= %d\n",x1, x2, y1, y2);
//	   	   	      saveLog("", buff);



            if(lowCount > 50 && maxZero < 80){
// do again conversion changing threshold
            	k = 0.2;
        		for(idxH=0;idxH<sizeH;idxH++){
        			gridH = grid;
        			if(idxH == sizeH-1) gridH = lastH;
        		    for(idxW=0;idxW<sizeW;idxW++){
        		    	gridW = grid;
        		    	if(idxW == sizeW-1) gridW = lastW;
        		    	m = *(lpImgMap_m+idxH*sizeW+ idxW);
        		    	s = *(lpImgMap_s+idxH*sizeW+ idxW);
                        tH = m*(1 + k*(s/R-1));
        		    	 convertBMPTo_mono_BMPLum_Next(img->pixPtr, biWidthInp, biWidthPadInp, biHeightInp,
        		    			                    imgOut->pixPtr, biWidthOut, biWidthPadOut, biHeightOut,
        		    			                    idxW, idxH, grid, gridW, gridH, tH);
        			}
        		}


            }
		}

   	     if(imgOut->bi.biWidth > 1400)
     	       convertDPI_in_BMP(imgOut, 300, 300);
   	     else{
    	    	 imgOut->bi.biXPelsPerMeter=300;
    	    	 imgOut->bi.biYPelsPerMeter=300;
    	    	    bi = (LPBITMAPINFOHEADER)(img->ptr + sizeof (BITMAPFILEHEADER));
    	    		set4Bytes(bi->biXPelsPerMeter, imgOut->bi.biXPelsPerMeter);
    	    	    set4Bytes(bi->biYPelsPerMeter, imgOut->bi.biYPelsPerMeter);
    	     }
     }

     void setWhitePixels(LPBYTE ptr, int w, int h, int x1, int x2, int y1, int y2){
   	  int sww, i ,j, dy, sum, w_bt;
   	  int xStart, xStop;
   	  LONG nY;
   	  BYTE  *hptr;
//	  BYTE mask;

   	              w_bt=w;
   	              dy=y2-y1+1;
   	              i=0;
   	              xStart = x1/8;
   	              xStop =  x2/8;
   	              while(i<dy)  {
   	                sum=0;
   	                sww=y1+i;
   	                sww=h-1-sww;
   	                nY=(LONG)sww*(LONG)w_bt;
   	                for(j = xStart;j<=xStop;j++) {
   	                       hptr=ptr+nY+j;
                           *hptr = 255;
   	                }
   	                i++;
                   }

     }
     void setWhitePixelsX(LPBYTE ptr, int w, int h, int x1, int x2, int x12, int x22, int y1, int y2){
   	  int sww, i ,j, dy, sum, w_bt;
   	  int xStart, xStop;
   	  LONG nY;
   	  BYTE  *hptr;
//   	  char buff[500];

   	              w_bt=w;
   	              dy=y2-y1+1;
   	              i=0;
//   	              xStart = x1/8;
//   	              xStop =  x2/8;
   	              while(i<dy)  {
   	                sum=0;
   	                sww=y1+i;
   	                xStart = (i*(x12-x1))/(y2-y1) + x1;
   	                xStop =  (i*(x22-x2))/(y2-y1) + x2;
   	                xStart /= 8;
   	                xStop /= 8;
//  	      sprintf(buff,"xStart = %d, xStop= %d\n",xStart, xStop);
//  	      saveLog("", buff);

   	                sww=h-1-sww;
   	                nY=(LONG)sww*(LONG)w_bt;
   	                for(j = xStart;j<=xStop;j++) {
   	                       hptr=ptr+nY+j;
                           *hptr = 255;
   	                }
   	                i++;
                   }

     }
     int testCleanLeftSide(int av, int *hist){
    	 int c = 0;
    	 int i, k, cc;
    	 int tH = 30;
    	 int stop = hist[0]-3;
    	 k  = 0;
    	 for(i=1;i<stop;i++){
    		 if(hist[i] > av) k++;
    		 else break;
    	 }
    	 if(k < 10) return c;
    	 cc = 0;
    	 c = 0;
    	 for(i=k;i<stop;i++){
    		 if(hist[i] < av) {if(cc == 0) c = i; cc++;}
    		 else cc = 0;
    		 if(cc > tH) break;
    	 }
    	 if(cc > tH && c > k) return c;
    	 return 0;
     }
     int testCleanLeftSide2(int av, int *hist){
    	 int c = 0;
    	 int i, k, cc;
    	 int tH = 10;
    	 int stop = hist[0]-3;
    	 int av1;
//    	 char buff[500];

    	 av1 = 3*av;
    	 k  = 0;
    	 for(i=1;i<stop;i++){
    		 if(hist[i] < av1) k++;
    		 else break;
    	 }

// 	     sprintf(buff,"k = %d, av1 = %d\n",k, av1);
// 	     saveLog("", buff);

    	 if(k < 10) return c;
    	 cc = 0;
    	 c = 0;
    	 for(i=k;i<stop;i++){
    		 if(hist[i] > av1) {if(cc == 0) c = i; cc++;}
    		 else cc = 0;
    		 if(cc > tH) break;
    	 }
// 	     sprintf(buff,"c = %d, cc = %d\n",c, cc);
// 	     saveLog("", buff);

    	 if(cc > tH && c > k) return c;
    	 return 0;
     }
     int testCleanLeftSide_fromCenter(int av, int *hist){
    	 int c = 0;
    	 int i, k, cc;
    	 int tH = 1;
    	 int av1;
    	 int start = hist[0]-50;

    	 k  = start;
    	 av1 = 5*av;
    	 if(av1 < 300) av1 = 300;
    	 cc = 0;
    	 for(i=start;i>0;i--){
    		 if(hist[i] > av1) {if(cc == 0) k = i; cc++;}
    		 else cc = 0;
    		 if(cc >= tH) break;
    	 }
    	 if(cc < tH || k >= start) return 0;
         c = k;
    	 for(i=k;i<k+20;i++){
    		 if(hist[i] < av) {c = i; break;}
    	 }
    	 return c;
     }

     int adjustCleanEdge(int av, int *hist, int x){
    	 int i, k, min;
    	 int start = x-15, stop = x+15;
         k = x;
         min = hist[x];
         if(start < 1) start = 1;
         if(stop > hist[0]) stop = hist[0];
    	 for(i=start;i<stop;i++){
    		 if(hist[i] < min) {min = hist[i]; k = i;}
    	 }
    	 return k;
     }

     int testCleanRightSide_fromCenter(int av, int *hist){
    	 int c = 0;
    	 int i, k, cc;
    	 int tH = 1;
    	 int av1;
    	 int start = 50, stop;
//    	 char buff[500];

    	 k  = start;
    	 av1 = 5*av;
    	 if(av1 < 300) av1 = 300;
    	 cc = 0;
    	 for(i=start;i<hist[0]-3;i++){
    		 if(hist[i] > av1) {if(cc == 0) k = i; cc++;}
    		 else cc = 0;
    		 if(cc >= tH) break;
    	 }
// 	      sprintf(buff,"cc = %d, k= %d, av1= %d\n",cc, k, av1);
//   	      saveLog("", buff);

    	 if(cc < tH || k <= start) return 0;
         c = k;
         stop = k-20;
         if(stop < 1) stop = 1;
    	 for(i=k;i>stop;i--){
    		 if(hist[i] < av) {c = i; break;}
    	 }
    	 return c;
     }
     int testCleanTop(int av, int *hist){
    	 int c = 0;
    	 int i, k, cc;
    	 int tH = 1;
    	 int av1;
    	 int start = 1, stop, min;
//    	 char buff[500];

    	 k  = start;
    	 av1 = 5*av;
    	 cc = 0;
    	 stop = hist[0] - 10;
    	 for(i=start;i<stop;i++){
    		 if(hist[i] > av1) {if(cc == 0) k = i; cc++;}
    		 else cc = 0;
    		 if(cc >= tH) break;
    	 }

// 	      sprintf(buff,"cc = %d, k= %d, av1= %d\n",cc, k, av1);
//   	      saveLog("", buff);

    	 if(cc < tH || k <= start) return 0;
    	 stop = k+20;
         if(stop > hist[0]-3) stop = hist[0]-3;
         min = hist[k];
         c = k;
    	 for(i=k;i<stop;i++){
    		 if(hist[i] < min) {min=hist[i]; c = i;}
    	 }

    	 return c;
     }

     int testCleanBottom(int av, int *hist){
    	 int c = 0;
    	 int i, k, cc;
    	 int tH = 1;
    	 int av1;
    	 int start = hist[0], stop, min;
//    	 char buff[500];

    	 k  = start;
    	 av1 = 5*av;
    	 cc = 0;
    	 stop = hist[0]/2;
    	 for(i=start;i>stop;i--){
    		 if(hist[i] > av1) {if(cc == 0) k = i; cc++;}
    		 else cc = 0;
    		 if(cc >= tH) break;
    	 }

// 	      sprintf(buff,"cc = %d, k= %d, av1= %d\n",cc, k, av1);
//   	      saveLog("", buff);

    	 if(cc < tH || k >= start) return 0;
    	 stop = k-20;
         if(stop < 1) stop = 1;
         min = hist[k];
         c = k;
    	 for(i=k;i>stop;i--){
    		 if(hist[i] < min) {min=hist[i]; c = i;}
    	 }

    	 return c;
     }

     int testCleanRightSide(int av, int *hist){
    	 int c = 0;
    	 int i, k, cc;
    	 int tH = 30;
    	 int start = hist[0]-3;

    	 k  = 0;
    	 for(i=start;i>0;i--){
    		 if(hist[i] > av) k++;
    		 else break;
    	 }
    	 if(k < 10) return c;
    	 cc = 0;
    	 c = 0;
    	 k = hist[0] - k;
    	 for(i=k;i>0;i--){
    		 if(hist[i] < av) {if(cc == 0) c = i; cc++;}
    		 else cc = 0;
    		 if(cc > tH) break;
    	 }
    	 if(cc > tH && c < k) return c;
    	 return 0;
     }
     int testCleanRightSide2(int av, int *hist){
    	 int c = 0;
    	 int i, k, cc;
    	 int tH = 15;
    	 int av1;
    	 int start = hist[0]-3;

    	 k  = 0;
    	 av1 = av+av/2;
    	 for(i=start;i>0;i--){
    		 if(hist[i] < av) k++;
    		 else break;
    	 }
    	 if(k < 10) return c;
    	 cc = 0;
    	 c = 0;
    	 k = hist[0] - k;
    	 av1 = 2*av;
    	 for(i=k;i>0;i--){
    		 if(hist[i] > av) {if(cc == 0) c = i; cc++;}
    		 else cc = 0;
    		 if(cc > tH) break;
    	 }
    	 if(cc > tH && c < k) return c;
    	 return 0;
     }

     void cleanTicket(LPIMGDATA img){

    	 int i;
    	 int biWidthPad;
    	 int x1, x2, y1, y2;
//    	 char buff[500];
    	 long av = 0;
    	 int hAv;
    	 int clean1, clean2;
    	 int rest;
    	 int xC, yC;

   		 biWidthPad = ((img->bi.biWidth*img->bi.biBitCount+31)/32)*4;
   		 rest = biWidthPad*8-img->bi.biWidth*img->bi.biBitCount;
         xC = (img->bi.biWidth*img->bi.biBitCount)/2;
         yC = img->bi.biHeight/2;

// left side
   		 x1 = 8;
   		 x2 = xC;
   		 y1 = yC-yC/2;
   		 y2 = yC+yC/2;
   		 histVPtrPix(img->pixPtr, biWidthPad, img->bi.biHeight, x2-x1-1, x1, x2, y1, y2, hHistPreProc);
	     for(i=3;i<=hHistPreProc[0];i++) {
	    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
	    	 hHistPreProc[i-2] = hAv;
	     }
	     hHistPreProc[0] -= 2;
   	     for(i=1;i<=hHistPreProc[0];i++) {
              av += hHistPreProc[i];
   	     }
   	     av /= hHistPreProc[0];

         clean1 = testCleanLeftSide_fromCenter(av, hHistPreProc);

//   	     for(i=1;i<=hHistPreProc[0];i++) {
//   	  	      sprintf(buff,"%ld,%d\n",av, hHistPreProc[i]);
//   	   	      saveLog("", buff);
//   	       }
   		 x1 = 8;
   		 x2 = xC;
   		 y2 = yC-yC/2;
   		 y1 = y2-yC/4;
   		 histVPtrPix(img->pixPtr, biWidthPad, img->bi.biHeight, x2-x1-1, x1, x2, y1, y2, hHistPreProc);
	     for(i=3;i<=hHistPreProc[0];i++) {
	    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
	    	 hHistPreProc[i-2] = hAv;
	     }
	     hHistPreProc[0] -= 2;
   	     for(i=1;i<=hHistPreProc[0];i++) {
              av += hHistPreProc[i];
   	     }
   	     av /= hHistPreProc[0];
//   	     	     for(i=1;i<=hHistPreProc[0];i++) {
//   	     	  	      sprintf(buff,"%ld,%d\n",av, hHistPreProc[i]);
//   	     	   	      saveLog("", buff);
//   	     	       }
   	    clean2 = clean1;
        if(clean2 != 0)
           clean1 = adjustCleanEdge(av, hHistPreProc, clean1);
//   	   printf(" clean1= %d, adjusted clean1=%d \n", clean2, clean1);

 		 y1 = yC+yC/2;
 		 y2 = y1+yC/4;
 		 histVPtrPix(img->pixPtr, biWidthPad, img->bi.biHeight, x2-x1-1, x1, x2, y1, y2, hHistPreProc);
	     for(i=3;i<=hHistPreProc[0];i++) {
	    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
	    	 hHistPreProc[i-2] = hAv;
	     }
	     hHistPreProc[0] -= 2;
 	     for(i=1;i<=hHistPreProc[0];i++) {
            av += hHistPreProc[i];
 	     }
 	     av /= hHistPreProc[0];
// 	     	     for(i=1;i<=hHistPreProc[0];i++) {
// 	     	  	      sprintf(buff,"%ld,%d\n",av, hHistPreProc[i]);
// 	     	   	      saveLog("", buff);
// 	     	       }
      if(clean2 != 0)
          clean2 = adjustCleanEdge(av, hHistPreProc, clean2);
// 	   printf(" adjclean2= %d \n", clean2);

 	   if(clean1 != 0 ||clean2 != 0)
 	          setWhitePixelsX(img->pixPtr, biWidthPad, img->bi.biHeight, 0, clean1, 0, clean2, 0, img->bi.biHeight-1);

// right side
   		 x1 = xC;
   		 x2 = (img->bi.biWidth*img->bi.biBitCount)-rest;
   		 y1 = yC-yC/2;
   		 y2 = yC+yC/2;
   		 histVPtrPix(img->pixPtr, biWidthPad, img->bi.biHeight, x2-x1-1, x1, x2, y1, y2, hHistPreProc);
	     for(i=3;i<=hHistPreProc[0];i++) {
	    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
	    	 hHistPreProc[i-2] = hAv;
	     }
	     hHistPreProc[0] -= 2;
   	     for(i=1;i<=hHistPreProc[0];i++) {
              av += hHistPreProc[i];
   	     }
   	     av /= hHistPreProc[0];

//   	     for(i=1;i<=hHistPreProc[0];i++) {
//   	  	      sprintf(buff,"%ld,%d\n",av, hHistPreProc[i]);
//   	   	      saveLog("", buff);
//   	       }
   	     clean1 = testCleanRightSide_fromCenter(av, hHistPreProc);


   		 y2 = yC-yC/2;
   		 y1 = y2-yC/4;
   		 histVPtrPix(img->pixPtr, biWidthPad, img->bi.biHeight, x2-x1-1, x1, x2, y1, y2, hHistPreProc);
	     for(i=3;i<=hHistPreProc[0];i++) {
	    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
	    	 hHistPreProc[i-2] = hAv;
	     }
	     hHistPreProc[0] -= 2;
   	     for(i=1;i<=hHistPreProc[0];i++) {
              av += hHistPreProc[i];
   	     }
   	     av /= hHistPreProc[0];
   	     clean2 = clean1;
         if(clean2 != 0)
           clean1 = adjustCleanEdge(av, hHistPreProc, clean1);

//   	   printf(" clean1= %d, adjusted clean1=%d \n", clean2, clean1);

 		 y1 = yC+yC/2;
 		 y2 = y1+yC/4;
 		 histVPtrPix(img->pixPtr, biWidthPad, img->bi.biHeight, x2-x1-1, x1, x2, y1, y2, hHistPreProc);
	     for(i=3;i<=hHistPreProc[0];i++) {
	    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
	    	 hHistPreProc[i-2] = hAv;
	     }
	     hHistPreProc[0] -= 2;
 	     for(i=1;i<=hHistPreProc[0];i++) {
            av += hHistPreProc[i];
 	     }
 	     av /= hHistPreProc[0];
         if(clean2 != 0)
           clean2 = adjustCleanEdge(av, hHistPreProc, clean2);
// 	   printf(" adjclean2= %d \n", clean2);

 	   if(clean1 != 0 ||clean2 != 0) {
 		    clean1 += x1;
 		  	clean2 += x1;
            setWhitePixelsX(img->pixPtr, biWidthPad, img->bi.biHeight, clean1, img->bi.biWidth-1, clean2, img->bi.biWidth-1, 0, img->bi.biHeight-1);
 	   }


 // top
 	     		 x1 = 0;
 	     		 x2 = xC;
 	     		 y1 = 1;
 	     		 y2 = yC/2;
 	      	     histHTest_X1_X1(img->pixPtr, biWidthPad, img->bi.biHeight,  x1, x2, y1, y2, hHistPreProc);
 	  	     for(i=3;i<=hHistPreProc[0];i++) {
 	  	    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
 	  	    	 hHistPreProc[i-2] = hAv;
 	  	     }
 	  	     hHistPreProc[0] -= 2;
 	     	     for(i=1;i<=hHistPreProc[0];i++) {
 	                av += hHistPreProc[i];
 	     	     }
 	     	     av /= hHistPreProc[0];

 	           clean1 = testCleanTop(av, hHistPreProc);

// 	     	     for(i=1;i<=hHistPreProc[0];i++) {
// 	     	  	      sprintf(buff,"%ld,%d\n",av, hHistPreProc[i]);
// 	     	   	      saveLog("", buff);
// 	     	       }
 	   	   if(clean1 != 0)
 	   	        setWhitePixels(img->pixPtr, biWidthPad, img->bi.biHeight, 0, img->bi.biWidth-1, 0, clean1+20);
/*
// bottom
 	   	 	     		 x1 = xC-xC/2;
 	   	 	     		 x2 = xC+xC/2;
 	   	 	     		 y1 = yC;
 	   	 	     		 y2 = img->bi.biHeight-1;
 	   	 	      	     histHTest_X1_X1(img->pixPtr, biWidthPad, img->bi.biHeight,  x1, x2, y1, y2, hHistPreProc);
 	   	 	  	     for(i=3;i<=hHistPreProc[0];i++) {
 	   	 	  	    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
 	   	 	  	    	 hHistPreProc[i-2] = hAv;
 	   	 	  	     }
 	   	 	  	     hHistPreProc[0] -= 2;
 	   	 	     	     for(i=1;i<=hHistPreProc[0];i++) {
 	   	 	                av += hHistPreProc[i];
 	   	 	     	     }
 	   	 	     	     av /= hHistPreProc[0];

 	   	 	           clean1 = testCleanBottom(av, hHistPreProc);

 	   	// 	     	     for(i=1;i<=hHistPreProc[0];i++) {
 	   	// 	     	  	      sprintf(buff,"%ld,%d\n",av, hHistPreProc[i]);
 	   	// 	     	   	      saveLog("", buff);
 	   	// 	     	       }
 	   	 	   	   if(clean1 != 0) {
 	   	 	   		    clean1 += x1;
 	   	 	   	        setWhitePixels(img->pixPtr, biWidthPad, img->bi.biHeight, 0, img->bi.biWidth-1, clean1, img->bi.biHeight-1);
 	   	 	   	   }
*/
 	   	// 	   	          setWhitePixelsY(img->pixPtr, biWidthPad, img->bi.biHeight, 0, img->bi.biWidth, 0, clean1, 0, clean2);


     }

     int getPixeSmearing(LPBYTE ptr, int xn, int yn, int wPad, int height){
     	    long nY;
     	    long nX;
     	    BYTE mask;
     	    LPBYTE ptrP;
     	    BYTE b;

     	        yn=height-1-yn;
     	        nY=yn*wPad;
       	        nX = (long)(xn/8);
                 ptrP = ptr+nY+nX;

     	         mask=LEFT_BIT;
     	         mask >>=(xn%8);

     	         b=(~(*ptrP))&mask;
     	         if(b!=0) {
     	               return(0);
     	               }
     	         else {
     	               return(1);
     	               }
     }


     int getValueForSmearing(LPBYTE imgPtr, int idxW, int idxH, int biWidthPad, int biHeight, int gridx, int gridy, int gridW, int gridH){
        int  sum = 0;
        int xna, yna;
        int iW, iH, xn, yn;
//        LPBYTE ptr;
//        int max;
     //  char buff[500];

         yna = idxH*gridy;
         xna = idxW*gridx;
     	for(iH=0;iH<gridH;iH++){
     		yn =yna+iH;
     		for(iW=0;iW<gridW;iW++){
    			xn = xna+iW;
     			sum +=   getPixeSmearing(imgPtr, xn, yn, biWidthPad, biHeight);
     		}
     	}
        if(sum > (5*(gridH*gridW))/6)
            return 255;
        return 0;
     }

/*
    void setPixelsOut(LPBYTE ptr, int w, int h, int xinp, int yinp, int v){
      	  int  w_bt;
      	  int x, y;
      	  int nY;
      	  BYTE  *hptr;
  	      BYTE mask;

            w_bt=w;
            x = xinp/8;
            y=h-1-yinp;
            nY=y*w_bt;
            hptr=ptr+nY+x;

	        mask=LEFT_BIT;
	        mask >>=(x%8);
            if(v!=0){
	        	 *hptr = *hptr | mask;
	         }
	         else {
	        	 *hptr = *hptr & (~mask);
	         }
    }
*/
    void setPixelsOut(LPBYTE ptr, int w, int h, int x1, int x2, int y1, int y2, int v){
  	  int sww, i ,j, dy, sum, w_bt;
  	  int xStart, xStop;
  	  LONG nY;
  	  BYTE  *hptr;

  	              w_bt=w;
  	              dy=y2-y1+1;
  	              i=0;
  	              xStart = x1/8;
  	              xStop =  x2/8;
  	              while(i<dy)  {
  	                sum=0;
  	                sww=y1+i;
  	                sww=h-1-sww;
  	                nY=(LONG)sww*(LONG)w_bt;
  	                for(j = xStart;j<=xStop;j++) {
  	                       hptr=ptr+nY+j;
                          *hptr = v;
  	                }
  	                i++;
                  }

    }
    int AverageValueOfHistogram(int *hHistPreProc, int start, int dd){
    	int i, av, cc, stop;
    	stop = start-dd;
    	if(stop< 1) stop = 1;
    	av = 0;
    	cc = 0;
    	for(i=start;i>stop;i--){
             av += hHistPreProc[i];
             cc++;
    	}
    	if(cc > 0)
    		av /= cc;
        return av;
    }


    int testSmearingRightOld(int *hHistPreProc, int av){
    	int clean = -1;
    	int i, a, k, n, cc;
//    	char buff[500];
    	int  th = (5*av)/6;

//    sprintf(buff,"th = %d\n",th);
//      saveLog("", buff);

    	k = -1;
    	for(i = hHistPreProc[0]-12;i>3; i--){
    		a = (hHistPreProc[i]+hHistPreProc[i-1]+hHistPreProc[i-2])/3;
    		if(a < th) {k = i;break;}
    	}

// 	    sprintf(buff,"k = %d\n",k);
//      saveLog("", buff);

    	if (k > 100){
    		cc = 0;
    		n = -1;
        	for(i = k;i>k-30; i--){
        		a = (hHistPreProc[i]+hHistPreProc[i-1]+hHistPreProc[i-2])/3;
        		if(a < 10) cc++;
        		else cc = 0;
        		if(cc > 10) {n = i;break;}
        	}
//    	    sprintf(buff,"n = %d\n",n);
//   	      saveLog("", buff);

        	if(n > 100) clean = n+10;
    	}

    	return clean;
    }
    int testSmearingLeftOld(int *hHistPreProc, int av){
    	int clean = -1;
    	int i, a, k, n, cc;
//    	char buff[500];

    	k = -1;
    	for(i = 10;i<hHistPreProc[0]-100; i++){
    		a = (hHistPreProc[i]+hHistPreProc[i+1]+hHistPreProc[i+2])/3;
    		if(a < av) {k = i;break;}
    	}

// 	    sprintf(buff,"k = %d\n",k);
//      saveLog("", buff);

    	if (k > 10){
    		cc = 0;
    		n = -1;
        	for(i = k;i<k+30; i++){
        		a = (hHistPreProc[i]+hHistPreProc[i+1]+hHistPreProc[i+2])/3;
        		if(a < 10) cc++;
        		else cc = 0;
        		if(cc > 10) {n = i;break;}
        	}
//    	    sprintf(buff,"n = %d\n",n);
//   	      saveLog("", buff);

        	if(n > 10) clean = n+10;
    	}

    	return clean;
    }
    int testSmearingRight(int *hHistPreProc, int av){
    	int clean = -1;
    	int dd = 8;
    	int a1, a2, max, maxn;
   	    int i, k, n, i1, i2;
   	    int thRatio = 10;
    	int ratio[500];
    	int stop = hHistPreProc[0]/2;
    	int count = 0;
//    	char buff[500];

//    	sprintf(buff," av= %d\n", av);
//    	saveLog("", buff);
   	   for(i = hHistPreProc[0]- dd;i>hHistPreProc[0]- 3*dd; i--){
   		 a1 = AverageValueOfHistogram(hHistPreProc, i, dd);
   		if(a1<av/5) count++;
   	   }
        if(count > 10)
     	   return clean;

       k = 0;
  	   for(i = hHistPreProc[0]- dd;i>stop; i--){
  		   a1 = AverageValueOfHistogram(hHistPreProc, i, dd);
  		   a2 = AverageValueOfHistogram(hHistPreProc, i-dd, dd);
  		   ratio[k] = 0;
  		   if(a2 != 0)
  			 ratio[k] = a1/a2;
//        sprintf(buff," i= %d ,a1= %d, a2= %d, ratio = %d\n",i, a1, a2, ratio[k]);
//         saveLog("", buff);
  		   k++;
  	  }


//        for(i=0;i<k;i++){
// 	        sprintf(buff," i= %d, n= %d, ratio = %d\n",i, hHistPreProc[0]- dd-i-dd, ratio[i]);
//            saveLog("", buff);
//        }
        for(i=0;i<k;i++){
        	if(ratio[i] > thRatio){
        		i1 = i-dd/2; if(i1 < 0) i1 = 0;
        		i2 = i+dd; if(i2 >k) i2 = k;
        		max = 0;maxn = 0;
        		for(n = i1;n<i2;n++){
        			if(ratio[n] > max) {max = ratio[n];maxn = n;}
        		}
//     	        sprintf(buff,"  n= %d, max = %d\n", maxn, max);
//                saveLog("", buff);

        		if(max > thRatio) {
        			clean = hHistPreProc[0]- dd-maxn-dd;
        			break;
        		}
        	}
        }

    	return clean;
    }

    int testSmearingLeft(int *hHistPreProc, int av){
    	int clean = -1;
    	int dd = 8;
    	int a1, a2, max, maxn;
   	    int i, k, n, i1, i2;
   	    int thRatio = 6;
    	int ratio[500];
    	int stop = hHistPreProc[0]/2;
    	int count = 0;
//    	char buff[500];


//		 sprintf(buff," av/5= %d\n",av/5);
//  		 saveLog("", buff);

   	   for(i = dd+5;i< 3*dd+5; i++){
   		 a1 = AverageValueOfHistogram(hHistPreProc, i, dd);
   		if(a1<av/5) count++;
   	   }
        if(count > 10)
     	   return clean;

        k = 0;
    	for(i = dd+5;i<stop; i++){
    		a1 = AverageValueOfHistogram(hHistPreProc, i, dd);
    		a2 = AverageValueOfHistogram(hHistPreProc, i+dd, dd);
   		    ratio[k] = 0;
      	    		if(a2 != 0)
      			 ratio[k] = a1/a2;

//    		 sprintf(buff," i= %d ,a1= %d, a2= %d, ratio = %d\n",i, a1, a2, ratio[k]);
//    		 saveLog("", buff);

            k++;
    	}
//    	     for(i=0;i<k;i++){
//    	       sprintf(buff," i= %d, n= %d, ratio = %d\n",i,dd+5+i, ratio[i]);
//    	        saveLog("", buff);
//    	     }

        for(i=0;i<k;i++){
        	if(ratio[i] > thRatio){
        		i1 = i-dd/2; if(i1 < 0) i1 = 0;
        		i2 = i+dd; if(i2 >k) i2 = k;
        		max = 0;maxn = 0;
        		for(n = i1;n<i2;n++){
        			if(ratio[n] > max) {max = ratio[n];maxn = n;}
        		}
//     	        sprintf(buff,"  n= %d, max = %d\n", maxn, max);
//                saveLog("", buff);

        		if(max > thRatio) {
        			clean = dd+5+maxn;
        			break;
        		}
        	}
        }


    	return clean;
    }

    int testSmearingBottom(int *hHistPreProc, int av){
    	int clean = -1;
    	int dd = 8;
    	int a1, a2, max, maxn;
   	    int i, k, n, i1, i2;
   	    int thRatio = 10;
    	int ratio[500];
    	int stop = hHistPreProc[0]/2;
    	int count = 0;
//    	char buff[500];

//    	sprintf(buff," av= %d\n", av);
//    	saveLog("", buff);


  	   for(i = hHistPreProc[0]- dd;i>hHistPreProc[0]- 3*dd; i--){
  		 a1 = AverageValueOfHistogram(hHistPreProc, i, dd);
  		if(a1<av/5) count++;
  	   }
       if(count > 10)
    	   return clean;

       k = 0;
  	   for(i = hHistPreProc[0]- dd;i>stop; i--){
  		   a1 = AverageValueOfHistogram(hHistPreProc, i, dd);
  		   a2 = AverageValueOfHistogram(hHistPreProc, i-dd, dd);
  		   ratio[k] = 0;
  		   if(a2 != 0)
  			 ratio[k] = a1/a2;
//        sprintf(buff," i= %d ,a1= %d, a2= %d, ratio = %d \n",i, a1, a2, ratio[k] );
//         saveLog("", buff);
  		   k++;
  	  }


//        for(i=0;i<k;i++){
// 	        sprintf(buff," i= %d, n= %d, ratio = %d\n",i, hHistPreProc[0]- dd-i-dd, ratio[i]);
//            saveLog("", buff);
//        }

        for(i=0;i<k;i++){
        	if(ratio[i] > thRatio){
        		i1 = i-dd/2; if(i1 < 0) i1 = 0;
        		i2 = i+dd; if(i2 >k) i2 = k;
        		max = 0;maxn = 0;
        		for(n = i1;n<i2;n++){
        			if(ratio[n] > max) {max = ratio[n];maxn = n;}
        		}
//     	        sprintf(buff,"  n= %d, max = %d\n", maxn, max);
//                saveLog("", buff);

        		if(max > thRatio) {
        			clean = hHistPreProc[0]- dd-maxn-dd;
        			break;
        		}
        	}
        }

    	return clean;
    }


  void smearing(LPIMGDATA imgInp, LPIMGDATA imgOut){
	  int ddSmearingx = 16;
	  int ddSmearingy = 16;

    	int sizeW, sizeH, lastW, lastH;
    	int idxW, idxH;
    	int gridW, gridH;
    	int m;
    	int biWidthPad;
	    LPBITMAPFILEHEADER bf;
	    LPBITMAPINFOHEADER bi;
	    LPRGBQUAD          qd;
	//    int           sizeOut;
	    int offBits = sizeof (BITMAPFILEHEADER) +
	              sizeof (BITMAPINFOHEADER) +
	              sizeof (RGBQUAD) * 2;
	    LPBYTE ptrImgOut, ptrBuffOut;



	   biWidthPad = ((imgInp->bi.biWidth*imgInp->bi.biBitCount+31)/32)*4;

	  	sizeW = imgInp->bi.biWidth/ddSmearingx;
	  	lastW = imgInp->bi.biWidth - sizeW*ddSmearingx;
	  	if(lastW > 0)
	  	   sizeW++;
	  	sizeH = imgInp->bi.biHeight/ddSmearingy;
	  	lastH = imgInp->bi.biHeight - sizeH*ddSmearingy;
	  	if(lastH > 0)
	  	   sizeH++;

//	    sizeOut = offBits + sizeWbyte * sizeH;

	    ptrImgOut = calloc(imgInp->bf.bfSize, sizeof(BYTE));
	    if(ptrImgOut == NULL) {
	    	printf("calloc failed, ptrImgOut");
	    	return;
	    }
        bf = (LPBITMAPFILEHEADER) ptrImgOut;
        bf->bfType[0] = 0x42;
        bf->bfType[1] = 0x4d;
	    set4Bytes(bf->bfSize, imgInp->bf.bfSize);
	    bf->bfReserved1[0] = 0;
	    bf->bfReserved1[1] = 0;
        set4Bytes(bf->bfOffBits, offBits);

        bi = (LPBITMAPINFOHEADER)((LPBYTE)bf + sizeof (BITMAPFILEHEADER));
	    set4Bytes(bi->biSize, sizeof (BITMAPINFOHEADER));
        set4Bytes(bi->biWidth, imgInp->bi.biWidth);
	    set4Bytes(bi->biHeight, imgInp->bi.biHeight);
	     bi->biPlanes[0] = 1;
	     bi->biPlanes[1] = 0;
	     bi->biBitCount[0]= 1;
	     bi->biBitCount[1]= 0;
	    set4Bytes(bi->biCompression, BI_RGB);
		set4Bytes(bi->biSizeImage, 0);
		set4Bytes(bi->biXPelsPerMeter, imgInp->bi.biXPelsPerMeter);
	    set4Bytes(bi->biYPelsPerMeter, imgInp->bi.biYPelsPerMeter);
		set4Bytes(bi->biClrUsed, 0);
		set4Bytes(bi->biClrImportant, 0);

        qd = (LPRGBQUAD)((LPBYTE)bi + sizeof (BITMAPINFOHEADER));
        *(LPDWORD)qd       = 0x00000000;
        *(LPDWORD)(qd + 1) = 0x00ffffff;
        ptrBuffOut = ptrImgOut+ offBits;

	  	for(idxH=0;idxH<sizeH;idxH++){
	  		gridH = ddSmearingy;
	  		if(idxH == sizeH-1) gridH = lastH;
	  	    for(idxW=0;idxW<sizeW;idxW++){
	  	    	gridW = ddSmearingx;
	  	    	if(idxW == sizeW-1) gridW = lastW;

	  	         m = getValueForSmearing(imgInp->pixPtr, idxW, idxH, biWidthPad, imgInp->bi.biHeight, ddSmearingx, ddSmearingy, gridW, gridH);

	  		     setPixelsOut(ptrBuffOut,  biWidthPad, imgInp->bi.biHeight, idxW*ddSmearingx, idxW*ddSmearingx + gridW,
	  		    		 idxH*ddSmearingy, idxH*ddSmearingy+gridH-1, m);
	  		     }

	  		}

// 	    saveBMPbuffer (ptrImgOut, "smearing.bmp", imgInp->szOutPath);

	  	{
	    	 int x1, x2, y1, y2;
//	    	 char buff[500];
	    	 long av = 0;
	    	 int hAv;
	    	 int clean1;
	    	 int rest;
	    	 int xC, yC;
	    	 int i;

	   		    rest = biWidthPad*8-imgInp->bi.biWidth*imgInp->bi.biBitCount;
	            xC = (imgInp->bi.biWidth*imgInp->bi.biBitCount)/2;
	            yC = imgInp->bi.biHeight/2;

  	// left side
	 	     		 x1 = 8;
	 	     		 x2 = xC;
	 	  	   		 y1 = yC-(2*yC)/3;
	 	  	   	     y2 = yC+(2*yC)/3;
	 	  	   		 histVPtrPix(imgInp->pixPtr, biWidthPad, imgInp->bi.biHeight, x2-x1-1, x1, x2, y1, y2, hHistPreProc);

	 	  		     for(i=3;i<=hHistPreProc[0];i++) {
	 	  		    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
	 	  		    	 hHistPreProc[i-2] = hAv;
	 	  		     }
	 	  		     hHistPreProc[0] -= 2;
	 	  	   	     for(i=1;i<=hHistPreProc[0];i++) {
	 	  	              av += hHistPreProc[i];
	 	  	   	     }
	 	  	   	     av /= hHistPreProc[0];

//	  	   	     for(i=1;i<=hHistPreProc[0];i++) {
//	  	   	  	      sprintf(buff,"%d, %ld, %d\n",i, av, hHistPreProc[i]);
//	   	   	   	      saveLog("", buff);
//	  	   	       }



	  	   	        clean1 = testSmearingLeft(hHistPreProc, av);
//	 	  	   	    sprintf(buff,"clean1 = %d\n",clean1);
//	 	  	   	  	      saveLog("", buff);

	 	  	 	   if(clean1 > 0) {
	 	  	          setWhitePixels(imgInp->pixPtr, biWidthPad, imgInp->bi.biHeight, 0, clean1, 0, imgInp->bi.biHeight-1);
	 	  	 	   }


	  	// right side

	  	   		 x1 = xC;
	  	   		 x2 = (imgInp->bi.biWidth*imgInp->bi.biBitCount)-rest;
	  	   		 y1 = yC-(2*yC)/3;
	  	   		 y2 = yC+(2*yC)/3;
	  	   		 histVPtrPix(imgInp->pixPtr, biWidthPad, imgInp->bi.biHeight, x2-x1-1, x1, x2, y1, y2, hHistPreProc);

	  		     for(i=3;i<=hHistPreProc[0];i++) {
	  		    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
	  		    	 hHistPreProc[i-2] = hAv;
	  		     }
	  		     hHistPreProc[0] -= 2;
	  	   	     for(i=1;i<=hHistPreProc[0];i++) {
	  	              av += hHistPreProc[i];
	  	   	     }
	  	   	     av /= hHistPreProc[0];


// 	   	     for(i=1;i<=hHistPreProc[0];i++) {
// 	   	  	      sprintf(buff,"%d, %ld, %d\n",i, av, hHistPreProc[i]);
//  	   	   	      saveLog("", buff);
// 	   	       }


	  	   	     clean1 = testSmearingRight(hHistPreProc, av);
//	  	   	    sprintf(buff,"clean1 = %d\n",clean1);
//	  	   	  	      saveLog("", buff);

	  	 	   if(clean1 > 0) {
	  	 		    clean1 += x1;
//	  	            setWhitePixelsX(imgInp->pixPtr, biWidthPad, imgInp->bi.biHeight, clean1, imgInp->bi.biWidth-1, clean2, img->bi.biWidth-1, 0, imgInp->bi.biHeight-1);
	  	          setWhitePixels(imgInp->pixPtr, biWidthPad, imgInp->bi.biHeight, clean1, imgInp->bi.biWidth-1, 0, imgInp->bi.biHeight-1);
	  	 	   }


//	  	   	     for(i=1;i<=hHistPreProc[0];i++) {
//	  	   	  	      sprintf(buff,"%ld,%d\n",av, hHistPreProc[i]);
//	  	   	   	      saveLog("", buff);
//	  	   	       }


 	// bottom
/*


	  		  	   	     x1 = xC-(2*xC)/3;
	  		  	   	     x2 = xC+(2*xC)/3;
	  		  	   		 x2 = (imgInp->bi.biWidth*imgInp->bi.biBitCount)-rest;
	  		  	   		 y1 = yC;
	  		  	   		 y2 = imgInp->bi.biHeight-1;
	  		  	   	     histHTest_X1_X1(imgInp->pixPtr, biWidthPad, imgInp->bi.biHeight,  x1, x2, y1, y2, hHistPreProc);

	  		  		     for(i=3;i<=hHistPreProc[0];i++) {
	  		  		    	 hAv = ((hHistPreProc[i] +hHistPreProc[i-1]+hHistPreProc[i-2])/3);
	  		  		    	 hHistPreProc[i-2] = hAv;
	  		  		     }
	  		  		     hHistPreProc[0] -= 2;
	  		  	   	     for(i=1;i<=hHistPreProc[0];i++) {
	  		  	              av += hHistPreProc[i];
	  		  	   	     }
	  		  	   	     av /= hHistPreProc[0];


 	 	   	        for(i=1;i<=hHistPreProc[0];i++) {
	  	 	   	  	      sprintf(buff,"%d, %ld, %d\n",i, av, hHistPreProc[i]);
	  	  	   	   	      saveLog("", buff);
	  	 	   	       }


	  		  	   	     clean1 = testSmearingBottom(hHistPreProc, av);
	  		  	   	    sprintf(buff,"clean1 = %d\n",clean1);
	  		  	   	  	      saveLog("", buff);

	  		  	 	   if(clean1 > 0) {
	  		  	 		    clean1 += y1;
	  		  	          setWhitePixels(imgInp->pixPtr, biWidthPad, imgInp->bi.biHeight, 0, imgInp->bi.biWidth-1, clean1, imgInp->bi.biHeight-1);
	  		  	 	   }


	  	//	  	   	     for(i=1;i<=hHistPreProc[0];i++) {
	  	//	  	   	  	      sprintf(buff,"%ld,%d\n",av, hHistPreProc[i]);
	  	//	  	   	   	      saveLog("", buff);
	  	//	  	   	       }

*/
	  	}



 	    free(ptrImgOut);

  	}

//-------------------------------------------------------------------------------------------------------------------------------
// photoshop sharpening image ---------------------------------------------------------------------------------------------------

  void setColor24(LPIMGDATA img, int xn, int yn, BYTE R, BYTE G, BYTE B){
      int  w;
      long nY;
      long nX;
      int sh = 3;
      LPBYTE ptr;

          w=img->bi.biWidthPad;
          yn=img->bi.biHeight-1-yn;
          nY=yn*w;
     	    nX = (long)xn*sh;
          ptr=img->pixPtr+nY+nX;
          *ptr = B;
          *(ptr+1) = G;
          *(ptr+2) = R;
  }
  void getColorFromImge24(LPIMGDATA img, int offset, LPBYTE R, LPBYTE G, LPBYTE B){
      int  w;
      long nY;
      long nX;
      int sh = 3;
      LPBYTE ptr;
      int xn,  yn;
//      char buff[500];

          xn = offset%img->bi.biWidth;
          yn = offset/img->bi.biWidth;

//   	sprintf(buff," xn= %d, yn= %d\n", xn, yn);
// 	saveLog("-------", buff);


          w=img->bi.biWidthPad;
          yn=img->bi.biHeight-1-yn;
          nY=yn*w;
     	    nX = (long)xn*sh;
          ptr=img->pixPtr+nY+nX;
          *B=*ptr;
          *G=*(ptr+1);
          *R=*(ptr+2);
  }
  void setColorFromImge24(LPIMGDATA img, int offset, BYTE R, BYTE G, BYTE B){
      int  w;
      long nY;
      long nX;
      int sh = 3;
      LPBYTE ptr;
      int xn,  yn;
          xn = offset%img->bi.biWidth;
          yn = offset/img->bi.biWidth;

          w=img->bi.biWidthPad;
          yn=img->bi.biHeight-1-yn;
          nY=yn*w;
     	    nX = (long)xn*sh;
          ptr=img->pixPtr+nY+nX;
          *ptr= B;
          *(ptr+1) = G;
          *(ptr+2) = R;
  }

#define FORC(cnt) for (c=0; c < cnt; c++)
#define FORC3 FORC(3)

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define LIM(x,min,max) MAX(min,MIN(x,max))
#define CLIP(x) LIM(x,0,65535)

   float (*lab)[3];
   double ep,ka;
   double xyz_rgb_2[3][3];
   float d50_white[3]={0.964220,1,0.825211};
   double rgb_xyz[3][3] =
        { { 0.7976748465, 0.1351917082, 0.0313534088 },
           { 0.2880402025, 0.7118741325, 0.0000856651 },
           { 0.0000000000, 0.0000000000, 0.8252114389 } };

   void  pseudoinverse (double (*in)[3], double (*out)[3], int size){
     double work[3][6], num;
     int i, j, k;

     for (i=0; i < 3; i++) {
       for (j=0; j < 6; j++)
         work[i][j] = j == i+3;
       for (j=0; j < 3; j++)
         for (k=0; k < size; k++)
       work[i][j] += in[k][i] * in[k][j];
     }
     for (i=0; i < 3; i++) {
       num = work[i][i];
       for (j=0; j < 6; j++)
         work[i][j] /= num;
       for (k=0; k < 3; k++) {
         if (k==i) continue;
         num = work[k][i];
         for (j=0; j < 6; j++)
           work[k][j] -= work[i][j] * num;
      }
     }
     for (i=0; i < size; i++)
       for (j=0; j < 3; j++)
         for (out[i][j]=k=0; k < 3; k++)
           out[i][j] += work[j][k+3] * in[i][k];
   }

   double f_cbrt(double r){
       r/=65535.0;
       return r>ep?pow(r,1/3.0):(ka*r+16)/116.0;
   }
   void  CIELab_Init(int width, int height){
       ep=216.0/24389.0;
       ka=24389.0/27.0;
       pseudoinverse(rgb_xyz,xyz_rgb_2,3);

       lab=(float (*)[3])calloc(width*height,sizeof *lab);
   }

   void  CIELab_End(){
       if(lab) free(lab);
   }
  void  RGB_to_CIELab(LPIMGDATA img){
      int offset,c;
      double xyz[3];
      int width, height;
      BYTE r, g, b;

	  width  = img->bi.biWidth;
	  height = img->bi.biHeight;

      for(offset=0;offset<width*height;offset++){
         // Convert RGB to XYZ
          xyz[0]=xyz[1]=xyz[2]=0;
          getColorFromImge24( img, offset, &r, &g, &b);
          FORC3{
              xyz[c]+=rgb_xyz[c][0]*(double)r;
              xyz[c]+=rgb_xyz[c][1]*(double)g;
              xyz[c]+=rgb_xyz[c][2]*(double)b;
          }

          // Convert XYZ to L*a*b*
          FORC3 xyz[c]=f_cbrt(xyz[c]/d50_white[c]);

          lab[offset][0]=116.0*xyz[1]-16.0;
          lab[offset][1]=500.0*(xyz[0]-xyz[1]);;
          lab[offset][2]=200.0*(xyz[1]-xyz[2]);
      }
  }

  void  CIELab_to_RGB(LPIMGDATA img){
      int offset,c;
      double L;
      double xyz[3],rgb[3],f[3];
      int width, height;

	  width  = img->bi.biWidth;
	  height = img->bi.biHeight;

      for(offset=0;offset<width*height;offset++){
          // Convert L*a*b* to XYZ
          L=(double)lab[offset][0];
          f[1]=(L+16.0)/116.0;    // fy
          f[0]=f[1]+(double)lab[offset][1]/500.0; // fx
          f[2]=f[1]-(double)lab[offset][2]/200.0; // fz

          xyz[0]=65535.0*d50_white[0]*(f[0]*f[0]*f[0]>ep?f[0]*f[0]*f[0]:(116.0*f[0]-16.0)/ka);
          xyz[1]=65535.0*d50_white[1]*(L>ka*ep?pow(f[1],3.0):L/ka);
          xyz[2]=65535.0*d50_white[2]*(f[2]*f[2]*f[2]>ep?f[2]*f[2]*f[2]:(116.0*f[2]-16.0)/ka);

          // Convert XYZ to RGB
          rgb[0]=rgb[1]=rgb[2]=0;
          FORC3{
              rgb[0]+=xyz[c]*xyz_rgb_2[c][0];
              rgb[1]+=xyz[c]*xyz_rgb_2[c][1];
              rgb[2]+=xyz[c]*xyz_rgb_2[c][2];
          }
          setColorFromImge24( img, offset, CLIP(rgb[0]), CLIP(rgb[1]), CLIP(rgb[2]));
//          FORC3 image[offset][c]=(ushort)CLIP(rgb[c]);
      }
  }



  void testImage(LPIMGDATA img){
      int offset;
      int width, height;
      BYTE r, g, b;
//      char buff[500];

	  width  = img->bi.biWidth;
	  height = img->bi.biHeight;

      for(offset=0;offset<width*height;offset++){
//	sprintf(buff," offset= %d, width= %d, height= %d\n",offset, width, height);
//	saveLog("-------", buff);

          getColorFromImge24( img, offset, &r, &g, &b);
          setColorFromImge24( img, offset, r, g, b);
      }

  }

  void sharpenImage(LPIMGDATA img,  int channels, int passes, float strength){
	      int offset,c,i,j,p,width2;
	      float *L,lumH,lumV,lumD1,lumD2,v,contrast,s;
	      float difL,difR,difT,difB,difLT,difRB,difLB,difRT,wH,wV,wD1,wD2,chmax[3];
	      float f1,f2,f3,f4;
	      int width, height;

	      width  = img->bi.biWidth;
		  height = img->bi.biHeight;

	      width2=2*width;

	      CIELab_Init(width, height);
		  RGB_to_CIELab(img);

	      L=(float *)calloc(width*height,sizeof(*L));
	      chmax[0]=8.0;
	      chmax[1]=3.0;
	      chmax[2]=3.0;

	      for(c=0;c<channels;c++)
//	      c = 2;
	          for(p=0;p<passes;p++){
	              for(offset=0;offset<width*height;offset++)
	                  L[offset]=lab[offset][c];

	              for(j=2;j<height-2;j++)
	                  for(i=2,offset=j*width+i;i<width-2;i++,offset++){
  // weight functions
	                      wH=fabs(L[offset+1]-L[offset-1]);
	                      wV=fabs(L[offset+width]-L[offset-width]);

	                      s=1.0+fabs(wH-wV)/2.0;
	                      wD1=fabs(L[offset+width+1]-L[offset-width-1])/s;
	                      wD2=fabs(L[offset+width-1]-L[offset-width+1])/s;
	                      s=wD1;
	                      wD1/=wD2;
	                      wD2/=wD1;

 // initial values
	                      lumH=lumV=lumD1=lumD2=v=lab[offset][c];

// contrast detection
	                      contrast=sqrt(fabs(L[offset+1]-L[offset-1])*fabs(L[offset+1]-L[offset-1])+fabs(L[offset+width]-L[offset-width])*fabs(L[offset+width]-L[offset-width]))/chmax[c];
	                      if(contrast>1.0) contrast=1.0;

// new possible values
	                      if(((L[offset]<L[offset-1]) && (L[offset]>L[offset+1])) ||
                    		        ((L[offset]>L[offset-1])&&(L[offset]<L[offset+1]))){
	                         f1=fabs(L[offset-2]-L[offset-1]);
	                         f2=fabs(L[offset-1]-L[offset]);
	                         f3=fabs(L[offset-1]-L[offset-width])*fabs(L[offset-1]-L[offset+width]);
	                         f4=sqrt(fabs(L[offset-1]-L[offset-width2])*fabs(L[offset-1]-L[offset+width2]));
	                         difL=f1*f2*f2*f3*f3*f4;
	                         f1=fabs(L[offset+2]-L[offset+1]);
	                         f2=fabs(L[offset+1]-L[offset]);
	                         f3=fabs(L[offset+1]-L[offset-width])*fabs(L[offset+1]-L[offset+width]);
	                         f4=sqrt(fabs(L[offset+1]-L[offset-width2])*fabs(L[offset+1]-L[offset+width2]));
	                         difR=f1*f2*f2*f3*f3*f4;
	                         if((difR!=0)&&(difL!=0)){
	                             lumH=(L[offset-1]*difR+L[offset+1]*difL)/(difL+difR);
	                              lumH=v*(1-contrast)+lumH*contrast;
	                         }
	                     }
	                      if(((L[offset]<L[offset-width]) && (L[offset]>L[offset+width]))
			                           || ((L[offset]>L[offset-width])&&(L[offset]<L[offset+width]))){
	                          f1=fabs(L[offset-width2]-L[offset-width]);
	                          f2=fabs(L[offset-width]-L[offset]);
	                          f3=fabs(L[offset-width]-L[offset-1])*fabs(L[offset-width]-L[offset+1]);
	                          f4=sqrt(fabs(L[offset-width]-L[offset-2])*fabs(L[offset-width]-L[offset+2]));
	                          difT=f1*f2*f2*f3*f3*f4;
	                          f1=fabs(L[offset+width2]-L[offset+width]);
	                          f2=fabs(L[offset+width]-L[offset]);
	                          f3=fabs(L[offset+width]-L[offset-1])*fabs(L[offset+width]-L[offset+1]);
	                          f4=sqrt(fabs(L[offset+width]-L[offset-2])*fabs(L[offset+width]-L[offset+2]));
	                          difB=f1*f2*f2*f3*f3*f4;
	                          if((difB!=0)&&(difT!=0)){
	                              lumV=(L[offset-width]*difB+L[offset+width]*difT)/(difT+difB);
	                              lumV=v*(1-contrast)+lumV*contrast;
	                          }
	                      }

	                      if(((L[offset]<L[offset-1-width]) && (L[offset]>L[offset+1+width])) ||
	                    		  ((L[offset]>L[offset-1-width])&&(L[offset]<L[offset+1+width]))){
	                          f1=fabs(L[offset-2-width2]-L[offset-1-width]);
	                          f2=fabs(L[offset-1-width]-L[offset]);
	                          f3=fabs(L[offset-1-width]-L[offset-width+1])*fabs(L[offset-1-width]-L[offset+width-1]);
	                          f4=sqrt(fabs(L[offset-1-width]-L[offset-width2+2])*fabs(L[offset-1-width]-L[offset+width2-2]));
	                          difLT=f1*f2*f2*f3*f3*f4;
	                          f1=fabs(L[offset+2+width2]-L[offset+1+width]);
	                          f2=fabs(L[offset+1+width]-L[offset]);
	                          f3=fabs(L[offset+1+width]-L[offset-width+1])*fabs(L[offset+1+width]-L[offset+width-1]);
	                          f4=sqrt(fabs(L[offset+1+width]-L[offset-width2+2])*fabs(L[offset+1+width]-L[offset+width2-2]));
	                          difRB=f1*f2*f2*f3*f3*f4;
	                          if((difLT!=0)&&(difRB!=0)){
	                              lumD1=(L[offset-1-width]*difRB+L[offset+1+width]*difLT)/(difLT+difRB);
	                              lumD1=v*(1-contrast)+lumD1*contrast;
	                          }
	                      }

	                      if(((L[offset]<L[offset+1-width]) && (L[offset]>L[offset-1+width])) ||
	                    		  ((L[offset]>L[offset+1-width])&&(L[offset]<L[offset-1+width]))){
	                          f1=fabs(L[offset-2+width2]-L[offset-1+width]);
	                          f2=fabs(L[offset-1+width]-L[offset]);
	                          f3=fabs(L[offset-1+width]-L[offset-width-1])*fabs(L[offset-1+width]-L[offset+width+1]);
	                          f4=sqrt(fabs(L[offset-1+width]-L[offset-width2-2])*fabs(L[offset-1+width]-L[offset+width2+2]));
	                          difLB=f1*f2*f2*f3*f3*f4;
	                          f1=fabs(L[offset+2-width2]-L[offset+1-width]);
	                          f2=fabs(L[offset+1-width]-L[offset])*fabs(L[offset+1-width]-L[offset]);
	                          f3=fabs(L[offset+1-width]-L[offset+width+1])*fabs(L[offset+1-width]-L[offset-width-1]);
	                          f4=sqrt(fabs(L[offset+1-width]-L[offset+width2+2])*fabs(L[offset+1-width]-L[offset-width2-2]));
	                          difRT=f1*f2*f2*f3*f3*f4;
	                          if((difLB!=0)&&(difRT!=0)){
	                              lumD2=(L[offset+1-width]*difLB+L[offset-1+width]*difRT)/(difLB+difRT);
	                              lumD2=v*(1-contrast)+lumD2*contrast;
	                          }
	                      }

	                      s=strength;

// avoid sharpening diagonals too much
	                      if(((fabs(wH/wV)<0.45)&&(fabs(wH/wV)>0.05))||((fabs(wV/wH)<0.45)&&(fabs(wV/wH)>0.05)))
	                          s=strength/3.0;

// final mix
	                      if((wH!=0)&&(wV!=0)&&(wD1!=0)&&(wD2!=0))
	                          lab[offset][c]=v*(1-s)+(lumH*wH+lumV*wV+lumD1*wD1+lumD2*wD2)/(wH+wV+wD1+wD2)*s;
	                  }
	          }

        CIELab_to_RGB(img);
        CIELab_End();
	    free(L);
  }





void microcontrast(LPIMGDATA img, float strength){
    int offset,offset2,c,i,j,col,row,n;
    float *L,v,s,contrast;
    int signs[9];
    int width, height;

    width  = img->bi.biWidth;
    height = img->bi.biHeight;

//    width2=2*width;

    CIELab_Init(width, height);
	 RGB_to_CIELab(img);


        L=(float *)calloc(width*height,sizeof *L);
       c=0;

    for(offset=0;offset<width*height;offset++)
        L[offset]=lab[offset][c];

    for(j=1;j<height-1;j++)
        for(i=1,offset=j*width+i;i<width-1;i++,offset++){
            s=strength;
            v=L[offset];

            n=0;
            for(row=j-1;row<=j+1;row++)
                for(col=i-1,offset2=row*width+col;col<=i+1;col++,offset2++){
                    signs[n]=0;
                    if(v<L[offset2]) signs[n]=-1;
                    if(v>L[offset2]) signs[n]=1;
                    n++;
                }

            contrast=sqrt(fabs(L[offset+1]-L[offset-1])*fabs(L[offset+1]-L[offset-1])+fabs(L[offset+width]-L[offset-width])*fabs(L[offset+width]-L[offset-width]))/8.0;
            if(contrast>1.0) contrast=1.0;

            lab[offset][c]+=(v-L[offset-width-1])*sqrt(2)*s;
            lab[offset][c]+=(v-L[offset-width])*s;
            lab[offset][c]+=(v-L[offset-width+1])*sqrt(2)*s;

            lab[offset][c]+=(v-L[offset-1])*s;
            lab[offset][c]+=(v-L[offset+1])*s;

            lab[offset][c]+=(v-L[offset+width-1])*sqrt(2)*s;
            lab[offset][c]+=(v-L[offset+width])*s;
            lab[offset][c]+=(v-L[offset+width+1])*sqrt(2)*s;

            if(lab[offset][c]<0) lab[offset][c]=0;

            // Reduce halo looking artifacs
            v=lab[offset][c];
            n=0;
            for(row=j-1;row<=j+1;row++)
                for(col=i-1,offset2=row*width+col;col<=i+1;col++,offset2++){
                    if(((v<L[offset2])&&(signs[n]>0))||((v>L[offset2])&&(signs[n]<0)))
                        lab[offset][c]=v*0.75+L[offset2]*0.25;
                    n++;
                }
            lab[offset][c]=lab[offset][c]*(1-contrast)+L[offset]*contrast;
        }

    CIELab_to_RGB(img);
    CIELab_End();

    free(L);
}

/*
01.// To the extent possible under law, Manuel Llorens <manuelllorens@gmail.com>
02.// has waived all copyright and related or neighboring rights to this work.
03.// This code is licensed under CC0 v1.0, see license information at
04.// http://creativecommons.org/publicdomain/zero/1.0/
05.
06.void microcontrast(float strength){
07.    int offset,offset2,c,i,j,col,row,n;
08.    float *L,v,s,contrast;
09.    int signs[9];
10.
11.    if(verbose)
12.                 printf("Increasing microcontrast...n");
13.
14.    CIELab_Init();
15.    RGB_to_CIELab();
16.
17.        L=(float *)calloc(width*height,sizeof *L);
18.    merror(L,"microcontrast()");
19.
20.    c=0;
21.    #pragma omp parallel for private(offset) shared(L)
22.    for(offset=0;offset<width*height;offset++)
23.        L[offset]=lab[offset][c];
24.
25.    #pragma omp parallel for private(j,i,offset,s,signs,v,n,row,col,offset2,contrast) shared(lab)
26.    for(j=1;j<height-1;j++)
27.        for(i=1,offset=j*width+i;i<width-1;i++,offset++){
28.            s=strength;
29.            v=L[offset];
30.
31.            n=0;
32.            for(row=j-1;row<=j+1;row++)
33.                for(col=i-1,offset2=row*width+col;col<=i+1;col++,offset2++){
34.                    signs[n]=0;
35.                    if(v<L[offset2]) signs[n]=-1;
36.                    if(v>L[offset2]) signs[n]=1;
37.                    n++;
38.                }
39.
40.            contrast=sqrt(fabs(L[offset+1]-L[offset-1])*fabs(L[offset+1]-L[offset-1])+fabs(L[offset+width]-L[offset-width])*fabs(L[offset+width]-L[offset-width]))/8.0;
41.            if(contrast>1.0) contrast=1.0;
42.
43.            lab[offset][c]+=(v-L[offset-width-1])*sqrt(2)*s;
44.            lab[offset][c]+=(v-L[offset-width])*s;
45.            lab[offset][c]+=(v-L[offset-width+1])*sqrt(2)*s;
46.
47.            lab[offset][c]+=(v-L[offset-1])*s;
48.            lab[offset][c]+=(v-L[offset+1])*s;
49.
50.            lab[offset][c]+=(v-L[offset+width-1])*sqrt(2)*s;
51.            lab[offset][c]+=(v-L[offset+width])*s;
52.            lab[offset][c]+=(v-L[offset+width+1])*sqrt(2)*s;
53.
54.            if(lab[offset][c]<0) lab[offset][c]=0;
55.
56.            // Reduce halo looking artifacs
57.            v=lab[offset][c];
58.            n=0;
59.            for(row=j-1;row<=j+1;row++)
60.                for(col=i-1,offset2=row*width+col;col<=i+1;col++,offset2++){
61.                    if(((v<L[offset2])&&(signs[n]>0))||((v>L[offset2])&&(signs[n]<0)))
62.                        lab[offset][c]=v*0.75+L[offset2]*0.25;
63.                    n++;
64.                }
65.            lab[offset][c]=lab[offset][c]*(1-contrast)+L[offset]*contrast;
66.        }
67.
68.    CIELab_to_RGB();
69.    CIELab_End();
70.
71.    free(L);
72.}


  view plaincopy to clipboardprint?
  01.// To the extent possible under law, Manuel Llorens <manuelllorens@gmail.com>
  02.// has waived all copyright and related or neighboring rights to this work.
  03.// This code is licensed under CC0 v1.0, see license information at
  04.// http://creativecommons.org/publicdomain/zero/1.0/
  05.
  06.float (*lab)[3];
  07.double ep,ka;
  08.double xyz_rgb_2[3][3];
  09.float d50_white[3]={0.964220,1,0.825211};
  10.static const double rgb_xyz[3][3] =
  11.      { { 0.7976748465, 0.1351917082, 0.0313534088 },
  12.        { 0.2880402025, 0.7118741325, 0.0000856651 },
  13.        { 0.0000000000, 0.0000000000, 0.8252114389 } }; // From Jacques Desmis
  14.
  15.double f_cbrt(double r){
  16.    r/=65535.0;
  17.    return r>ep?pow(r,1/3.0):(ka*r+16)/116.0;
  18.}
  19.
  20.void CLASS CIELab_Init(){
  21.    int i;
  22.
  23.    ep=216.0/24389.0;
  24.    ka=24389.0/27.0;
  25.    pseudoinverse(rgb_xyz,xyz_rgb_2,3);
  26.
  27.    lab=(float (*)[3])calloc(width*height,sizeof *lab);
  28.    merror(lab,"CIELab_Init()");
  29.}
  30.
  31.void CLASS CIELab_End(){
  32.    if(lab) free(lab);
  33.}
  34.
  35.void CLASS RGB_to_CIELab(){
  36.    int offset,c;
  37.    double xyz[3];
  38.
  39.    #pragma omp parallel for private(offset,xyz,c) shared(lab)
  40.    for(offset=0;offset<width*height;offset++){
  41.        // Convert RGB to XYZ
  42.        xyz[0]=xyz[1]=xyz[2]=0;
  43.        FORC3{
  44.            xyz[c]+=rgb_xyz[c][0]*(double)image[offset][0];
  45.            xyz[c]+=rgb_xyz[c][1]*(double)image[offset][1];
  46.            xyz[c]+=rgb_xyz[c][2]*(double)image[offset][2];
  47.        }
  48.
  49.        // Convert XYZ to L*a*b*
  50.        FORC3 xyz[c]=f_cbrt(xyz[c]/d50_white[c]);
  51.
  52.        lab[offset][0]=116.0*xyz[1]-16.0;
  53.        lab[offset][1]=500.0*(xyz[0]-xyz[1]);;
  54.        lab[offset][2]=200.0*(xyz[1]-xyz[2]);
  55.    }
  56.}
  57.
  58.void CLASS CIELab_to_RGB(){
  59.    int offset,c;
  60.    double L;
  61.    double xyz[3],rgb[3],f[3];
  62.
  63.    #pragma omp parallel for private(offset,xyz,c,f,L,rgb) shared(image)
  64.    for(offset=0;offset<width*height;offset++){
  65.        // Convert L*a*b* to XYZ
  66.                L=(double)lab[offset][0];
  67.        f[1]=(L+16.0)/116.0;    // fy
  68.        f[0]=f[1]+(double)lab[offset][1]/500.0; // fx
  69.        f[2]=f[1]-(double)lab[offset][2]/200.0; // fz
  70.
  71.        xyz[0]=65535.0*d50_white[0]*(f[0]*f[0]*f[0]>ep?f[0]*f[0]*f[0]:(116.0*f[0]-16.0)/ka);
  72.        xyz[1]=65535.0*d50_white[1]*(L>ka*ep?pow(f[1],3.0):L/ka);
  73.        xyz[2]=65535.0*d50_white[2]*(f[2]*f[2]*f[2]>ep?f[2]*f[2]*f[2]:(116.0*f[2]-16.0)/ka);
  74.
  75.        // Convert XYZ to RGB
  76.        rgb[0]=rgb[1]=rgb[2]=0;
  77.        FORC3{
  78.            rgb[0]+=xyz[c]*xyz_rgb_2[c][0];
  79.            rgb[1]+=xyz[c]*xyz_rgb_2[c][1];
  80.            rgb[2]+=xyz[c]*xyz_rgb_2[c][2];
  81.        }
  82.        FORC3 image[offset][c]=(ushort)CLIP(rgb[c]);
  83.    }
  84.}

  Uses one function and some macros from dcraw code by David Coffin:

  view plaincopy to clipboardprint?
  01.#define FORC(cnt) for (c=0; c < cnt; c++)
  02.#define FORC3 FORC(3)
  03.
  04.#define MIN(a,b) ((a) < (b) ? (a) : (b))
  05.#define MAX(a,b) ((a) > (b) ? (a) : (b))
  06.#define LIM(x,min,max) MAX(min,MIN(x,max))
  07.#define CLIP(x) LIM(x,0,65535)
  08.
  09.void CLASS pseudoinverse (double (*in)[3], double (*out)[3], int size)
  10.{
  11.  double work[3][6], num;
  12.  int i, j, k;
  13.
  14.  for (i=0; i < 3; i++) {
  15.    for (j=0; j < 6; j++)
  16.      work[i][j] = j == i+3;
  17.    for (j=0; j < 3; j++)
  18.      for (k=0; k < size; k++)
  19.    work[i][j] += in[k][i] * in[k][j];
  20.  }
  21.  for (i=0; i < 3; i++) {
  22.    num = work[i][i];
  23.    for (j=0; j < 6; j++)
  24.      work[i][j] /= num;
  25.    for (k=0; k < 3; k++) {
  26.      if (k==i) continue;
  27.      num = work[k][i];
  28.      for (j=0; j < 6; j++)
  29.    work[k][j] -= work[i][j] * num;
  30.    }
  31.  }
  32.  for (i=0; i < size; i++)
  33.    for (j=0; j < 3; j++)
  34.      for (out[i][j]=k=0; k < 3; k++)
  35.    out[i][j] += work[j][k+3] * in[i][k];
  36.}



01.// To the extent possible under law, Manuel Llorens <manuelllorens@gmail.com>
02.// has waived all copyright and related or neighboring rights to this work.
03.// This code is licensed under CC0 v1.0, see license information at
04.// http://creativecommons.org/publicdomain/zero/1.0/
05.
06.void sharpen(int channels, int passes, float strength){
07.    int offset,c,i,j,p,width2;
08.    float *L,lumH,lumV,lumD1,lumD2,v,contrast,med,s;
09.    float difL,difR,difT,difB,difLT,difRB,difLB,difRT,wH,wV,wD1,wD2,chmax[3];
10.    float f1,f2,f3,f4;
11.
12.    if(verbose)
13.        printf("Sharpening...n");
14.
15.    width2=2*width;
16.
17.    CIELab_Init();
18.    RGB_to_CIELab();
19.
20.    L=(float *)calloc(width*height,sizeof *L);
21.    merror(L,"sharpen()");
22.    chmax[0]=8.0;
23.    chmax[1]=3.0;
24.    chmax[2]=3.0;
25.
26.    for(c=0;c<=channels;c++)
27.        for(p=0;p<passes;p++){
28.            #pragma omp parallel for private(offset) shared(L)
29.            for(offset=0;offset<width*height;offset++)
30.                L[offset]=lab[offset][c];
31.
32.            #pragma omp parallel for private(j,i,offset,wH,wV,wD1,wD2,s,lumH,lumV,lumD1,lumD2,v,contrast,f1,f2,f3,f4,difT,difB,difL,difR,difLT,difLB,difRT,difRB) shared(lab)
33.            for(j=2;j<height-2;j++)
34.                for(i=2,offset=j*width+i;i<width-2;i++,offset++){
35.                    // weight functions
36.                    wH=fabs(L[offset+1]-L[offset-1]);
37.                    wV=fabs(L[offset+width]-L[offset-width]);
38.
39.                    s=1.0+fabs(wH-wV)/2.0;
40.                    wD1=fabs(L[offset+width+1]-L[offset-width-1])/s;
41.                    wD2=fabs(L[offset+width-1]-L[offset-width+1])/s;
42.                    s=wD1;
43.                    wD1/=wD2;
44.                    wD2/=wD1;
45.
46.                    // initial values
47.                    lumH=lumV=lumD1=lumD2=v=lab[offset][c];
48.
49.                    // contrast detection
50.                    contrast=sqrt(fabs(L[offset+1]-L[offset-1])*fabs(L[offset+1]-L[offset-1])+fabs(L[offset+width]-L[offset-width])*fabs(L[offset+width]-L[offset-width]))/chmax[c];
51.                    if(contrast>1.0) contrast=1.0;
52.
53.                    // new possible values
54.                    if((L[offset]<L[offset-1])&&(L[offset]>L[offset+1])||(L[offset]>L[offset-1])&&(L[offset]<L[offset+1])){
55.                        f1=fabs(L[offset-2]-L[offset-1]);
56.                        f2=fabs(L[offset-1]-L[offset]);
57.                        f3=fabs(L[offset-1]-L[offset-width])*fabs(L[offset-1]-L[offset+width]);
58.                        f4=sqrt(fabs(L[offset-1]-L[offset-width2])*fabs(L[offset-1]-L[offset+width2]));
59.                        difL=f1*f2*f2*f3*f3*f4;
60.                        f1=fabs(L[offset+2]-L[offset+1]);
61.                        f2=fabs(L[offset+1]-L[offset]);
62.                        f3=fabs(L[offset+1]-L[offset-width])*fabs(L[offset+1]-L[offset+width]);
63.                        f4=sqrt(fabs(L[offset+1]-L[offset-width2])*fabs(L[offset+1]-L[offset+width2]));
64.                        difR=f1*f2*f2*f3*f3*f4;
65.                        if((difR!=0)&&(difL!=0)){
66.                            lumH=(L[offset-1]*difR+L[offset+1]*difL)/(difL+difR);
67.                            lumH=v*(1-contrast)+lumH*contrast;
68.                        }
69.                    }
70.
71.                    if((L[offset]<L[offset-width])&&(L[offset]>L[offset+width])||(L[offset]>L[offset-width])&&(L[offset]<L[offset+width])){
72.                        f1=fabs(L[offset-width2]-L[offset-width]);
73.                        f2=fabs(L[offset-width]-L[offset]);
74.                        f3=fabs(L[offset-width]-L[offset-1])*fabs(L[offset-width]-L[offset+1]);
75.                        f4=sqrt(fabs(L[offset-width]-L[offset-2])*fabs(L[offset-width]-L[offset+2]));
76.                        difT=f1*f2*f2*f3*f3*f4;
77.                        f1=fabs(L[offset+width2]-L[offset+width]);
78.                        f2=fabs(L[offset+width]-L[offset]);
79.                        f3=fabs(L[offset+width]-L[offset-1])*fabs(L[offset+width]-L[offset+1]);
80.                        f4=sqrt(fabs(L[offset+width]-L[offset-2])*fabs(L[offset+width]-L[offset+2]));
81.                        difB=f1*f2*f2*f3*f3*f4;
82.                        if((difB!=0)&&(difT!=0)){
83.                            lumV=(L[offset-width]*difB+L[offset+width]*difT)/(difT+difB);
84.                            lumV=v*(1-contrast)+lumV*contrast;
85.                        }
86.                    }
87.
88.                    if((L[offset]<L[offset-1-width])&&(L[offset]>L[offset+1+width])||(L[offset]>L[offset-1-width])&&(L[offset]<L[offset+1+width])){
89.                        f1=fabs(L[offset-2-width2]-L[offset-1-width]);
90.                        f2=fabs(L[offset-1-width]-L[offset]);
91.                        f3=fabs(L[offset-1-width]-L[offset-width+1])*fabs(L[offset-1-width]-L[offset+width-1]);
92.                        f4=sqrt(fabs(L[offset-1-width]-L[offset-width2+2])*fabs(L[offset-1-width]-L[offset+width2-2]));
93.                        difLT=f1*f2*f2*f3*f3*f4;
94.                        f1=fabs(L[offset+2+width2]-L[offset+1+width]);
95.                        f2=fabs(L[offset+1+width]-L[offset]);
96.                        f3=fabs(L[offset+1+width]-L[offset-width+1])*fabs(L[offset+1+width]-L[offset+width-1]);
97.                        f4=sqrt(fabs(L[offset+1+width]-L[offset-width2+2])*fabs(L[offset+1+width]-L[offset+width2-2]));
98.                        difRB=f1*f2*f2*f3*f3*f4;
99.                        if((difLT!=0)&&(difRB!=0)){
100.                            lumD1=(L[offset-1-width]*difRB+L[offset+1+width]*difLT)/(difLT+difRB);
101.                            lumD1=v*(1-contrast)+lumD1*contrast;
102.                        }
103.                    }
104.
105.                    if((L[offset]<L[offset+1-width])&&(L[offset]>L[offset-1+width])||(L[offset]>L[offset+1-width])&&(L[offset]<L[offset-1+width])){
106.                        f1=fabs(L[offset-2+width2]-L[offset-1+width]);
107.                        f2=fabs(L[offset-1+width]-L[offset]);
108.                        f3=fabs(L[offset-1+width]-L[offset-width-1])*fabs(L[offset-1+width]-L[offset+width+1]);
109.                        f4=sqrt(fabs(L[offset-1+width]-L[offset-width2-2])*fabs(L[offset-1+width]-L[offset+width2+2]));
110.                        difLB=f1*f2*f2*f3*f3*f4;
111.                        f1=fabs(L[offset+2-width2]-L[offset+1-width]);
112.                        f2=fabs(L[offset+1-width]-L[offset])*fabs(L[offset+1-width]-L[offset]);
113.                        f3=fabs(L[offset+1-width]-L[offset+width+1])*fabs(L[offset+1-width]-L[offset-width-1]);
114.                        f4=sqrt(fabs(L[offset+1-width]-L[offset+width2+2])*fabs(L[offset+1-width]-L[offset-width2-2]));
115.                        difRT=f1*f2*f2*f3*f3*f4;
116.                        if((difLB!=0)&&(difRT!=0)){
117.                            lumD2=(L[offset+1-width]*difLB+L[offset-1+width]*difRT)/(difLB+difRT);
118.                            lumD2=v*(1-contrast)+lumD2*contrast;
119.                        }
120.                    }
121.
122.                    s=strength;
123.
124.                    // avoid sharpening diagonals too much
125.                    if(((fabs(wH/wV)<0.45)&&(fabs(wH/wV)>0.05))||((fabs(wV/wH)<0.45)&&(fabs(wV/wH)>0.05)))
126.                        s=strength/3.0;
127.
128.                    // final mix
129.                    if((wH!=0)&&(wV!=0)&&(wD1!=0)&&(wD2!=0))
130.                        lab[offset][c]=v*(1-s)+(lumH*wH+lumV*wV+lumD1*wD1+lumD2*wD2)/(wH+wV+wD1+wD2)*s;
131.                }
132.        }
133.
134.    CIELab_to_RGB();
135.    CIELab_End();
136.
137.    free(L);
138.}


*/


  void createEdge(LPIMGDATA img,  int channels){
	      int offset,c;
	      int width, height;
	      float *L;
	      width  = img->bi.biWidth;
		  height = img->bi.biHeight;

//	      width2=2*width;

	      CIELab_Init(width, height);
		  RGB_to_CIELab(img);

	      L=(float *)calloc(width*height,sizeof(*L));
//	      chmax[0]=8.0;
//	      chmax[1]=3.0;
//	      chmax[2]=3.0;

	      for(c=0;c<channels;c++) {
//c = 2;
	              for(offset=0;offset<width*height;offset++)
	                  L[offset]= sqrt(fabs(lab[offset+1][c]-lab[offset-1][c])*fabs(lab[offset+1][c]-lab[offset-1][c])+
	                		  fabs(lab[offset+width][c]-lab[offset-width][c])*fabs(lab[offset+width][c]-lab[offset-width][c]));

	              for(offset=0;offset<width*height;offset++)
	            	  lab[offset][c]= L[offset];

	      }



        CIELab_to_RGB(img);
        CIELab_End();
	    free(L);
  }

void calculateGradient(LPBYTE ptr, LPIMGDATA imgInp){
	      int x, y;
	      int width, height, wPad;
//          BYTE v0, vX1, vX_1, vY1, vY_1, vOut;
          BYTE v0, vX1, vY1, vOut;

	      width  = imgInp->bi.biWidth;
		  height = imgInp->bi.biHeight;
		  wPad = ((width+31)/32)*4;

         for(y=1;y<height-1;y++){
             for(x=1;x<width-1;x++){
            	 v0 = getPixelCompress(imgInp->pixPtr, x, y, wPad, height);
            	 vX1 = getPixelCompress(imgInp->pixPtr, x+1, y, wPad, height);
//            	 vX_1 = getPixelCompress(imgInp->pixPtr, x-1, y, wPad, height);
            	 vY1 = getPixelCompress(imgInp->pixPtr, x, y+1, wPad, height);
//            	 vY_1 = getPixelCompress(imgInp->pixPtr, x, y-1, wPad, height);
//            	 vOut = (vX1 -v0)*(vX1 -v0) + (vY1 -v0)*(vY1 -v0) +(vX_1 -v0)*(vX_1 -v0) + (vY_1 -v0)*(vY_1 -v0);
              	 vOut = (vX1 -v0)*(vX1 -v0) + (vY1 -v0)*(vY1 -v0);
            	 setPixelDecompress(ptr,  wPad, height, x, y, vOut);
             }
         }
         y=0;
         for(x=0;x<width;x++){
           	 v0 = getPixelCompress(imgInp->pixPtr, x, y, wPad, height);
           	 setPixelDecompress(ptr,  wPad, height, x, y, v0);
           }
         y=height-1;
         for(x=0;x<width;x++){
           	 v0 = getPixelCompress(imgInp->pixPtr, x, y, wPad, height);
           	 setPixelDecompress(ptr,  wPad, height, x, y, v0);
           }
         for(y=0;y<height;y++){
        	 x = 0;
           	 v0 = getPixelCompress(imgInp->pixPtr, x, y, wPad, height);
           	 setPixelDecompress(ptr,  wPad, height, x, y, v0);
        	 x = width-1;
           	 v0 = getPixelCompress(imgInp->pixPtr, x, y, wPad, height);
           	 setPixelDecompress(ptr,  wPad, height, x, y, v0);
         }


}

void createEdgeMonochrom(LPIMGDATA imgInp, LPIMGDATA img){
   LPBYTE ptrOut = NULL;
   LPBYTE ptrT = NULL;
   int bufforSize;
   int imgWidth, imgHeight, biWidthPad;

   LPRGBQUAD          qd;

   LPBITMAPFILEHEADER bf;
   LPBITMAPINFOHEADER bi;
   int offBits = sizeof (BITMAPFILEHEADER) +
             sizeof (BITMAPINFOHEADER)
            + sizeof (RGBQUAD) * 2;

   imgWidth = imgInp->bi.biWidth;
   imgHeight = imgInp->bi.biHeight;
   biWidthPad = ((imgWidth+31)/32)*4;
   bufforSize = biWidthPad*imgHeight+offBits;

   ptrOut = calloc(bufforSize, sizeof(BYTE));
   if(ptrOut == NULL) {
    	    printf(img->error,"deCompressImage ( \"%s\" ) calloc ( %d ) failed: %s\n",img->szIMGName, bufforSize, strerror(errno));
    	    return;
     }
   bf = (LPBITMAPFILEHEADER) ptrOut;
   bf->bfType[0] = 0x42;
   bf->bfType[1] = 0x4d;
   set4Bytes(bf->bfSize, bufforSize);
	  bf->bfReserved1[0] = 0;
	  bf->bfReserved1[1] = 0;
   set4Bytes(bf->bfOffBits, offBits);

   bi = (LPBITMAPINFOHEADER)((LPBYTE)bf + sizeof (BITMAPFILEHEADER));
   set4Bytes(bi->biSize, sizeof (BITMAPINFOHEADER));
   set4Bytes(bi->biWidth, imgWidth);
   set4Bytes(bi->biHeight, imgHeight);
   bi->biPlanes[0] = 1;
	  bi->biPlanes[1] = 0;
	  bi->biBitCount[0]= 1;
	  bi->biBitCount[1]= 0;
   set4Bytes(bi->biCompression, BI_RGB);
	  set4Bytes(bi->biSizeImage, bufforSize - offBits);
	  set4Bytes(bi->biXPelsPerMeter, imgInp->bi.biXPelsPerMeter);
	  set4Bytes(bi->biYPelsPerMeter, imgInp->bi.biYPelsPerMeter);
	  set4Bytes(bi->biClrUsed, 0);
	  set4Bytes(bi->biClrImportant, 0);

   qd = (LPRGBQUAD)((LPBYTE)bi + sizeof (BITMAPINFOHEADER));
   *(LPDWORD)qd       = 0x00000000;
   *(LPDWORD)(qd + 1) = 0x00ffffff;

	  img->ptr = ptrOut;
   img->bf.bfType = 0x4d42;
   img->bf.bfSize = bufforSize;
	  img->bf.bfReserved1 =  0;
	  img->bf.bfReserved2 =  0;
	  img->bf.bfOffBits =   offBits;
	  img->pixPtr = ptrOut +img->bf.bfOffBits;

   img->bi.biSize = offBits - 14;
	  img->width = imgWidth;
	  img->bi.biWidth = img->width;
	  img->height = imgHeight;
	  img->bi.biHeight = img->height;
	  img->bi.biPlanes = 1;
	  img->bi.biBitCount = 1;
	  img->bi.biCompression = 0;
	  img->bi.biSizeImage = bufforSize - offBits;
	  img->bi.biXPelsPerMeter = imgInp->bi.biXPelsPerMeter;
	  img->bi.biYPelsPerMeter = imgInp->bi.biYPelsPerMeter;
   img->bi.biClrUsed = 0;
	  img->bi.biClrImportant = 0;
	  img->bi.biWidthPad = ((img->bi.biWidth*img->bi.biBitCount+31)/32)*4;

     ptrT = ptrOut+offBits;
     calculateGradient(ptrT, imgInp);
}

void addEdgeMonochrom(LPIMGDATA img, LPIMGDATA imgEdge){

    int x, y;
    int width, height, wPad;
    BYTE v0, v1;

    width  = img->bi.biWidth;
    height = img->bi.biHeight;
    wPad = ((width+31)/32)*4;

   for(y=0;y<height;y++){
       for(x=0;x<width;x++){
      	 v0 = getPixelCompress(img->pixPtr, x, y, wPad, height);
      	 v1 = getPixelCompress(imgEdge->pixPtr, x, y, wPad, height);
      	 if(v1 > 0) v0 = 0;
      	 setPixelDecompress(img->pixPtr,  wPad, height, x, y, v0);
       }
   }


}


