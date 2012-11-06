/*
 * main.c
 *
 *  Created on: Jul 24, 2012
 *      Author: Wicek
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include  "../headers/Image.h"
#include  "../headers/Prototype.h"
#include  "../headers/Functions.h"

int waitForInput();
void setGlobalDataPrototype(LPTICKETDATA ticketDataPtr, char *mainPath);
void setGlobalDataImgIdx(LPIMGDATA img, int idx, int inp, int mode);
void setGlobalDataImg(LPIMGDATA img);
void setGlobalDataImg2(LPIMGDATA img);
void setGlobalDataImg3(LPIMGDATA img);
void saveLog(char *str1, char *str2);
int RGB_OCR_Call(unsigned char *ptr, int imgWidth, int imgHeight, char *mainPath,
		            LPTICKETRESULTS lpTicketResults, char *error, char *imgName);
int BMP_OCR_Call(unsigned char *ptr, int imgWidth, int imgHeight, char *mainPath,
		            LPTICKETRESULTS lpTicketResults, char *error, char *imgName);

int RGB_OCR_FirstStepCall(unsigned char *ptr, int imgWidth, int imgHeight, LPIMGDATA lpImgOut, char *error);
int BMP_OCR_FirstStepCall(unsigned char *ptr, int imgWidth, int imgHeight, LPIMGDATA lpImgOut, char *error);
int RGB_OCR_ServerCall(LPIMGDATA lpImgOut, char *mainPath,
		            LPTICKETRESULTS lpTicketResults, char *error, char *imgName);

LPBYTE compressImage(LPIMGDATA img);
int deCompressImage(LPBYTE ptrC, LPIMGDATA img);

void testRGBA_RGB(LPIMGDATA img);
void PrintOutputResults(LPTICKETRESULTS ticketResults);
void copyOutputResults(LPTICKETRESULTS lpTicketResults, LPTICKETRESULTS ticketResultsInp);
void initTicketResults(LPTICKETRESULTS lpTicketResults);

//void addBottomPart(LPIMGDATA img, int dd);

IMGDATA img;
IMGDATA imgOut;
LPTICKETDATA ticketData;
int imgIndx = 0;
int countImgGap = 0;

int main(int argc, char **argv) {
int inp;
LPBYTE buff = NULL;
int ok;
RECTANGLE area;
int ver;
HLPBYTE workAreaPtr;
LPBYTE iDStructPtr;
//LPBYTE ticketStruct;
int  angle;
RADON radonStruct;
char globalPath[500];
char buffLog[500];
//strcpy(globalPath, "c:\\Users\\Wicek\\My Documents\\GitHub\\OCR\\");
strcpy(globalPath, "c:\\Users\\Wicek\\YoolottoVin\\");
//extern int charIdxTmp;


  printf("Start program...\n");
  inp = waitForInput() ;

  printf("\nuser input= %d \n", inp);

  if(inp == 1) {
	  setGlobalDataImg2(&img);
	  setGlobalDataImg3(&imgOut);
	  img.ptr = NULL;
	  imgOut.ptr = NULL;
	  printf("\nreading image= %s from path= %s\n", img.szIMGName,img.szIMGPath );

	  convertInp24BMPtoMonochrome(&img, &imgOut);
/*
//	  buff = LoadBMPFile (&img);
	  img.width = 1936;
	  img.height = 2592;
	  img.dpiX = 512;
	  img.dpiY = 512;
	  buff = LoadRGBAPFile (&img);
	  ok = 1;
	  if (buff == NULL) {
		printf("buff = null \n");
		ok = 0;
	  }

	  if (strlen(img.error) > 0) {
		printf("-------error= %s\n", img.error);
		ok = 0;
	  }
      if(ok){
    	  printf("\n load successful \n");
//    	  printf(" biSizeImage= %ld \n", img.bi.biSizeImage);
    	  convertRGBAcolor_to_RGBAwb(img.pixPtr, img.width, img.height);
//    	  saveRGBAasBMP(img.pixPtr, img.width, img.height, &img);
//    	   testRGBAcolors (&img);
    	   convertRGBAToBMP(&img);

//    	  changeLines(&img, 0, 200, 255);
//    	  convertToWB(&img, 150);
    	  convertBMPTo_mono_BMP(&img, &imgOut, 120);
//   	   printBMPData (&imgOut);
    	  convertDPI_in_BMP(&imgOut, 300, 300);


    	   printBMPData (&imgOut);
//    	   printBMPData (&img);

//    	   convertRGBATo_mono_BMP(&img, 120);
//    	   convertRGBAToBMP_filterWB(&img, 120);
//    	  setColors(&img);
//     	  saveBMP (&img);
    	  saveBMP (&imgOut);
      }
*/
      if(img.ptr != NULL)
    	  free(img.ptr);
      if(imgOut.ptr != NULL)
    	  free(imgOut.ptr);

  }
  if(inp == 2 || inp == 3 || inp == 4 ) {
printf("\n-----------------------------------------------------------------Start\n");
saveLog("\n-----------------------------------------------------------------Start\n", NULL);
      imgIndx = 0;
      countImgGap = 0;
	  while(1) {
		  TICKETRESULTS ticketResults;
		  TICKETRESULTS ticketResultsOut;
		  initTicketResults(&ticketResults);
		  initTicketResults(&ticketResultsOut);

		  img.ptr = NULL;
		  imgOut.ptr = NULL;
	      ticketData = (LPTICKETDATA)calloc(sizeof(TICKETDATA), sizeof(BYTE));
	      if(ticketData == NULL) {
	  	     printf("-------ticketData, error= %s\n", strerror(errno));
	  	     break;
	      }

	         setGlobalDataPrototype(ticketData, globalPath);
	         ticketData->fieldBMPPtr = NULL;

	         setGlobalDataImgIdx(&img, imgIndx, inp, 0);
	         setGlobalDataImgIdx(&imgOut, imgIndx, inp, 1);
//	         printf("\nreading image= %s from path= %s\n", img.szIMGName,img.szIMGPath );
//       saveLog("\nreading image= ",img.szIMGName);
//       saveLog("\n","\n");
	         if(inp == 2 || inp == 4) {
	            buff = LoadBMPFile (&img);

//	            addBottomPart(&img, 200);
//  	            saveBMPbuffer (imgOut.ptr, imgOut.szIMGName, imgOut.szWorkPath);
//	            break;

	         }
	         if(inp == 3) {
		    	    img.width = 1936;
		    	    img.height = 2592;
		    	    img.dpiX = 512;
		    	    img.dpiY = 512;
		    	    buff = LoadRGBAPFile (&img);
	         }
             if (buff == NULL || strlen(img.error) > 0) {
//		         if (buff == NULL)
//		             printf("buff = null \n");
   	             if (strlen(img.error) > 0)
			         printf("-------error= %s\n", img.error);
   	             if(countImgGap > 10)
   	                 break;
   	             countImgGap++;
   	             imgIndx++;
   	             continue;
	         }
//	         else {
//    	         printf(" reading successful \n");
//	         }
             countImgGap = 0;

     printf("\nloaded image= %s from path= %s\n", img.szIMGName,img.szIMGPath );
     saveLog("\nloaded image= ",img.szIMGName);
     saveLog("\n","\n");

     if(inp == 2) {
       BMP_OCR_Call(img.pixPtr, img.width, img.height, globalPath,
       		     &ticketResults, img.error, img.szIMGName);

     }

     if(inp == 3) {
      RGB_OCR_Call(img.pixPtr, img.width, img.height, globalPath,
    		     &ticketResults, img.error, img.szIMGName);
     }
     PrintOutputResults(&ticketResults);
     goto SKIP_REC2;

      if(inp == 2) {
    	  BMP_OCR_FirstStepCall(img.pixPtr, img.width, img.height, &imgOut, img.error);
      }
      if(inp == 3) {
         testRGBA_RGB(&img);
         RGB_OCR_FirstStepCall(img.pixPtr, img.width, img.height, &imgOut, img.error);
      }
/*
      {LPBYTE compressedImg;
       IMGDATA imgDecompressed;
        compressedImg = compressImage(&imgOut);
        if(compressedImg != NULL) {

        	deCompressImage(compressedImg, &imgDecompressed);

        	free(compressedImg);
        	printf("free(compressedImg)\n");
        	if(imgDecompressed.ptr != NULL) {
   	      saveBMPbuffer (imgDecompressed.ptr, imgOut.szIMGName, imgOut.szWorkPath);
        		free(imgDecompressed.ptr);
            	printf("free(imgDecompressed.ptr)\n");
        	}else
        		printf("deCompressedImg = NULL\n");

        }else
        	printf("compressedImg = NULL\n");
      }

     goto SKIP_REC2;
*/
      saveBMPbuffer (imgOut.ptr, imgOut.szIMGName, imgOut.szWorkPath);
      RGB_OCR_ServerCall(&imgOut, globalPath,
    	   	     &ticketResults, imgOut.error, imgOut.szIMGName);
      ticketPostProcessing(ticketData, &ticketResults);
      copyOutputResults(&ticketResultsOut, &ticketResults);
      PrintOutputResults(&ticketResultsOut);


      goto SKIP_REC2;

//      printBMPData (&img);

//  printf(" reading successful \n");
//  saveLog(" reading successful  ", "\n");

// test for image pre-processing
//        pre_ProcessColorImageBMP(&img);


// reading prototypes
//	         printf("\nreading prototype= %s from path= %s\n", ticketData->prototypeName, ticketData->prototypePath );
//  saveLog("\nreading prototype\n", ticketData->prototypeName);
//  saveLog("\n","\n");
//	         ok = LoadPrototype (&ticketData);
//	         if (strlen(ticketData->error) > 0 || ok < 0) {
//		         printf("-------error= %s\n", ticketData->error);
//		         break;
//	         }
//             if(ok>=0){
//    	        printf("reading successful \n");
//       saveLog(" reading successful  ", "\n");
//             }

             if(inp == 2 || inp == 4) {
//    	        convert24BMPtoMonochromeIter(&img, &imgOut);
//    	  saveLog("pre_ProcessColorImageBMP", "\n");
            	pre_ProcessColorImageBMP(&img, &imgOut);
             }
             if(inp == 3) {
        	    convertRGBAtoMonochromeIter(&img, &imgOut);
             }
//  	  saveLog("saveBMPbuffer", "\n");
//  	  if(imgOut.ptr == NULL)
//  	  	  saveLog("imgOut.ptr == NULL", "\n");

//       printBMPSize (&imgOut);
//       if(imgOut.bi.biWidth > imgOut.bi.biHeight)
//    	   rotate(&imgOut, 90);
//       saveBMPbuffer (imgOut.ptr, imgOut.szIMGName, imgOut.szWorkPath);
//       imgIndx++;
//       continue;

//    	  saveLog("blackMarginRemoval", "\n");
// Black margin removal
	         if(inp != 4)
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
 saveLog(" ticketRecognitionInit  ", "\n");
//             printf("  ZoneQuad.p1X= %d, ZoneQuad.p1Y= %d \n", ticketData->zone->ZoneQuad.p1X, ticketData->zone->ZoneQuad.p1Y);
//             printf("  ZoneQuad.p2= %d, ZoneQuad.p2Y= %d \n", ticketData->zone->ZoneQuad.p2X, ticketData->zone->ZoneQuad.p2Y);
//             printf("  ZoneQuad.p3X= %d, ZoneQuad.p3Y= %d \n", ticketData->zone->ZoneQuad.p3X, ticketData->zone->ZoneQuad.p3Y);
//             printf("  ZoneQuad.p4X= %d, ZoneQuad.p4Y= %d \n", ticketData->zone->ZoneQuad.p4X, ticketData->zone->ZoneQuad.p4Y);

//printf("-------ticketRecognitionInit; workAreaPtr.size= %ld\n", workAreaPtr.size);
             if(workAreaPtr.ptr != NULL) {
       	        free(workAreaPtr.ptr);
       	        workAreaPtr.size = 0;
             }
             ticketData->iAngle = angle;

             ok = IdentifyTicketCall (&radonStruct, &workAreaPtr, ticketData, iDStructPtr);
             if(ok < 0){
            	 saveLog(" IdentifyTicketCall failed  ", "\n");
             break;
             }
             if(workAreaPtr.ptr != NULL) {
       	        free(workAreaPtr.ptr);
       	        workAreaPtr.size = 0;
             }

        	 saveLog("reading prototype= ", ticketData->prototypeName);
        	 saveLog("","\n");
//        	 printf("-------reading prototype= %s\n", ticketData->prototypeName);

        	 ok = LoadPrototype (&ticketData, 0);
        	 if (strlen(ticketData->error) > 0 || ok < 0) {
        		 saveLog(" -------error= %s  ", ticketData->error);
        		 saveLog("\n","\n");
        	     printf("-------error= %s\n", ticketData->error);
        	     break;
        	 }
//        	 if(ok>=0){
//        	      printf("reading successful \n");
//        	      saveLog(" reading successful  ", "\n");
//      	     }


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
//            printf("  ZoneQuad.p1X= %d, ZoneQuad.p1Y= %d \n", ticketData->zone->ZoneQuad.p1X, ticketData->zone->ZoneQuad.p1Y);
//            printf("  ZoneQuad.p2X= %d, ZoneQuad.p2Y= %d \n", ticketData->zone->ZoneQuad.p2X, ticketData->zone->ZoneQuad.p2Y);
//            printf("  ZoneQuad.p3X= %d, ZoneQuad.p3Y= %d \n", ticketData->zone->ZoneQuad.p3X, ticketData->zone->ZoneQuad.p3Y);
//            printf("  ZoneQuad.p4X= %d, ZoneQuad.p4Y= %d \n", ticketData->zone->ZoneQuad.p4X, ticketData->zone->ZoneQuad.p4Y);

            initLib (ticketData);
//printf("-------ProcessField= %d\n", ok);
//if(imgIndx == 3)
//   	break;
  saveLog("before ProcessField\n", "");
            ok = ProcessField ( &imgOut, ticketData, &radonStruct, &ticketResults);
//    saveLog(" ProcessField  ", "\n");
// if(imgIndx == 2)
//   	break;

            if(ok < 0){
//printf("-------break; return= %d\n", ok);
            break;
            }
/*
            {char name[60];
               strcpy(name, imgOut.szIMGName);
   	    	   name[strlen(name)-4] = 0;
               strcat(name, "_");
               strcat(name, ticketData->fieldAddress->name);
               strcat(name, ".BMP");
//	           printBMPData (&imgOut);
               saveBMPbuffer (ticketData->fieldBMPPtr, name, imgOut.szOutPath);
            }
 */
// postprocessing
            ticketPostProcessing(ticketData, &ticketResults);
            copyOutputResults(&ticketResultsOut, &ticketResults);
            PrintOutputResults(&ticketResultsOut);


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

     SKIP_REC2:
            if(img.ptr != NULL){
      	      free(img.ptr);
      	      img.ptr = NULL;
            }
            if(imgOut.ptr != NULL) {
      	     free(imgOut.ptr);
      	     imgOut.ptr = NULL;
            }

		  imgIndx++;
//          break;
	    } // while
  }



	  printf("\ndone \n");


  return 0;
}

void setGlobalDataImg(LPIMGDATA img){
	  strcpy(img->szIMGPath,"c:\\Users\\Wicek\\YoolottoVin\\input\\");
//	  strcpy(img->szIMGName,"IMG_0400.jpg");
//	  strcpy(img->szIMGName,"IMG_0400.bmp");
//	  strcpy(img->szIMGName,"IMG_wb.bmp");
//	  strcpy(img->szIMGName,"RGBA_WB.bmp");
//	  strcpy(img->szIMGName,"IMG_BLACK.bmp");
//	  strcpy(img->szIMGName,"IMG_R.bmp");
//	  strcpy(img->szIMGName,"IMG_B.bmp");
	  strcpy(img->szIMGName,"IMG_1.bmp");
//	  strcpy(img->szIMGName,"IMG_test1.bmp");

//	  strcpy(img->szIMGName,"IMG_RGBA.rgb");
	  strcpy(img->szOutPath,"c:\\Users\\Wicek\\YoolottoVin\\output\\");
}
void setGlobalDataImg2(LPIMGDATA img){
	  strcpy(img->szIMGPath,"c:\\Users\\Wicek\\YoolottoVin\\input\\");
//	  strcpy(img->szIMGName,"IMG_0400.jpg");
//	  strcpy(img->szIMGName,"IMG_0400.bmp");
//	  strcpy(img->szIMGName,"IMG_wb.bmp");
//	  strcpy(img->szIMGName,"RGBA_WB.bmp");
//	  strcpy(img->szIMGName,"IMG_BLACK.bmp");
//	  strcpy(img->szIMGName,"IMG_R.bmp");
//	  strcpy(img->szIMGName,"IMG_B.bmp");
//	  strcpy(img->szIMGName,"IMG_1.bmp");
//	  strcpy(img->szIMGName,"IMG_test1.bmp");

	  strcpy(img->szIMGName,"IMG_RGBA.rgb");
	  strcpy(img->szOutPath,"c:\\Users\\Wicek\\YoolottoVin\\output\\");
}
void setGlobalDataImg3(LPIMGDATA img){
	  strcpy(img->szIMGPath,"c:\\Users\\Wicek\\YoolottoVin\\input\\");
//	  strcpy(img->szIMGName,"IMG_0400.jpg");
//	  strcpy(img->szIMGName,"IMG_0400.bmp");
//	  strcpy(img->szIMGName,"IMG_wb.bmp");
	  strcpy(img->szIMGName,"RGBA_WB.bmp");
//	  strcpy(img->szIMGName,"IMG_BLACK.bmp");
//	  strcpy(img->szIMGName,"IMG_R.bmp");
//	  strcpy(img->szIMGName,"IMG_B.bmp");
//	  strcpy(img->szIMGName,"IMG_1.bmp");
//	  strcpy(img->szIMGName,"IMG_test1.bmp");

//	  strcpy(img->szIMGName,"IMG_RGBA.rgb");
	  strcpy(img->szOutPath,"c:\\Users\\Wicek\\YoolottoVin\\output\\");
}


void setGlobalDataImgIdx(LPIMGDATA img, int idx, int inp, int mode){
	  char strNum[10];
	  strcpy(img->szIMGPath,"c:\\Users\\Wicek\\YoolottoVin\\input\\");
	  strcpy(img->szOutPath,"c:\\Users\\Wicek\\YoolottoVin\\output\\");
	  strcpy(img->szWorkPath,"c:\\Users\\Wicek\\YoolottoVin\\work\\");
	  if(idx <10){
		 strNum[0] = '0'+idx;
		 strNum[1] = 0;
	  }else{
		 strNum[0] = '0'+idx/10;
		 strNum[1] = '0'+idx%10;
		 strNum[2] = 0;
	  }
	  if(inp == 2 || inp == 4){
	      strcpy(img->szIMGName,"IMG_BMP_");
		  strcat(img->szIMGName, strNum);
		  strcat(img->szIMGName, ".bmp");
	  }
	  if(inp == 3){
	      strcpy(img->szIMGName,"IMG_RGBA_");
		  strcat(img->szIMGName, strNum);
		  if(mode == 0)
		     strcat(img->szIMGName, ".rgb");
		  if(mode == 1)
		     strcat(img->szIMGName, ".bmp");

	  }
}

//If 16777215 - rgbval1 > rgbval1 Then
//rgbval = SetPixel(Form1.hdc, x, y, 0)
//Else
//rgbval = SetPixel(Form1.hdc, x, y, 16777215)
//'16777215 corresponds to value for white and 0 corresponds to black

void saveLog(char *str1, char *str2){
   FILE *f;
//   int  len;
   char fileName[500];
   char buff[200];
   strcpy(fileName, "c:\\Users\\Wicek\\YoolottoVin\\log\\log.txt");
   f = fopen(fileName, "a");
    if(f == NULL) {
    	fprintf(stderr,"saveCSVFile (\"%s\") failed: %s\n",fileName, strerror(errno));
    	return;
    }
//    fseek(f, 0L,SEEK_END);
//    len = ftell(f);

   buff[0] = 0;
   if(str1) {
     strcat(buff, str1);
     strcat(buff, " ");
   }
   if(str2)
     strcat(buff, str2);

//   if(buff[0]) fprintf (f, "%s\n", buff);
   if(buff[0]) fprintf (f, "%s", buff);
   fclose (f);
}


