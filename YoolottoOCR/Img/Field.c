/*
 * Field.c
 *
 *  Created on: Aug 22, 2012
 *      Author: Wicek
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include  "../headers/Image.h"
#include  "../headers/Functions.h"
#include  "../headers/Prototype.h"

//extern int imgIndx;
int testLogoField(LPTICKETDATA ticketDataPtr);
int testLogoField2(LPTICKETDATA ticketDataPtr, int y1);
void saveLog(char *str1, char *str2);
void histVPtrPix(LPBYTE ptrPix, int widthImg, int hightImg, int MaxSize, int x1, int x2, int y1, int y2, int *vHist);
int getxShiftForNumbersField(int *vHist, int start, int d);
int getyShiftForNumbersField(int *vHist, int start, int d);
void verticalLineRemovalField(LPBYTE pTBuffer, int width, int height, int y1, int y2);


int TrimField (LPTICKETDATA ticketDataPtr){
    int i = 0, j = 0;
    int nofirst = 0;
    int skip    = 0;
    int newline = 0;
    int delim   = 0;
    char ch;

    while (ticketDataPtr->strings.lpResult[i]) {
    	ch = ticketDataPtr->strings.lpResult[i];
        delim = (ch == '\r');
        if (ch <= ' ') {
            if (nofirst) {
                skip = 1;
                if (ch == '\r')
                    newline = 1;
            }
            i++;
            continue;
        }
        nofirst = 1;

        if (skip) {
        	ticketDataPtr->strings.lpResult[j] =
   			ticketDataPtr->strings.lpCand1 [j] =
		    ticketDataPtr->strings.lpCand2 [j] = (newline) ? '\r' : ' ';
        	ticketDataPtr->flConf          [j] = 100;
            memset (&ticketDataPtr->flQuad [j], 0, sizeof (QUADRANGLE));
            j++;
            skip    = 0;
            newline = 0;
        }
        ticketDataPtr->strings.lpResult[j] = ticketDataPtr->strings.lpResult[i];
        ticketDataPtr->strings.lpCand1 [j] = ticketDataPtr->strings.lpCand1 [i];
        ticketDataPtr->strings.lpCand2 [j] = ticketDataPtr->strings.lpCand2 [i];
        ticketDataPtr->flConf          [j] = ticketDataPtr->flConf          [i];
        ticketDataPtr->flQuad          [j] = ticketDataPtr->flQuad          [i];
        i++;
        j++;
    }
    ticketDataPtr->strings.lpResult[j] = 0;
    ticketDataPtr->strings.lpCand1 [j] = 0;
    ticketDataPtr->strings.lpCand2 [j] = 0;
    return delim;
}



int ProcessField (LPIMGDATA img, LPTICKETDATA ticketDataPtr, LPRADON radonStruct, LPTICKETRESULTS ticketResults)
{
    DWORD  offset=0;         // offset to current field info block
    WORD   currentZone;    // number of current zone
    DWORD  protlth = 0;         // length of prototype file
    int delim   = 0;
    LPZONEDATA zone;

    BYTE* lpProt = ticketDataPtr->prototypePtr;
    char zeroLine[1] = "";      // zero-string to refer in case of failure
//    LPBYTE  lpField;
    int  i, fieldNumber;
    fieldNumber = 1;


    char buff[500];



    while(fieldNumber<=ticketDataPtr->numFields) {


       if (fieldNumber == 1) {
          offset      = ticketDataPtr->fieldChain;
          currentZone = 0xffff;
          protlth = ticketDataPtr->allocLength;
      }
//  printf("fieldNumber= %d, offset= %d, protlth= %d \n ", fieldNumber,offset, protlth);

    while  (1) {
        char        name[60];       // name of the field
        LPFIELDDATA field      = (LPFIELDDATA)((LPBYTE)ticketDataPtr + offset);
        BYTE*  descr      = lpProt + field->descr;
        DWORD attrOffset = field->descr + OffFlBuffer +
                                 *(WORD *)(descr + OffFlSize);
        BYTE*  attr       = lpProt + attrOffset;
//        char        buf[100];
        int        firstField;
        FIELD      f;
//        double        fMass;

//        WORD conf        = *(WORD*)(attr + OffFlConfThreshold);
        WORD logZoneNum  = 0xfff0;
        WORD validation  = 2;
        int failed      = 0;
        int extractOnly = 0;
        if (protlth >= attrOffset + OffFlSubset) {
            logZoneNum  = *(WORD*)(attr + OffFlLogZoneNo);
            validation  = *(WORD*)(attr + OffFlValidation);
            extractOnly = *(WORD*)(attr + OffFlExtractOnly);
        }
        offset = field->next;   // set pointer to the next field

//        printf ("Field %u of %u\n", fieldNumber, ticketDataPtr->numFields);
//        printf ("conf th= %d \n", conf);

//        printf("currentZone= %d, logZoneNum= %d \n ", currentZone,logZoneNum);

        // reset skip if we are in the next logical zone
        if (currentZone != logZoneNum) {
            currentZone  = logZoneNum;
            firstField   = 1;
//            concat       = 0;
        } else
            firstField   = 0;

        for (i = 0; i < 32; i++)
            name[i] = attr[i];
        name[32] = 0;
//        printf ( "field name:  %s\n", name);
        strcpy(field->name, name);


    sprintf(buff,"field name:  %s\n", name);
    saveLog("", buff);


  // extract field
               zone = &ticketDataPtr->zone[field->zone];  // get zone data block
        	   if(extractTicketsFieldcall(img, ticketDataPtr, field, radonStruct, ticketResults, name)  < 0)   {

                printf ("extractFormsField returns %i\n",
                		ticketDataPtr->returnCode);
                failed = 1;                  // set failure flag
 // set field quadrangle from prototype
                for (i = 0; i < sizeof (QUADRANGLE); i++) {
                    field->quad.p1X = field->quad.p4X =
                        *(WORD*)(descr);
                    field->quad.p1Y = field->quad.p2Y =
                        *(WORD*)(descr + 2);
                    field->quad.p2X = field->quad.p3X =
                        field->quad.p1X + *(WORD*)(descr + 4);
                    field->quad.p3Y = field->quad.p4Y =
                        field->quad.p1Y + *(WORD*)(descr + 6);
                }
            }
//sprintf(buff,"------------------   ticketDataPtr->returnCode= %d\n",ticketDataPtr->returnCode);
//saveLog("", buff);

//if(strcmp(name, "LOGO") == 0){
	// debug-----------------------------------------
//	saveField(img, ticketDataPtr);
	// debug-----------------------------------------
// }
if(strcmp(name, "LOGO") == 0){
	int yShift;
//	char buff[500];
	int h;
	ticketResults->yShift = 0;
    yShift = testLogoField2(ticketDataPtr, field->quad.p1Y);
	if(abs(yShift) > 20) {
// sprintf(buff,"------------------   yShift= %d\n",yShift);
// saveLog("", buff);
// sprintf(buff,"---x1= %d, y1= %d, x2= %d, y3= %d\n",field->quad.p1X, field->quad.p1Y, field->quad.p2X, field->quad.p3Y);
// saveLog("", buff);
		field->quad.p1Y -= yShift;
		field->quad.p2Y -= yShift;
		field->quad.p3Y -= yShift;
		field->quad.p4Y -= yShift;
		if(field->quad.p1Y<8) field->quad.p1Y=8;if(field->quad.p2Y<8) field->quad.p2Y=8;
		if(field->quad.p3Y<8) field->quad.p3Y=8;if(field->quad.p4Y<8) field->quad.p4Y=8;
		h = img->bi.biHeight-8;
		if(field->quad.p1Y>h) field->quad.p1Y=h;if(field->quad.p2Y>h) field->quad.p2Y=h;
		if(field->quad.p3Y>h) field->quad.p3Y=h;if(field->quad.p4Y>h) field->quad.p4Y=h;

		extractFieldAgain(img, field, ticketDataPtr);
		ticketResults->yShift = yShift;
//printf ("  yShift= %d\n", yShift);
//sprintf(buff,"---x1= %d, y1= %d, x2= %d, y3= %d\n",field->quad.p1X, field->quad.p1Y, field->quad.p2X, field->quad.p3Y);
//saveLog("", buff);

//		saveField(img, ticketDataPtr);
	}

	extractOnly = 1;
//	fMass = getFieldMass(ticketDataPtr);
	ticketResults->type = getLogoType(ticketDataPtr);

//	setTicketTypeFromMass(ticketResults, fMass);

//	sprintf(buff,"---fMass= %e\n",fMass);
//	saveLog("", buff);

//	saveField(img, ticketDataPtr);
}
//if(strcmp(name, "NUMBERS") == 0){
//    verticalLineRemovalField(ticketData->fieldBMPPtr, widthT, heightT, p1Y, p4Y);
//}

//printf ("  p1X= %d, p1Y= %d, p2X= %d, p2Y= %d\n", field->quad.p1X,field->quad.p1Y,field->quad.p2X,field->quad.p2Y);
//printf ("  p3X= %d, p3Y= %d, p4X= %d, p4Y= %d\n", field->quad.p3X,field->quad.p3Y,field->quad.p4X,field->quad.p4Y);
        if (!failed) {
              if (extractOnly)
            	  failed = 0;
//                  printf ("Recognition not requested\n");
              else
  // recognize field
                if (!RecognitionCall (img, ticketDataPtr)) {                        // if failed:
                    printf ("recognition returns %i\n",
                    		                 ticketDataPtr->returnCode);
                    failed = 1;                              // set failure flag
                }
        }
     if(strcmp(name, "NUMBERS") == 0){
        	saveField(img, ticketDataPtr);
        }


//        GlobalLock (lpFormData->hgWorkArea);        // lock results

        // set strings parameter to empty
        if (extractOnly || failed) {
        	ticketDataPtr->strings.lpResult = zeroLine;
        	ticketDataPtr->strings.lpCand1  = zeroLine;
        	ticketDataPtr->strings.lpCand2  = zeroLine;
        	ticketDataPtr->strings.lpReject = zeroLine;
//        	return -1;
        }

        // if there was field before, remove quads from it
//        if (hField = GetFieldWindow)
//            ClearAllQuads (hField);

//        DisplayField (lpFormData->hFieldDib, 0L);   // display field
          delim = TrimField (ticketDataPtr);

        // show low confidence characters on the field and recalculate coords
        if (ticketDataPtr->strings.lpResult[0]) {
            recalculateCharsPoints(img, &field->quad, &ticketDataPtr->strings, ticketDataPtr->flQuad);

        }

        // see if this and next fields must be skipped

        // convert to uppercase
        strupr  (ticketDataPtr->strings.lpResult);
        strupr  (ticketDataPtr->strings.lpCand1);
        strupr  (ticketDataPtr->strings.lpCand2);

        // save recognized field
        f.result = ticketDataPtr->strings.lpResult;
        f.cand1  = ticketDataPtr->strings.lpCand1;
        f.cand2  = ticketDataPtr->strings.lpCand2;
        f.conf   = ticketDataPtr->flConf;
        f.quad   = ticketDataPtr->flQuad;
/*
        if (f.result[0]) {                  // show string
            printf ("recognition:-----------\n");
            printf (f.result);
            printf ("\n");
            printf ("-----------------------\n");

            {int iCnf;
               for(iCnf = 0;iCnf<strlen(f.result);iCnf++){
            	if(f.result[iCnf] != 13)
            	   printf("%c,(%d)", f.result[iCnf], f.conf[iCnf]);
            	else
            	   printf("\n");
               }
            }

        }
*/
        copyFieldResults(ticketResults, &f, fieldNumber-1, field->name);
        fieldNumber++;

//        ConcatField (lpFormData->hwnd,      // hwnd for msg routing
//                         lpFormData->hgValData, // val file block
//                         delim,                 // TRUE for inserting \r
//                         &f,                    // field contents
//                         &field->quad);         // field quadrangle
//        }

        if (!offset) break;     // if this is the last field, done

        // look into next field's parameters
        field      = (LPFIELDDATA)((LPBYTE)ticketDataPtr + offset);
        descr      = ticketDataPtr->prototypePtr + field->descr;
        attrOffset = field->descr + OffFlBuffer +
                             *(WORD*)(descr + OffFlSize);
        if (protlth >= attrOffset + OffFlSubset) {
            attr       = ticketDataPtr->prototypePtr + attrOffset;
            logZoneNum = *(WORD*)(attr + OffFlLogZoneNo);
//            concat     = *(BOOL*)(attr + OffFlIsConcat);
        }
//        if (!concat) break;                     // if not concat, done
        if (currentZone != logZoneNum) break;   // if end of zone, done
    }
//    GlobalUnlock (lpFormData->hFDF);
   }

    return (offset);
}
//int _export WINAPI recFormsChars
//(IMAGE far * pForm,
 //QUADRANGLE far * pFieldQuad,
 //STRINGS far * pResults,
 //QUADRANGLE far * pPattern
//)

int  recalculateCharsPoints(LPIMGDATA pImg, LPQUADRANGLE pFieldQuad,
                                          LPSTRINGS pResults, LPQUADRANGLE pPattern){
 WORD wWidthT;
 WORD wHeightT;
 WORD i;

 LPSTR lpResultBuffer;

 int origX0;
 int origY0;
 int origX1;
 int origY1;
 int xOrig;
 int yOrig;
 int xCorn;
 int yCorn;
 int x;
 int y;
 int xn;
 int yn;
 long xl;
 long yl;

 int dx;
 int dy;
 double r;

 long cosTeta;
 long sinTeta;
 LPQUADRANGLE pTmp;

 wWidthT = pImg ->bi.biWidth;
 wHeightT = pImg ->bi.biHeight;

 origX0 = (int)(pFieldQuad ->p1X);
 origY0 = (int)(pFieldQuad -> p1Y);
 origX1 = EXTRACTMARGIN;
 origY1 = EXTRACTMARGIN;

 dx = (int)(pFieldQuad -> p2X) - origX0;
 dy = (int)(pFieldQuad -> p2Y) - origY0;
 r = sqrt((double)((long)dx*(long)dx + (long)dy*(long)dy));
 if(r != 0.0)
  {cosTeta = (long)((dx/r)*SCALE);
   sinTeta = (long)((dy/r)*SCALE);
  }
 else
  {cosTeta = SCALE;
   sinTeta = 0L;
  }

 lpResultBuffer = pResults -> lpReject;

 for(i=0; i<1000; i++)
 {if(*(lpResultBuffer+(long)i) == 0) break;


  pTmp = pPattern+(DWORD)i;

  xOrig = (int)(pTmp -> p1X);
  yOrig = (int)(pTmp -> p1Y);
  xCorn = (int)(pTmp -> p3X);
  yCorn = (int)(pTmp -> p3Y);

  x = xOrig-origX1; y = yOrig-origY1;
  xl = x*cosTeta - y*sinTeta;
  xn = origX0 + (int)(xl>>SCALEEXP);
  if(xn < 0) xn = 0;
  if(xn > (int)wWidthT - 1) xn = (int)wWidthT - 1;
  (pPattern+(DWORD)i) -> p1X = (WORD)xn;
  yl = x*sinTeta + y*cosTeta;
  yn = origY0 + (int)(yl>>SCALEEXP);
  if(yn < 0) yn = 0;
  if(yn > (int)wHeightT - 1) yn = (int)wHeightT - 1;
  (pPattern+(DWORD)i) -> p1Y = (WORD)yn;

  x = xCorn-origX1; y = yOrig-origY1;
  xl = x*cosTeta - y*sinTeta;
  xn = origX0 + (int)(xl>>SCALEEXP);
  if(xn < 0) xn = 0;
  if(xn > (int)wWidthT - 1) xn = (int)wWidthT - 1;
  (pPattern+(DWORD)i) -> p2X = (WORD)xn;
  yl = x*sinTeta + y*cosTeta;
  yn = origY0 + (int)(yl>>SCALEEXP);
  if(yn < 0) yn = 0;
  if(yn > (int)wHeightT - 1) yn = (int)wHeightT - 1;
  (pPattern+(DWORD)i) -> p2Y = (WORD)yn;

  x = xCorn-origX1; y = yCorn-origY1;
  xl = x*cosTeta - y*sinTeta;
  xn = origX0 + (int)(xl>>SCALEEXP);
  if(xn < 0) xn = 0;
  if(xn > (int)wWidthT - 1) xn = (int)wWidthT - 1;
  (pPattern+(DWORD)i) -> p3X = (WORD)xn;
  yl = x*sinTeta + y*cosTeta;
  yn = origY0 + (int)(yl>>SCALEEXP);
  if(yn < 0) yn = 0;
  if(yn > (int)wHeightT - 1) yn = (int)wHeightT - 1;
  (pPattern+(DWORD)i) -> p3Y = (WORD)yn;

  x = xOrig-origX1; y = yCorn-origY1;
  xl = x*cosTeta - y*sinTeta;
  xn = origX0 + (int)(xl>>SCALEEXP);
  if(xn < 0) xn = 0;
  if(xn > (int)wWidthT - 1) xn = (int)wWidthT - 1;
  (pPattern+(DWORD)i) -> p4X = (WORD)xn;
  yl = x*sinTeta + y*cosTeta;
  yn = origY0 + (int)(yl>>SCALEEXP);
  if(yn < 0) yn = 0;
  if(yn > (int)wHeightT - 1) yn = (int)wHeightT - 1;
  (pPattern+(DWORD)i) -> p4Y = (WORD)yn;
 }

 return(0);
}



int extractTicketsFieldcall(LPIMGDATA img, LPTICKETDATA ticketDataPtr, LPFIELDDATA field, LPRADON radonStruct,
		LPTICKETRESULTS ticketResults, char * fName){
    LPBITMAPFILEHEADER bf;
    LPBITMAPINFOHEADER bi;
    LPRGBQUAD          qd;

    BYTE* lpProt;

    IMAGEH       fieldH;
    LPDWORD      lpImage;
    DWORD        offBits;
    BYTE*        descr;
    LPZONEDATA   zone;
    int        err;

    lpProt = ticketDataPtr->prototypePtr;
    descr      = lpProt + field->descr;
    zone = &ticketDataPtr->zone[field->zone];

       ticketDataPtr->fieldAddress = NULL;

       if (zone->ptrZoneStruct == NULL) {              // if no zone map:
             if (zone->iAngle || !createZoneStructCall (img, ticketDataPtr, field->zone, radonStruct))
                  return -10;     // zone map cannot be obtained
        }
//printf("------------fieldH.himgPtr= %u \n",  (unsigned int)fieldH.himgPtr.ptr);
//       if(imgIndx == 2)
//          	return -1;
        fieldH.himgPtr.ptr = NULL;
        fieldH.himgPtr.size = 0;

        err = 0;
        fieldH.himgPtr.ptr = calloc(8, sizeof(BYTE));
        fieldH.himgPtr.size = 8;

//        err = reallocMemoryBuffer (&fieldH.himgPtr, 8);
        if (err < 0 || fieldH.himgPtr.ptr == NULL) {
            return -11;     // out of memory
        }

        offBits = sizeof (BITMAPFILEHEADER) +
                  sizeof (BITMAPINFOHEADER) +
                  sizeof (RGBQUAD) * 2;
        lpImage = (LPDWORD) fieldH.himgPtr.ptr;
        *lpImage = 0;
        *(lpImage + 1) = offBits;


        ticketDataPtr->returnCode = extractTicketsField(img, descr, *(LPWORD)(lpProt + OffFdDpiX), *(LPWORD)(lpProt + OffFdDpiY),
                                                    zone->ptrZoneStruct, zone->iAngle, &field->quad, &fieldH, ticketResults, fName);


        if (ticketDataPtr->returnCode < 0) {
            freeh (&fieldH.himgPtr);
//            fieldH.himgPtr.ptr = NULL;
//            field->ptrField = NULL;  // ??????????? check it up  !!!!!!!!!!!!!!!!!
            return -12;
        }

        {//	LPBYTE ptr = fieldH.himgPtr.ptr;
//        	LPBYTE biPtr = ptr + BFOOFBITSOFF+4;
        // fill in BITMAPFILEHEADER


        bf = (LPBITMAPFILEHEADER) fieldH.himgPtr.ptr;
//	    bf->bfType= 0x424d;
//        bf->bfSize =  offBits + (((fieldH.wWidth /** biBitCount*/ + 31) /32) * 4) * fieldH.wHeight;
//	    bf->bfReserved1 = 0;
//	    bf->bfReserved2 = 0;
//	    bf->bfOffBits =  offBits;
        bf->bfType[0] = 0x42;
        bf->bfType[1] = 0x4d;
	    set4Bytes(bf->bfSize, offBits + (((fieldH.wWidth /** biBitCount*/ + 31) /32) * 4) * fieldH.wHeight);
	    bf->bfReserved1[0] = 0;
	    bf->bfReserved1[1] = 0;
        set4Bytes(bf->bfOffBits, offBits);


//	    *(LPWORD)(ptr+BFTYPEOFF) = 0x424d;
//	    *(LPDWORD)(ptr+BFSIZEOFF) = offBits + (((fieldH.wWidth /** biBitCount*/ + 31) /32) * 4) * fieldH.wHeight;
//	    *(LPWORD)(ptr+BFRESERVED1OFF) = 0;
//	    *(LPWORD)(ptr+BFRESERVED2OFF) = 0;
//	    *(LPWORD)(ptr+BFOOFBITSOFF) = offBits;


//	     *(LPDWORD)(biPtr+BISIZEOFF) = sizeof (BITMAPINFOHEADER);
//	     *(LPDWORD)(biPtr+BIWIDTHOFF)= fieldH.wWidth;
//	     *(LPDWORD)(biPtr+BIHEIGHTOFF)=  fieldH.wHeight;
//	     *(LPWORD)(biPtr+BIPLANESOFF) = 1;
//	     *(LPWORD)(biPtr+BIBITCOUNTOFF) = 1;
//	     *(LPDWORD)(biPtr+BICOMPRESSIONOFF) =  BI_RGB;
//	     *(LPDWORD)(biPtr+BISIZEIMAGEOFF) = 0;
//	     *(LPDWORD)(biPtr+BIXPELSPERMETEROFF) =  fieldH.wDpiX;
//	     *(LPDWORD)(biPtr+BIYPELSPERMETEROFF) =  fieldH.wDpiY;
//	     *(LPDWORD)(biPtr+BICLRUSEDOFF) = 0;
//	     *(LPDWORD)(biPtr+BICLRIMPORTANTOFF) = 0;


        // fill in BITMAPINFOHEADER

        bi = (LPBITMAPINFOHEADER)((LPBYTE)bf + sizeof (BITMAPFILEHEADER));
/*
        bi->biSize =  sizeof (BITMAPINFOHEADER);
        bi->biWidth =  fieldH.wWidth;
        bi->biHeight =  fieldH.wHeight;
        bi->biPlanes = 1;
        bi->biBitCount= 1;

        bi->biCompression =  BI_RGB;
        bi->biSizeImage =  0;
        bi->biXPelsPerMeter =  fieldH.wDpiX;
        bi->biYPelsPerMeter =  fieldH.wDpiY;
        bi->biClrUsed =  0;
        bi->biClrImportant =  0;
*/
	    set4Bytes(bi->biSize, sizeof (BITMAPINFOHEADER));
        set4Bytes(bi->biWidth, fieldH.wWidth);
	    set4Bytes(bi->biHeight, fieldH.wHeight);
	     bi->biPlanes[0] = 1;
	     bi->biPlanes[1] = 0;
	     bi->biBitCount[0]= 1;
	     bi->biBitCount[1]= 0;
//		*bi->biPlanes = (short)1;
//		*bi->biBitCount= (short)1;
	    set4Bytes(bi->biCompression, BI_RGB);
		set4Bytes(bi->biSizeImage, 0);
		set4Bytes(bi->biXPelsPerMeter, fieldH.wDpiX);
	    set4Bytes(bi->biYPelsPerMeter, fieldH.wDpiY);
		set4Bytes(bi->biClrUsed, 0);
		set4Bytes(bi->biClrImportant, 0);

//printf("------------bfsize= %d, fieldH.offbits= %d \n",  *(LPDWORD)(ptr+BFSIZEOFF), *(LPWORD)(ptr+BFOOFBITSOFF));
//printf("------------*bi->biPlanes= %d, fieldH.wDpiY= %d \n",  *bi->biPlanes, *bi->biBitCount);


        // generate pair of RGBQUADs (black and white)
        qd = (LPRGBQUAD)((LPBYTE)bi + sizeof (BITMAPINFOHEADER));
        *(LPDWORD)qd       = 0x00000000;
        *(LPDWORD)(qd + 1) = 0x00ffffff;

        }

        if(ticketDataPtr->fieldBMPPtr)
            free (ticketDataPtr->fieldBMPPtr);
        ticketDataPtr->fieldBMPPtr = fieldH.himgPtr.ptr;

// printf("----111111111--------ticketDataPtr->fieldBMPPtr= %d \n",  (int)ticketDataPtr->fieldBMPPtr);

        ticketDataPtr->fieldAddress = field;

//        {char buff[500];
//         sprintf(buff,"---------------------------end  \n");
//         saveLog("", buff);
//             }

        return 0;

}



int extractTicketsField(LPIMGDATA pImg, LPBYTE lpFieldDescr, WORD wDpiX0, WORD wDpiY0, LPBYTE pZoneStruct,
                        int iAngle,  LPQUADRANGLE pFieldQuad, IMAGEH* pFieldH, LPTICKETRESULTS ticketResults, char *fName){


// HGLOBAL hgZoneStruct;
 BYTE* lpZoneStruct;

 WORD wDpiX;
 WORD wDpiY;

 int err;

 WORD originX0;
 WORD originY0;

 WORD origX0;
 WORD origY0;
 WORD cornX0;
 WORD cornY0;

// HGLOBAL hgBuffer;
 LPBYTE pBuffer;

 WORD * pwBuffer;
 DWORD sizeBuffer;

 WORD* pwidth;
 WORD* pheight;
 WORD width;
 WORD height;

 WORD widthRec0;
 WORD heightRec0;

 WORD* pwFieldDescr;

 RECT fieldRect0;
 RECT * pFieldRect0;
 BYTE  * lpFieldBuffer0;
 WORD sizeFieldBuffer0;

 WORD p1X;
 WORD p1Y;
 WORD p2X;
 WORD p2Y;
 WORD p3X;
 WORD p3Y;
 WORD p4X;
 WORD p4Y;

 long shift;

 WORD extractMargin;

 pwidth = &width;
 pheight = &height;

 pTBuffer   = pImg ->pixPtr;
 widthT     = pImg -> bi.biWidth;
 heightT    = pImg -> bi.biHeight;
 wDpiX      = pImg -> bi.biXPelsPerMeter;
 wDpiY      = pImg -> bi.biYPelsPerMeter;


// printf("----widthT= %d, heightT= %d \n",  widthT,heightT);

 if(((int)wDpiX < 80)||((int)wDpiX > 420)) return(-112);
 if(((int)wDpiY < 80)||((int)wDpiY > 420)) return(-112);

 byteWidthT = ((widthT + 31)/32)*4;

 pwFieldDescr = (WORD *)lpFieldDescr;
 originX0       = *(pwFieldDescr+1L);
 originY0       = *(pwFieldDescr+2L);
 widthRec0      = *(pwFieldDescr+3L);
 heightRec0     = *(pwFieldDescr+4L);

 if(((int)wDpiX0 < 80)||((int)wDpiX0 > 420)) return(-112);
 if(((int)wDpiY0 < 80)||((int)wDpiY0 > 420)) return(-112);

 if((originX0 != 0)||(originY0 != 0)||(widthRec0 != 5)||(heightRec0 != 5))
 {

//  hgZoneStruct = *phgZoneStruct;
  lpZoneStruct = pZoneStruct + 8;

  pFieldRect0 = &fieldRect0;

  origX0 = originX0-FIELDMARGIN;
  origY0 = originY0-FIELDMARGIN;
  cornX0 = originX0+widthRec0+FIELDMARGIN;
  cornY0 = originY0+heightRec0+FIELDMARGIN;

  if((int)origX0 < 0) origX0 = 0;
  if((int)origY0 < 0) origY0 = 0;
  if((int)cornX0 > (int)widthT - 1) cornX0 = widthT - 1;
  if((int)cornY0 > (int)heightT - 1) cornY0 = heightT - 1;

  pFieldRect0 -> left = origX0;
  pFieldRect0 -> top = origY0;
  pFieldRect0 -> right = cornX0;
  pFieldRect0 -> bottom = cornY0;

  sizeFieldBuffer0 = *(pwFieldDescr+5L);

  if(sizeFieldBuffer0 != 0)
  {shift = 6L;
   lpFieldBuffer0 = (BYTE *)(pwFieldDescr+shift);

   err = removeFieldBack
         (pTBuffer, widthT, heightT,
          iAngle, lpZoneStruct,
          pFieldRect0, lpFieldBuffer0
         );

   if(err < 0) { return(err);}
  }

  pFieldQuad -> p1X = originX0;
  pFieldQuad -> p1Y = originY0;
  pFieldQuad -> p2X = originX0+widthRec0;
  pFieldQuad -> p2Y = originY0;
  pFieldQuad -> p3X = originX0+widthRec0;
  pFieldQuad -> p3Y = originY0+heightRec0;
  pFieldQuad -> p4X = originX0;
  pFieldQuad -> p4Y = originY0+heightRec0;

  err =
  mapQ(pFieldQuad, iAngle, lpZoneStruct, 0);
  if(err < 0) { return(err);}

//  GlobalUnlock(hgZoneStruct);

  p1X = (pFieldQuad -> p1X);
      if((int)p1X < 0) p1X = 0;
  p1Y = (pFieldQuad -> p1Y);
      if((int)p1Y < 0) p1Y = 0;
  p2X = (pFieldQuad -> p2X);
      if(p2X > widthT - 1) p2X = widthT - 1;
  p2Y = (pFieldQuad -> p2Y);
      if((int)p2Y < 0) p2Y = 0;
  p3X = (pFieldQuad -> p3X);
      if(p3X > widthT - 1) p3X = widthT - 1;
  p3Y = (pFieldQuad -> p3Y);
      if(p3Y > heightT - 1) p3Y = heightT - 1;
  p4X = (pFieldQuad -> p4X);
      if((int)p4X < 0) p4X = 0;
  p4Y = (pFieldQuad -> p4Y);
      if(p4Y > heightT - 1) p4Y = heightT - 1;

  shift = 12L + sizeFieldBuffer0;

  if(*(lpFieldDescr + shift) == 94)      // rotates field on +90 degrees ('^')
     {WORD tx, ty;                       //
      tx = p4X;                          //
      ty = p4Y;                          //
      p4X = p3X;                         //
      p4Y = p3Y;                         //
      p3X = p2X;                         //
      p3Y = p2Y;                         //
      p2X = p1X;                         //
      p2Y = p1Y;                         //
      p1X = tx;                          //
      p1Y = ty;                          //
     }


  if(*(lpFieldDescr + shift) == '<')      // rotates field on -90 degrees
     {WORD tx, ty;                       //
      tx = p1X;                          //
      ty = p1Y;                          //
      p1X = p2X;                         //
      p1Y = p2Y;                         //
      p2X = p3X;                         //
      p2Y = p3Y;                         //
      p3X = p4X;                         //
      p3Y = p4Y;                         //
      p4X = tx;                          //
      p4Y = ty;                          //
     }


  if(*(lpFieldDescr + shift) == '>')      // rotates field on +90 degrees
     {WORD tx, ty;                       //
      tx = p4X;                          //
      ty = p4Y;                          //
      p4X = p3X;                         //
      p4Y = p3Y;                         //
      p3X = p2X;                         //
      p3Y = p2Y;                         //
      p2X = p1X;                         //
      p2Y = p1Y;                         //
      p1X = tx;                          //
      p1Y = ty;                          //
     }

//  hgBuffer = pFieldH -> hgBuffer;
  pBuffer = pFieldH->himgPtr.ptr;

  pwidth = &width;
  pheight = &height;

  extractMargin = EXTRACTMARGIN;
  {
  HLPBYTE hpFieldH;
  hpFieldH.ptr = pFieldH->himgPtr.ptr;
  if(  ticketResults->yShift > 0){
	  p1Y -= ticketResults->yShift;
	  p2Y -= ticketResults->yShift;
//	  p3Y -= ticketResults->yShift;
//	  p4Y -= ticketResults->yShift;
	  if(p1Y < 8) p1Y = 8;
	  if(p2Y < 8) p2Y = 8;
//	  if(p3Y < 8) p3Y = 8;
//	  if(p4Y < 8) p4Y = 8;
	  if(p1Y > heightT-8) p1Y = heightT-8;
	  if(p2Y > heightT-8) p2Y = heightT-8;
//	  if(p3Y > heightT-8) p3Y = heightT-8;
//	  if(p4Y > heightT-8) p4Y = heightT-8;
  }
  if(  ticketResults->yShift < 0){
//	  p1Y -= ticketResults->yShift;
//	  p2Y -= ticketResults->yShift;
	  p3Y -= ticketResults->yShift;
	  p4Y -= ticketResults->yShift;
//	  if(p1Y < 8) p1Y = 8;
//	  if(p2Y < 8) p2Y = 8;
	  if(p3Y < 8) p3Y = 8;
	  if(p4Y < 8) p4Y = 8;
//	  if(p1Y > heightT-8) p1Y = heightT-8;
//	  if(p2Y > heightT-8) p2Y = heightT-8;
	  if(p3Y > heightT-8) p3Y = heightT-8;
	  if(p4Y > heightT-8) p4Y = heightT-8;
  }

  if(strcmp(fName, "NUMBERS") == 0){
	  if(p1X > 200){
 // x shift
		  	  int h_size = 400, xShift;
		  	  int ddTest = 100;
		  	  int vHist[h_size];
		  	  WORD byteWidthHist = ((widthT + 31)/32)*4;
		  //	  int i;

		          histVPtrPix(pTBuffer, byteWidthHist, heightT, h_size, p1X-ddTest, p1X+h_size-ddTest, p1Y, p4Y, vHist);
		          xShift = getxShiftForNumbersField(vHist, ddTest, ddTest/2);
		          if(xShift > 0){
		            if(xShift < 70) xShift += 10;
		        	  p1X -= xShift;
		        	  p4X -= xShift;
		        	  if(p1X < 8) p1X = 8;
		        	  if(p4X < 8) p4X = 8;
		          }
		  //	 for(i=0;i<vHist[0];i++){
		  //		    printf("   i= %d, vHist= %d \n", i, vHist[i]);
		  //	  }
		    }
		    if(strcmp(fName, "NUMBERS") == 0){
		  	  if(pImg->bi.biWidth -p2X > 150){
		  		  p2X += 50;
		  		  p3X += 50;
		  	  }
		  //      printf("   p1Y= %d, pImg->bi.biWidth= %d\n", p1Y, pImg->bi.biHeight);

	  }
	  if(pImg->bi.biWidth -p2X > 150){
		  p2X += 50;
		  p3X += 50;
	  }

//      printf("   p1Y= %d, pImg->bi.biWidth= %d\n", p1Y, pImg->bi.biHeight);
	  if( p1Y > 400){
	// y shift
		  int yShift;
		  int ddTest1 = 100, ddTest2 = 20;
		  int vHist[200];
		  WORD byteWidthHist = ((widthT + 31)/32)*4;
//		  int i;

	  	     histHTest_X1_X1(pTBuffer, byteWidthHist, heightT,  p1X, p2X, p1Y-ddTest1, p1Y+ddTest2, vHist);

//	  		 for(i=0;i<vHist[0];i++){
//	  			    printf("   i= %d, vHist= %d \n", i, vHist[i]);
//	  		  }


	         yShift = getyShiftForNumbersField(vHist, ddTest1, 2*ddTest2);

//	       printf("   yShift= %d \n", yShift);

	        if(yShift > 0){
	        	yShift += 5;
	      	  p1Y -= yShift;
	      	  p2Y -= yShift;
	      	  if(p1Y < 8) p1Y = 8;
	      	  if(p2Y < 8) p2Y = 8;
	        }

	  }

	  verticalLineRemovalField(pTBuffer, widthT, heightT, p1Y, p4Y);

  }



  pFieldQuad -> p1X = p1X;
  pFieldQuad -> p1Y = p1Y;
  pFieldQuad -> p2X = p2X;
  pFieldQuad -> p2Y = p2Y;
  pFieldQuad -> p3X = p3X;
  pFieldQuad -> p3Y = p3Y;
  pFieldQuad -> p4X = p4X;
  pFieldQuad -> p4Y = p4Y;
/*
  {char buff[500];
   saveLog("extractField; name= ", fName);
   sprintf(buff,"\n widthT= %d, heightT= %d\n", widthT, heightT);
   saveLog("", buff);

   sprintf(buff,"\n p1X= %d, p2X= %d, p3X= %d, p4X= %d\n", p1X, p2X, p3X, p4X);
   saveLog("", buff);
   sprintf(buff,"\n p1Y= %d, p2Y= %d, p3Y= %d, p4Y= %d\n", p1Y, p2Y, p3Y, p4Y);
   saveLog("", buff);
       }
*/

  err =
  extractField(pTBuffer, widthT, heightT,
         p1X, p1Y,
         p2X, p2Y,
         p4X, p4Y,
         &pFieldH->himgPtr,
//         phgBuffer,
         pwidth,
         pheight,
         extractMargin
        );
    if(err < 0) return(err);
  }
//  hgBuffer = *phgBuffer;

  sizeBuffer = (DWORD)(*pwidth)*(DWORD)(*pheight + 2*extractMargin);

  pwBuffer = (WORD *)pFieldH->himgPtr.ptr;
  *(pwBuffer+2)= (WORD)(sizeBuffer%65536);
  *(pwBuffer+3)= (WORD)(sizeBuffer/65536);
//  GlobalUnlock(hgBuffer);
 }

//  pFieldH -> hgBuffer = hgBuffer;
  pFieldH -> wWidth   = *pwidth;
  pFieldH -> wHeight  = *pheight;
  pFieldH -> wDpiX    = wDpiX;
  pFieldH -> wDpiY    = wDpiY;

//  saveLog("extractField end ", "\n");
 return(0);
}


int  createZoneStructCall (LPIMGDATA img, LPTICKETDATA ticketDataPtr, WORD wZoneNo, LPRADON radonStruct)
{
//    BYTE * pFormDescr;

    ticketDataPtr->zone[wZoneNo].ptrZoneStruct = NULL;

    if (!ticketDataPtr->prototypePtr) {
       return -1;
    }

    ticketDataPtr->zone[wZoneNo].iAngle = ticketDataPtr->iAngle;
    ticketDataPtr->returnCode = createZoneStruct(img, ticketDataPtr->prototypePtr, &ticketDataPtr->workArea,  ticketDataPtr->ptrTicketStruct,
    		   &ticketDataPtr->zone[wZoneNo].iAngle,  wZoneNo + 1,
    		   &ticketDataPtr->zone[wZoneNo].ptrZoneStruct, &ticketDataPtr->zone[wZoneNo].ZoneQuad, radonStruct);

    if (ticketDataPtr->returnCode < 0) {
        // set flags once again: we had failure
    	ticketDataPtr->zone[wZoneNo].iAngle = 1;
        if (ticketDataPtr->zone[wZoneNo].ptrZoneStruct) {
        	free(ticketDataPtr->zone[wZoneNo].ptrZoneStruct);
        	ticketDataPtr->zone[wZoneNo].ptrZoneStruct = NULL;
        }
        return 0;
    }
    return 1;
}

int createZoneStruct(LPIMGDATA pImg, LPBYTE pTDescr, HLPBYTE* pWorkArea,  LPBYTE pTicketStruct,
		             int* pAngle,  WORD wZoneNo,  LPBYTE* pZoneStruct, LPQUADRANGLE pZoneQuad, LPRADON radonStruct) {

 int iAngle;

 BYTE * lpWorkArea;
 BYTE* lpTicketStruct;
 DWORD sizeZoneStructAlloc;
 DWORD sizeZoneStruct;
 WORD * lpwZoneStruct;
 BYTE * lpZoneStruct;

 WORD wDpiX;
 WORD wDpiY;
 WORD wDpiX0;
 WORD wDpiY0;

 WORD zoneMargin;

 int retVal;
 int err;
 long shift;
 DWORD sizeWorkAreaAlloc;
 WORD * pwFormDescr;


 WORD sizeDFX0;
 int dMinFX0;
 WORD stepDFX0;
 WORD sizeDFX1;
 int dMinFX1;
 WORD stepDFX1;
 WORD * pRadonMainTeta0;
 WORD sizeDFY0;
 int dMinFY0;
 WORD stepDFY0;
 WORD sizeDFY1;
 int dMinFY1;
 WORD stepDFY1;
 WORD * pRadonDualTeta0;

 DWORD size1;
 DWORD size2;

 DWORD dist;
 DWORD * pDist;
 WORD  *  pPosX0;
 WORD  *  pPosEndX;
 WORD posX0;
 WORD posEndX;
 WORD * pAllignX;
 WORD *  pPosY0;
 WORD *  pPosEndY;
 WORD posY0;
 WORD posEndY;
 WORD * pAllignY;

 WORD originX0;
 WORD originY0;

 DWORD offsetZone;
 WORD wZoneNoFD;
 WORD widthRec0;
 WORD heightRec0;

 WORD * pwZoneDescr;
 WORD * pwZoneStructX;
 WORD * pwZoneStructY;
 WORD cornerX;
 WORD cornerY;

 WORD p1X;
 WORD p1Y;
 WORD p2X;
 WORD p2Y;
 WORD p3X;
 WORD p3Y;
 WORD p4X;
 WORD p4Y;

 WORD sizeZoneStruct0;
 DWORD offsetNFields;
 WORD wNFields;

 WORD nFields;
 int stepXL;
 int stepYL;
 int phiMinL;
 int phiMaxL;
 WORD stepPhiL;
 int iZoneMargin;

 radonStruct->local = 1;

 *pAngle = radonStruct->tetaG;

 iAngle = *pAngle;

 pDist = &dist;
 pPosX0 = &posX0;
 pPosEndX = &posEndX;
 pPosY0 = &posY0;
 pPosEndY = &posEndY;

 pTBuffer   = pImg ->pixPtr;
 widthT     = pImg -> bi.biWidth;
 heightT    = pImg -> bi.biHeight;
 wDpiX      = pImg -> bi.biXPelsPerMeter;
 wDpiY      = pImg -> bi.biYPelsPerMeter;



 if(((int)wDpiX < 80)||((int)wDpiX > 420)) return(-112);
 if(((int)wDpiY < 80)||((int)wDpiY > 420)) return(-112);

 byteWidthT = ((widthT + 31)/32)*4;

 pwFormDescr = (WORD *)pTDescr;
 wDpiX0 = *(pwFormDescr+1);
 wDpiY0 = *(pwFormDescr+2);
 offsetZone = *(pwFormDescr+5L+(DWORD)(2*(wZoneNo-1))) +
         65536*(*(pwFormDescr+6L+(DWORD)(2*(wZoneNo-1))));
 pwZoneDescr = (WORD*)(pTDescr + offsetZone);
 wZoneNoFD = *pwZoneDescr;
 if(wZoneNoFD != wZoneNo)
       return(-1);
 originX0       = *(pwZoneDescr+1L);
 originY0       = *(pwZoneDescr+2L);
 widthRec0      = *(pwZoneDescr+3L);
 heightRec0     = *(pwZoneDescr+4L);

 sizeZoneStruct0 = *(pwZoneDescr+5L);
 offsetNFields = 6L+(long)(sizeZoneStruct0/2);
 wNFields = *(pwZoneDescr+offsetNFields);

 if(((int)wDpiX0 < 80)||((int)wDpiX0 > 420)) return(-112);
 if(((int)wDpiY0 < 80)||((int)wDpiY0 > 420)) return(-112);

 pwZoneStructX = pwZoneDescr + 6L;


 sizeZoneStruct = (DWORD)(*(pwZoneDescr + 5L));
 nFields = *(pwZoneDescr + 6L + sizeZoneStruct/2);
 shift = (long)(6L + sizeZoneStruct/2 + 1L + 2*nFields);
 stepXL      = (int)*(pwZoneDescr + shift + 0);
 stepYL      = (int)*(pwZoneDescr + shift + 1);
 phiMinL     = (int)*(pwZoneDescr + shift + 2);
 phiMaxL     = (int)*(pwZoneDescr + shift + 3);
 stepPhiL    =     *(pwZoneDescr + shift + 4);
 radonStruct->tgP         =     *(pwZoneDescr + shift + 5);
 iZoneMargin = (int)*(pwZoneDescr + shift + 6);


// printf("------------phiMinL= %d \n",  phiMinL);
 phiMinL = -20; // ??????????????????????????????

 shift = (long)(*pwZoneStructX/2) + 1L;
 sizeDFX0 = *(pwZoneStructX+1L);
 dMinFX0 = (int)(*(pwZoneStructX+2L));
 stepDFX0 = *(pwZoneStructX+3L);
 pRadonMainTeta0 = pwZoneStructX+4L;
 pwZoneStructY = pwZoneStructX + shift;
 shift = (long)(*pwZoneStructY/2) + 1L;
 sizeDFY0 = *(pwZoneStructY+1L);
 dMinFY0 = (int)(*(pwZoneStructY+2L));
 stepDFY0 = *(pwZoneStructY+3L);
 pRadonDualTeta0 = pwZoneStructY+4L;

// hgFormStruct = *phgFormStruct;
 lpTicketStruct = (BYTE*)pTicketStruct + 8;

 pZoneQuad -> p1X = originX0;
 pZoneQuad -> p1Y = originY0;
 pZoneQuad -> p2X = originX0+widthRec0;
 pZoneQuad -> p2Y = originY0;
 pZoneQuad -> p3X = originX0+widthRec0;
 pZoneQuad -> p3Y = originY0+heightRec0;
 pZoneQuad -> p4X = originX0;
 pZoneQuad -> p4Y = originY0+heightRec0;

 zoneMargin = (WORD)((iZoneMargin*wDpiX0 + 300*4)/(300*8))*8;
// printf("zoneMargin=%d, pZoneQuad -> p1X= %d, pZoneQuad -> p1Y= %d, pZoneQuad -> p2X= %d \n",zoneMargin, pZoneQuad -> p1X, pZoneQuad -> p1Y, pZoneQuad -> p2X);
// printf("pTicketStruct= %u \n", (unsigned int)pTicketStruct);
 err =
 mapQ(pZoneQuad, iAngle, lpTicketStruct, zoneMargin);
//      GlobalUnlock(hgFormStruct);
      if(err < 0) return(err);

 if((pZoneQuad -> p1X) < (pZoneQuad -> p4X))
                                originX = (pZoneQuad -> p1X);
                           else originX = (pZoneQuad -> p4X);
 if((int)originX < 0) originX = 0;
 originX = (originX/8)*8;
 if((pZoneQuad -> p1Y) < (pZoneQuad -> p2Y))
                                originY = (pZoneQuad -> p1Y);
                           else originY = (pZoneQuad -> p2Y);
 if((int)originY < 0) originY = 0;
 if((pZoneQuad -> p2X) < (pZoneQuad -> p3X))
                                cornerX = (pZoneQuad -> p3X);
                           else cornerX = (pZoneQuad -> p2X);
 if(cornerX > widthT - 1) cornerX = widthT - 1;
 cornerX = (cornerX/8)*8;
 if((pZoneQuad -> p3Y) < (pZoneQuad -> p4Y))
                                cornerY = (pZoneQuad -> p4Y);
                           else cornerY = (pZoneQuad -> p3Y);
 if(cornerY > heightT - 1) cornerY = heightT - 1;
 widthRec  = cornerX - originX;
 heightRec = cornerY - originY;

 size1 = (widthRec + 2*zoneMargin)/stepYL + 1;
 size2 = (heightRec + 2*zoneMargin)/stepXL + 1;
 if(size1 > size2) sizeWorkAreaAlloc = sizeDFY0*size1 + 100L;
              else sizeWorkAreaAlloc = sizeDFX0*size2 + 100L;
 sizeWorkAreaAlloc = sizeWorkAreaAlloc*3/2;

// err = getMemoryHuge(phgWorkArea, sizeWorkAreaAlloc, 500000L);

 recalculateAllocBuffer(&sizeWorkAreaAlloc, 500000);
// printf(" pWorkArea->size= %ld, sizeWorkAreaAlloc= %d \n", pWorkArea->size, sizeWorkAreaAlloc);
 if(pWorkArea->size < sizeWorkAreaAlloc) {
	free(pWorkArea->ptr);
	pWorkArea->ptr = calloc(sizeWorkAreaAlloc, sizeof(BYTE));
	 if(pWorkArea->ptr == NULL) {
	       	sprintf(pImg->error,"createZoneStruct (\"%s\") calloc failed: %s\n",pImg->szIMGName, strerror(errno));
	       	return -1;
	   }
	 (pWorkArea)->size = sizeWorkAreaAlloc;

 }

// reallocMemoryBufferTest(pWorkArea, sizeWorkAreaAlloc);

 lpWorkArea = (BYTE *)(pWorkArea->ptr + 8);

 sizeZoneStructAlloc =
           (DWORD)(44 + 2*((widthRec + 2*zoneMargin)/stepYL + (heightRec + 2*zoneMargin)/stepXL + 2) + 100);

// err = getMemoryHuge(phgZoneStruct, sizeZoneStructAlloc, 4000L);
 recalculateAllocBuffer(&sizeZoneStructAlloc, 4000);
// *pZoneStruct = calloc(sizeWorkAreaAlloc, sizeof(BYTE));
  *pZoneStruct = calloc(sizeZoneStructAlloc, sizeof(BYTE));
 if(*pZoneStruct == NULL) {
       	sprintf(pImg->error,"createZoneStruct (\"%s\") calloc failed: %s\n",pImg->szIMGName, strerror(errno));
       	return -1;
   }

// hgZoneStruct = *phgZoneStruct;
 lpZoneStruct = (BYTE *)(*pZoneStruct);
 lpwZoneStruct = ((WORD *)lpZoneStruct) + 4;

 radonStruct->teta = iAngle;
 radonStruct->stepD          = stepXL;
 radonStruct->phiMin = iAngle + phiMinL; //???????????????????????????????
 radonStruct->phiMax = iAngle + phiMaxL;
 radonStruct->stepPhi = stepPhiL;

// local = 1;

 radonStruct->pRadonBuffer = (WORD *)lpWorkArea;
 radonStruct->pRadonMainTeta = &buffer[0];

      retVal = radonMainG(radonStruct);
      if(retVal < 0) {
                      return(retVal);
                     }

//      retVal = radonMainTeta();
      retVal = radMT(radonStruct);
      if(retVal < 0) {
                      return(retVal);
                     }

 iAngle = radonStruct->teta;
 *pAngle = iAngle;

 pAllignX = lpwZoneStruct + 9L;

 err = allignArrays(radonStruct, lpWorkArea,
                    pRadonMainTeta0,
                    radonStruct->pRadonMainTeta,
                    sizeDFX0,
                    radonStruct->sizeDTeta,
                    pDist,
                    pAllignX,
                    pPosX0,
                    pPosEndX
                   );
         if(err < 0) { return(err);}

 *(lpwZoneStruct + 0L) = 2*(sizeDFX0 + 8);
 *(lpwZoneStruct + 1L) = sizeDFX0;
 *(lpwZoneStruct + 2L) = radonStruct->sizeDTeta;
 *(lpwZoneStruct + 3L) = (WORD)dMinFX0;
 *(lpwZoneStruct + 4L) = (WORD)(radonStruct->dMinTeta/SCALE);
 *(lpwZoneStruct + 5L) = stepDFX0;
 *(lpwZoneStruct + 6L) = radonStruct->stepD;
 *(lpwZoneStruct + 7L) = posX0;
 *(lpwZoneStruct + 8L) = posEndX;

 sizeDFX1 = radonStruct->sizeDTeta;
 dMinFX1  = (int)(radonStruct->dMinTeta/SCALE);
 stepDFX1 = radonStruct->stepD;

 sizeZoneStruct = (DWORD)(2*(sizeDFX0 + 9));
 shift = (long)(sizeDFX0 + 9);
 lpwZoneStruct = lpwZoneStruct + shift;

 radonStruct->stepD          = stepYL;
 radonStruct->pRadonDualTeta = &buffer[0];

      retVal = radDTL(radonStruct);
      if(retVal < 0) { return(retVal);}

 pAllignY = lpwZoneStruct + 9L;

 err = allignArrays(radonStruct,  lpWorkArea,  pRadonDualTeta0, radonStruct->pRadonDualTeta,
                    sizeDFY0, radonStruct->sizeDTeta, pDist, pAllignY, pPosY0, pPosEndY );
         if(err < 0) { return(err);}

 *(lpwZoneStruct + 0L) = 2*(sizeDFY0 + 8);
 *(lpwZoneStruct + 1L) = sizeDFY0;
 *(lpwZoneStruct + 2L) = radonStruct->sizeDTeta;
 *(lpwZoneStruct + 3L) = (WORD)dMinFY0;
 *(lpwZoneStruct + 4L) = (WORD)(radonStruct->dMinTeta/SCALE);
 *(lpwZoneStruct + 5L) = stepDFY0;
 *(lpwZoneStruct + 6L) = radonStruct->stepD;
 *(lpwZoneStruct + 7L) = posY0;
 *(lpwZoneStruct + 8L) = posEndY;

 sizeDFY1 = radonStruct->sizeDTeta;
 dMinFY1  = (int)(radonStruct->dMinTeta/SCALE);
 stepDFY1 = radonStruct->stepD;

 sizeZoneStruct = sizeZoneStruct + (DWORD)(2*(sizeDFY0 + 9));

 lpwZoneStruct = (WORD *)lpZoneStruct;
 *(lpwZoneStruct + 2L) = (WORD)(sizeZoneStruct%65536);
 *(lpwZoneStruct + 3L) = (WORD)(sizeZoneStruct/65536);

 lpZoneStruct = lpZoneStruct + 8L;

// GlobalUnlock(hgWorkArea);

 pZoneQuad -> p1X = originX0;
 pZoneQuad -> p1Y = originY0;
 pZoneQuad -> p2X = originX0+widthRec0;
 pZoneQuad -> p2Y = originY0;
 pZoneQuad -> p3X = originX0+widthRec0;
 pZoneQuad -> p3Y = originY0+heightRec0;
 pZoneQuad -> p4X = originX0;
 pZoneQuad -> p4Y = originY0+heightRec0;

 err =
 mapQ(pZoneQuad, iAngle, lpZoneStruct, 0);
 if(err < 0) return(err);

 p1X = (pZoneQuad -> p1X);
      if((int)p1X < 0) p1X = 0;
 p1Y = (pZoneQuad -> p1Y);
      if((int)p1Y < 0) p1Y = 0;
 p2X = (pZoneQuad -> p2X);
      if(p2X > widthT - 1) p2X = widthT - 1;
 p2Y = (pZoneQuad -> p2Y);
      if((int)p2Y < 0) p2Y = 0;
 p3X = (pZoneQuad -> p3X);
      if(p3X > widthT - 1) p3X = widthT - 1;
 p3Y = (pZoneQuad -> p3Y);
      if(p3Y > heightT - 1) p3Y = heightT - 1;
 p4X = (pZoneQuad -> p4X);
      if((int)p4X < 0) p4X = 0;
 p4Y = (pZoneQuad -> p4Y);
      if(p4Y > heightT - 1) p4Y = heightT - 1;

 pZoneQuad -> p1X = p1X;
 pZoneQuad -> p1Y = p1Y;
 pZoneQuad -> p2X = p2X;
 pZoneQuad -> p2Y = p2Y;
 pZoneQuad -> p3X = p3X;
 pZoneQuad -> p3Y = p3Y;
 pZoneQuad -> p4X = p4X;
 pZoneQuad -> p4Y = p4Y;

 return(0);
}

	int  extractField(LPBYTE pFormBufferA, WORD widthFormA, WORD heightFormA,
			          int ax, int ay, int bx, int by, int dx, int dy,
			          LPHLPBYTE hpFieldBufferAInp, WORD *pwidth, WORD *pheight, WORD extractMargin){

 int err;
 int x;
 int y;
 double r;
 long cosTeta;
 long sinTeta;
 WORD width;
 WORD height;
 WORD byteWidth;
 DWORD sizeFieldBufferAlloc;
 DWORD sizeFieldBuffer;
// HGLOBAL hgFieldBufferA;
 BYTE  * pFieldBufferA;
 WORD  * pwFieldBufferA;
 DWORD * pdwFieldBufferA;

 DWORD size;
 DWORD offset;
 DWORD dw0=0, dw1=0;

// WORD byteWidthHist = ((widthFormA + 31)/32)*4;
// float maxAlpha = 0.9;
// float stepAlpha = 0.1;
 float ang;
// int  scAngleH;
 float shfx1, shfx2, shfy1, shfy2;
 float siN, coS, tG;
 int angleTest;

// char buff[500];

 // x shift



 angleTest = getAngle(pFormBufferA, widthFormA, heightFormA, ax, ay, bx-ax, dy-ay,
 		-60, 60, 5);

// printf("------------ widthFormA= %d, heightFormA= %d, ax= %d, ay= %d, bx-ax= %d, dy-ay = %d \n", widthFormA, heightFormA, ax, ay, bx-ax, dy-ay);
// printf("------------ angleTest= %d\n", angleTest);

//sprintf(buff,"------------ widthFormA= %d, heightFormA= %d, ax= %d, ay= %d, bx-ax= %d, dy-ay = %d \n", widthFormA, heightFormA, ax, ay, bx-ax, dy-ay);
//saveLog("", buff);

//  getAngleFromHistH(pFormBufferA, byteWidthHist, heightFormA,
// 		              ax, ay, bx, dy, maxAlpha, stepAlpha, &ang);

  ang  = -(float)angleTest/10.0;
//   printf("------------ ang= %e \n", ang);

//   sprintf(buff,"------------ ang= %e \n", ang);
//   saveLog("", buff);

//  ang = 3;

   siN = sin((double)(ang*PI)/180.0);
   coS = cos((double)(ang*PI)/180.0);
   tG  = tan((double)(ang*PI)/180.0);
//   printf("------------ siN= %e, coS= %e, tG= %e \n", siN, coS, tG);

   shfy1 = (bx-ax)*siN;
   shfx1 = shfy1*tG;
   shfx2 = (dy-ay)*siN+shfx1;
   shfy2 = (dy-ay)*coS-(dy-ay-shfy1);
//   printf("------------ shfx1= %e, shfx2 = %e, shfy1= %e, shfy2= %e \n", shfx1, shfx2, shfy1, shfy2);

//   printf("------------ ax= %d bx= %d \n", ax, bx);
//   printf("------------ ay= %d by= %d \n", ay, by);
//   printf("------------ dx= %d dy= %d \n", dx, dy);

   ax = ax + (int)(shfx1+0.5);
   ay = ay + (int)(shfy1+0.5);
   dx = dx + (int)(shfx2+0.5);
   dy = dy + (int)(shfy2+0.5);
   if(ax >= widthFormA) ax = widthFormA-1;
   if(ax < 0) ax = 0;
   if(ay >= heightFormA) ay = heightFormA-1;
   if(ay < 0) ay = 0;
   if(dx >= widthFormA) dx = widthFormA-1;
   if(dx < 0) ax = 0;
   if(dy >= heightFormA) dy = heightFormA-1;
   if(dy < 0) dy = 0;

//   sprintf(buff,"------------ widthFormA= %d, heightFormA= %d, ax= %d, ay= %d, bx= %d, dy = %d \n", widthFormA, heightFormA, ax, ay, bx, dy);
//   saveLog("", buff);

//   sprintf(buff,"------------ widthFormA= %d, heightFormA= %d, ax= %d, ay= %d, bx-ax= %d, dy-ay = %d \n", widthFormA, heightFormA, ax, ay, bx-ax, dy-ay);
//   saveLog("", buff);

//   printf("------------ ax= %d bx= %d \n", ax, bx);
//   printf("------------ ay= %d by= %d \n", ay, by);
//   printf("------------ dx= %d dy= %d \n", dx, dy);

 x = bx - ax;
 y = by - ay;
 r = sqrt((double)((long)x*(long)x + (long)y*(long)y));
 width = ((WORD)r) + 1;
 if(r != 0.0)
    {cosTeta = (long)((x/r)*SCALE);
     sinTeta = (long)((y/r)*SCALE);
    }
 else
    {cosTeta = SCALE;
     sinTeta = 0L;
    }
 x = dx - ax;
 y = dy - ay;
 r = sqrt((double)((long)x*(long)x + (long)y*(long)y));
 height = ((WORD)r) + 1;

 byteWidth = ((width + 2*extractMargin + 31)/32)*4;


 if(widthFormA < width)
	 return (-100);

 if(heightFormA < height)
	 return (-100);

// sizeFieldBufferAlloc =
//     (DWORD)(byteWidth)*(DWORD)(height + 2*extractMargin) +
//     (DWORD)(offset/2) + 100;

// sizeFieldBuffer = (DWORD)(byteWidth)*(DWORD)(height + 2*extractMargin);

// hgFieldBufferA = *phgFieldBufferA;

 if(hpFieldBufferAInp->ptr != NULL)
    {pdwFieldBufferA = (DWORD *)(hpFieldBufferAInp->ptr);
     dw0 =  *pdwFieldBufferA; dw1 = *(pdwFieldBufferA + 1);
     size = dw0;
     if(size != 0L) offset = 8;
     else offset = dw1;
//     GlobalUnlock(hgFieldBufferA);
    }
 else offset = 8;

 sizeFieldBufferAlloc =
     (DWORD)(byteWidth)*(DWORD)(height + 2*extractMargin) +
     (DWORD)(offset/2) + 100L;
 sizeFieldBuffer = (DWORD)(byteWidth)*(DWORD)(height + 2*extractMargin);

 recalculateAllocBuffer(&sizeFieldBufferAlloc, 300000);
 err = reallocMemoryBuffer (hpFieldBufferAInp, sizeFieldBufferAlloc);
 if(err < 0) {
//       	sprintf(pImg->error,"createZoneStruct (\"%s\") calloc failed: %s\n",pImg->szIMGName, strerror(errno));
       	return -1;
   }

// hgFieldBufferA = *phgFieldBufferA;
 pFieldBufferA = (BYTE *)hpFieldBufferAInp->ptr;
 pwFieldBufferA = (WORD *)pFieldBufferA;
 if(offset != 8)
    {pdwFieldBufferA = (DWORD *)pFieldBufferA;
     *pdwFieldBufferA        = dw0;
     *(pdwFieldBufferA + 1L) = dw1;
    }

 pFieldBufferA = pFieldBufferA + offset;
// printf(" total size= %ld, offset= %d, sizeFieldBufferAlloc= %d \n", hpFieldBufferAInp->size, offset, sizeFieldBufferAlloc);

// sprintf(buff," total size= %ld, offset= %d, sizeFieldBufferAlloc= %d \n", hpFieldBufferAInp->size, offset, sizeFieldBufferAlloc);
// saveLog("", buff);
// sprintf(buff," fieldStart= %u, fieldEnd= %u \n", (unsigned)pFieldBufferA, (unsigned)(pFieldBufferA+ sizeFieldBufferAlloc));
// saveLog("", buff);


 err = desKew
       (pFormBufferA,
        widthFormA,
        heightFormA,
        ax,
        ay,
        cosTeta,
        sinTeta,
        pFieldBufferA,
        width,
        height,
        extractMargin
       );

 if(err < 0) {
              return(err);
             }
 *pwidth = width + 2*extractMargin;
 *pheight = height + 2*extractMargin;

// sprintf(buff," *pwidth= %d, *pheight= %d \n", *pwidth, *pheight);
// saveLog("", buff);

 return(0);
}

int desKew(BYTE * pFormBuffer, WORD widthForm, WORD heightForm, int originX, int originY,
           long cosTeta, long sinTeta, BYTE * pFieldBuffer, WORD width, WORD height, WORD extractMargin){
 WORD i,m;
 int j;
 DWORD k;
 BYTE * pField;
 BYTE byteForm;
 BYTE byteField;
 WORD x,y;
 long xScale,yScale;
 WORD byteWidth;
 DWORD sizeFieldBuffer;
 DWORD halfScale;
 WORD extractMargin8;
 WORD width8;
 WORD scaleexp;
 BYTE masc[] = {127, 191, 223, 239, 247, 251, 253, 254};
 int j1,j2,j3,jmin,jmax;

// char buff[500];

//  printf(" widthForm= %d, heightForm= %d, originX= %d, originY = %d \n", widthForm, heightForm, originX, originY);
//  printf(" width= %d, height= %d \n", width, height);

// sprintf(buff," widthForm= %d, heightForm= %d, originX= %d, originY = %d \n", widthForm, heightForm, originX, originY);
// saveLog("", buff);
// sprintf(buff," width= %d, height= %d \n", width, height);
// saveLog("", buff);



 scaleexp = 16;
 halfScale = 1L<<15;
 extractMargin8 = extractMargin/8;

 byteWidth = ((width + 2*extractMargin + 31)/32)*4;
 sizeFieldBuffer = (DWORD)byteWidth*(DWORD)(height + 2*extractMargin);
 width8 = width/8;

 j1 = (int)(originY + ((width*sinTeta)>>scaleexp));
 j2 = (int)(originY + ((height*cosTeta)>>scaleexp));
 j3 = (int)(originY + ((width*sinTeta + height*cosTeta)>>scaleexp));

 jmin = (int)(originY - 1);
 if(j1<jmin) jmin = j1;
 if(j2<jmin) jmin = j2;
 if(j3<jmin) jmin = j3;
 if(jmin<0) jmin = 0;

 jmax = (int)(originY + 1);
 if(j1>jmax) jmax = j1;
 if(j2>jmax) jmax = j2;
 if(j3>jmax) jmax = j3;
 if(jmax>=(int)heightForm) jmax = (int)(heightForm - 1);

 for(j=jmin; j<=jmax; j++)
     pFormLine[j] =
            pFormBuffer + (long)(heightForm - j - 1)*(long)byteWidthT;

 for(k=0L; k<sizeFieldBuffer; k++) *(pFieldBuffer+k) = 255;
/*
 sprintf(buff," jmin= %d, jmax= %d\n", jmin, jmax);
 saveLog("", buff);
// printf(" width= %d, height= %d \n", width, height);
 {
 sprintf(buff," widthForm= %d, heightForm= %d, byteWidthT= %d \n", widthForm, heightForm, byteWidthT);
 saveLog("", buff);
 sprintf(buff," width= %d, height= %d, byteWidth= %d, extractMargin= %d, extractMargin8= %d\n", width, height, byteWidth, extractMargin, extractMargin8);
 saveLog("", buff);
 { unsigned int aa, bb;
    aa = heightForm*byteWidthT;
    bb = (unsigned int)(pFormBuffer+(long)(heightForm)*(long)byteWidthT);
 sprintf(buff," sizeForm= %u, formStart= %u, formEnd= %u\n", aa, (unsigned int)pFormBuffer, bb);
 saveLog("", buff);
 }
 }
*/
// return -1;
 for(j=0; j<(int)height; j++)
    {pField = pFieldBuffer +
              (long)(height + extractMargin - j - 1)*(long)byteWidth +
              (long)extractMargin8;

//  sprintf(buff," pField= %u\n", (unsigned)pField);
//  saveLog("", buff);


     xScale = halfScale - j*sinTeta;
     yScale = halfScale + j*cosTeta;

     for(i=0; i<width8; i++)
        {byteField = 255;
         for(m=0; m<8; m++)
            {x = originX + (int)(xScale>>scaleexp);
             y = originY + (int)(yScale>>scaleexp);
             byteForm = *(pFormLine[y]+(long)(x>>3));

//           sprintf(buff,"1 y= %d, byteForm= %u\n", y, (unsigned)(pFormLine[y]+(long)(x>>3)));
//           saveLog("", buff);

             if(!((byteForm<<(x%8))&128)) byteField &= masc[m];
             xScale += cosTeta;
             yScale += sinTeta;
            }
         *(pField+(long)i) = byteField;
        }
     if((width%8) != 0)
        {byteField = 255;
         for(m=0; m<(width%8); m++)
            {x = originX + (int)(xScale>>scaleexp);
             y = originY + (int)(yScale>>scaleexp);
             byteForm = *(pFormLine[y]+(long)(x>>3));
//    sprintf(buff,"2 y= %d, byteForm= %u\n", y, (unsigned)(pFormLine[y]+(long)(x>>3)));
//    saveLog("", buff);

             if(!((byteForm<<(x%8))&128)) byteField &= masc[m];
             xScale += cosTeta;
             yScale += sinTeta;
            }
         *(pField+(long)width8) = byteField;
        }
    }
// sprintf(buff," out\n");
// saveLog("", buff);
 return(0);
}

int RecognitionCall (LPIMGDATA img, LPTICKETDATA ticketDataPtr) {
	BYTE* lpProt;
	BYTE* pFieldDescr;
	BYTE * pDataBlock;
	LPFIELDDATA field = ticketDataPtr->fieldAddress;
	IMGDATA fieldImg;
	LPBYTE ptr = ticketDataPtr->fieldBMPPtr;
	DWORD bitsOfset = *(LPDWORD)(ptr + BFOOFBITSOFF);
	LPBYTE biPtr = ptr + BFOOFBITSOFF+4;

//	char buff[500];

	fieldImg.pixPtr = ptr+bitsOfset;
	fieldImg.bi.biWidth = *(LPDWORD)(biPtr + BIWIDTHOFF);
	fieldImg.bi.biHeight = *(LPDWORD)(biPtr + BIHEIGHTOFF);
	fieldImg.bi.biXPelsPerMeter = *(LPDWORD)(biPtr + BIXPELSPERMETEROFF);
	fieldImg.bi.biYPelsPerMeter = *(LPDWORD)(biPtr + BIYPELSPERMETEROFF);

	fieldImg.bi.biWidthPad = ((fieldImg.bi.biWidth+31)/32)*4;

	blackMarginRemoval(&fieldImg, ticketDataPtr->fieldAddress->name);

	strcpy(fieldImg.szIMGName, img->szIMGName);
	lpProt = ticketDataPtr->prototypePtr;
	pFieldDescr = lpProt + field->descr;


    reallocMemoryBuffer(&ticketDataPtr->workArea, (DWORD)(MAXCHARS * 22 + 4));
    pDataBlock = ticketDataPtr->workArea.ptr;

    ticketDataPtr->strings.lpCand1  = (char*)pDataBlock;
    ticketDataPtr->strings.lpCand2  = (char*)(pDataBlock + MAXCHARS + 1);
    ticketDataPtr->strings.lpResult = (char*)(pDataBlock + MAXCHARS * 2 + 2);
    ticketDataPtr->strings.lpReject = (char*)(pDataBlock + MAXCHARS * 3 + 3);
    ticketDataPtr->flConf = (LPWORD) (pDataBlock  + MAXCHARS * 4 + 4);
    ticketDataPtr->flQuad = (LPQUADRANGLE)(pDataBlock  + MAXCHARS * 6 + 4);

    ticketDataPtr->sizeWImg = img->bi.biWidth;
    ticketDataPtr->sizeHImg = img->bi.biHeight;

// printf("  bitsOfset= %d, biWidth= %d, biHeight=%d, biXPelsPerMeter= %d, biYPelsPerMeter= %d \n", bitsOfset, fieldImg.bi.biWidth, fieldImg.bi.biHeight, fieldImg.bi.biXPelsPerMeter, fieldImg.bi.biYPelsPerMeter);
//return 0;
//   sprintf(buff,"before Recognition  \n");
//   saveLog("", buff);

	return  Recognition(&fieldImg, ticketDataPtr, pFieldDescr, *(LPWORD)(lpProt + OffFdDpiX),  *(LPWORD)(lpProt + OffFdDpiY),  ticketDataPtr->wCorrectionLevel,
			&ticketDataPtr->strings, ticketDataPtr->flConf, ticketDataPtr->flQuad);

//    sprintf(buff,"after Recognition  \n");
//    saveLog("", buff);


//	char zeroLine[1] = "";
//	ticketDataPtr->strings.lpResult = zeroLine;
//	ticketDataPtr->strings.lpCand1  = zeroLine;
//	ticketDataPtr->strings.lpCand2  = zeroLine;
//	ticketDataPtr->strings.lpReject = zeroLine;
//    return 0;
}
void histVPtrPix(LPBYTE ptrPix, int widthImg, int hightImg, int MaxSize, int x1, int x2, int y1, int y2, int *vHist)
 {
   int   j, kk, curInd, yy;
   int  yD, ch;
   int lByte, rByte, rL, rR, w_bt;
   long nY;
   BYTE *hptr;
   BYTE mask;

     w_bt= widthImg;
     memset(vHist,0x00,(unsigned int)(sizeof(int)*MaxSize));
     vHist[0]=x2-x1+1;

      lByte=x1/8;
      rL   =x1%8;
      if(rL!=0) {lByte++;}

      rByte=x2/8;
      rR   =x2%8;
      if(rR!=7) {rByte--;}

      for(yy=y1;yy<=y2;yy++) {
         yD=yy;
         yD= hightImg-1-yD;
         nY=(long)yD*(long)w_bt;
         hptr=ptrPix+nY+(long)(lByte-1);

         curInd=1;
         if(rL!=0) {
              curInd=9-rL;
              if((*hptr)<255) {
                  mask=LEFT_BIT;
                  mask >>=rL;
                  for(kk=1;kk<curInd;kk++) {
                      ch=(~hptr[0])&mask;
                      if(ch!=0) {
                           vHist[kk]++;
                           }
                      mask >>=1;
                      }
                  }
               }

         for(j=lByte;j<=rByte;j++) {
              hptr+=1L;
              if((*hptr)<255) {
                  mask=LEFT_BIT;
                  for(kk=0;kk<8;kk++) {
                      ch=(~hptr[0])&mask;
                      if(ch!=0) {
                           vHist[curInd]++;
                           }
                      curInd++;
                      mask >>=1;
                      }
                  }
              else {curInd+=8;}
              }

           if(rR!=7) {
              hptr+=1L;
              if((*hptr)<255) {
                  mask=LEFT_BIT;
                  for(kk=0;kk<=rR;kk++) {
                      ch=(~hptr[0])&mask;
                      if(ch!=0) {
                           vHist[curInd]++;
                           }
                      curInd++;
                      mask >>=1;
                      }
                  }
              }
         }
    }/* of histVPtrPix()*/

void cleanMargin(LPBYTE ptrPix, int widthImg, int hightImg, int x1, int x2){
	   int   j, kk, curInd,  yy;
	   int  yD;
	   int lByte, rByte, rL, rR, w_bt;
	   long nY;
	   BYTE *hptr;
	   BYTE mask;

	      w_bt= widthImg;
	      lByte=x1/8;
	      rL   =x1%8;
	      if(rL!=0) {lByte++;}

	      rByte=x2/8;
	      rR   =x2%8;
	      if(rR!=7) {rByte--;}

	      for(yy=0;yy<hightImg;yy++) {
	         yD=yy;
	         yD= hightImg-1-yD;
	         nY=(long)yD*(long)w_bt;
	         hptr=ptrPix+nY+(long)(lByte-1);

	         curInd=1;
	         if(rL!=0) {
	              curInd=9-rL;
	              if((*hptr)<255) {
	                  mask=LEFT_BIT;
	                  mask >>=rL;
	                  for(kk=1;kk<curInd;kk++) {
//                          mask=LEFT_BIT;
//                          mask >>=kk;
                          hptr[0]|=mask;

//	                      ch=(~hptr[0])&mask;
//	                      if(ch!=0) {
//	                           vHist[kk]++;
//	                           }
	                      mask >>=1;
	                      }
	                  }
	               }

	         for(j=lByte;j<=rByte;j++) {
	              hptr+=1L;
	              if((*hptr)<255) {
	                  mask=LEFT_BIT;
	                  for(kk=0;kk<8;kk++) {
                          hptr[0]|=mask;
//	                      ch=(~hptr[0])&mask;
//	                      if(ch!=0) {
//	                           vHist[curInd]++;
//	                           }
//	                      curInd++;
	                      mask >>=1;
	                      }
	                  }
	              else {curInd+=8;}
	              }

	           if(rR!=7) {
	              hptr+=1L;
	              if((*hptr)<255) {
	                  mask=LEFT_BIT;
	                  for(kk=0;kk<=rR;kk++) {
                          hptr[0]|=mask;
//	                      ch=(~hptr[0])&mask;
//	                      if(ch!=0) {
//	                           vHist[curInd]++;
//	                           }
//	                      curInd++;
	                      mask >>=1;
	                      }
	                  }
	              }
	         }
   }/* of cleanMargin()*/

void getBmarginIdx(int *vHist, int h_size, int th, int* i1, int* i2){
	 int mass, flg;
	 int i;

	   mass = 0;
	   flg = 0; *i1 = 0; *i2 = 0;
	   for(i=3;i<h_size;i++){
		   mass = (vHist[i-2]+vHist[i-1]+vHist[i])/3;
		   if(mass > th){
			   if(flg == 0){
				   *i1 = i-2;
				   flg++;
			   }
		   }
		   if(mass < th){
			   if(flg == 1){
				   *i2 = i;
				   flg++;
			   }
		   }
	       if(flg > 1) break;
	   }
	   if(*i1 > 0 && *i2 == 0)
		   *i2 = h_size-1;
}
void getBmarginIdx_0(int *vHist, int h_size,  int* i1, int* i2){
	int av = 0;
	int i, cc, k;

	*i1 = 0; *i2 = 0;
	cc = 0;
    for(i=10;i<h_size-10;i++){
	   if(vHist[i] > 5){
		   av += vHist[i];
		   cc++;
	   }
    }
    if(cc>0)
	   av /= cc;
    av += av;
//    printf("   av= %d\n", av);

    cc = 0;
	for(i=1;i<h_size;i++){
	   if(vHist[i] > av || i > 150){
		   cc = i;
		   break;
	   }
	 }
//	printf("   cc= %d \n", cc);

	 k = 0;
     if(cc > 0) {
	    for(i=cc;i<h_size;i++){
	 	    if(vHist[i] < av || i > 250){
	 		   k = i;
 		      break;
		   }
	   }
		   if(k-cc>8){
//	printf("   k= %d, cc= %d \n", k, cc);
			   if(cc > 4) cc--;
			   *i1 = cc;
			   *i2 = k+1;
		   }
	   }
//	printf("   av= %d \n", av);
}
void getBlackLine_L(int *vHist,  int th, int* i1, int* i2){
	int i, cc, k, k1, k2;
	int stop = 100;

	*i1 = 0; *i2 = 0;
	cc = 0;
	k = 0;
	if(vHist[0] < stop) stop = vHist[0];
    for(i=1;i<stop;i++){
	   if(vHist[i] > th)
		     cc++;
	   else
		   cc = 0;
	   if(cc > 2) {k=i;break;}
    }
    k1 = k;
    k2 = k;
    if(k > 0){
        for(i=k;i>0;i--){
     	   if(vHist[i] == 0) {k1 = i;break;}
        }
        for(i=k;i<stop;i++){
     	   if(vHist[i] == 0) {k2 = i;break;}
        }
    }
    if(k2-k1 > 20 || k2-k1 < 2)
    	return;
    *i1 = k1; *i2 = k2;
  }


void blackMarginRemoval(LPIMGDATA img, char* name){
 int h_size = 500;
 int h_size1 = 400;
 LPBYTE ptr = img->pixPtr;
 int widthImg = img->bi.biWidthPad;
 int hightImg = img->bi.biHeight;
 int vHist[h_size];
 int i1, i2, d;
// int i;
	histVPtrPix(ptr, widthImg, hightImg, h_size, 0, h_size, 0, hightImg-1, vHist);


/*
	if(name != NULL && strcmpi(name, "NUMBERS") == 0){
		int i;
		printf("   widthImg= %d, hightImg= %d \n", widthImg, hightImg);
		 for(i=0;i<h_size1;i++){
			    printf("   i= %d, vHist= %d \n", i, vHist[i]);
		  }
	}
*/

	   getBmarginIdx_0(vHist, h_size,  &i1, &i2);
	   d = i2-i1;
	   if(d > 5) {
//  printf("   i1= %d, i2= %d, d= %d  \n", i1, i2, d);
          cleanMargin(ptr, widthImg, hightImg, i1, i2);
	   }
	   else {
	     getBmarginIdx(vHist, h_size, (2*hightImg/3), &i1, &i2);

//   printf("   i1= %d, i2= %d \n", i1, i2);

         d = i2-i1;
        if(d > 5) {
	       if(d< 200) i2 +=8;
	       i1 -= 8;
	       if(i1 < 0) i1 = 0;
	       cleanMargin(ptr, widthImg, hightImg, i1, i2);
        }
   }
   histVPtrPix(ptr, widthImg, hightImg, h_size, 0, h_size, 0, hightImg-1, vHist);
   getBlackLine_L(vHist, (hightImg*4)/5,  &i1, &i2);
   d = i2-i1;
//   printf("   i1= %d, i2= %d, d= %d \n", i1, i2, d);
   if(d > 0) {
     cleanMargin(ptr, widthImg, hightImg, i1, i2);
   }


   histVPtrPix(ptr, widthImg, hightImg, 500, img->bi.biWidth-400, img->bi.biWidth-1, 0, hightImg-1, vHist);

   getBmarginIdx(vHist, h_size1, (2*hightImg/3), &i1, &i2);
   d = i2-i1;
   if(d > 5) {
	   if(d< 200) i1 -=8;
	   cleanMargin(ptr, widthImg, hightImg, img->bi.biWidth-400+i1, img->bi.biWidth-400+i2);
   }


}
int   histTmp[3000];
double getFieldMass(LPTICKETDATA ticketDataPtr){
	double mass = 0;
    int i;
    LPBYTE ptr = ticketDataPtr->fieldBMPPtr ;

   	LPBYTE biPtr = ptr + BFOOFBITSOFF+4;
   	int wWidth = *(LPDWORD)(biPtr + BIWIDTHOFF);
   	int wHeight = *(LPDWORD)(biPtr + BIHEIGHTOFF);
   	int wWidthPad = ((wWidth + 31) /32) * 4;

	histVPtrPix(ptr, wWidthPad, wHeight, wWidth-1, 0, wWidth-1, 0, wHeight-1, histTmp);
    for(i=1;i<wWidth;i++){
		    mass += histTmp[i];
    }
    mass = ((double)(wWidthPad*wHeight))/mass;
//    printf("   mass= %e \n", mass);

    return mass;
}
void copyFieldResults(LPTICKETRESULTS ticketResults, FIELD* f, int idx, char *name){
	int i, len;
	if(idx < NUMFIELDS_RESULTS) {
		len = strlen(f->result);
		strcpy(ticketResults->fieldsResults[idx].name, name);
		strcpy(ticketResults->fieldsResults[idx].result, f->result);
		strcpy(ticketResults->fieldsResults[idx].cand2, f->cand2);
	    for(i=0;i<len;i++){
		    ticketResults->fieldsResults[idx].flConf[i] = f->conf[i];
	    }
	    copyQuad(ticketResults->fieldsResults[idx].flQuad, f->quad, 0, len-1);
	}
}
void setTicketTypeFromMass(LPTICKETRESULTS ticketResults, double fMass){
	int i;
	ticketResults->type = -1;
	ticketResults->typeCnf = 0;
	if(fMass < 0.5) ticketResults->type = 0; // Mega million
	if(fMass > 0.8) ticketResults->type = 1; // Power ball
    for(i=0;i<NUMFIELDS_RESULTS;i++){
    	ticketResults->fieldsResults[i].name[0] = 0;
    }
}

 int testLogoField(LPTICKETDATA ticketDataPtr){
        int i, k, cc;
//        char buff[500];
        int tH1 = 15;
        LPBYTE ptr = ticketDataPtr->fieldBMPPtr;

       	LPBYTE biPtr = ptr + BFOOFBITSOFF+4;
       	int wWidth = *(LPDWORD)(biPtr + BIWIDTHOFF);
       	int wHeight = *(LPDWORD)(biPtr + BIHEIGHTOFF);
       	int wWidthPad = ((wWidth + 31) /32) * 4;

    	histHTest_X1_X1(ptr, wWidthPad, wHeight,  0, wWidth-1, 0, wHeight-1, histTmp);
    	cc = 0;
    	k = histTmp[0];
        for(i=1;i<histTmp[0];i++){
        	if(histTmp[i] > tH1) cc++;
        	if(cc > 15) {k = i; break;}
//        	 sprintf(buff,"h---i= %d, histTmp[i]= %d\n",i, histTmp[i]);
//        	saveLog("", buff);
       }
//   	 sprintf(buff,"h---k= %d, histTmp[0]/2= %d\n",k, histTmp[0]/2);
//   	  saveLog("", buff);

        if(k > histTmp[0]/2){
        	return k;
        }

     return 0;
    }

 int testLogoField2(LPTICKETDATA ticketDataPtr, int y1){
//        int i, k, cc;
	    int i, k;
	    int start, startN, stop;
        char buff[500];
        int d, yShift, cc;
        double ccD;
//        int mass;
        LPBYTE ptr = ticketDataPtr->fieldBMPPtr;

       	LPBYTE biPtr = ptr + BFOOFBITSOFF+4;
       	int wWidth = *(LPDWORD)(biPtr + BIWIDTHOFF);
       	int wHeight = *(LPDWORD)(biPtr + BIHEIGHTOFF);
       	int wWidthPad = ((wWidth + 31) /32) * 4;

    	 maxBlackWidth(ptr, wWidthPad, wHeight,  8, wWidth-9, 8, wHeight-9, histTmp);
// debug
//         for(i = 1;i<histTmp[0];i++) {
//      	     sprintf(buff,"mass; i= %d, histTmp[i]= %d\n",i, histTmp[i]);
//      	     saveLog("", buff);
//         }


// clean trash
    	     {int s1[5], s2[5];
    	         for(i = 0;i<5;i++) {
    	    	     s1[i] = 0; s2[i] = 0;
    	          }
    	         k = 0;
    	         for(i = 1;i<histTmp[0];i++) {
    	   	        if(histTmp[i] > 7) {if( s1[k] == 0) {s1[k] = i;}}
    	   	        else {if(s1[k] != 0){ s2[k] = i;k++;}}
    	   	        if(k>=5)break;
    	         }
    	         if(s1[k] != 0){ s2[k] = histTmp[0];}
    	         for(i = 0;i<5;i++) {
//      	     sprintf(buff,"s1[i]= %d, s2[i]= %d\n",s1[i], s2[i]);
//  	      	     saveLog("", buff);

    	            if(s1[i]<s2[i] && s2[i]-s1[i] < 40){
    	        	   if(s1[i] > 10) s1[i] -= 4;
    	        	   if(s2[i] < histTmp[0]-10) s2[i] += 4;
    	               for(k = s1[i];k<s2[i];k++)
    	       	           histTmp[k] = 0;
   	               }
    	        }
    	     }

//    	     saveLog("\n", "\n");
// debug
//        for(i = 1;i<histTmp[0];i++) {
//      	     sprintf(buff,"mass; i= %d, histTmp[i]= %d\n",i, histTmp[i]);
//      	     saveLog("", buff);
//        }


// logo start, stop
   	  start = histTmp[0];
      for(i = 1;i<histTmp[0];i++) {
    	    if(histTmp[i] > 2) {start = i;break;}
//   	     sprintf(buff,"mass; i= %d, histTmp[i]= %d\n",i, histTmp[i]);
//   	     saveLog("", buff);
      }
   	  stop = 1;
      for(i = histTmp[0]-1;i>0;i--) {
    	    if(histTmp[i] > 2) {stop = i;break;}
//   	     sprintf(buff,"mass; i= %d, histTmp[i]= %d\n",i, histTmp[i]);
//   	     saveLog("", buff);
      }
      d = stop-start;
// 	     sprintf(buff,"------------------   start= %d, stop= %d, histTmp[0]= %d, stop-start= %d\n",start, stop, histTmp[0], d);
//  	     saveLog("", buff);
//    sprintf(buff,"------------------   start= %d, histTmp[0]-stop= %d\n",start, histTmp[0]-stop);
//    saveLog("", buff);


    // shift up
          if(start < 5 && histTmp[0]-stop > 50 && d > 5){
        	  yShift = histTmp[0]-stop - 10;
//    	      sprintf(buff,"-----------shift up   yShift= %d\n",yShift);
//    	      saveLog("", buff);
        	  return yShift;
          }

// maybe shift up
          startN = start;
          if(d < 100 && start < 50 && histTmp[0]-stop > 50){
              for(i = 1;i<start;i++) {
            	    if(histTmp[i] > 1) {startN = i;break;}
                }
              d = stop-startN;
        	  if(startN < start && startN < 20 && d > 10) {
            	 yShift = histTmp[0]-stop - 10;
//          	    sprintf(buff,"-----------shift up   yShift= %d, d= %d\n",yShift, d);
//          	    saveLog("", buff);
              	  return yShift;
        	  }

          }
 // shift down
    if(start > 50 && histTmp[0]-stop < 5){
          	  yShift = 10 - start;
//       	      sprintf(buff,"-----------shift down   histTmp[0]-stop - 10= %d\n",yShift);
//       	      saveLog("", buff);
           	  return yShift;
     }
    if(d < 40 || d > 300){
 	      sprintf(buff,"-----------------------------------------------------------------additional test\n");
 	      saveLog("", buff);
 	      cc = 0;
 	      for(i = 2;i<histTmp[0];i++) {
 	    	    if(histTmp[i]!=histTmp[i-1]) cc++;
 	      }
 	      ccD = (double)cc/(double)histTmp[0];
  	      sprintf(buff,"-----------ccD= %e, y1 = %d\n",ccD,  y1);
  	      saveLog("", buff);
  	      if(ccD>0.05 && y1 < 300){
// shift up
  	    	  yShift = 140;
  	    	  if(yShift> y1-8) yShift = y1-8;
  	    	  return yShift;
  	      }
  	      if(ccD<0.02){
// shift down
  	    	  yShift = -140;
  	    	  return yShift;
  	      }

    }

     return 0;
    }

 int getLogoType(LPTICKETDATA ticketDataPtr){
	    int i;
	    int start,  stop, mass;
//        char buff[500];
        int d;
        LPBYTE ptr = ticketDataPtr->fieldBMPPtr;

    	LPBYTE biPtr = ptr + BFOOFBITSOFF+4;
    	int wWidth = *(LPDWORD)(biPtr + BIWIDTHOFF);
    	int wHeight = *(LPDWORD)(biPtr + BIHEIGHTOFF);
    	int wWidthPad = ((wWidth + 31) /32) * 4;

 	 maxBlackWidth(ptr, wWidthPad, wHeight,  8, wWidth-9, 8, wHeight-9, histTmp);
// logo start, stop
 	   	  start = histTmp[0];
 	      for(i = 1;i<histTmp[0];i++) {
 	    	    if(histTmp[i] > 2) {start = i;break;}
 	//   	     sprintf(buff,"mass; i= %d, histTmp[i]= %d\n",i, histTmp[i]);
 	//   	     saveLog("", buff);
 	      }
 	   	  stop = 1;
 	      for(i = histTmp[0]-1;i>0;i--) {
 	    	    if(histTmp[i] > 2) {stop = i;break;}
 	//   	     sprintf(buff,"mass; i= %d, histTmp[i]= %d\n",i, histTmp[i]);
 	//   	     saveLog("", buff);
 	      }
 	      d = stop-start+1;
 	      mass = 0;
 	      for(i = start;i<=stop;i++) {
 	    	    mass += histTmp[i];
 	      }
// 	      mass /= d;
// 	 	     sprintf(buff,"------------------   start= %d, stop= %d, mass= %d, stop-start= %d\n",start, stop, mass, d);
 //	  	     saveLog("", buff);
          if(d > 110) return 0;
 	  	  return 1;

 }

   int extFieldAgain(LPIMGDATA pImg, LPFIELDDATA field, LPTICKETDATA ticketDataPtr, IMAGEH* pFieldH){
	   WORD p1X = field->quad.p1X;
	   WORD p1Y = field->quad.p1Y;
	   WORD p2X = field->quad.p2X;
	   WORD p2Y = field->quad.p2Y;
	   WORD p4X = field->quad.p4X;
	   WORD p4Y = field->quad.p4Y;
	   WORD* pwidth;
	   WORD* pheight;
	   WORD width;
	   WORD height;
	   int  err;
	   WORD * pwBuffer;
	   DWORD sizeBuffer;
	   WORD wDpiX;
       WORD wDpiY;



       pTBuffer   = pImg ->pixPtr;
       widthT     = pImg -> bi.biWidth;
       heightT    = pImg -> bi.biHeight;
       wDpiX      = pImg -> bi.biXPelsPerMeter;
       wDpiY      = pImg -> bi.biYPelsPerMeter;
       pwidth = &width;
       pheight = &height;
       HLPBYTE hpFieldH;
       hpFieldH.ptr = pFieldH->himgPtr.ptr;
       err =
    extractField(pTBuffer, widthT, heightT,
           p1X, p1Y,
           p2X, p2Y,
           p4X, p4Y,
           &pFieldH->himgPtr,
           pwidth,
           pheight,
           EXTRACTMARGIN
          );
      if(err < 0) return(err);

  //  hgBuffer = *phgBuffer;

    sizeBuffer = (DWORD)(*pwidth)*(DWORD)(*pheight + 2*EXTRACTMARGIN);

    pwBuffer = (WORD *)pFieldH->himgPtr.ptr;
    *(pwBuffer+2)= (WORD)(sizeBuffer%65536);
    *(pwBuffer+3)= (WORD)(sizeBuffer/65536);
  //  GlobalUnlock(hgBuffer);
    pFieldH -> wWidth   = *pwidth;
    pFieldH -> wHeight  = *pheight;
    pFieldH -> wDpiX    = wDpiX;
    pFieldH -> wDpiY    = wDpiY;
    return 0;

   }

  //  pFieldH -> hgBuffer = hgBuffer;

    int extractFieldAgain(LPIMGDATA pImg, LPFIELDDATA field, LPTICKETDATA ticketDataPtr){
        LPBITMAPFILEHEADER bf;
        LPBITMAPINFOHEADER bi;
        LPRGBQUAD          qd;
        IMAGEH       fieldH;
        DWORD        offBits;
 	    int  err;
 	    LPDWORD      lpImage;

        fieldH.himgPtr.ptr = NULL;
        fieldH.himgPtr.size = 0;

        err = 0;
        fieldH.himgPtr.ptr = calloc(8, sizeof(BYTE));
        fieldH.himgPtr.size = 8;

        if (err < 0 || fieldH.himgPtr.ptr == NULL) {
            return -11;     // out of memory
        }
        if(ticketDataPtr->fieldBMPPtr) {
            free (ticketDataPtr->fieldBMPPtr);
            ticketDataPtr->fieldBMPPtr = NULL;
        }
        offBits = sizeof (BITMAPFILEHEADER) +
                  sizeof (BITMAPINFOHEADER) +
                  sizeof (RGBQUAD) * 2;
        lpImage = (LPDWORD) fieldH.himgPtr.ptr;
        *lpImage = 0;
        *(lpImage + 1) = offBits;

        err = extFieldAgain(pImg, field, ticketDataPtr, &fieldH);
        if (err < 0 || fieldH.himgPtr.ptr == NULL) {
            return -11;     // out of memory
        }


        bf = (LPBITMAPFILEHEADER) fieldH.himgPtr.ptr;
        bf->bfType[0] = 0x42;
        bf->bfType[1] = 0x4d;
	    set4Bytes(bf->bfSize, offBits + (((fieldH.wWidth /** biBitCount*/ + 31) /32) * 4) * fieldH.wHeight);
	    bf->bfReserved1[0] = 0;
	    bf->bfReserved1[1] = 0;
        set4Bytes(bf->bfOffBits, offBits);

        bi = (LPBITMAPINFOHEADER)((LPBYTE)bf + sizeof (BITMAPFILEHEADER));
	    set4Bytes(bi->biSize, sizeof (BITMAPINFOHEADER));
        set4Bytes(bi->biWidth, fieldH.wWidth);
	    set4Bytes(bi->biHeight, fieldH.wHeight);
	     bi->biPlanes[0] = 1;
	     bi->biPlanes[1] = 0;
	     bi->biBitCount[0]= 1;
	     bi->biBitCount[1]= 0;
	    set4Bytes(bi->biCompression, BI_RGB);
		set4Bytes(bi->biSizeImage, 0);
		set4Bytes(bi->biXPelsPerMeter, fieldH.wDpiX);
	    set4Bytes(bi->biYPelsPerMeter, fieldH.wDpiY);
		set4Bytes(bi->biClrUsed, 0);
		set4Bytes(bi->biClrImportant, 0);

        // generate pair of RGBQUADs (black and white)
        qd = (LPRGBQUAD)((LPBYTE)bi + sizeof (BITMAPINFOHEADER));
        *(LPDWORD)qd       = 0x00000000;
        *(LPDWORD)(qd + 1) = 0x00ffffff;

        if(ticketDataPtr->fieldBMPPtr)
            free (ticketDataPtr->fieldBMPPtr);
        ticketDataPtr->fieldBMPPtr = fieldH.himgPtr.ptr;


        ticketDataPtr->fieldAddress = field;
        return 0;
    }
    int getxShiftForNumbersField(int *vHist, int start, int d){
        int xShift, i, cc, k;
        cc = 0;
        k = 0;
        xShift = 0;
        for(i = start;i>start-d;i--){
        	if(vHist[i] > 10) cc++;
        	else cc = 0;
        	if(cc > 2) {k = i;break;}
        }
        if(k == 0) return xShift;

        for(i = k;i>1;i--){
        	if(vHist[i] == 0) {xShift = start -i;break;}
        }
        return xShift;
    }
    int getyShiftForNumbersField(int *vHist, int start, int d){
        int yShift, i, cc;
        int av1, av2, min, iMin;
        cc = 0;
        yShift = 0;
        av1 = 0;
        for(i = start;i<vHist[0]-2;i++){
        	av1 += vHist[i];
        	cc++;
        }
        if(cc > 0) av1 /= cc;

//        printf("   av1= %d \n", av1);

        if(av1 < 5) return yShift;
        cc = 0;
        av2 = 0;
        for(i = start;i>start-d;i--){
        	av2 += vHist[i];
        	cc++;
        }
        if(cc > 0) av2 /= cc;

//        printf("   av2= %d \n", av2);

        if(av2*2 < av1) {
        	cc = 0;
            av2 = 0;
            for(i = start;i>start-5;i--){
            	av2 += vHist[i];
            	cc++;
            }
            if(cc > 0) av2 /= cc;
//            printf("   av2= %d \n", av2);
            if(av2*2 < av1) return yShift;
        }

        min = 100; iMin = -1;
        for(i = start;i>start-d;i--){
        	av1 = (vHist[i]+vHist[i-1]+vHist[i-2])/3;
        	if(min > av1) {min = av1;iMin = i;}
        }

//      printf("   iMin= %d \n", iMin);

        if(iMin < 0) return yShift;
        yShift = start - iMin;
        return yShift;
    }
    void getBlackLine2_L(int *vHist,  int thR, int* i1, int* i2){
    	int i, cc, k;
    	int stop = vHist[0];
    	int a1, a2, ratioL, ratioR;
    	int a3, a4;
//    	char buff[500];

    	*i1 = 0; *i2 = 0;
    	cc = 0;
    	k = 0;
        for(i=1;i<stop-20;i++){
   		   a1 = (vHist[i]+vHist[i+1])/2;
   		   a2 = (vHist[i+2]+vHist[i+3])/2;
   		   ratioL = 0;
   		   if(a2 > 60) {
   		      if(a1 == 0 ) ratioL = 100;
   		      else ratioL = a2/a1;
   		      if(ratioL > 10){
   	   		      a3 = (vHist[i+1]+vHist[i+2])/2;
   	   		      a4 = (vHist[i+3]+vHist[i+4])/2;
   	   		      if(a4 > a2) {
   	   		          if(a3 == 0 ) i++;
   	   		          else if(a4/a3 > ratioL) i++;
   	   		      }
//   	   		sprintf(buff," ratioL= %d, i= %d\n",ratioL, i);
//   	   		saveLog("-------", buff);

//        printf("   ratioL= %d, i= %d\n", ratioL, i);
   		    	for(k=i+2;k<i+10;k++){
   		   		   a1 = (vHist[k]+vHist[k+1])/2;
   		   		   a2 = (vHist[k+2]+vHist[k+3])/2;
   		   		   ratioR = 0;
   		   		   if(a1 > 60) {
   		   		      if(a2 == 0 ) ratioR = 100;
   		   		      else ratioR = a1/a2;
   		   		   }
//        printf("   ratioR= %d, k= %d\n", ratioR, k);

//   		sprintf(buff,"   ratioR= %d, k= %d\n", ratioR, k);
//	   	saveLog("-------", buff);

  		    	   if(ratioR>10) {
  		    		   *i1 = i+2;*i2 = k+2;return;
  		    	   }
   		    	}

   		      }
           }
        }
      }


	  void verticalLineRemovalField(LPBYTE pTBuffer, int width, int height, int y1, int y2){
	  	  int h_size = 400;
	  	  int vHist[h_size];
	  	  WORD byteWidthHist = ((width + 31)/32)*4;
	  	  int i1, i2, d;
//    	char buff[500];

//	  	  int i;

          histVPtrPix(pTBuffer, byteWidthHist, height, h_size, 8, h_size, y1, y2, vHist);
//		   for(i=0;i<vHist[0];i++){
//		  		    printf("   i= %d, vHist= %d, h= %d \n", i, vHist[i], y2-y1);
//		  	  }


          getBlackLine2_L(vHist, 10,  &i1, &i2);
          d = i2-i1;
//         printf("   i1= %d, i2= %d, d= %d \n", i1, i2, d);
//    		sprintf(buff,"   i1= %d, i2= %d, d= %d \n", i1, i2, d);
// 	   	saveLog("-------", buff);

          if(d > 0) {
            cleanMargin(pTBuffer, byteWidthHist, height, i1, i2);
          }



	  }

