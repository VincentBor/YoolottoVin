/*
 * RGBA.c
 *
 *  Created on: Aug 1, 2012
 *      Author: Wicek
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include  "../headers/Functions.h"
//-----------------------------------------------------------------------------
//      Function to load RGBA file from disk in the memory block.
//      Input:   image structure .
//      Returns: address of memory with the DIB, including file header.
//
//      Function assumes that this is RGBA block (no tests)
//      Function saves all important data to the image structure
//


LPBYTE LoadRGBAPFile (LPIMGDATA img)
{
	FILE *f;
	int len;
	errno = 0;
	char fileName[500];
	LPBYTE ptr;
	size_t fSize = 0;
	char *szFileName = img->szIMGName;
	char *szPathName = img->szIMGPath;
	int bufforSize = 0;
	int hSize = 0;

	img->error[0] = 0;
	if (!szFileName) return NULL;  // test input file name
	strcpy(fileName, szPathName);
	strcat(fileName, szFileName);

	printf("open file %s\n",fileName);

    f= fopen(fileName, "r");
    if(f == NULL) {
    	sprintf(img->error,"LoadImage (\"%s\") failed: %s\n",fileName, strerror(errno));
    	return NULL;
    }
    fseek(f, 0L,SEEK_END);
//    printf("fseek %s\n",fileName);

    if(ferror(f)){
    	sprintf(img->error,"LoadImage (\"%s\") fseek failed: %s\n",fileName, strerror(errno));
    	return NULL;
    }
//    printf("ftell %s\n",fileName);

    len = ftell(f);
//    printf("fseek %s\n",fileName);
    fseek(f, 0L,SEEK_SET);

    printf("file %s length= %d\n",fileName, len);

    hSize = sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER);
    bufforSize = len + hSize;
    ptr = calloc(bufforSize, sizeof(BYTE));
    if(ptr == NULL) {
//    	printf("calloc failed");
    	sprintf(img->error,"LoadImage ( \"%s\" ) calloc ( %d ) failed: %s\n",fileName, bufforSize, strerror(errno));
    	return NULL;
    }
    fSize = fread((char *)(ptr+hSize), len, 1, f);


//    printf("fSize %ld \n",fSize*len);

    if(ferror(f) || fSize*len != len){
        if(ferror(f))
        	sprintf(img->error,"LoadImage (\"%s\") read failed: %s\n",fileName, strerror(errno));
        else
           sprintf(img->error,"LoadImage (\"%s\") read failed: (size= %d) != (len= %d\n)",fileName, fSize*len, len);
    	return NULL;
    }
	if(setHeaderDataForRGBA (ptr, fSize*len, img ) < 0) {
		free(ptr);
		ptr = NULL;
	}

     close_file(f);
 	return ptr;
}

int setHeaderDataForRGBA (LPBYTE ptr, size_t fSize, LPIMGDATA img) {

//	   LPBITMAPFILEHEADER bf = (LPBITMAPFILEHEADER)(ptr);
//	   LPBITMAPINFOHEADER bi = (LPBITMAPINFOHEADER)(ptr + sizeof (BITMAPFILEHEADER));
	   int hSize = sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER);
	   unsigned long uFlth;

	   img->ptr = ptr;
//	   img->bf.bfType = getLeftBytes((int*)bf->bfType, 2);
	   img->bf.bfType = 0x4d42;
	   img->bf.bfSize = fSize+hSize;
	   img->bf.bfReserved1 =  0;
	   img->bf.bfReserved2 =  0;
	   img->bf.bfOffBits =   hSize;
	   img->pixPtr = ptr +img->bf.bfOffBits;

	    img->bi.biSize = hSize - 14;
	    img->bi.biWidth = img->width;
	    img->bi.biHeight = img->height;
	    img->bi.biPlanes = 1;
	    img->bi.biBitCount = 32;
	    img->bi.biCompression = 0;
	    img->bi.biSizeImage = fSize;
	    img->bi.biXPelsPerMeter = img->dpiX;
	    img->bi.biYPelsPerMeter = img->dpiY;
	    img->bi.biClrUsed = 0;
	    img->bi.biClrImportant = 0;
	    img->bi.biWidthPad = ((img->bi.biWidth*img->bi.biBitCount+31)/32)*4;
	    img->colorPtr = ptr + 14 + img->bi.biSize;


	    uFlth = img->bi.biWidth * (img->bi.biBitCount/8)* img->bi.biHeight;

	    if ((img->bi.biWidth  > (LONG)MAX_WIDTH) ||
	         (img->bi.biHeight > (LONG)MAX_HEIGHT)) {
//	    	strcpy(img->error,  "Image size exceeds allowed limit");
        	sprintf(img->error,"Image width/height exceeds allowed limit; width= %d, height= %d\n",img->bi.biWidth, img->bi.biHeight);
	        return -5;
	    }
	    // verify if the the length of file is the same as in the bmp header
	    if (fSize != (LONG)uFlth) {
//	    	strcpy(img->error,  "Image/file length problem");
        	sprintf(img->error,"Image size problem; file size= %d, RGBA size= %ld\n", fSize, uFlth);
	        return -5;
	   }
	    printBMPData (img);
	    return 0;
   }


   void getRGBAcolor (LPIMGDATA img, int xn,int yn, LPRGBA rgba) {
	        int  w;
	        long nY;
	        long nX;
	        int sh= 4;
	        LPBYTE ptr;

	            w=img->bi.biWidth*sh;
//	            yn=img->bi.biHeight-1-yn;
	            nY=yn*w;
	       	    nX = (long)xn*sh;
	            ptr=img->pixPtr+nY+nX;
//	            rgba->R = *ptr;
//	            rgba->G = *(ptr+1);
//	            rgba->B = *(ptr+2);
//	            rgba->A = *(ptr+3);
	            rgba->B = *ptr;
	            rgba->G = *(ptr+1);
	            rgba->R = *(ptr+2);
	            rgba->A = *(ptr+3);

	    }

   void testRGBAcolors (LPIMGDATA img) {
	   RGBA rgba;
		int i, k;
//		for(i=0;i<img->bi.biHeight;i++){
		for(i=0;i<1;i++){
//			for(k=0;k<img->bi.biWidth;k++){
				for(k=0;k<100;k++){
				 getRGBAcolor (img, k, i, &rgba);
  printf("A= %x, R= %x, G= %x, B=%x \n", rgba.A, rgba.R, rgba.G, rgba.B);
			}
		}

   }

//   void set2Bytes(LPBYTE out, LPBYTE inp){
//	   *out = *inp;*(out+1) = *(inp+1);
//   }
   void set4Bytes(LPBYTE out, int inp){
	   LPBYTE b = (LPBYTE)&inp;
	   *out = *b;*(out+1) = *(b+1);
	   *(out+2) = *(b+2);*(out+3) = *(b+3);

   }
  void setOnePixelFromRGBA(LPBYTE ptrT, int xn, int yn, RGBA rgba, LPIMGDATA img){
	    int  w;
	    long nY;
	    long nX;
	    LPBYTE ptr;
	    int sh= 4;
	        w=img->bi.biWidthPad;
	        yn=img->bi.biHeight-1-yn;
	        nY=yn*w;
       	    nX = (long)(xn*sh);
            ptr=ptrT+nY+nX;
            *ptr= rgba.B;
            *(ptr+1)= rgba.G;
            *(ptr+2)= rgba.R;
            *(ptr+3)= rgba.A;
  }
   void setPixelArrayFromRGBA(LPBYTE ptrT, LPIMGDATA img){
	   RGBA rgba;
		int i, k;
		for(i=0;i<img->bi.biHeight;i++)
		{
			for(k=0;k<img->bi.biWidth;k++){
				 getRGBAcolor (img, k, i, &rgba);
				 setOnePixelFromRGBA(ptrT, k, i, rgba, img);
//  printf("A= %x, R= %x, G= %x, B=%x \n", rgba.A, rgba.R, rgba.G, rgba.B);
			}
		}
   }
   void setOnePixelFromRGBA_filterWB(LPBYTE ptrT, int xn, int yn, RGBA rgba, LPIMGDATA img, int v){
 	    int  w;
 	    long nY;
 	    long nX;
 	    LPBYTE ptr;
 	    int sh= 4;
 	        w=img->bi.biWidthPad;
 	        yn=img->bi.biHeight-1-yn;
 	        nY=yn*w;
        	    nX = (long)(xn*sh);
             ptr=ptrT+nY+nX;
             if (v == 0) {
             *ptr= 0;
             *(ptr+1)= 0;
             *(ptr+2)= 0;
             *(ptr+3)= 255;
             }else{
                 *ptr= 255;
                 *(ptr+1)= 255;
                 *(ptr+2)= 255;
                 *(ptr+3)= 255;

             }
   }

   void setPixelArrayFromRGBA_filterWB(LPBYTE ptrT, LPIMGDATA img, int tH){
	   RGBA rgba;
		int i, k;
		int v;
		for(i=0;i<img->bi.biHeight;i++)
		{
			for(k=0;k<img->bi.biWidth;k++){
				 getRGBAcolor (img, k, i, &rgba);
				 v = getWB_value(&rgba, tH);
				 setOnePixelFromRGBA_filterWB(ptrT, k, i, rgba, img, v);
//  printf("A= %x, R= %x, G= %x, B=%x \n", rgba.A, rgba.R, rgba.G, rgba.B);
			}
		}

   }

   void setOnePixelWBFromRGBA(LPIMGDATA imgOutp, int xn, int yn, int v){
	    int  w;
	    long nY;
	    long nX;
	    BYTE mask;
	    LPBYTE ptr;
	        w=imgOutp->bi.biWidthPad;
	        yn=imgOutp->bi.biHeight-1-yn;
	        nY=yn*w;
      	    nX = (long)(xn/8);

            ptr = imgOutp->pixPtr+nY+nX;

// printf("ptr-imgOutp->ptr= %d \n", ptr - imgOutp->ptr);

	         mask=LEFT_BIT;
	         mask >>=(xn%8);
	         if(v!=0){
	        	 *ptr = *ptr | mask;
	         }
	         else {
	        	 *ptr = *ptr & (~mask);
	         }
   }

   void setPadding(LPIMGDATA imgOutp, int yn){
	    int  w;
	    long nY;
	    long nX;
	    int  xn;
	    BYTE mask;
	    LPBYTE ptr;
	        w=imgOutp->bi.biWidthPad;
	        yn=imgOutp->bi.biHeight-1-yn;
	        nY=yn*w;
	        for(xn=imgOutp->bi.biWidth;xn<w*8;xn++){
	        	nX = (long)(xn/8);
	        	ptr = imgOutp->pixPtr+nY+nX;
		         mask=LEFT_BIT;
		         mask >>=(xn%8);
	        	 *ptr = *ptr | mask;

	        }
   }

   void testWhite(LPIMGDATA imgOutp, int xn, int yn){
	    int  w;
	    long nY;
	    long nX;
	    LPBYTE ptr;
	        w=imgOutp->bi.biWidthPad;
	        yn=imgOutp->bi.biHeight-1-yn;
	        nY=yn*w;
      	    nX = (long)(xn/8);

            ptr = imgOutp->pixPtr+nY+nX;

// printf("ptr-imgOutp->ptr= %d \n", ptr - imgOutp->ptr);
            *ptr = 255;
//            *ptr = 0;
   }

   void setPixelArrayFromRGBAToWB(LPIMGDATA imgOutp, LPIMGDATA img, int tH){
	    RGBA rgba;
		int i, k;
		int v;
		for(i=0;i<img->bi.biHeight;i++)
//		for(i=0;i<500;i++)
		{
			for(k=0;k<img->bi.biWidth;k++){
//				for(k=0;k<30;k++){
				 getRGBAcolor (img, k, i, &rgba);
				 v = getWB_value(&rgba, tH);
				 setOnePixelWBFromRGBA(imgOutp, k, i, v);
//				   testWhite(imgOutp, k, i);
			}
//			setPadding(imgOutp, i);
		}
   }


   void reloadImgInfo(LPIMGDATA imgOutp, LPBYTE ptr){
	   LPBITMAPFILEHEADER bf;
	   LPBITMAPINFOHEADER bi;

	    bf = (LPBITMAPFILEHEADER)(ptr);
	    bi = (LPBITMAPINFOHEADER)(ptr + sizeof (BITMAPFILEHEADER));

	    imgOutp->ptr = ptr;
	    imgOutp->bf.bfType = getLeftBytes((int*)bf->bfType, 2);
	    imgOutp->bf.bfSize = getLeftBytes((int*)bf->bfSize, 4);
	    imgOutp->bf.bfReserved1 =  getLeftBytes((int*)bf->bfReserved1, 2);
	    imgOutp->bf.bfReserved2 =  getLeftBytes((int*)bf->bfReserved2, 2);
	    imgOutp->bf.bfOffBits =   getLeftBytes((int*)bf->bfOffBits, 4);
	    imgOutp->pixPtr = ptr +imgOutp->bf.bfOffBits;


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

//printf("bi->biWidth= %d, imgOutp.bi.biWidth = %d \n",  *(int*)bi->biWidth, imgOutp.bi.biWidth);

	     imgOutp->bi.biWidthPad = ((imgOutp->bi.biWidth*imgOutp->bi.biBitCount+31)/32)*4;
	     imgOutp->colorPtr = ptr + 14 + imgOutp->bi.biSize;

   }

   void convertRGBAToBMP(LPIMGDATA img){
   int hSize = 122;
   int bufforSize = img->bi.biSizeImage+hSize;
   LPBYTE ptr, ptrT;
   LPBITMAPFILEHEADER bf;
   LPBITMAPINFOHEADER bi;
   int i;

//    printf("bufforSize= %d \n", bufforSize);

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

//  printf("bf->bfSize= %d \n",  *(int*)bf->bfSize);

//   printf("bf->bfType[0]= %x, bf->bfType[1]= %x \n", bf->bfType[0], bf->bfType[1]);
//   printf("bf->bfType= %x, bf->bfSize= %x \n", *(short*)bf->bfType, *bf->bfSize);
//   printf("bf->bfType= %d, bf->bfSize= %d \n", *(short*)bf->bfType, *bf->bfSize);

	    bf->bfReserved1[0] = 0;
	    bf->bfReserved1[1] = 0;
	    bf->bfReserved2[0] = 0;
	    bf->bfReserved2[1] = 0;
	    set4Bytes(bf->bfOffBits, hSize);

//   printf("bf->bfOffBits= %d \n",  *(int*)bf->bfOffBits);
//	    bf->bfOffBits = hSize;



      set4Bytes(bi->biSize, hSize-14);
      set4Bytes(bi->biWidth, img->width);
      set4Bytes(bi->biHeight, img->height);

		*bi->biPlanes = (short)1;
		*bi->biBitCount= (short)img->bi.biBitCount;
  set4Bytes(bi->biCompression, 0);
  set4Bytes(bi->biSizeImage, img->bi.biSizeImage);
  set4Bytes(bi->biXPelsPerMeter, img->dpiX);
  set4Bytes(bi->biYPelsPerMeter, img->dpiY);
  set4Bytes(bi->biClrUsed, 0);
  set4Bytes(bi->biClrImportant, 0);

  ptrT = ptr+54;
  set4Bytes(ptrT, 0x00FF0000);
  set4Bytes(ptrT+4, 0x0000FF00);
  set4Bytes(ptrT+8, 0x000000FF);
  set4Bytes(ptrT+12, 0xFF000000);
  ptrT += 16;
  set4Bytes(ptrT, 0x57696E20);
  ptrT += 4;
  set4Bytes(ptrT, 0x57696E20);
  for(i=0;i<9;i++){
	  ptrT += 4;
	  set4Bytes(ptrT, 0x00000000);
  }
  ptrT += 4;
  set4Bytes(ptrT, 0x00000000);
  ptrT += 4;
  set4Bytes(ptrT, 0x00000000);
  ptrT += 4;
  set4Bytes(ptrT, 0x00000000);
  ptrT += 4;

//   printf("img->bi.biWidthPad= %d, bi->biWidth= %d, img->width= %d \n", img->bi.biWidthPad, *(int*)bi->biWidth, img->width);
//   printf("ptr-ptrT= %d \n", ptr-ptrT);

  setPixelArrayFromRGBA(ptr + hSize, img);
  if(img->ptr != NULL) {
	  free(img->ptr);
	  img->ptr = NULL;
  }

  reloadImgInfo(img, ptr);
//  saveFile (img, "RGBA.BMP", ptr, *(int*)bf->bfSize);

//   free(ptr);

   }
   void saveFile (LPIMGDATA img, char *szIMGName, LPBYTE ptr, int len)
   {
   	FILE *f;
   	errno = 0;
   	char fileName[500];
   	size_t fSize = 0;
   	char *szFileName = szIMGName;
   	char *szPathName = img->szOutPath;

   	if (!szFileName) return;  // test only
   	strcpy(fileName, szPathName);
   	strcat(fileName, szFileName);

   	printf("save file %s\n",fileName);

       f= fopen(fileName, "w");
       if(f == NULL) {
       	fprintf(stderr,"saveBMP (\"%s\") failed: %s\n",fileName, strerror(errno));
       	return;
       }

       fSize = fwrite(ptr, len, 1, f);
       if(ferror(f) || fSize*len != len){
           if(ferror(f))
           	sprintf(img->error,"saveFile (\"%s\") read failed: %s\n",fileName, strerror(errno));
           else
              sprintf(img->error,"saveFile (\"%s\") read failed: (size= %d) != (len= %d\n)",fileName, fSize*len, len);
       	return;
       }

        close_file(f);
   }
   void convertRGBATo_mono_BMP(LPIMGDATA img, int tH){
   int hSize = 54;
   int bufforSize;
   LPBYTE ptr ;
   LPBITMAPFILEHEADER bf;
   LPBITMAPINFOHEADER bi;
   int  biSizeImage, biWidthPad;
   IMGDATA imgOutp;
   strcpy(imgOutp.szOutPath, img->szOutPath);

   biWidthPad = ((img->bi.biWidth+31)/32)*4;

   biSizeImage = biWidthPad*img->bi.biHeight;
   bufforSize= biSizeImage+hSize+8;

    printf("bufforSize= %d \n", bufforSize);

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

//  printf("bf->bfSize= %d \n",  *(int*)bf->bfSize);
//   printf("bf->bfType= %x, bf->bfSize= %x \n", *(short*)bf->bfType, *bf->bfSize);
//   printf("bf->bfType= %d, bf->bfSize= %d \n", *(short*)bf->bfType, *bf->bfSize);



	    bf->bfReserved1[0] = 0;
	    bf->bfReserved1[1] = 0;
	    bf->bfReserved2[0] = 0;
	    bf->bfReserved2[1] = 0;
	    set4Bytes(bf->bfOffBits, hSize+8);
	    imgOutp.ptr = ptr;
	    imgOutp.bf.bfType = getLeftBytes((int*)bf->bfType, 2);
	    imgOutp.bf.bfSize = getLeftBytes((int*)bf->bfSize, 4);
	    imgOutp.bf.bfReserved1 =  getLeftBytes((int*)bf->bfReserved1, 2);
	    imgOutp.bf.bfReserved2 =  getLeftBytes((int*)bf->bfReserved2, 2);
	    imgOutp.bf.bfOffBits =   getLeftBytes((int*)bf->bfOffBits, 4);
	    imgOutp.pixPtr = ptr +imgOutp.bf.bfOffBits;




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





	     imgOutp.bi.biSize = getLeftBytes((int*)bi->biSize, 4);
	     imgOutp.bi.biWidth = getLeftBytes((int*)bi->biWidth, 4);
	     imgOutp.bi.biHeight = getLeftBytes((int*)bi->biHeight, 4);
	     imgOutp.bi.biPlanes = getLeftBytes((int*)bi->biPlanes, 2);
	     imgOutp.bi.biBitCount = getLeftBytes((int*)bi->biBitCount, 2);
	     imgOutp.bi.biCompression = getLeftBytes((int*)bi->biCompression, 4);
	     imgOutp.bi.biSizeImage = getLeftBytes((int*)bi->biSizeImage, 4);
	     imgOutp.bi.biXPelsPerMeter = getLeftBytes((int*)bi->biXPelsPerMeter, 4);
	     imgOutp.bi.biYPelsPerMeter = getLeftBytes((int*)bi->biYPelsPerMeter, 4);
	     imgOutp.bi.biClrUsed = getLeftBytes((int*)bi->biClrUsed, 4);
	     imgOutp.bi.biClrImportant = getLeftBytes((int*)bi->biClrImportant, 4);

	     imgOutp.bi.biWidthPad = ((imgOutp.bi.biWidth*imgOutp.bi.biBitCount+31)/32)*4;
	     imgOutp.colorPtr = ptr + 14 + imgOutp.bi.biSize;
	     *(ptr+hSize) = 0;
	     *(ptr+hSize+1) = 0;
	     *(ptr+hSize+2) = 0;
	     *(ptr+hSize+3) = 0;
	     *(ptr+hSize+4) = 255;
	     *(ptr+hSize+5) = 255;
	     *(ptr+hSize+6) = 255;
	     *(ptr+hSize+7) = 0;

//	      printf("imgOutp.bi.biWidth= %d, imgOutp.bi.biBitCount= %d, imgOutp.bi.biWidthPad= %d \n",  imgOutp.bi.biWidth, imgOutp.bi.biBitCount, imgOutp.bi.biWidthPad);

         setPixelArrayFromRGBAToWB(&imgOutp, img, tH);
/*
	     {
			int y, x;
			int i, k;
//			for(y=0;y<img->bi.biHeight;y++)
			for(i=0;i<100;i++)
			{
				for(k=0;k<242;k++){
	//				for(k=0;k<30;k++){
					{
				    int  w;
				    long nY;
//				    long nX;
				    LPBYTE ptrTest;
				        w=244;
				        y = i;
				        x = k;
				        y=imgOutp.bi.biHeight-1-y;
//				        y=500-1-y;
				        nY=y*w;
//			      	    nX = (long)(x/8);

				        ptrTest = ptr+imgOutp.bf.bfOffBits+nY+x;

			// printf("ptr-imgOutp->ptr= %d \n", ptr - imgOutp->ptr);
			            *ptrTest = 255;
					}
				}
			}
	     }
*/
         saveFile (img, "RGBA_WB.BMP", ptr, bufforSize);

         free(ptr);

   }
   void convertRGBAToBMP_filterWB(LPIMGDATA img, int tH){
   int hSize = 122;
   int bufforSize = img->bi.biSizeImage+hSize;
   LPBYTE ptr, ptrT;
   LPBITMAPFILEHEADER bf;
   LPBITMAPINFOHEADER bi;
   int i;

    printf("bufforSize= %d \n", bufforSize);

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

//  printf("bf->bfSize= %d \n",  *(int*)bf->bfSize);


	    bf->bfReserved1[0] = 0;
	    bf->bfReserved1[1] = 0;
	    bf->bfReserved2[0] = 0;
	    bf->bfReserved2[1] = 0;
	    set4Bytes(bf->bfOffBits, hSize);

//   printf("bf->bfOffBits= %d \n",  *(int*)bf->bfOffBits);
//	    bf->bfOffBits = hSize;
      set4Bytes(bi->biSize, hSize-14);
      set4Bytes(bi->biWidth, img->width);
      set4Bytes(bi->biHeight, img->height);
		*bi->biPlanes = (short)1;
		*bi->biBitCount= (short)img->bi.biBitCount;
  set4Bytes(bi->biCompression, 3);
  set4Bytes(bi->biSizeImage, img->bi.biSizeImage);
  set4Bytes(bi->biXPelsPerMeter, img->dpiX);
  set4Bytes(bi->biYPelsPerMeter, img->dpiY);
  set4Bytes(bi->biClrUsed, 0);
  set4Bytes(bi->biClrImportant, 0);

  ptrT = ptr+54;
  set4Bytes(ptrT, 0x00FF0000);
  set4Bytes(ptrT+4, 0x0000FF00);
  set4Bytes(ptrT+8, 0x000000FF);
  set4Bytes(ptrT+12, 0xFF000000);
  ptrT += 16;
  set4Bytes(ptrT, 0x57696E20);
  ptrT += 4;
  set4Bytes(ptrT, 0x57696E20);
  for(i=0;i<9;i++){
	  ptrT += 4;
	  set4Bytes(ptrT, 0x00000000);
  }
  ptrT += 4;
  set4Bytes(ptrT, 0x00000000);
  ptrT += 4;
  set4Bytes(ptrT, 0x00000000);
  ptrT += 4;
  set4Bytes(ptrT, 0x00000000);
  ptrT += 4;

//   printf("img->bi.biWidthPad= %d, bi->biWidth= %d, img->width= %d \n", img->bi.biWidthPad, *(int*)bi->biWidth, img->width);
//   printf("ptr-ptrT= %d \n", ptr-ptrT);

  setPixelArrayFromRGBA_filterWB(ptrT, img, tH);
  saveFile (img, "RGBA_WB.BMP", ptr, *(int*)bf->bfSize);

   free(ptr);

   }

   void reloadPixel(unsigned char *ptrInp, unsigned char *ptrOtp, int imgWidth, int imgHeight, int x, int y){
       int  w;
       long nYOtp, nY;
       int yOtp;
       long nX;
       int sh= 4;
       unsigned char *pixelOtpPtr;
       unsigned char *pixelInpPtr;

           w=imgWidth*sh;
	       yOtp=imgHeight-1-y;
           nY=y*w;
           nYOtp=yOtp*w;
      	   nX = (long)x*sh;
      	   pixelOtpPtr=ptrOtp+nYOtp+nX;
      	   pixelInpPtr=ptrInp+nY+nX;
      	   *pixelOtpPtr = *pixelInpPtr;
      	   *(pixelOtpPtr+1) = *(pixelInpPtr+1);
      	   *(pixelOtpPtr+2) = *(pixelInpPtr+2);
      	   *(pixelOtpPtr+3) = *(pixelInpPtr+3);

   }
   void reloadRBAbuffer(unsigned char *ptrInp, unsigned char *ptrOtp, int imgWidth, int imgHeight) {
		int y, x;

		for(y=0;y<imgHeight;y++) {
			for(x=0;x<imgWidth;x++){
				reloadPixel(ptrInp, ptrOtp, imgWidth, imgHeight, x, y);
			}
		}


   }
   void saveRGBAasBMP(unsigned char *ptrInp, int imgWidth, int imgHeight, LPIMGDATA img){
	   int hSize = 122;
	   int bufforSize = img->bi.biSizeImage+hSize;
	   LPBYTE ptr, ptrT, ptrPix;
	   LPBITMAPFILEHEADER bf;
	   LPBITMAPINFOHEADER bi;
	   int i;

	    printf("bufforSize= %d \n", bufforSize);

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
		    set4Bytes(bf->bfOffBits, hSize);
		      set4Bytes(bi->biSize, hSize-14);
		      set4Bytes(bi->biWidth, img->width);
		      set4Bytes(bi->biHeight, img->height);


				*bi->biPlanes = (short)1;
				*bi->biBitCount= (short)img->bi.biBitCount;
		  set4Bytes(bi->biCompression, 3);
		  set4Bytes(bi->biSizeImage, img->bi.biSizeImage);
		  set4Bytes(bi->biXPelsPerMeter, img->dpiX);
		  set4Bytes(bi->biYPelsPerMeter, img->dpiY);
		  set4Bytes(bi->biClrUsed, 0);
		  set4Bytes(bi->biClrImportant, 0);
		  ptrT = ptr+54;
		  set4Bytes(ptrT, 0x00FF0000);
		  set4Bytes(ptrT+4, 0x0000FF00);
		  set4Bytes(ptrT+8, 0x000000FF);
		  set4Bytes(ptrT+12, 0xFF000000);
		  ptrT += 16;
		  set4Bytes(ptrT, 0x57696E20);
		  ptrT += 4;
		  set4Bytes(ptrT, 0x57696E20);
		  for(i=0;i<9;i++){
			  ptrT += 4;
			  set4Bytes(ptrT, 0x00000000);
		  }
		  ptrT += 4;
		  set4Bytes(ptrT, 0x00000000);
		  ptrT += 4;
		  set4Bytes(ptrT, 0x00000000);
		  ptrT += 4;
		  set4Bytes(ptrT, 0x00000000);
		  ptrT += 4;


		ptrPix = ptr + hSize;
	    reloadRBAbuffer(ptrInp, ptrPix, imgWidth, imgHeight);

        saveFile (img, "RGBAtest_WB.BMP", ptr, *(int*)bf->bfSize);

         free(ptr);

   }



//--------------------------------------------------------------------------------------------------------
// Iphone test -----------------------------------
//--------------------------------------------------------------------------------------------------------

   void setRGBAValues (unsigned char *ptr, int imgWidth, int imgHeight, int xn,int yn, int v) {
		        int  w;
		        long nY;
		        long nX;
		        int sh= 4;
		        unsigned char *pixelPtr;
		            w=imgWidth*sh;
		            nY=yn*w;
		       	    nX = (long)xn*sh;
		       	    pixelPtr=ptr+nY+nX;
		             if (v == 0) {
		             *pixelPtr= 0;
		             *(pixelPtr+1)= 0;
		             *(pixelPtr+2)= 0;
		             *(pixelPtr+3)= 255;
		             }else{
		                 *pixelPtr= 255;
		                 *(pixelPtr+1)= 255;
		                 *(pixelPtr+2)= 255;
		                 *(pixelPtr+3)= 255;
	                }
   }

   int getWB_value(RGBA *rgba, int tH){
       if(rgba->B < tH && rgba->G < tH && rgba->R < tH)
             return 0;
       return 1;
   }


   void getRGBAValues (unsigned char *ptr, int imgWidth, int imgHeight, int xn, int yn, LPRGBA rgba) {
	        int  w;
	        long nY;
	        long nX;
	        int sh= 4;
	        unsigned char *pixelPtr;

//	        long test;

	            w=imgWidth*sh;
	            nY=yn*w;
	       	    nX = (long)xn*sh;
	       	    pixelPtr=ptr+nY+nX;

	            rgba->B = *pixelPtr;
	            rgba->G = *(pixelPtr+1);
	            rgba->R = *(pixelPtr+2);
	            rgba->A = *(pixelPtr+3);
	    }

   void convertRGBAcolor_to_RGBAwb(unsigned char *ptr, int imgWidth, int imgHeight){
	   RGBA rgba;
		int y, x;
		int v;

		for(y=0;y<imgHeight;y++) {
			for(x=0;x<imgWidth;x++){
				 getRGBAValues (ptr, imgWidth, imgHeight, x, y, &rgba);
				 v = getWB_value(&rgba,120);
// printf(" x= %d, y= %d v= %d\n", x, y, v);
				 setRGBAValues (ptr, imgWidth, imgHeight, x, y, v);

			}
		}
   }

   void convertRGBAtoMonochromeIter(LPIMGDATA img, LPIMGDATA imgOut){
	   int tH1, tH2, tH3, idx;
   	  double drk1, drk2, drk3;
             convertRGBAToBMP(img);
//   	     saveBMP (img);
       	     idx = 0;
       	     tH2 = 80;
       	     while(idx < 5){
       	    	 if(imgOut->ptr != NULL)
       	    	     free(imgOut->ptr);
   	             convertBMPTo_mono_BMP(img, imgOut, tH2, tH2, tH2);

       	        drk1 = calculateDarknessMonoTop(imgOut);
    	        drk2 = calculateDarknessMonoCenter(imgOut);
    	        drk3 = calculateDarknessMonoBottom(imgOut);

//   printf(" ----------------------- fileName= %s, massTop= %f, masCenter= %f, massBottom= %f, tH= %d \n",img->szIMGName, drk1, drk2, drk3, tH2);
                  if(drk2 < 1.5 )
               	   tH2 = changeLowDark(tH2, drk2);
                      else {
                   	   tH1 = tH2;tH3 = tH2;
                   	   if(drk1/drk2 > 2.5 ) tH1 = tH2 - tH2/3 - 10;
                   	   if(drk3/drk2 > 2.5 ) tH3 = tH2 - tH2/4;
                   	   if(tH1 != tH2 || tH3 != tH2) {
//        printf(" ----------------------- fileName= %s, tH1= %d, tH2= %d, tH3= %d \n",img->szIMGName, tH1, tH2, tH3);
                   		   convertBMPTo_mono_BMP(img, imgOut, tH1, tH2, tH3);
                   	   }
                   	   break;
                      }
                  idx++;
       	     }
  	     convertDPI_in_BMP(imgOut, 300, 300);
//   	     saveBMP (imgOut);
        }

//-------------------------------------------------------------------------
// iPhone call -------------------------------------------------------------
	 void setRGBcolor (LPBYTE ptrOut, int wInp, int xn,int yn, BYTE r, BYTE g, BYTE b) {
       int  w;
	   long nY;
	   long nX;
	   int sh= 3;
	   LPBYTE ptrL;

			            w = wInp*sh;
		                nY=yn*w;
			       	    nX = (long)xn*sh;
			            ptrL=ptrOut+nY+nX;
			            *ptrL = r;
			            *(ptrL+1) = g;
			            *(ptrL+2) = b;

	 }
   void getRGBAcolor2 (LPBYTE ptrInp, int wInp, int xn,int yn, LPBYTE r, LPBYTE g, LPBYTE b) {
	        int  w;
	        long nY;
	        long nX;
	        int sh= 4;
	        LPBYTE ptr;

	            w=wInp*sh;
	            nY=yn*w;
	       	    nX = (long)xn*sh;
	            ptr=ptrInp+nY+nX;
	            *r = *ptr;
	            *g = *(ptr+1);
	            *b = *(ptr+2);
//	            rgba->A = *(ptr+3);

	    }

   void reloadImgBufferFromRGBAtoRGB(LPBYTE ptrInp, int w, int h, LPBYTE ptrOut){
	   BYTE r, g, b;
		int i, k;
		for(i=0;i<h;i++)	{
			for(k=0;k<w;k++){
				 getRGBAcolor2 (ptrInp,  w, k, i, &r, &g, &b);
				 setRGBcolor (ptrOut, w, k, i, r, g, b);
//  printf("A= %x, R= %x, G= %x, B=%x \n", rgba.A, rgba.R, rgba.G, rgba.B);
			}
		}
   }

   void testRGBA_RGB(LPIMGDATA img){
	   int bufforSize;
	   LPBYTE ptrT;
	//   int i;

	   bufforSize = img->bi.biWidth*img->bi.biHeight*24;

	   ptrT = calloc(bufforSize, sizeof(BYTE));
	   if(ptrT == NULL) {
	   	    sprintf(img->error,"convertRGBAToBMP ( \"%s\" ) calloc ( %d ) failed: %s\n",img->szIMGName, bufforSize, strerror(errno));
	   	    return;
	   }




	    reloadImgBufferFromRGBAtoRGB(img->pixPtr, img->bi.biWidth, img->bi.biHeight, ptrT);
	    img->pixPtr = ptrT;


   }

   void getRGBcolor (LPBYTE ptr, LPIMGDATA img, int xn,int yn, LPBYTE r, LPBYTE g, LPBYTE b) {
	        int  w;
	        long nY;
	        long nX;
	        int sh= 3;
	        LPBYTE ptrL;

	            w=img->bi.biWidth*sh;
//	            yn=img->bi.biHeight-1-yn;
	            nY=yn*w;
	       	    nX = (long)xn*sh;
	            ptrL=ptr+nY+nX;
	            *r = *ptrL;
	            *g = *(ptrL+1);
	            *b = *(ptrL+2);
	    }
   void setRGBcolorBMP (LPIMGDATA img, int xn,int yn, BYTE r, BYTE g, BYTE b) {
	        int  w;
	        long nY;
	        long nX;
	        int sh= 3;
	        LPBYTE ptrL;

	            w = ((img->bi.biWidth*24+31)/32)*4;
//	            w=img->bi.biWidth*sh;
	            yn=img->bi.biHeight-1-yn;
	            nY=yn*w;
	       	    nX = (long)xn*sh;
	            ptrL=img->pixPtr+nY+nX;
	            *ptrL = r;
	            *(ptrL+1) = g;
	            *(ptrL+2) = b;
	    }

   void reloadImgBufferFromRGBtoBMP(LPIMGDATA img, LPBYTE ptr){
	   BYTE r, g, b;
		int i, k;
		for(i=0;i<img->bi.biHeight;i++)	{
			for(k=0;k<img->bi.biWidth;k++){
				 getRGBcolor (ptr, img, k, i, &r, &g, &b);
				 setRGBcolorBMP (img, k, i, r, g, b);
//  printf("A= %x, R= %x, G= %x, B=%x \n", rgba.A, rgba.R, rgba.G, rgba.B);
			}
		}
   }


   void convertRGBToBMP(unsigned char *ptr, int imgWidth, int imgHeight, LPIMGDATA img){

   int bufforSize;
   int biWidthPad = ((imgWidth*24+31)/32)*4;
   LPBYTE ptrT;
   LPBITMAPFILEHEADER bf;
   LPBITMAPINFOHEADER bi;
//   int i;
   int offBits = sizeof (BITMAPFILEHEADER) +
             sizeof (BITMAPINFOHEADER);
//            + sizeof (RGBQUAD) * 2;

   bufforSize = biWidthPad*imgHeight+offBits;

   ptrT = calloc(bufforSize, sizeof(BYTE));
   if(ptrT == NULL) {
   	    sprintf(img->error,"convertRGBAToBMP ( \"%s\" ) calloc ( %d ) failed: %s\n",img->szIMGName, bufforSize, strerror(errno));
   	    return;
   }
   bf = (LPBITMAPFILEHEADER)(ptrT);
   bi = (LPBITMAPINFOHEADER)(ptrT + sizeof (BITMAPFILEHEADER));

   bf->bfType[0]= 0x42;
   bf->bfType[1]= 0x4d;
   set4Bytes(bf->bfSize, bufforSize);

   bf->bfReserved1[0] = 0;
   bf->bfReserved1[1] = 0;
   bf->bfReserved2[0] = 0;
   bf->bfReserved2[1] = 0;
   set4Bytes(bf->bfOffBits, offBits);

//   printf("bf->bfOffBits= %d \n",  *(int*)bf->bfOffBits);
//	    bf->bfOffBits = hSize;


 set4Bytes(bi->biSize, offBits-14);
 set4Bytes(bi->biWidth, imgWidth);
 set4Bytes(bi->biHeight, imgHeight);

	*bi->biPlanes = (short)1;
	*bi->biBitCount= (short)24;
set4Bytes(bi->biCompression, 0);
set4Bytes(bi->biSizeImage, bufforSize - offBits);
set4Bytes(bi->biXPelsPerMeter, 500);
set4Bytes(bi->biYPelsPerMeter, 500);
set4Bytes(bi->biClrUsed, 0);
set4Bytes(bi->biClrImportant, 0);



		   img->ptr = ptrT;
		   img->bf.bfType = 0x4d42;
		   img->bf.bfSize = bufforSize;
		   img->bf.bfReserved1 =  0;
		   img->bf.bfReserved2 =  0;
		   img->bf.bfOffBits =   offBits;
		   img->pixPtr = ptrT +img->bf.bfOffBits;

		    img->bi.biSize = offBits - 14;
		    img->width = imgWidth;
		    img->bi.biWidth = img->width;
		    img->height = imgHeight;
		    img->bi.biHeight = img->height;
		    img->bi.biPlanes = 1;
		    img->bi.biBitCount = 24;
		    img->bi.biCompression = 0;
		    img->bi.biSizeImage = bufforSize - offBits;
		    img->bi.biXPelsPerMeter = 500;
		    img->bi.biYPelsPerMeter = 500;
		    img->bi.biClrUsed = 0;
		    img->bi.biClrImportant = 0;
		    img->bi.biWidthPad = ((img->bi.biWidth*img->bi.biBitCount+31)/32)*4;
//		    img->colorPtr = ptr + 14 + img->bi.biSize;

//    printf("bufforSize= %d \n", bufforSize);

    reloadImgBufferFromRGBtoBMP(img, ptr);



   }


   void addSlash(char *path){
	   if(path[strlen(path)-1] != '\\' && path[strlen(path)-1] != '/')
		   strcat(path, "\\");
   }

   void set_I_O_Path(LPIMGDATA img, char *mainPath, char *test){
	   strcpy(img->szIMGPath, mainPath);
	   addSlash(img->szIMGPath);
   	   strcat(img->szIMGPath,"input\\");
	   strcpy(img->szOutPath, mainPath);
	   addSlash(img->szOutPath);
   	   strcat(img->szOutPath,"output\\");
	   strcpy(img->szWorkPath, mainPath);
	   addSlash(img->szWorkPath);
   	   strcat(img->szWorkPath,"work\\");

//      strcpy(img->szIMGName,"IMG_BMP_");
//      strcat(img->szIMGName, test);
//	  strcat(img->szIMGName, ".bmp");
       strcpy(img->szIMGName, test);

   }

   void setGlobalDataPrototype(LPTICKETDATA ticketDataPtr, char *mainPath){
	   strcpy(ticketDataPtr->prototypePath, mainPath);
	   addSlash(ticketDataPtr->prototypePath);
   	   strcat(ticketDataPtr->prototypePath,"prototype\\");
	   strcpy(ticketDataPtr->libraryPath, mainPath);
	   addSlash(ticketDataPtr->libraryPath);
	   strcat(ticketDataPtr->libraryPath,"library\\");
   	   ticketDataPtr->wCorrectionLevel = 20;
       ticketDataPtr->tgp = 20;                // distortion
       ticketDataPtr->wPar = 30;               // calibration
       ticketDataPtr->phi.phiMin = -50;
       ticketDataPtr->phi.phiMax =  50;
       ticketDataPtr->phi.stepPhi =  5;
   }


   void copyOutQuad(LPQUADRANGLE qO, LPQUADRANGLE qI, int len){
   	   int i;
   	   for(i=0;i<len;i++){
   		   qO[i].p1X = qI[i].p1X;
   		   qO[i].p1Y = qI[i].p1Y;
   		   qO[i].p2X = qI[i].p2X;
   		   qO[i].p2Y = qI[i].p2Y;
   		   qO[i].p3X = qI[i].p3X;
   		   qO[i].p3Y = qI[i].p3Y;
   		   qO[i].p4X = qI[i].p4X;
   		   qO[i].p4Y = qI[i].p4Y;
   	   }
   }

   void copyNewField(LPFIELDSRESULTS lpFieldsResults,  LPFIELDSRESULTS lpFieldsResultsInp){
	    int i, len;
		strcpy(lpFieldsResults->name, lpFieldsResultsInp->name);
		strcpy(lpFieldsResults->result, lpFieldsResultsInp->result);
		strcpy(lpFieldsResults->cand2, lpFieldsResultsInp->cand2);
		len = strlen(lpFieldsResults->result);
	    for(i=0;i<=len;i++){
	    	lpFieldsResults->flConf[i] = lpFieldsResultsInp->flConf[i];
	   }
	   copyOutQuad(lpFieldsResults->flQuad, lpFieldsResultsInp->flQuad, len);

   }
   void addOutputField(LPTICKETRESULTS lpTicketResults, LPFIELDSRESULTS lpFieldsResults){
        int i;
	    for(i=0;i<NUMFIELDS_RESULTS;i++){
	    	if(lpTicketResults->fieldsResults[i].name[0] == 0){
	    	    copyNewField(&lpTicketResults->fieldsResults[i], lpFieldsResults); break;
	    	}

       }
   }
   void copyOutputResults(LPTICKETRESULTS lpTicketResults, LPTICKETRESULTS ticketResultsInp){
	   int i;
	   lpTicketResults->status = ticketResultsInp->status;
	   lpTicketResults->type = ticketResultsInp->type;
	   lpTicketResults->typeCnf = ticketResultsInp->typeCnf;
	   lpTicketResults->yShift = ticketResultsInp->yShift;

	    for(i=0;i<NUMFIELDS_RESULTS;i++){
	    	if(ticketResultsInp->fieldsResults[i].name[0] != 0){
	    		if(strcmp(ticketResultsInp->fieldsResults[i].name, "LOGO") == 0 ||
   				  strcmp(ticketResultsInp->fieldsResults[i].name, "NUMBERS") == 0 ||
   				strcmp(ticketResultsInp->fieldsResults[i].name, "DATE") == 0 ||
   				strcmp(ticketResultsInp->fieldsResults[i].name, "PLAYER") == 0){

    			 addOutputField(lpTicketResults, &ticketResultsInp->fieldsResults[i]);

	    		}

	    	}
	    }

   }
   void initTicketResults(LPTICKETRESULTS lpTicketResults){
	   int i;
	   lpTicketResults->status = 0;
	   lpTicketResults->type = 0;
	   lpTicketResults->typeCnf = 0;
	   lpTicketResults->yShift = 0;

	    for(i=0;i<NUMFIELDS_RESULTS;i++){
	    	lpTicketResults->fieldsResults[i].name[0] = 0;
	    	}

   }
   int RGB_OCR_Call(unsigned char *ptr, int imgWidth, int imgHeight, char *mainPath,
		            LPTICKETRESULTS lpTicketResults, char *error, char *imgName){

	   IMGDATA img;
	   IMGDATA imgOut;
	   LPTICKETDATA ticketData;
	   TICKETRESULTS ticketResults;

	   int ok;
	   RECTANGLE area;
	   int ver;
	   HLPBYTE workAreaPtr;
	   LPBYTE iDStructPtr;
	   int  angle;
	   RADON radonStruct;
	   char buffLog[500];

while(1){
	   initTicketResults(lpTicketResults);
	   initTicketResults(&ticketResults);
//	   lpTicketResults->yShift = 0;
//	   ticketResults.yShift = 0;
 	   img.ptr = NULL;
 	   imgOut.ptr = NULL;
 	   ticketData = (LPTICKETDATA)calloc(sizeof(TICKETDATA), sizeof(BYTE));
 	   if(ticketData == NULL) {
 		  sprintf(error, "calloc(ticketData) error= %s\n", strerror(errno));
  	      return -1;
      }

      setGlobalDataPrototype(ticketData, mainPath);
      ticketData->fieldBMPPtr = NULL;

      set_I_O_Path(&img, mainPath, imgName);
      set_I_O_Path(&imgOut, mainPath, imgName);


//	         printf("\nreading image= %s from path= %s\n", img.szIMGName,img.szIMGPath );
 //       saveLog("\nreading image= ",img.szIMGName);
 //       saveLog("\n","\n");

        convertRGBToBMP(ptr, imgWidth, imgHeight, &img);
//        saveBMPbuffer (img.ptr, img.szIMGName, img.szWorkPath);
//        return 0;


        pre_ProcessColorImageBMP(&img, &imgOut);
//    	pre_ProcessColorImageBMP(&img, &imgOut);

 //    	  saveLog("blackMarginRemoval", "\n");
 // Black margin removal
        blackMarginRemoval(&imgOut, ticketData->fieldAddress->name);
 //  	         saveBMP (&imgOut);
         saveBMPbuffer (imgOut.ptr, imgOut.szIMGName, imgOut.szWorkPath);
 //imgIndx++;
 //continue;

 //------------------------------------------
   	         LoadPrototypes (ticketData);
 //------------------------------------------


    	         area.wOriginX = 0;area.wOriginY = 0;
    	         area.wWidth = imgOut.bi.biWidth;area.wHeight = imgOut.bi.biHeight;
    	         ver = 0;

 //printf("-------init\n");
              ok = ticketRecognitionInit(&imgOut, &radonStruct, &area, ver, &workAreaPtr,  &iDStructPtr,
                                 &angle, &ticketData->phi);
              ticketData->zone->ZoneQuad =   *(LPQUADRANGLE)(workAreaPtr.ptr + 8);
//  saveLog(" ticketRecognitionInit  ", "\n");
 //printf("-------ticketRecognitionInit; workAreaPtr.size= %ld\n", workAreaPtr.size);
              if(workAreaPtr.ptr != NULL) {
        	        free(workAreaPtr.ptr);
        	        workAreaPtr.size = 0;
              }
              ticketData->iAngle = angle;

              ok = IdentifyTicketCall (&radonStruct, &workAreaPtr, ticketData, iDStructPtr);
              if(ok < 0){
//             	 saveLog(" IdentifyTicketCall failed  ", "\n");
              break;
              }
              if(workAreaPtr.ptr != NULL) {
        	        free(workAreaPtr.ptr);
        	        workAreaPtr.size = 0;
              }

//         	 saveLog("reading prototype= ", ticketData->prototypeName);
//         	 saveLog("","\n");
 //        	 printf("-------reading prototype= %s\n", ticketData->prototypeName);

         	 ok = LoadPrototype (&ticketData, 0);
         	 if (strlen(ticketData->error) > 0 || ok < 0) {
         		 saveLog(" -------error= %s  ", ticketData->error);
         		 saveLog("\n","\n");
         	     printf("-------error= %s\n", ticketData->error);
         	     break;
         	 }


 //printf("-------angle= %d, phi.phiMax= %d, phi.phiMin= %d, phi.stepPhi= %d\n", angle, ticketData->phi.phiMax, ticketData->phi.phiMin, ticketData->phi.stepPhi);

 //printf("-------ticketRecognitionStruct\n");
     	 saveLog(" before ticketRecognitionStruct  ", "\n");
             ok = ticketRecognitionStruct(&imgOut, &radonStruct, ticketData->prototypePtr, &workAreaPtr, &ticketData->ptrTicketStruct,
     		                             angle, ticketData->tgp);

             sprintf(buffLog,"ok = %d\n",ok);
             saveLog("", buffLog);
             if(ok < 0 || workAreaPtr.size == 0)
             	goto SKIP_REC;
             ticketData->zone->ZoneQuad =   *(LPQUADRANGLE)(workAreaPtr.ptr + 8);
             ticketData->workArea = workAreaPtr;
  saveLog(" ticketRecognitionStruct  ", "\n");
 //printf("-------ticketRecognitionStruct; ticketData->workArea.size= %ld\n", ticketData->workArea.size);

             if(ok < 0){
 //printf("-------break; ok= %d\n", ok);
             break;
             }
             initLib (ticketData);
 //printf("-------ProcessField= %d\n", ok);
   saveLog("before ProcessField\n", "");
             ok = ProcessField ( &imgOut, ticketData, &radonStruct, &ticketResults);
 //    saveLog(" ProcessField  ", "\n");
 // if(imgIndx == 2)
 //   	break;

             if(ok < 0){
 //printf("-------break; return= %d\n", ok);
             break;
             }
 // postprocessing
             ticketPostProcessing(ticketData, &ticketResults);

             copyOutputResults(lpTicketResults, &ticketResults);
 //            printResult(ticketData);

  SKIP_REC:
             ReleasePrototypes (ticketData);

             if(ticketData->ptrTicketStruct)
             	free(ticketData->ptrTicketStruct);
             if(iDStructPtr)
             	free(iDStructPtr);
             ClearZones (ticketData);
             if(ticketData->fieldBMPPtr!= NULL)
                free(ticketData->fieldBMPPtr);

              ticketData->fieldBMPPtr= NULL; // ????????????????

             if(ticketData->workArea.ptr != NULL && ticketData->workArea.size> 0) {
       	     free(ticketData->workArea.ptr);
       	     ticketData->workArea.ptr = NULL;
       	     ticketData->workArea.size = 0;
             }
             if(ticketData->prototypePtr != NULL) {
       	     free(ticketData->prototypePtr);
       	     ticketData->prototypePtr = NULL;
             }

             free(ticketData);
             ticketData = NULL;
             if(img.ptr != NULL){
       	      free(img.ptr);
       	      img.ptr = NULL;
             }
             if(imgOut.ptr != NULL) {
       	     free(imgOut.ptr);
       	     imgOut.ptr = NULL;
             }
             break;
        }
   return 0;
   }


   int BMP_OCR_Call(unsigned char *ptr, int imgWidth, int imgHeight, char *mainPath,
		            LPTICKETRESULTS lpTicketResults, char *error, char *imgName){

	   IMGDATA img;
	   IMGDATA imgOut;
	   LPTICKETDATA ticketData;
	   TICKETRESULTS ticketResults;

	   int ok;
	   RECTANGLE area;
	   int ver;
	   HLPBYTE workAreaPtr;
	   LPBYTE iDStructPtr;
	   int  angle;
	   RADON radonStruct;
	   char buffLog[500];

//	   IMGDATA img;
//       int ok = 0;
       int bufforSize;
       int biWidthPad = ((imgWidth*24+31)/32)*4;
       int offBits = sizeof (BITMAPFILEHEADER) +
                 sizeof (BITMAPINFOHEADER);
    //            + sizeof (RGBQUAD) * 2;

       bufforSize = biWidthPad*imgHeight+offBits;

//  	  char globalPath[500];

// 	   lpImgOut->ptr = NULL;

	   img.ptr = ptr;
	   img.bf.bfType = 0x4d42;
	   img.bf.bfSize = bufforSize;
	   img.bf.bfReserved1 =  0;
	   img.bf.bfReserved2 =  0;
	   img.bf.bfOffBits =   offBits;
	   img.pixPtr = ptr +img.bf.bfOffBits;

	    img.bi.biSize = offBits - 14;
	    img.width = imgWidth;
	    img.bi.biWidth = img.width;
	    img.height = imgHeight;
	    img.bi.biHeight = img.height;
	    img.bi.biPlanes = 1;
	    img.bi.biBitCount = 24;
	    img.bi.biCompression = 0;
	    img.bi.biSizeImage = bufforSize - offBits;
	    img.bi.biXPelsPerMeter = 500;
	    img.bi.biYPelsPerMeter = 500;
	    img.bi.biClrUsed = 0;
	    img.bi.biClrImportant = 0;
	    img.bi.biWidthPad = ((img.bi.biWidth*img.bi.biBitCount+31)/32)*4;
//		    img->colorPtr = ptr + 14 + img->bi.biSize;


//        pre_ProcessColorImageBMP(&img, lpImgOut);
//        blackMarginRemoval(lpImgOut, "");






while(1){
	   initTicketResults(lpTicketResults);
	   initTicketResults(&ticketResults);
//	   lpTicketResults->yShift = 0;
//	   ticketResults.yShift = 0;
 	   img.ptr = NULL;
 	   imgOut.ptr = NULL;
 	   ticketData = (LPTICKETDATA)calloc(sizeof(TICKETDATA), sizeof(BYTE));
 	   if(ticketData == NULL) {
 		  sprintf(error, "calloc(ticketData) error= %s\n", strerror(errno));
  	      return -1;
      }

      setGlobalDataPrototype(ticketData, mainPath);
      ticketData->fieldBMPPtr = NULL;

      set_I_O_Path(&img, mainPath, imgName);
      set_I_O_Path(&imgOut, mainPath, imgName);



//        convertRGBToBMP(ptr, imgWidth, imgHeight, &img);


        pre_ProcessColorImageBMP(&img, &imgOut);
//    	pre_ProcessColorImageBMP(&img, &imgOut);

 //    	  saveLog("blackMarginRemoval", "\n");
 // Black margin removal
        blackMarginRemoval(&imgOut, ticketData->fieldAddress->name);
 //  	         saveBMP (&imgOut);
         saveBMPbuffer (imgOut.ptr, imgOut.szIMGName, imgOut.szWorkPath);
 //imgIndx++;
 //continue;

 //------------------------------------------
   	         LoadPrototypes (ticketData);
 //------------------------------------------


    	         area.wOriginX = 0;area.wOriginY = 0;
    	         area.wWidth = imgOut.bi.biWidth;area.wHeight = imgOut.bi.biHeight;
    	         ver = 0;

 //printf("-------init\n");
              ok = ticketRecognitionInit(&imgOut, &radonStruct, &area, ver, &workAreaPtr,  &iDStructPtr,
                                 &angle, &ticketData->phi);
              ticketData->zone->ZoneQuad =   *(LPQUADRANGLE)(workAreaPtr.ptr + 8);
//  saveLog(" ticketRecognitionInit  ", "\n");
 //printf("-------ticketRecognitionInit; workAreaPtr.size= %ld\n", workAreaPtr.size);
              if(workAreaPtr.ptr != NULL) {
        	        free(workAreaPtr.ptr);
        	        workAreaPtr.size = 0;
              }
              ticketData->iAngle = angle;

              ok = IdentifyTicketCall (&radonStruct, &workAreaPtr, ticketData, iDStructPtr);
              if(ok < 0){
//             	 saveLog(" IdentifyTicketCall failed  ", "\n");
              break;
              }
              if(workAreaPtr.ptr != NULL) {
        	        free(workAreaPtr.ptr);
        	        workAreaPtr.size = 0;
              }

//         	 saveLog("reading prototype= ", ticketData->prototypeName);
//         	 saveLog("","\n");
 //        	 printf("-------reading prototype= %s\n", ticketData->prototypeName);

         	 ok = LoadPrototype (&ticketData, 0);
         	 if (strlen(ticketData->error) > 0 || ok < 0) {
         		 saveLog(" -------error= %s  ", ticketData->error);
         		 saveLog("\n","\n");
         	     printf("-------error= %s\n", ticketData->error);
         	     break;
         	 }


 //printf("-------angle= %d, phi.phiMax= %d, phi.phiMin= %d, phi.stepPhi= %d\n", angle, ticketData->phi.phiMax, ticketData->phi.phiMin, ticketData->phi.stepPhi);

 //printf("-------ticketRecognitionStruct\n");
     	 saveLog(" before ticketRecognitionStruct  ", "\n");
             ok = ticketRecognitionStruct(&imgOut, &radonStruct, ticketData->prototypePtr, &workAreaPtr, &ticketData->ptrTicketStruct,
     		                             angle, ticketData->tgp);

             sprintf(buffLog,"ok = %d\n",ok);
             saveLog("", buffLog);
             if(ok < 0 || workAreaPtr.size == 0)
             	goto SKIP_REC;
             ticketData->zone->ZoneQuad =   *(LPQUADRANGLE)(workAreaPtr.ptr + 8);
             ticketData->workArea = workAreaPtr;
  saveLog(" ticketRecognitionStruct  ", "\n");
 //printf("-------ticketRecognitionStruct; ticketData->workArea.size= %ld\n", ticketData->workArea.size);

             if(ok < 0){
 //printf("-------break; ok= %d\n", ok);
             break;
             }
             initLib (ticketData);
 //printf("-------ProcessField= %d\n", ok);
   saveLog("before ProcessField\n", "");
             ok = ProcessField ( &imgOut, ticketData, &radonStruct, &ticketResults);
 //    saveLog(" ProcessField  ", "\n");
 // if(imgIndx == 2)
 //   	break;

             if(ok < 0){
 //printf("-------break; return= %d\n", ok);
             break;
             }
 // postprocessing
             ticketPostProcessing(ticketData, &ticketResults);

             copyOutputResults(lpTicketResults, &ticketResults);
 //            printResult(ticketData);

  SKIP_REC:
             ReleasePrototypes (ticketData);

             if(ticketData->ptrTicketStruct)
             	free(ticketData->ptrTicketStruct);
             if(iDStructPtr)
             	free(iDStructPtr);
             ClearZones (ticketData);
             if(ticketData->fieldBMPPtr!= NULL)
                free(ticketData->fieldBMPPtr);

              ticketData->fieldBMPPtr= NULL; // ????????????????

             if(ticketData->workArea.ptr != NULL && ticketData->workArea.size> 0) {
       	     free(ticketData->workArea.ptr);
       	     ticketData->workArea.ptr = NULL;
       	     ticketData->workArea.size = 0;
             }
             if(ticketData->prototypePtr != NULL) {
       	     free(ticketData->prototypePtr);
       	     ticketData->prototypePtr = NULL;
             }

             free(ticketData);
             ticketData = NULL;
             if(img.ptr != NULL){
       	      free(img.ptr);
       	      img.ptr = NULL;
             }
             if(imgOut.ptr != NULL) {
       	     free(imgOut.ptr);
       	     imgOut.ptr = NULL;
             }
             break;
        }
   return 0;
   }


   int RGB_OCR_FirstStepCall(unsigned char *ptr, int imgWidth, int imgHeight, LPIMGDATA lpImgOut, char *error){
	   IMGDATA img;

       int ok = 0;
//  	  char globalPath[500];

 	   img.ptr = NULL;
 	   lpImgOut->ptr = NULL;




        convertRGBToBMP(ptr, imgWidth, imgHeight, &img);
//   printf("-------img.szIMGName= %s, img.szWorkPath= %s\n", img.szIMGName, img.szWorkPath);

//        saveBMPbuffer (img.ptr, img.szIMGName, img.szWorkPath);


        pre_ProcessColorImageBMP(&img, lpImgOut);

 //    	  saveLog("blackMarginRemoval", "\n");
 // Black margin removal
        blackMarginRemoval(lpImgOut, "");

//         saveBMPbuffer (imgOut.ptr, imgOut.szIMGName, imgOut.szWorkPath);
//         break;
    return ok;
}

   int RGB_OCR_ServerCall(LPIMGDATA lpImgOut, char *mainPath,
		            LPTICKETRESULTS lpTicketResults, char *error, char *imgName){

	   LPTICKETDATA ticketData;
	   TICKETRESULTS ticketResults;

	   int ok = 0;
	   RECTANGLE area;
	   int ver;
	   HLPBYTE workAreaPtr;
	   LPBYTE iDStructPtr;
	   int  angle;
	   RADON radonStruct;
	   char buffLog[500];

while(1){
	   initTicketResults(lpTicketResults);
	   initTicketResults(&ticketResults);
 	   if(lpImgOut->ptr == NULL){
  		  sprintf(error, "image buffer = NULL\n");
   	      return -1;
 	   }
 	   ticketData = (LPTICKETDATA)calloc(sizeof(TICKETDATA), sizeof(BYTE));
 	   if(ticketData == NULL) {
 		  sprintf(error, "calloc(ticketData) error= %s\n", strerror(errno));
  	      return -1;
      }

      setGlobalDataPrototype(ticketData, mainPath);
      ticketData->fieldBMPPtr = NULL;

//      set_I_O_Path(&img, mainPath,  "0");
      set_I_O_Path(lpImgOut, mainPath,  imgName);

//	         printf("\nreading image= %s from path= %s\n", img.szIMGName,img.szIMGPath );
 //       saveLog("\nreading image= ",img.szIMGName);
 //       saveLog("\n","\n");

      saveBMPbuffer (lpImgOut->ptr, lpImgOut->szIMGName, lpImgOut->szWorkPath);

 //------------------------------------------
   	         LoadPrototypes (ticketData);
 //------------------------------------------


    	         area.wOriginX = 0;area.wOriginY = 0;
    	         area.wWidth = lpImgOut->bi.biWidth;area.wHeight = lpImgOut->bi.biHeight;
    	         ver = 0;

 //printf("-------init\n");
              ok = ticketRecognitionInit(lpImgOut, &radonStruct, &area, ver, &workAreaPtr,  &iDStructPtr,
                                 &angle, &ticketData->phi);
              ticketData->zone->ZoneQuad =   *(LPQUADRANGLE)(workAreaPtr.ptr + 8);
//  saveLog(" ticketRecognitionInit  ", "\n");
 //printf("-------ticketRecognitionInit; workAreaPtr.size= %ld\n", workAreaPtr.size);
              if(workAreaPtr.ptr != NULL) {
        	        free(workAreaPtr.ptr);
        	        workAreaPtr.size = 0;
              }
              ticketData->iAngle = angle;

              ok = IdentifyTicketCall (&radonStruct, &workAreaPtr, ticketData, iDStructPtr);
              if(ok < 0){
//             	 saveLog(" IdentifyTicketCall failed  ", "\n");
              break;
              }
              if(workAreaPtr.ptr != NULL) {
        	        free(workAreaPtr.ptr);
        	        workAreaPtr.size = 0;
              }

//         	 saveLog("reading prototype= ", ticketData->prototypeName);
//         	 saveLog("","\n");
 //        	 printf("-------reading prototype= %s\n", ticketData->prototypeName);

         	 ok = LoadPrototype (&ticketData, 0);
         	 if (strlen(ticketData->error) > 0 || ok < 0) {
         		 saveLog(" -------error= %s  ", ticketData->error);
         		 saveLog("\n","\n");
         	     printf("-------error= %s\n", ticketData->error);
         	     break;
         	 }


 //printf("-------angle= %d, phi.phiMax= %d, phi.phiMin= %d, phi.stepPhi= %d\n", angle, ticketData->phi.phiMax, ticketData->phi.phiMin, ticketData->phi.stepPhi);

 //printf("-------ticketRecognitionStruct\n");
     	 saveLog(" before ticketRecognitionStruct  ", "\n");
             ok = ticketRecognitionStruct(lpImgOut, &radonStruct, ticketData->prototypePtr, &workAreaPtr, &ticketData->ptrTicketStruct,
     		                             angle, ticketData->tgp);

             sprintf(buffLog,"ok = %d\n",ok);
             saveLog("", buffLog);
             if(ok < 0 || workAreaPtr.size == 0)
             	goto SKIP_REC;
             ticketData->zone->ZoneQuad =   *(LPQUADRANGLE)(workAreaPtr.ptr + 8);
             ticketData->workArea = workAreaPtr;
  saveLog(" ticketRecognitionStruct  ", "\n");
 //printf("-------ticketRecognitionStruct; ticketData->workArea.size= %ld\n", ticketData->workArea.size);

             if(ok < 0){
 //printf("-------break; ok= %d\n", ok);
             break;
             }
             initLib (ticketData);
 //printf("-------ProcessField= %d\n", ok);
   saveLog("before ProcessField\n", "");
             ok = ProcessField ( lpImgOut, ticketData, &radonStruct, &ticketResults);
 //    saveLog(" ProcessField  ", "\n");
 // if(imgIndx == 2)
 //   	break;

             if(ok < 0){
 //printf("-------break; return= %d\n", ok);
             break;
             }
 // postprocessing
             ticketPostProcessing(ticketData, &ticketResults);

             copyOutputResults(lpTicketResults, &ticketResults);
 //            printResult(ticketData);

  SKIP_REC:
             ReleasePrototypes (ticketData);

             if(ticketData->ptrTicketStruct)
             	free(ticketData->ptrTicketStruct);
             if(iDStructPtr)
             	free(iDStructPtr);
             ClearZones (ticketData);
             if(ticketData->fieldBMPPtr!= NULL)
                free(ticketData->fieldBMPPtr);

              ticketData->fieldBMPPtr= NULL; // ????????????????

             if(ticketData->workArea.ptr != NULL && ticketData->workArea.size> 0) {
       	     free(ticketData->workArea.ptr);
       	     ticketData->workArea.ptr = NULL;
       	     ticketData->workArea.size = 0;
             }
             if(ticketData->prototypePtr != NULL) {
       	     free(ticketData->prototypePtr);
       	     ticketData->prototypePtr = NULL;
             }

             free(ticketData);
             ticketData = NULL;

             break;
        }
   return 0;
   }
   int BMP_OCR_FirstStepCall(unsigned char *ptr, int imgWidth, int imgHeight, LPIMGDATA lpImgOut, char *error){
	   IMGDATA img;
       int ok = 0;
       int bufforSize;
       int biWidthPad = ((imgWidth*24+31)/32)*4;
       int offBits = sizeof (BITMAPFILEHEADER) +
                 sizeof (BITMAPINFOHEADER);
    //            + sizeof (RGBQUAD) * 2;

       bufforSize = biWidthPad*imgHeight+offBits;

//  	  char globalPath[500];

 	   lpImgOut->ptr = NULL;

	   img.ptr = ptr;
	   img.bf.bfType = 0x4d42;
	   img.bf.bfSize = bufforSize;
	   img.bf.bfReserved1 =  0;
	   img.bf.bfReserved2 =  0;
	   img.bf.bfOffBits =   offBits;
	   img.pixPtr = ptr +img.bf.bfOffBits;

	    img.bi.biSize = offBits - 14;
	    img.width = imgWidth;
	    img.bi.biWidth = img.width;
	    img.height = imgHeight;
	    img.bi.biHeight = img.height;
	    img.bi.biPlanes = 1;
	    img.bi.biBitCount = 24;
	    img.bi.biCompression = 0;
	    img.bi.biSizeImage = bufforSize - offBits;
	    img.bi.biXPelsPerMeter = 500;
	    img.bi.biYPelsPerMeter = 500;
	    img.bi.biClrUsed = 0;
	    img.bi.biClrImportant = 0;
	    img.bi.biWidthPad = ((img.bi.biWidth*img.bi.biBitCount+31)/32)*4;
//		    img->colorPtr = ptr + 14 + img->bi.biSize;


        pre_ProcessColorImageBMP(&img, lpImgOut);
        blackMarginRemoval(lpImgOut, "");

    return ok;
}

 BYTE getPixelCompress(LPBYTE ptr, int xn, int yn, int wPad, int height){
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

 void setColorArray(LPBYTE ptrInp, int x, int color){
	    BYTE mask;
	    LPBYTE ptr;

	         ptr = ptrInp+x/8;
	         mask=LEFT_BIT;
	         mask >>=(x%8);
	         if(color!=0){
	        	 *ptr = *ptr | mask;
	         }
	         else {
	        	 *ptr = *ptr & (~mask);
	         }
 }

   int compressPixels(LPBYTE ptrO, LPBYTE ptrI, int imgWidth, int imgHeight,
		   LPBYTE ptrOutColors, int *colorSize){
	   int biWidthPad = ((imgWidth+31)/32)*4;
	   int x, y, num, size, sizeC;
	   BYTE color, colorC;
//       char buff[500];

	    sizeC = 0;
	    color = getPixelCompress(ptrI, 0, 0, biWidthPad, imgHeight);

//	    setColorArray(ptrOutColors, sizeC, color);sizeC++;

	    size = 0;
	    num = 0;
	    colorC = color;
	    for(y=0;y<imgHeight;y++){
            for(x=0;x<imgWidth;x++){
              	color = getPixelCompress(ptrI, x, y, biWidthPad, imgHeight);
            	if(color != colorC || num >= 255){

//     sprintf(buff,"-------color= %d, colorC= %d, num= %d\n", color, colorC, num);
//     saveLog("", buff);

            		ptrO[size] = num;size++;
            		setColorArray(ptrOutColors, sizeC, colorC);sizeC++;
            	    num = 1;
            	    colorC = color;
            	}else{
            		num++;
            	}
            }
        }
	    if(num > 0) {
    		ptrO[size] = num-1;size++;
    	    num = 1;
	    }
	    *colorSize = sizeC;
        return size;
   }
   void copyColorsToPixels(LPBYTE ptrT, LPBYTE ptrOutColors, int colorSize){
	   int i, x;
	   x = colorSize/8+1;
	   for(i=0;i<x;i++){
		   ptrT[i] = ptrOutColors[i];

	   }
   }
   LPBYTE compressImage(LPIMGDATA img){
      LPBYTE ptrOut = NULL;
      LPBYTE ptrOutColors = NULL;
      LPBYTE ptrT = NULL;
      int bufforSize, bufforSizeC, size;
      int offBits = 28;
      int  colorSize;

//      char buff[500];

      bufforSize = img->bi.biWidth*img->bi.biHeight+offBits;
      bufforSizeC = (img->bi.biWidth/8)*img->bi.biHeight+offBits;

      ptrOut = calloc(bufforSize, sizeof(BYTE));
      if(ptrOut == NULL) {
      	    sprintf(img->error,"compressImage ( \"%s\" ) calloc ( %d ) failed: %s\n",img->szIMGName, bufforSize, strerror(errno));
      	    return ptrOut;
      }
      ptrOutColors = calloc(bufforSizeC, sizeof(BYTE));
      if(ptrOutColors == NULL) {
      	    sprintf(img->error,"compressImage ( \"%s\" ) calloc ( %d ) failed: %s\n",img->szIMGName, bufforSize, strerror(errno));
      	    return ptrOut;
      }

// pixels
      ptrT = ptrOut+offBits;
      size = compressPixels(ptrT, img->pixPtr, img->bi.biWidth, img->bi.biHeight, ptrOutColors, &colorSize);
// header
      ptrT = ptrOut;
      set4Bytes(ptrT, size+offBits);ptrT += 4;
      set4Bytes(ptrT, colorSize);ptrT += 4;
      *ptrT = offBits;ptrT++;
      set4Bytes(ptrT, img->bi.biWidth);ptrT += 4;
      set4Bytes(ptrT, img->bi.biHeight);ptrT += 4;
      set4Bytes(ptrT, img->bi.biXPelsPerMeter);ptrT += 4;
      set4Bytes(ptrT, img->bi.biYPelsPerMeter);ptrT += 4;
      ptrT = ptrOut+offBits+size;


      copyColorsToPixels(ptrT, ptrOutColors, colorSize);
      if(ptrOutColors != NULL) free(ptrOutColors);

//      *ptrT = firstColor;

/*
      printf("compress-------sizeInp= %d, offBitsInp= %d\n", size+offBits, offBits);
      printf("-------imgWidth= %d, imgHeight= %d\n", img->bi.biWidth, img->bi.biHeight);
      printf("-------xDpi= %d, yDpi= %d\n", img->bi.biXPelsPerMeter, img->bi.biYPelsPerMeter);
      printf("-------firstColor= %d\n", colorSize);

      sprintf(buff,"compress-------sizeInp= %d, offBitsInp= %d\n", size+offBits, offBits);
      saveLog("", buff);
      sprintf(buff,"-------imgWidth= %d, imgHeight= %d\n", img->bi.biWidth, img->bi.biHeight);
      saveLog("", buff);
      sprintf(buff,"-------xDpi= %d, yDpi= %d\n", img->bi.biXPelsPerMeter, img->bi.biYPelsPerMeter);
      saveLog("", buff);
      sprintf(buff,"-------colorSize= %d\n", colorSize);
      saveLog("", buff);

   printf("-------size= %d\n", size);
   sprintf(buff,"-------size= %d\n", size);
   saveLog("", buff);
*/
      return ptrOut;
  }
  void setPixelDecompress(LPBYTE ptr,  int wPad, int height, int x, int y, int color){
 	    long nY;
 	    long nX;
 	    BYTE mask;
 	    LPBYTE ptrP;

 	        y=height-1-y;
 	        nY=y*wPad;
   	        nX = (long)(x/8);
             ptrP = ptr+nY+nX;

	         mask=LEFT_BIT;
	         mask >>=(x%8);
	         if(color!=0){
	        	 *ptrP = *ptrP | mask;
	         }
	         else {
	        	 *ptrP = *ptrP & (~mask);
	         }

  }
  void getNextXY(int imgWidth, int imgHeight, int *x, int *y){
	  int xx = *x;
	  int yy = *y;

 	   xx++;
 	   if(xx >= imgWidth) {xx = 0;yy++;}
 	   *x = xx;*y = yy;
  }
 void setOutPixels(LPBYTE ptrOut, int imgWidth, int imgHeight, int num, int color, int *x, int *y){
		 int i;
		 int wPad;
		 wPad = ((imgWidth+31)/32)*4;
		 for(i=0;i<num;i++){
			 getNextXY(imgWidth, imgHeight, x, y);
			 setPixelDecompress(ptrOut, wPad, imgHeight, *x, *y, color);
		 }

	 }
 int getColorC(LPBYTE ptrColor, int idxC){
	    BYTE mask;
	    LPBYTE ptr;
	    BYTE b;

             ptr = ptrColor+idxC/8;
	         mask=LEFT_BIT;
	         mask >>=(idxC%8);

	         b=(~(*ptr))&mask;
	         if(b!=0) {
	               return(0);
	               }
	         else {
	               return(1);
	               }

 }
   int deCompressPixels(LPBYTE ptrOut, LPBYTE ptrInp, int imgWidth, int imgHeight, int sizeInp, LPBYTE ptrColor, int colorSize){
         int size = 0;
         int x, y, idxC;
         int i, num, color;
//         char buff[500];

         x = 0, y = 0;
         idxC = 0;
         for(i=0;i<sizeInp;i++){
        	 color = getColorC(ptrColor, idxC);
        	 num = ptrInp[i];

//      sprintf(buff,"i= %d, num = %d, x= %d, y=%d, color= %d\n", i, num , x, y, color);
//       saveLog("", buff);

        	 setOutPixels(ptrOut, imgWidth, imgHeight, num, color, &x, &y);
        	 idxC++;
         }

	     return size;
   }

   int deCompressImage(LPBYTE ptrC, LPIMGDATA img){
      LPBYTE ptrOut = NULL;
      LPBYTE ptrT = NULL;
      LPBYTE ptrColor = NULL;
      int bufforSize, sizeInp, sizeOut;
      int imgWidth, imgHeight, biWidthPad;
      int xDpi, yDpi;
      int offBitsInp, colorSize;
//      BYTE firstColor;
      LPRGBQUAD          qd;

      LPBITMAPFILEHEADER bf;
      LPBITMAPINFOHEADER bi;
      int offBits = sizeof (BITMAPFILEHEADER) +
                sizeof (BITMAPINFOHEADER)
               + sizeof (RGBQUAD) * 2;

//      char buff[500];


      ptrT = ptrC;
      sizeInp = getLeftBytes((int*)ptrT, 4);ptrT += 4;
      colorSize = getLeftBytes((int*)ptrT, 4);ptrT += 4;
      offBitsInp = *ptrT; ptrT++;
      imgWidth = getLeftBytes((int*)ptrT, 4);ptrT += 4;
      imgHeight = getLeftBytes((int*)ptrT, 4);ptrT += 4;
      xDpi = getLeftBytes((int*)ptrT, 4);ptrT += 4;
      yDpi = getLeftBytes((int*)ptrT, 4);ptrT += 4;
      ptrColor = ptrC+sizeInp;


/*
     printf("decompress-------sizeInp= %d, offBitsInp= %d\n", sizeInp, offBitsInp);
     printf("-------imgWidth= %d, imgHeight= %d\n", imgWidth, imgHeight);
     printf("-------xDpi= %d, yDpi= %d\n", xDpi, yDpi);
     printf("-------colorSize= %d\n", colorSize);

     sprintf(buff,"decompress-------sizeInp= %d, offBitsInp= %d\n", sizeInp, offBitsInp);
     saveLog("", buff);
     sprintf(buff,"-------imgWidth= %d, imgHeight= %d\n", imgWidth, imgHeight);
     saveLog("", buff);
     sprintf(buff,"-------xDpi= %d, yDpi= %d\n", xDpi, yDpi);
     saveLog("", buff);
     sprintf(buff,"-------colorSize= %d\n", colorSize);
     saveLog("", buff);
*/


      biWidthPad = ((imgWidth+31)/32)*4;
      bufforSize = biWidthPad*imgHeight+offBits;

      ptrOut = calloc(bufforSize, sizeof(BYTE));
      if(ptrOut == NULL) {
       	    printf(img->error,"deCompressImage ( \"%s\" ) calloc ( %d ) failed: %s\n",img->szIMGName, bufforSize, strerror(errno));
       	    return -1;
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
 	  set4Bytes(bi->biXPelsPerMeter, xDpi);
	  set4Bytes(bi->biYPelsPerMeter, yDpi);
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
	  img->bi.biXPelsPerMeter = xDpi;
	  img->bi.biYPelsPerMeter = yDpi;
      img->bi.biClrUsed = 0;
	  img->bi.biClrImportant = 0;
	  img->bi.biWidthPad = ((img->bi.biWidth*img->bi.biBitCount+31)/32)*4;

      ptrT = ptrOut+offBits;
      sizeOut = deCompressPixels(ptrT, ptrC+offBitsInp, img->bi.biWidth, img->bi.biHeight, sizeInp-offBitsInp, ptrColor, colorSize);


//     printf("-------sizeOut= %d\n", sizeOut);
      return 0;
  }
