/*
 * UtilRecognition.c
 *
 *  Created on: Aug 31, 2012
 *      Author: Wicek
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
//#include  "../headers/Image.h"
//#include  "../headers/Functions.h"
#include  "../headers/UtilRecognition.h"


int  initFieldR(
	 char* fieldName,
	 int sizeWImg,
	 LPIMAGEL  lpDataIm,     /* image structure */
     int        iTypeField,    /* filed type
                                    0 - color dropout,
                                    1 - frame box,
                                    2 - cell box,
                                    3 - frame box ( second method ),
                                    4 - check box
                                    5 - check box color dropout,
                                    6 - multiline color dropout,
                                    7 - multiline frame box,
                                    8 - multiline cell box (not supported)
                                    9 - divided cell box   (not supported)
                                   10 - divided frame box  (not supported)
                                   11 - lines
                                   30 - separate cell boxes

                              */
   int         iCharType,    /* type of expected characters:
                                 0 - handprinted digits,
                                 1 - handprinted small letters,
                                 2 - handprinted capital letter
                                 3 - handprinted mixed letters
                                 4 - OCR digits,
                                 5 - OCR small letters,
                                 6 - OCR capital letters,
                                 7 - OCR mixed lettrs
                                 8 - Custom Set 1,
                                 9 - Custom Set 2,
                                10 - Custom set 3,
                              */
   int          iSegmType,    /* segmentation type                */
                              /* 0 - cuting from cells,           */
                              /* 1 - segmentation                 */
   int          angle,        /* angle of field,
                                 if 0 field was descewed          */
   int          skewHist,     /* if calculate skew histograms     */

   PREPROCINFO *lpPreprocInfo, /* structure for frames and lines
                                      removal functions (old staff) */
   TEXTLINE    *lpTextLine,    /* structure for text lines
                                      recognition                   */
   int             HLRecog,        /* horizontal lines recognition
                                      switch
                                      0 - off
                                      1 - on                        */
   int             VLRecog,        /* vertical lines recognition
                                      switch
                                      0 - off
                                      1 - on                        */
   SEGMENT     *lpSegment,     /* structure for segmentation    */

/*   CELLBOXINFO FAR *lpCellBoxInfo,  structure for cell boxes
                                      processing                    */

   SPECK       *lpSpeck,       /* structure for specks removal  */
   STRUCTWB    *lpWorkBuffer,  /* pointer to work area -
                                      at least 10Kbytes             */
// ??????????????????????????????????????????????????????
   int      smSignPer,             /* area percentage to ignore
                                      small signs                   */
// ??????????????????????????????????????????????????????
   RECCHARSTRUCT  *lpRecCharStruct, /* structure for character
                                           recognition              */
   LPBYTE   lpAlphaMapA,
   int      scale
   )

{

int frWidth, i, wdt, wdtH, countL, tHh, ret, rAdd; //,j
BYTE *lpPointer;
int wLineWidth, wImgHeight, wXBegin, wYBegin, wWidth, wHeight;
//int *lWdt;
int frameWMax, cellWdt, /* lRem, */ cBoxDo = 0, maxL;
float alphH;
SREMOVAL *lpData;
SREMOVAL Data;
int cellLineR;
int frWWT;
int lRecalc;
int horLineM;
int typeOfCellB; // ,countCC
int smallSSt;
int anyLines;
int lHAndVertLrem;
int nOffCut600;
int lineTxtOrder;
int lineTxtNumber;

//char buffLog[500];


//printf("1------------ initFieldR; iTypeField= %d \n", iTypeField);

   lpAlphaMap = lpAlphaMapA;
   if((lpTextLine->iVer) == 2)  (lpTextLine->iVer) = 11;
   if((lpSegment->iVer)  == 2)  (lpSegment->iVer)  = 11;


    if(smSignPer < 0)
                smSignPer = 0;

    if(smSignPer > 100)
                smSignPer = 100;
    typeOfCellB = 0; //CELLBOX, 1- SEPCELLBOX
    if(iTypeField == SEPCELLBOX) {
            typeOfCellB = 7;
            iTypeField = CELLBOX;
            }
   if((iTypeField != CELLBOX) && (iSegmType == 1))
            typeOfCellB = 9; // cutting from cells for new cellboxes

    lpData=(SREMOVAL *)&Data;


    lpData->lpPointer   = lpDataIm->lpPointer;
    lpData->iLineWidth  = lpDataIm->iLineWidth;
    lpData->iImgHeight  = lpDataIm->iImgHeight;
    lpData->iXBegin     = lpDataIm->iXBegin;
    lpData->iYBegin     = lpDataIm->iYBegin;
    lpData->iWidth      = lpDataIm->iWidth;
    lpData->iHeight     = lpDataIm->iHeight;
    lpData->iMinLH      = lpPreprocInfo->iMinLH;
    lpData->iMinLV      = lpPreprocInfo->iMinLV;
    lpData->iSpeckSizeH = lpSpeck->iHSize;
    lpData->iSpeckSizeV = lpSpeck->iVSize;

   lHAndVertLrem = 0;
   horLineM = 0;
   anyLines = 0;
   smallSSt = 1;
// very light horizontal line removal
/*
   if(((lpData->iSpeckSizeH) >= 820) || ((lpData->iSpeckSizeV) >= 820)) {   // 08-29-95
                        horLineM = 9;
                        if((lpData->iSpeckSizeH) >= 820)
                                    (lpData->iSpeckSizeH) -= 820;
                        if((lpData->iSpeckSizeV) >= 820)
                                    (lpData->iSpeckSizeV) -= 820;
                        }

   if(((lpData->iSpeckSizeH) >= 800) || ((lpData->iSpeckSizeV) >= 800)) {
                        horLineM = 8;
                        if((lpData->iSpeckSizeH) >= 800)
                                    (lpData->iSpeckSizeH) -= 800;
                        if((lpData->iSpeckSizeV) >= 800)
                                    (lpData->iSpeckSizeV) -= 800;
                        }
*/
/*

   if((iTypeField != CELLBOX) && (lpData->iSpeckSizeH >= 600)) {
      typeOfCellB = 55;
      nOffCut600 = (lpData->iSpeckSizeH) - 600; //!!!! number of cuts
      if((lpData->iSpeckSizeV >= 600))
         (lpData->iSpeckSizeV) -= 600;
      (lpData->iSpeckSizeH) = (lpData->iSpeckSizeV); // !!!!!!
      }

   if((iTypeField == CELLBOX) &&
      (lpData->iSpeckSizeH >= 400)) {
      typeOfCellB = 5;
      (lpData->iSpeckSizeH) -= 400;
      if((lpData->iSpeckSizeV >= 400))
         (lpData->iSpeckSizeV) -= 400;
      }
*/
    lineTxtOrder   = 0;   // from top to bottom
    lineTxtNumber  = -1;  // all lines

/*
   if((iTypeField == MULTILINEBOXD) &&
      (lpData->iSpeckSizeH >= 400)) {
      lineTxtOrder = ((lpData->iSpeckSizeH) - 400)/10;
      (lpData->iSpeckSizeH) = ((lpData->iSpeckSizeH) - 400)%10;
      if(lpData->iSpeckSizeV >= 10) {
         lineTxtNumber = lpData->iSpeckSizeV/10;
         (lpData->iSpeckSizeV) = (lpData->iSpeckSizeV)%10;
         }
      }
*/
//----------------------------------------------
/*
    if((lpData->iSpeckSizeH) >= 300) {
                        typeOfCellB = 7;
                        (lpData->iSpeckSizeH) -= 300;
                        if((lpData->iSpeckSizeV) >= 300)
                              lpData->iSpeckSizeV -= 300;
                        }
*/

    lRecalc = 0;
/*
    if((lpTextLine->iVer) == 14) {
                    lRecalc = 1;
                    (lpTextLine->iVer) = 11;
                    }
    if((lpTextLine->iVer) == 4) {
                    lRecalc = 1;
                    (lpTextLine->iVer) = 1;
                    }
*/
    initRemStruct(lpData);

    frWWT = FRAMEWIDTH;
    if( ((lpTextLine->iVer) != 3) && ((lpTextLine->iVer) != 13) ) {
         frWWT = 300;
        }

    lpPointer  = lpData->lpPointer;
    wLineWidth = lpData->iLineWidth;
    wImgHeight = lpData->iImgHeight;
    wXBegin    = lpData->iXBegin;
    wYBegin    = lpData->iYBegin;
    wWidth     = lpData->iWidth;
    wHeight    = lpData->iHeight;


    cellLineR = 0;
    if(((lpSegment->iCellInfo)==1)&&((lpSegment->iCellWidth)<=0)) {return(CELLBOX_ERR);}

    if((iTypeField==CELLBOX)||(iTypeField==CELLBOXDIVIDED)||(iTypeField==MULTIBOX)) {
       cellLineR = 7;
       if(((lpSegment->iVer)==0) || ((lpSegment->iVer)==10) ||
            ((lpSegment->iVer)==20)) {cBoxDo=0;}
       else {
         cBoxDo=1;
         if(((lpSegment->iCellInfo)==0)&&((lpSegment->iCellWidth)>0)) {cBoxDo=2;}
         }
       if(cBoxDo  >0)  {iSegmType=1;}
       }
     lpData->iRemoval      = iSegmType;


/* check size of rectangle */

    if( (wXBegin+wWidth>=wLineWidth) ) wWidth=wLineWidth-wXBegin-1;
    if( (wImgHeight<MINSIZE) || (wLineWidth<MINSIZE) ) return(SIZE_ERR1);
    if( (wYBegin+wHeight>=wImgHeight) ) wHeight=wImgHeight-wYBegin-1;
    if( (wHeight<MINSIZE) || (wWidth<MINSIZE) ) return(SIZE_ERR1);
    if( (wHeight>MAXSIZE) || (wWidth>MAXSIZE) ) return(SIZE_ERR3);

   if(((lpData->iSpeckSizeH) >= 250) || ((lpData->iSpeckSizeV) >= 250)) {
                        horLineM = 5;
                        if((lpData->iSpeckSizeH) >= 250)
                                    (lpData->iSpeckSizeH) -= 250;
                        if((lpData->iSpeckSizeV) >= 250)
                                    (lpData->iSpeckSizeV) -= 250;
                        }


   if(((lpData->iSpeckSizeH) >= 200) || ((lpData->iSpeckSizeV) >= 200)) {
                        horLineM = 1;
                        if((lpData->iSpeckSizeH) >= 200)
                                    (lpData->iSpeckSizeH) -= 200;
                        if((lpData->iSpeckSizeV) >= 200)
                                    (lpData->iSpeckSizeV) -= 200;
                        }

/* ???????  different order for speck removal ?????? */
     if(((lpData->iSpeckSizeH) > 100) || ((lpData->iSpeckSizeV) > 100)) {
              if(((lpData->iSpeckSizeH)>0)&&((lpData->iSpeckSizeV)>0)) {
                    if((lpData->iSpeckSizeH) > 100)
                                       (lpData->iSpeckSizeH) -= 100;
                    if((lpData->iSpeckSizeV) > 100)
                                       (lpData->iSpeckSizeV) -= 100;
                    cleanSpecks(lpData);
                    }
        }

   if((((lpData->iSpeckSizeH) >= 50) || ((lpData->iSpeckSizeV) >= 50))
      && (typeOfCellB != 9) && (typeOfCellB != 7)) {
                        typeOfCellB = 177; // strong test for mass
                        if((lpData->iSpeckSizeH) >= 50)
                                    (lpData->iSpeckSizeH) -= 50;
                        if((lpData->iSpeckSizeV) >= 50)
                                    (lpData->iSpeckSizeV) -= 50;
                        }

/* ???????  different order for speck removal ?????? */

 /* save data */

    memset(lpWorkBuffer,0x00,(unsigned int)sizeof(STRUCTWB));

    (lpWorkBuffer -> lpRBuffer) =lpPointer;
    (lpWorkBuffer -> widthImg)  =((wLineWidth+31)/32)*4;
    (lpWorkBuffer -> hightImg)  = wImgHeight;
    (lpWorkBuffer -> begRect.x) = wXBegin;
    (lpWorkBuffer -> begRect.y) = wYBegin;
    (lpWorkBuffer -> Height)    = wHeight;
    (lpWorkBuffer -> Width)     = wWidth;
    (lpWorkBuffer -> OffH)      = 0;
    (lpWorkBuffer -> OffW)      = 0;
    (lpWorkBuffer -> Dpi.x)     = lpDataIm->iDpiX;
    (lpWorkBuffer -> Dpi.y)     = lpDataIm->iDpiY;
    (lpWorkBuffer -> newLine)   = 1;
    (lpWorkBuffer -> newSegm)   = 1;
    (lpWorkBuffer -> flgTraceR) = 0;
    (lpWorkBuffer -> blank)     = 1;
    (lpWorkBuffer -> flgCut)    = 0;
    (lpWorkBuffer -> fieldType) = iTypeField;
    (lpWorkBuffer -> segmType)  = iSegmType;
    (lpWorkBuffer->numbOfChar)  = wXBegin;
    (lpWorkBuffer->flgUNP)      = 0;
    (lpWorkBuffer->flgItalic)   = 0;
    (lpWorkBuffer->flgSpSegm)   = 0;
    (lpWorkBuffer->realWidth)   = wLineWidth ;
    (lpWorkBuffer->chType)      = iCharType;
    (lpWorkBuffer->horLineMM)   = 0;
    lpWorkBuffer -> txtLineVer  = lpTextLine->iVer;
    lpWorkBuffer -> txtLineHight = lpTextLine->iLineHeight;

    nOffCut600  = 0;
    if((iTypeField != CELLBOX) && (lpData->iSpeckSizeH >= 600)) {
       typeOfCellB = 55;
       nOffCut600 = (lpData->iSpeckSizeH) - 600; //!!!! number of cuts
       if((lpData->iSpeckSizeV >= 600))
          (lpData->iSpeckSizeV) -= 600;
       (lpData->iSpeckSizeH) = (lpData->iSpeckSizeV); // !!!!!!
       }

//------ 04-22-96
    lpWorkBuffer ->nOffCut600    = nOffCut600;
//------ 04-22-96
// ??????????????????????????
    (lpWorkBuffer->smallSignPer)= smSignPer;
// ??????????????????????????
    (lpWorkBuffer->recChar) = lpRecCharStruct;
    if(lpSegment->iCellWidth==998) { (lpWorkBuffer->flgBarC)=1;}


    (lpWorkBuffer -> heightForVert) = 0;
    if( (lpTextLine->iVer) >= 10 ) {
         (lpTextLine->iVer) -= 10;
         (lpWorkBuffer -> heightForVert) = (lpTextLine->iLineHeight);
         if(!lHAndVertLrem)
              (lpTextLine->iLineHeight) = 0;
         }

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

      lpWorkBuffer->smallSignStrat = smallSSt;

//    kF=1;
//    if((iTypeField==CELLBOX)||(iTypeField==CELLBOXDIVIDED)||(iTypeField==MULTIBOX)) {kF=0;}


//   if((lpSegment->iVer==0)&&(lpSegment->iCellWidth==0)) {


//   if(tmpSpeck)
//        lpWorkBuffer -> segmType = 500 + tmpSpeck;


   if((iTypeField == CELLBOX) &&
      (typeOfCellB == 9)) {
      typeOfCellB = 0;
      (lpWorkBuffer -> segmType)  = 9;
      lpWorkBuffer->nOfSpConstSegm = (lpData->iSpeckSizeH)/10;
      if(lpWorkBuffer->nOfSpConstSegm > 10)
              lpWorkBuffer->nOfSpConstSegm = 0;
      if(lpData->iSpeckSizeH > 10)
              lpData->iSpeckSizeH = (lpData->iSpeckSizeH)%10;
      }


   if((iTypeField == CELLBOX) &&
      (typeOfCellB == 5)) {
      typeOfCellB = 0;
      (lpWorkBuffer->flgItalic) = 1;
      }

   if((iTypeField == CELLBOX) &&
      (typeOfCellB == 7)) {
      typeOfCellB = 0;
      (lpWorkBuffer -> segmType)  = 57;
//      iTypeField = MULTILINEBOXD;
      iTypeField = DROPBOX;
//      (lpWorkBuffer -> fieldType) =  MULTILINEBOXD;
      (lpWorkBuffer -> fieldType) =  DROPBOX;

      lpWorkBuffer->nOfSpConstSegm = (lpData->iSpeckSizeH)/10;
      if(lpWorkBuffer->nOfSpConstSegm > 10)
              lpWorkBuffer->nOfSpConstSegm = 0;
      if(lpData->iSpeckSizeH > 10)
              lpData->iSpeckSizeH = (lpData->iSpeckSizeH)%10;

      }


   if(typeOfCellB == 55) {
       (lpWorkBuffer -> segmType)  = 55;
       typeOfCellB = 0;
       }

   if(typeOfCellB == 177) {
       (lpWorkBuffer -> segmType)  = 177;
       typeOfCellB = 0;
       }

   if(typeOfCellB == 65) {                   // 08-07-95
       lpWorkBuffer -> txtLineVer  = 65;
       typeOfCellB = 0;
       }


   if((lpSegment->iVer) == 30 ) {
                 (lpWorkBuffer->flgSpSegm) = 1;
                 (lpWorkBuffer -> blank)   = 0;
                 (lpSegment->iVer) = 0;
                 };

   if((lpSegment->iVer >= 10) && ((lpSegment->iVer < 20))) {
        (lpWorkBuffer->flgItalic) = 1;
        (lpSegment->iVer) -= 10;
        }

   (lpWorkBuffer->hndOrOCR) = 0;
   if(lpSegment->iVer == 20) {
            (lpWorkBuffer->hndOrOCR) = 1;
            iTypeField = 0;
            (lpWorkBuffer -> fieldType) = 0;
            }


   if(lpSegment->iVer == 0) {
        if(iCharType>3) {
                if((lpSegment->iCellWidth >= 900) && (lpSegment->iCellWidth < 998)) {
//                                    (lpWorkBuffer->flgUNP)=2;
                                    (lpWorkBuffer->  Method)  =0;/* proportional pitch for hand printed */
                                    (lpSegment->iCellWidth) -= 900;
                                    }
                else  {
                           (lpWorkBuffer->  Method)  =1;
                           (lpWorkBuffer->flgUNP)=2;  //proportional pitch for OCR
                           }
                }
        else  {
                (lpWorkBuffer->  Method)  =0;/* proportional pitch for hand printed */
                }
        }
   else {
        (lpWorkBuffer->  Method)  = 1;
        if(lpSegment->iVer == 1) {
           if(lpSegment->iCellWidth==998) { (lpWorkBuffer->flgUNP)=4;  lpSegment->iCellWidth = 0;/* (998) proportional pitch for OCR*/ }
           else {
//                if((lpSegment->iCellWidth >= 900) && (lpSegment->iCellWidth < 998)) {
//                                    (lpWorkBuffer->flgUNP)=2;
//                                    (lpSegment->iCellWidth) -= 900;
//                                    }
//                else {
                  if(lpSegment->iCellInfo==0) {
                     if(lpSegment->iCellWidth==0) {(lpWorkBuffer->flgUNP)=1; /* unknown cell width for unproportional pitch */ }
                     else                        {(lpWorkBuffer->flgUNP)=5; /* estimated cell width for unproportional pitch */ }
                     }
                  else {
                     (lpWorkBuffer->flgUNP)=3; /* known cell width for unproportional pitch */
                     }
//                  }
                }
           }

        else  {
               if(lpSegment->iVer == 2) {  (lpWorkBuffer->flgUNP)=2;  /* (998) proportional pitch for OCR */ }
//                return(CELLBOX_ERR);
                }

        }


    lpWorkBuffer->horLineMM = 0;
    if(horLineM == 5)
           lpWorkBuffer->horLineMM = 1;
// 05-28-96-------------
    if(horLineM == 6)
           lpWorkBuffer->horLineMM = 4;
// ---------------------
    if(horLineM == 8)
           lpWorkBuffer->horLineMM = 2;
    if(horLineM == 9)
           lpWorkBuffer->horLineMM = 3;            //08-29-95
    if(horLineM >= 10)
           lpWorkBuffer->horLineMM = horLineM;     //11-14-95

/* check digit !!!!!! */
/*
     if(((lpData->iSpeckSizeH) == 100) || ((lpData->iSpeckSizeV) == 100)) {
              if(((lpData->iSpeckSizeH)>0)&&((lpData->iSpeckSizeV)>0)) {
                    (lpData->iSpeckSizeH) = 0;
                    (lpData->iSpeckSizeV) = 0;
                    cleanCheckDig(lpWorkBuffer);
                    }
        }
*/
/* check digit !!!!!! */


    (lpWorkBuffer->  lettWidth) = lpSegment->iCellWidth;
    (lpWorkBuffer->  lettWidthG) =lpSegment->iCellWidth;
    (lpWorkBuffer->  CellW)     =lpSegment->iCellWidth;


//#if COMP_DEMO
//    (lpWorkBuffer -> xxxxxxx) = GTESS+(LONG)ADD_G;
//#endif
//     (lpWorkBuffer ->indUnpSp)=0;

if( ((lpTextLine->iVer) == 3) && ((lpTextLine->iLineHeight) > 0)) {
    remBlackA(lpWorkBuffer,wXBegin,wYBegin,wXBegin+wWidth-1,wYBegin+wHeight-1);
//    histHSkewed(lpWorkBuffer,0.6,(float)0.1,(int FAR *)hntHist,(int FAR *)&hntHist[0],(LONG FAR *)&angleH,(int FAR *)&scAngleH);
    }


/* find top and bottom lines */
    (lpWorkBuffer ->newLeftTopP).x    =wXBegin;
    (lpWorkBuffer ->newLeftTopP).y    =wYBegin;
    (lpWorkBuffer ->newLeftBottomP).x =wXBegin;
    (lpWorkBuffer ->newLeftBottomP).y =wYBegin+wHeight-1;
    (lpWorkBuffer ->newRightTopP).x   =wXBegin+wWidth-1;
    (lpWorkBuffer ->newRightTopP).y   =wYBegin;
    (lpWorkBuffer ->newRightBottomP).x=wXBegin+wWidth-1;
    (lpWorkBuffer ->newRightBottomP).y=wYBegin+wHeight-1;


    (lpWorkBuffer->lenLeftC)=0;
    if( ((lpWorkBuffer->flgSpSegm) == 0) && (lpWorkBuffer -> segmType != 55)) {
      if(((iTypeField == MULTILINEBOXD)||
         (iTypeField == DROPBOX))&&((lpWorkBuffer->lettWidth)!=0)) {
         searchCloseP(lpData,(POINT *)&(lpWorkBuffer ->newLeftTopP),(POINT *)&(lpWorkBuffer ->newLeftBottomP),(POINT *)&(lpWorkBuffer ->newRightTopP),(POINT *)&(lpWorkBuffer ->newRightBottomP),0);
         }
      }

      if(anyLines)
         cleanLines(lpData);

    if((iTypeField ==  MULTILINEBOXD)||(iTypeField == DROPBOX)||(iTypeField == CHECKBOXDROP)) {
        if( (lpWorkBuffer->flgSpSegm) == 0) {
          wXBegin=(lpWorkBuffer ->newLeftTopP).x;
          wWidth=max((lpWorkBuffer ->newRightTopP).x-(lpWorkBuffer ->newLeftTopP).x,(lpWorkBuffer ->newRightBottomP).x-(lpWorkBuffer ->newLeftBottomP).x);

          (lpWorkBuffer -> begRect.x)=wXBegin;
          (lpWorkBuffer -> Width)=wWidth;
          }
        (lpWorkBuffer->numbOfChar)=wXBegin;
        ret=0;
        }
    else {
        ret=0;


        switch(iTypeField) {
/*
                case FRAMEBOX:
                case CHECKBOX:
                case MULTILINEBOX:
                case FRAMEBDIVIDED:
                              rAdd=lpData->iRemoval;
                              lpData->iRemoval=1;
                              ret=CheckBoxRemoval(NULL,lpData,(STRUCT4POINTS FAR *)&(lpWorkBuffer -> arrCellP[0]));
                              lpData->iRemoval=rAdd;
                              if((iTypeField==CHECKBOX)&&(ret>=0)) {
                                 (lpWorkBuffer ->newLeftTopP).x    =(lpWorkBuffer -> arrCellP[0]).lT.x;
                                 (lpWorkBuffer ->newLeftTopP).y    =(lpWorkBuffer -> arrCellP[0]).lT.y;
                                 (lpWorkBuffer ->newLeftBottomP).x =(lpWorkBuffer -> arrCellP[0]).lB.x;
                                 (lpWorkBuffer ->newLeftBottomP).y =(lpWorkBuffer -> arrCellP[0]).lB.y;
                                 (lpWorkBuffer ->newRightTopP).x   =(lpWorkBuffer -> arrCellP[0]).rT.x;
                                 (lpWorkBuffer ->newRightTopP).y   =(lpWorkBuffer -> arrCellP[0]).rT.y;
                                 (lpWorkBuffer ->newRightBottomP).x=(lpWorkBuffer -> arrCellP[0]).rB.x;
                                 (lpWorkBuffer ->newRightBottomP).y=(lpWorkBuffer -> arrCellP[0]).lB.y;
                                }
                              break;
                case CELLBOX:

countCC = 0;
                              if(cBoxDo==0) {
                                  rAdd=lpData->iFiltration;
                                  if(!lpData->iRemoval) lpData->iFiltration=0;
                                  (lpWorkBuffer ->lenLeftC)=CellBoxRemoval(NULL,lpData,(STRUCT4POINTS *)&(lpWorkBuffer -> arrCellP[0]));
                                  ret=(lpWorkBuffer ->lenLeftC);
                                  lpData->iFiltration=rAdd;
                                  }
                              else {
                               histVer(lpWorkBuffer,(int *)hntHist,(lpWorkBuffer->begRect.x)+(lpWorkBuffer->OffW),
                                       (lpWorkBuffer->begRect.x)+(lpWorkBuffer->OffW)+(lpWorkBuffer -> Width)-1,
                                       (lpWorkBuffer->begRect.y)+(lpWorkBuffer->OffH),
                                       (lpWorkBuffer->begRect.y)+(lpWorkBuffer->OffH)+(lpWorkBuffer -> Height)-1);
                                if(cBoxDo>0 ) {
                                    cellWdt=lpSegment->iCellWidth;
//                                    if(lpSegment->iCellInfo==0) cellWdt=0;
                                    memset(lowProf,0x00,100);
                                    memset(upProf,0x00,100);
                                    ret=segBoxes(lpWorkBuffer -> Dpi.x,lpWorkBuffer -> Dpi.y,
                                             (int *)&hntHist[1],hntHist[0],(int *)&cellWdt,(int *)lowProf,
                                             (int *)upProf,(int *)traceI,(int *)&rAdd,(LONG *)Cost,0);
                                    if(ret<0) return(ret);
//                                    ret=remVlines(lpWorkBuffer,(int FAR *)lowProf,(int FAR *)upProf,(int FAR *)traceI,(int FAR *)&rAdd);
                                    lpSegment->iCellWidth      =cellWdt;
                                    lpWorkBuffer->lettWidth =cellWdt;
                                    lpWorkBuffer->lettWidthG =cellWdt;
                                    (lpWorkBuffer->  Method)  =1;
                                    }
                                else {
                                    cellWdt=lpSegment->iCellWidth;
//                                    if(lpData->iCellWidth==999) cellWdt=0;
                                    _fmemset(lowProf,0x00,100);
                                    _fmemset(upProf,0x00,100);
                                    ret=segBoxes(lpWorkBuffer -> Dpi.x,lpWorkBuffer -> Dpi.y,
                                             (int *)&hntHist[1],hntHist[0],(int *)&cellWdt,(int *)lowProf,
                                             (int *)upProf,(int *)traceI,(int *)&rAdd,(LONG *)Cost,1);
                                     if(ret<0) return(ret);
//                                    ret=remVlines(lpWorkBuffer,(int FAR *)lowProf,(int FAR *)upProf,(int FAR *)traceI,(int FAR *)&rAdd);
                                    lpSegment->iCellWidth      =cellWdt;
                                    lpWorkBuffer->lettWidth =cellWdt;
                                    lpWorkBuffer->lettWidthG =cellWdt;
                                    (lpWorkBuffer->  Method)  =1;
                                    }

                                    if((lpWorkBuffer -> segmType) == 9){
                                       cutInsideCells(lpWorkBuffer,(int *)lowProf,(int *)upProf,(int *)&rAdd);
//                                       lpWorkBuffer -> segmType = 57;
                                       break;
                                       }
                                    maxL = 3*cellWdt;
                                    if((lpWorkBuffer -> Width)<5*cellWdt) {maxL = 2*cellWdt;}
// 05-28-96
                                    i=(int)((float)1.5*(float)cellWdt);
                                    if(lpWorkBuffer->horLineMM != 4) {
//                                      i=(int)((float)1.5*(float)cellWdt);
                                      if((lpWorkBuffer -> Width)<3*cellWdt) {
                                             i=(int)((float)0.9*(float)cellWdt);
                                             maxL = i;
                                            }
                                      }
                                    alphH=(float)0.0;
                                    if(skewHist) {alphH=(float)0.5;}
                                    histHSkewed(lpWorkBuffer,alphH,(float)0.1,(int *)&hntHist[0],(int *)&hntHist[0],(LONG *)&angleH,(int *)&scAngleH);
                                    linesHCellB(lpWorkBuffer,(int *)&hntHist[1],hntHist[0],i,maxL,angleH,scAngleH);
                                    if((lpWorkBuffer->flgItalic) == 0) {
                                          callRemLineH(lpWorkBuffer,angleH,scAngleH);
                                          ret=remVlines(lpWorkBuffer,(int *)lowProf,(int *)upProf,(int *)traceI,(int *)&rAdd);
                                          }
                                    (lpWorkBuffer->flgUNP)=3;
                                    insideCells(lpWorkBuffer,(int *)lowProf,(int *)upProf,(int *)&rAdd);

                                }
                              break;
                case CELLBOXDIVIDED:
                              rAdd=lpData->iFiltration;
                              if(!lpData->iRemoval) lpData->iFiltration=0;
                              (lpWorkBuffer ->lenLeftC)=MCellBoxRemoval(NULL,lpData,(STRUCT4POINTS *)&(lpWorkBuffer -> arrCellP[0]));
                              ret=(lpWorkBuffer ->lenLeftC);
                              lpData->iFiltration=rAdd;
                              break;

                case FRAMEBOXCPLX:
                              rAdd=lpData->iRemoval;
                              lpData->iRemoval=1;
                              ret=FrameRemoval(NULL,lpData,(STRUCT4POINTS *)&(lpWorkBuffer -> arrCellP[0]));
                              lpData->iRemoval=rAdd;
                              break;
*/
                case MULTIBOX:
                              if(cBoxDo==0) {
                                 rAdd=lpData->iFiltration;
//                                 if(!lpData->iRemoval) lpData->iFiltration=0;
//                                 (lpWorkBuffer ->lenLeftC)=MultiBoxRemoval(NULL,lpData,(STRUCT4POINTS *)&(lpWorkBuffer -> arrCellP[0]),lpTextLine->iLineHeight);
//                                 ret=(lpWorkBuffer ->lenLeftC);
//                                 lpData->iFiltration=rAdd;
//                                 filtrTopBott(lpWorkBuffer);
                                 }
                              else {
                                histVer(lpWorkBuffer,(int *)hntHist,(lpWorkBuffer->begRect.x)+(lpWorkBuffer->OffW),
                                       (lpWorkBuffer->begRect.x)+(lpWorkBuffer->OffW)+(lpWorkBuffer -> Width)-1,
                                       (lpWorkBuffer->begRect.y)+(lpWorkBuffer->OffH),
                                       (lpWorkBuffer->begRect.y)+(lpWorkBuffer->OffH)+(lpWorkBuffer -> Height)-1);
                                if(cBoxDo>0 ) {
                                    cellWdt=lpSegment->iCellWidth;
                                    memset(lowProf,0x00,100);
                                    memset(upProf,0x00,100);
                                    ret=segBoxes(lpWorkBuffer -> Dpi.x,lpWorkBuffer -> Dpi.y,
                                             (int *)&hntHist[1],hntHist[0],(int *)&cellWdt,(int *)lowProf,
                                             (int *)upProf,(int *)traceI,(int *)&rAdd,(LONG *)Cost,0);
                                    if(ret<0) return(ret);
//                                    ret=remVlines(lpWorkBuffer,(int FAR *)lowProf,(int FAR *)upProf,(int FAR *)traceI,(int FAR *)&rAdd);
                                    lpSegment->iCellWidth      =cellWdt;
                                    lpWorkBuffer->lettWidth =cellWdt;
                                    lpWorkBuffer->lettWidthG =cellWdt;
                                    (lpWorkBuffer->  Method)  =1;
                                    }
                                else {
                                    cellWdt=lpSegment->iCellWidth;
                                    memset(lowProf,0x00,100);
                                    memset(upProf,0x00,100);
                                    ret=segBoxes(lpWorkBuffer -> Dpi.x,lpWorkBuffer -> Dpi.y,
                                             (int *)&hntHist[1],hntHist[0],(int *)&cellWdt,(int *)lowProf,
                                             (int *)upProf,(int *)traceI,(int *)&rAdd,(LONG *)Cost,1);
                                    if(ret<0) return(ret);
//                                    ret=remVlines(lpWorkBuffer,(int FAR *)lowProf,(int FAR *)upProf,(int FAR *)traceI,(int FAR *)&rAdd);
                                    lpSegment->iCellWidth      =cellWdt;
                                    lpWorkBuffer->lettWidth =cellWdt;
                                    lpWorkBuffer->lettWidthG =cellWdt;
                                    (lpWorkBuffer->  Method)  =1;
                                    }
                                    maxL = 3*cellWdt;
                                    if((lpWorkBuffer -> Width)<5*cellWdt) {maxL = 2*cellWdt;}
                                    i=(int)((float)1.5*(float)cellWdt);
                                    if((lpWorkBuffer -> Width)<3*cellWdt) {
                                        i=(int)((float)0.9*(float)cellWdt);
                                        maxL=i;
                                        }
                                    alphH=(float)0.0;
                                    if(skewHist) {alphH=(float)0.5;}
                                    histHSkewed(lpWorkBuffer,alphH,(float)0.1,(int *)&hntHist[0],(int *)&hntHist[0],(LONG *)&angleH,(int *)&scAngleH);
                                    linesHCellB(lpWorkBuffer,(int *)&hntHist[1],hntHist[0],i,maxL,angleH,scAngleH);
//                                    callRemLineH(lpWorkBuffer,angleH,scAngleH);
//                                    ret=remVlines(lpWorkBuffer,(int *)lowProf,(int *)upProf,(int *)traceI,(int  *)&rAdd);
                                    (lpWorkBuffer->flgUNP)=3;

                                 }
                              break;
//                case LINES:
//                              ret=cleanLines(NULL,lpData);

//                              break;
                }

         }

      if(ret<-100) return(LINE_ERR2);
/*

      if((lpWorkBuffer ->lenLeftC)<0) (lpWorkBuffer ->lenLeftC)=0;
      if((iTypeField==CHECKBOX)||(iTypeField == CHECKBOXDROP)) {
        return(checkBox(lpWorkBuffer,NULL));
          }
*/

/* calculate number of frames */

    if( (iSegmType==0)&&(((iTypeField==CELLBOX)&&((lpWorkBuffer->Method)!=1))||(iTypeField==CELLBOXDIVIDED)||(iTypeField==MULTIBOX)||(iTypeField==LINES)) ) {
        (lpWorkBuffer -> currCellIndx)=0;
        return(1);
        }

//printf("  after switch \n");

    if((iTypeField==MULTILINEBOX)||(iTypeField==MULTIBOX)) {

       tHh=(int)((float)20*((float)(lpWorkBuffer -> Dpi.x)/(float)300.0));
//printf("1------------  searchCloseP, tHh= %d \n", tHh);

       searchCloseP(lpData,(POINT *)&(lpWorkBuffer ->newLeftTopP),(POINT *)&(lpWorkBuffer ->newLeftBottomP),(POINT *)&(lpWorkBuffer ->newRightTopP),(POINT *)&(lpWorkBuffer ->newRightBottomP),0);
          wXBegin=max((lpWorkBuffer -> begRect.x),(lpWorkBuffer ->newLeftTopP).x-tHh);
          wWidth=max((lpWorkBuffer ->newRightTopP).x-(lpWorkBuffer ->newLeftTopP).x,(lpWorkBuffer ->newRightBottomP).x-(lpWorkBuffer ->newLeftBottomP).x)+2*tHh;
          wWidth=min((int)wWidth,(lpWorkBuffer -> begRect.x)+(lpWorkBuffer -> Width)-wXBegin-1);
          (lpWorkBuffer -> begRect.x)=wXBegin;
          (lpWorkBuffer -> Width)=wWidth;
          (lpWorkBuffer->numbOfChar)=wXBegin;
          }
    frameWMax=frWWT;
//    if((angle==0)&&(iCharType>3)&&((lpWorkBuffer -> Height)<MAXPROFILE)&&kF) {frameWMax=5000;}
    if((lpTextLine->iVer)==1) {frameWMax=5000;}
    else {
        if((lpTextLine->iVer)==0) {
            (lpTextLine->iLineHeight)=0;
            }
        }

//    wdt=wWidth-wOffsetX;
    wdt=wWidth;
    frWidth=(int)((float)frameWMax*((float)(lpWorkBuffer -> Dpi.x)/(float)200.0));
    i=1;
    while( (i<MAXFRAMES) && ((wdt/i)>frWidth) ) i++;
    if(i>1) i--;
    (lpWorkBuffer -> frNumber) =i;
    wdtH=wdt/i;
    rAdd=wdt%i;
    if(rAdd<2) rAdd=0;


/* lineremoval for hand printed textline recognition */

/*
     if((frameWMax==frWWT)&&((lpTextLine->iVer) == 0)) {
//          if(VLRecog || HLRecog) {
//             hist_h(lpWorkBuffer,wXBegin,wYBegin,wXBegin+wWidth,wYBegin+wHeight);
//             linesUnp((int FAR *)hntHist,hntHist[0],lpWorkBuffer,0);
//             }


          if(VLRecog) {
             hist_h(lpWorkBuffer,wXBegin,wYBegin,wXBegin+wWidth,wYBegin+wHeight);
             linesUnp((int *)hntHist,hntHist[0],lpWorkBuffer,0);
             histVer(lpWorkBuffer,(int *)hntHist,wXBegin,wXBegin+wWidth,wYBegin,wYBegin+wHeight);
             filtrHistVert(NULL,lpWorkBuffer,(int FAR *)&hntHist[1],hntHist[0],wXBegin,0,NULL,0,0);
             }

         if(HLRecog) {
             hist_h(lpWorkBuffer,wXBegin,wYBegin,wXBegin+wWidth,wYBegin+wHeight);
             linesUnp((int *)hntHist,hntHist[0],lpWorkBuffer,0);
             filtrHist(lpWorkBuffer,(int *)hntHist,hntHist[0],(int *)hntHist,3  ,
                       wXBegin,wXBegin+wWidth ,0,1);

             for(j=0;j<(lpWorkBuffer->infFrame[0]).nLines;j++) {
                tmpL[j].indTop=(lpWorkBuffer->line[0]).y1[j];
                tmpL[j].indBot=(lpWorkBuffer->line[0]).y2[j];
                }
             for(i=1;i<(lpWorkBuffer -> frNumber);i++)  {
                for(j=0;j<(lpWorkBuffer->infFrame[i]).nLines;j++) {
                   if(tmpL[j].indTop>(lpWorkBuffer->line[i]).y1[j]) {tmpL[j].indTop=(lpWorkBuffer->line[i]).y1[j];}
                   if(tmpL[j].indBot<(lpWorkBuffer->line[i]).y2[j]) {tmpL[j].indBot=(lpWorkBuffer->line[i]).y2[j];}
                   }
                }
             j=0;
             for(i=0;i<(lpWorkBuffer -> frNumber);i++)  {
                if(j<(lpWorkBuffer->infFrame[i]).nLines) {j=(lpWorkBuffer->infFrame[i]).nLines;}
                }

             filtrHistInsL(NULL,lpWorkBuffer,(int *)hntHist,hntHist[0],wXBegin,wXBegin+wWidth,
                           (TMPLINES  *)tmpL,j,0,1,iCharType);
             }
        }
     */

/* calculate lines for each frames */
// printf("2------------ calculate lines for each frames, lpWorkBuffer -> frNumber= %d \n", lpWorkBuffer -> frNumber);
    countL=0;
    for(i=0;i<(lpWorkBuffer -> frNumber);i++)  {
        (lpWorkBuffer->infFrame)[i].x1=wXBegin+i*wdtH;
        (lpWorkBuffer->infFrame)[i].x2=wXBegin+(i+1)*wdtH;

        if(i==(lpWorkBuffer -> frNumber)-1) (lpWorkBuffer->infFrame)[i].x2+=rAdd;

// 	printf("3------------ calculate lines for each frames, frameWMax= %d frWWT= %d \n", frameWMax, frWWT);

        if(frameWMax==frWWT) {
           hist_h(lpWorkBuffer,wXBegin+i*wdtH,wYBegin,wXBegin+(i+1)*wdtH,wYBegin+wHeight);
           }

        if(frameWMax!=frWWT) {
        if((lpWorkBuffer->infFrame)[(lpWorkBuffer -> frNumber)-1].x2>=(int)(wXBegin+wWidth)) {
                (lpWorkBuffer->infFrame)[(lpWorkBuffer -> frNumber)-1].x2=wXBegin+wWidth-1;
                }


              if(HLRecog || VLRecog) {
//                 alphH=(float)0.2;
                 alphH=(float)0.4;
                 }
              else {
                 alphH=(float)0.0;
                 }

              if(skewHist) {alphH=(float)0.4;}
            	  lpData->iSpeckSizeH = 4;
            	  lpData->iSpeckSizeV = 4;

                cleanSpecks(lpData);

          	  lpData->iSpeckSizeH = 0;
          	  lpData->iSpeckSizeV = 0;

// if(strcmp(fieldName, "NUMBERS") == 0){
//printf("4------------ alphH= %e skewHist= %d \n", alphH, skewHist);
//printf("4------------ lpData->iSpeckSizeH= %d lpData->iSpeckSizeV= %d \n", lpData->iSpeckSizeH, lpData->iSpeckSizeV);
 //}

//              lpWorkBuffer->begRect.x += 500;
              histHSkewed(lpWorkBuffer,alphH,(float)0.1,(int *)hntHist,(int *)&hntHist[0],(LONG *)&angleH,(int *)&scAngleH);


            lpWorkBuffer->horLineMM = 0;
            if(horLineM == 5)
                    lpWorkBuffer->horLineMM = 1;
            if(horLineM == 8)
                    lpWorkBuffer->horLineMM = 2;
            if(horLineM == 9)
                    lpWorkBuffer->horLineMM = 3;
            if(horLineM >= 10)
                    lpWorkBuffer->horLineMM = horLineM;

//printf("4------------ linesForFonts, HLRecog= %d horLineM= %d \n", HLRecog, horLineM);

//          sprintf(buffLog,"4------------ linesForFonts, HLRecog= %d horLineM= %d \n", HLRecog, horLineM);
//          saveLog("", buffLog);

            linesForFonts((int *)hntHist, hntHist[0], lpWorkBuffer, lpTextLine->iLineHeight, angleH, scAngleH, (int)iCharType, HLRecog, VLRecog, horLineM);

//            sprintf(buffLog,"5------------------------------------------- linesForFonts, sizeWImg= %d, \n", sizeWImg);
//            saveLog("", buffLog);

//            lpWorkBuffer->begRect.x -= 500;
// printf("5------------ linesForFonts, lpWorkBuffer->infFrame[0]).nLines= %d \n", (lpWorkBuffer->infFrame[0]).nLines);
// printf("5------------ fieldName %s \n", fieldName);

           if(strcmp(fieldName, "NUMBERS") == 0)
               secondTestForLine(lpWorkBuffer, 0, sizeWImg);


//           sprintf(buffLog,"5------------ linesForFonts, lpWorkBuffer->infFrame[0]).nLines= %d \n", (lpWorkBuffer->infFrame[0]).nLines);
//           saveLog("", buffLog);

// printf("5------------ linesForFonts, lpWorkBuffer->infFrame[0]).nLines= %d \n", (lpWorkBuffer->infFrame[0]).nLines);

// printf("5------------ linesForFonts, lpWorkBuffer->infFrame[0]).nLines= %d \n", (lpWorkBuffer->infFrame[0]).nLines);

  if(lRecalc) {
      lineRec((int *)hntHist,hntHist[0],lpWorkBuffer,i);
      }

            }
        else {

if(((lpTextLine->iVer) == 3)&&(lpTextLine->iLineHeight > 0)) {

//printf("6------------ linesTxtSp, lpTextLine->iLineHeight= %d \n", lpTextLine->iLineHeight);
linesTxtSp((int *)hntHist,hntHist[0],lpWorkBuffer,lpTextLine->iLineHeight,i);
}
else {

// ?????????????????????????????????????????????? */

          if((iTypeField ==  MULTILINEBOXD)||(iTypeField==MULTILINEBOX)||(iTypeField==MULTIBOX)) {
//  printf("7------------ linesFrameM, lpTextLine->iLineHeight= %d \n", lpTextLine->iLineHeight);
             linesFrameM(i,lpWorkBuffer,lpTextLine->iLineHeight,(lpWorkBuffer->  Method));
             }
          else {
// printf("7------------ linesFrame, cellLineR= %d \n", cellLineR);
             linesFrame(i,lpWorkBuffer,cellLineR);
             }


}

          }

        if((lpWorkBuffer->infFrame[i]).nLines>0) {
//   	printf("8------------ maxLineH, i= %d \n", i);
                     maxLineH(lpWorkBuffer,i,(int *)hntHist,hntHist[0]);
                     countL++;
                     }
        }
     if(countL == 0) {
//printf("9------------ maxLineH, lpWorkBuffer -> segmType= %d \n", lpWorkBuffer -> segmType);

        if(lpWorkBuffer -> segmType  == 55) {
           int y1, y2, n;
           n = ((lpWorkBuffer -> Height) - lpTextLine->iLineHeight)/2;
           if(n<0) n = 1;
           y1 = (lpWorkBuffer->begRect.y) + n;
           y2 = (lpWorkBuffer->begRect.y) + (lpWorkBuffer -> Height) - 1 - n;
           if(y2 <= y1) return(LINE_ERR1);
           for(i=0;i<(lpWorkBuffer -> frNumber);i++)  {
               (lpWorkBuffer->line[i]).y1[0] = y1;
               (lpWorkBuffer->line[i]).y2[0] = y2;
               lpWorkBuffer->infFrame[i].nLines = 1;
               (lpWorkBuffer->infFrame[i]).x1 = (lpWorkBuffer->begRect.x);
               (lpWorkBuffer->infFrame[i]).x2 = (lpWorkBuffer->begRect.x) + lpWorkBuffer -> Width - 1;
               }
           }
        else
           return(LINE_ERR1);
        }


if(((lpTextLine->iVer) == 3)&&(lpTextLine->iLineHeight > 0)) {

//printf("10------------ linesOrder, lpTextLine->iLineHeight= %d \n", lpTextLine->iLineHeight);

   linesOrder(lpWorkBuffer);
   if(VLRecog) {
     histVer(lpWorkBuffer,(int *)hntHist,wXBegin,wXBegin+wWidth,wYBegin,wYBegin+wHeight);
     filtrHistVert(lpWorkBuffer,(int *)&hntHist[1],hntHist[0],-wXBegin-1,0,NULL,0,0);
     }
    if(HLRecog) {
     hist_h(lpWorkBuffer,wXBegin,wYBegin,wXBegin+wWidth,wYBegin+wHeight);
     filtrHist(lpWorkBuffer,(int *)hntHist,hntHist[0],(int *)hntHist,3 /* nn */ ,
               wXBegin,wXBegin+wWidth ,0,1);
     }
   }

//printf("11------------ lineTxtOrder= %d, lineTxtNumber= %d \n", lineTxtOrder, lineTxtNumber);

  if(lineTxtOrder) changeLineOrder(lpWorkBuffer, lineTxtNumber);
  else
    if(lineTxtNumber >= 0) cutNofLines(lpWorkBuffer, lineTxtNumber);


     i=1;
     if((iTypeField !=  MULTILINEBOXD)&&(iTypeField!=MULTILINEBOX)&&(iTypeField!=MULTIBOX)) {
// printf("12------------ isMainLine \n");
        i=isMainLine(lpWorkBuffer);
        }
     else {
         lpWorkBuffer->maxLH = (int)(((float)(lpWorkBuffer->maxLH)*(float)30)/(float)100);
        }

//   printf("13------------ lpWorkBuffer->hndOrOCR= %d \n", lpWorkBuffer->hndOrOCR);

       if((lpWorkBuffer->hndOrOCR) == 1 ) {
    	   return 0;
//           return(ourNextForm(NULL,lpPointer,lpWorkBuffer,(int *)&lWdt,(LPBYTE)NULL,0,0,0,1));
        }
      if(((lpData->iSpeckSizeH)>0)&&((lpData->iSpeckSizeV)>0)) {
//  printf("14------------ cleanSpecks \n");
        cleanSpecks(lpData);
        }
     return(i);

}  /* of initFieldR() */

int  nextFieldR(
   char* fieldName,
   int sizeW,
   BYTE *lpBuffer,         /* pointer to image */

   STRUCTWB * lpWorkBuffer, /* pointer to work area - at least 6.7Kbytes         */
   int      *lpLwidth,      /* pointer to output cellWidth */
   LPBYTE lpExtractCh,          /* pointer where extracted character is sent     */
                                /* at least
                                   ((wWidth+31)/32)*4* wHeight
                                   bytes ( use 1.2KByte)                         */
   WORD   wWidth,               /* max width of extracted charcter (in pixels)     */
                                /* use 96 */
   WORD   wHeight,              /* max height of extracted charcter  (in pixels    */
                                /* use 96 */
   WORD   wCharType,            /* type of expected characters:
                                    0 - handprinted digits,
                                    1 - handprinted small letters,
                                    2 - handprinted capital letters,
                                    3 - handprinted mixed letters
                                        ( bContextOn always TRUE) ,
                                    4 - OCR digits,
                                    5 - OCR small letters,
                                    6 - OCR capital letters,
                                    7 - ORC mixed lettrs
                                        (bContextOn always TRUE)
                                    8 - Custom Set 1,
                                    9 - Custom Set 2,
                                   10 - Custom set 3,
                               */
int  scal
   )
{
  int nFr, i, crR, higR, rm, x, y, tHm;
  int y1, y2, x1, x2, yT, yB, difY, difX;
  int ret, tHMass;
//  int Ok;

      (lpWorkBuffer-> lpRBuffer)=lpBuffer;
      tHMass = (int)((float)50*((float)(lpWorkBuffer -> Dpi.x)/(float)200.0));  // ????????????????????????????

      nFr=0;
     if(wCharType>7) {
         wCharType=6;
        }
  if((lpWorkBuffer->fieldType == CELLBOX) && (lpWorkBuffer->flgItalic == 1))
                             goto aBC;
/*----------------------------------------------------------------*/
/* one step segmentation for un-proportional fonts */

     if( ((lpWorkBuffer->  lettWidth)>0)&&((lpWorkBuffer->  Method)==1) ) {
        if( ((lpWorkBuffer->newLine) == 1) && ((lpWorkBuffer -> segmType) != 9) ) {
//printf("------------1  NLINE1:\n");
             (lpWorkBuffer -> trush) = 0;
             if(((lpWorkBuffer -> fieldType)==MULTILINEBOXD)||((lpWorkBuffer -> fieldType)==MULTILINEBOX)||((lpWorkBuffer -> fieldType)==MULTIBOX)) {

//      	 printf("------------2  NLINE1:\n");
NLINE1:          if( (nFr=maxYstruct(lpWorkBuffer))>=0 )  {

//printf("------------ searchBeg, nFr= %d \n", nFr);

                   searchBeg(lpWorkBuffer,nFr);
                   searchEnd(lpWorkBuffer,nFr);
                   }
               else {
                  return(0);
                  }
             if(!isLineValid(lpWorkBuffer,wCharType)) {
                     goto NLINE1;
                     }

               }

             goto NXT;
             }
//  printf("------------ after NLINE1 \n");

        if((lpWorkBuffer -> segmType == 55) && (lpWorkBuffer->nOfSpConstSegm > 0)) {
             lpWorkBuffer->nOfSpConstSegm--;
             return(45);
             }
// if(strcmp(fieldName, "NUMBERS") == 0) {
//    printf("------------ sendNextFont \n");
// }
       ret=sendNextFont(lpWorkBuffer, fieldName);

//if(strcmp(fieldName, "NUMBERS") == 0) {
//      printf("------------ ret= %d, lpWorkBuffer -> segmType= %d \n", ret, lpWorkBuffer -> segmType);
// }
        if(((lpWorkBuffer -> segmType == 57)
            || (lpWorkBuffer -> segmType == 9)) && (ret <=1000) &&
           (lpWorkBuffer ->traceC[1] - lpWorkBuffer ->traceL[1] <
           (lpWorkBuffer->  lettWidth) - (lpWorkBuffer->  lettWidth)/3))
                            ret=sendNextFont(lpWorkBuffer, "NO");
        if(((lpWorkBuffer -> segmType == 57)
            || (lpWorkBuffer -> segmType == 9)) && (lpWorkBuffer->lengthTraceL <(lpWorkBuffer->  lettWidth)/2))
                                   ret=sendNextFont(lpWorkBuffer, "NO");
        if(((lpWorkBuffer -> segmType == 57)
           || (lpWorkBuffer -> segmType == 9)) && (ret > 1000)) // separate cellboxes
                                   ret=sendNextFont(lpWorkBuffer, "NO");
        if(ret==3) {
                if(((lpWorkBuffer -> fieldType)!=MULTILINEBOXD)&&((lpWorkBuffer -> fieldType)!=MULTILINEBOX)&&((lpWorkBuffer -> fieldType)!=MULTIBOX)) {
                        return(0);
                        }
                (lpWorkBuffer->newLine)=1;
                (lpWorkBuffer->  lettWidth) = (lpWorkBuffer->  lettWidthG);
                }
        nFr=ret;
        if((lpWorkBuffer -> segmType == 57) || (lpWorkBuffer -> segmType == 9)) {
                    sepBoxes(lpWorkBuffer);
                    if(massBox(lpWorkBuffer)) {
                              setEmptyBuff(lpExtractCh,wWidth,wHeight);
                              return(1001);
                              }
//                    if((wCharType == 2) || (wCharType == 0)) {
//---------------------------------------------------------------------
//Ok = checkITest(lpWorkBuffer->lpRBuffer,(lpWorkBuffer->widthImg)*8,
//           lpWorkBuffer->hightImg,lpWorkBuffer->traceL[1] + 1,
//           lpWorkBuffer->traceL[0] + 1,lpWorkBuffer->traceC[1] - lpWorkBuffer->traceL[1] - 2,
//           lpWorkBuffer->lengthTraceL - 2);
//---------------------------------------------------------------------
//                      if(Ok)
//                        return(4);
//                      }
                    }

        if(lpWorkBuffer -> segmType == 177)
                    if(strongMassTest(lpWorkBuffer, tHMass )) {
                       setEmptyBuff(lpExtractCh,wWidth,wHeight);
                       return(1001);
                       }

        if(ret==1)  goto SNT;

//?????????????        ONLY ONE SPACE

//        if(strcmp(fieldName, "NUMBERS") == 0) {
//       printf("-ONLY ONE SPACE- ret= %d, lpWorkBuffer -> segmType= %d \n", ret, lpWorkBuffer -> segmType);
//        }

        if(ret>1000) {
                 setEmptyBuff(lpExtractCh,wWidth,wHeight);
                 if((lpWorkBuffer -> segmType) == 55) {
                    lpWorkBuffer->nOfSpConstSegm = ret - 1000;
                    ret = 45;
                    lpWorkBuffer->nOfSpConstSegm--;
                    }
                 else
                    ret = 1001;
                 }
        return(ret);

        }

/*----------------------------------------------------------------*/

aBC:
     if( (((lpWorkBuffer->segmType)==0)&&(((lpWorkBuffer->fieldType)==CELLBOX)||((lpWorkBuffer->fieldType)==CELLBOXDIVIDED)||((lpWorkBuffer->fieldType)==MULTIBOX))) ||
         ((lpWorkBuffer->fieldType == CELLBOX) && (lpWorkBuffer->flgItalic == 1)) ) {
         if((lpWorkBuffer ->currCellIndx)>=(lpWorkBuffer ->lenLeftC)) return(0);


         yT=max(lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].lT.y,lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].rT.y);
         yB=min(lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].lB.y,lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].rB.y);

         y1=lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].lT.y;
         y2=lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].lB.y;

     if((lpWorkBuffer->fieldType)==CELLBOXDIVIDED) {
         if((yT<0)||(yB<0)||(y1<0)||(y2<0)) {
                (lpWorkBuffer ->currCellIndx)++;
                return(7);
                }
         }
     if((lpWorkBuffer->fieldType)==MULTIBOX) {
         if(lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].lT.x<0) {
                (lpWorkBuffer ->currCellIndx)++;
                return(3);
                }
         }

         x1=lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].lT.x;
         x2=lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].lB.x;
         difY=y2-y1;
         difX=x2-x1;
         (lpWorkBuffer ->traceL[0])=yT;
         (lpWorkBuffer ->lengthTraceL)=yB-yT+1;
         x=x1;
         i=1;
         for(y=yT;y<=yB;y++) {
                if((difY!=0)&&(difX!=0)) x=(int)((float)(difX)*(float)(y-y1)/(float)difY)+x1;
                (lpWorkBuffer ->traceL[i])=x;
                i++;
                }

         y1=lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].rT.y;
         y2=lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].rB.y;
         x1=lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].rT.x;
         x2=lpWorkBuffer->arrCellP[(lpWorkBuffer ->currCellIndx)].rB.x;
         difY=y2-y1;
         difX=x2-x1;
         (lpWorkBuffer ->traceC[0])=yT;
         (lpWorkBuffer ->lengthTraceC)=yB-yT+1;
         x=x1;
         i=1;
         for(y=yT;y<=yB;y++) {
                if((difY!=0)&&(difX!=0)) x=(int)((float)(difX)*(float)(y-y1)/(float)difY)+x1;
                (lpWorkBuffer ->traceC[i])=x;
                i++;
                }
         (lpWorkBuffer ->currCellIndx)++;

         if(wCharType<4) {
                tHm=45;
                }
         else    {
                tHm=20;
                }

        tHm=(int)((float)tHm*((float)(lpWorkBuffer -> Dpi.x)/(float)300.0));

        rm=0;
        i=1;
  if((lpWorkBuffer->fieldType == CELLBOX) && (lpWorkBuffer->flgItalic == 1)) {
        tHm = 60;
        while(i<=(lpWorkBuffer ->lengthTraceL)) {
            y=(lpWorkBuffer->traceL[0])+i-1;
            for(x=(lpWorkBuffer->traceL[i]);x<=(lpWorkBuffer->traceC[i]);x++)  {
                if(whatIsThis(lpWorkBuffer,x,y)) {
                   rm++;
                   }
                }
            i++;
            }
        if(rm<tHm) {setEmptyBuff(lpExtractCh,wWidth,wHeight);return(45);} //???????????
        i= sendToBuffer(lpWorkBuffer,lpExtractCh,wWidth,wHeight,wCharType);
        if(i<0) return(i);
        return(1);
         }
  else {
        while((i<=(lpWorkBuffer ->lengthTraceL))&&(rm<tHm)) {
            y=(lpWorkBuffer->traceL[0])+i-1;
            for(x=(lpWorkBuffer->traceL[i]);x<=(lpWorkBuffer->traceC[i]);x++)  {
                if(whatIsThis(lpWorkBuffer,x,y)) {
                   rm++;
                   }
                }
            i++;
            }
        }

        if(rm<tHm) {setEmptyBuff(lpExtractCh,wWidth,wHeight);return(1001);}

        i= sendToBuffer(lpWorkBuffer,lpExtractCh,wWidth,wHeight,wCharType);
        if(i<0) return(i);
        return(1);
        } /* of CELLBOX */



     if(((lpWorkBuffer -> fieldType) !=  MULTILINEBOXD)&&((lpWorkBuffer -> fieldType)!=MULTILINEBOX)&&((lpWorkBuffer -> fieldType)!=MULTIBOX)) {
        if( (lpWorkBuffer->newLine)==5 ) {
                return(0);
                }
        }
     if( (lpWorkBuffer->newLine)==1 ) {
          (lpWorkBuffer -> trush) = 0;
          if(((lpWorkBuffer -> fieldType)==MULTILINEBOXD)||((lpWorkBuffer -> fieldType)==MULTILINEBOX)||((lpWorkBuffer -> fieldType)==MULTIBOX)) {

NLINE:       if( (nFr=maxYstruct(lpWorkBuffer))>=0 )  {

                searchBeg(lpWorkBuffer,nFr);
                searchEnd(lpWorkBuffer,nFr);
                }
             else {
                return(0);
                }
             if(!isLineValid(lpWorkBuffer,wCharType)) {
                     goto NLINE;
                     }
             }
          }

        if((((lpWorkBuffer -> fieldType)==MULTILINEBOXD)||((lpWorkBuffer -> fieldType)==MULTILINEBOX)||((lpWorkBuffer -> fieldType)==MULTIBOX))&&((lpWorkBuffer->newLine)==1)) goto NXT;
        if((lpWorkBuffer->blank)==0) {
             for(i=0;i<=(lpWorkBuffer->lengthTraceC);i++) {
                  (lpWorkBuffer->traceL[i])=(lpWorkBuffer->traceC[i]);
                  }
             (lpWorkBuffer->lengthTraceL)=(lpWorkBuffer->lengthTraceC);

             if((lpWorkBuffer->flgCut)) {
                  if((lpWorkBuffer->flgTraceR)) {
                     crR=(lpWorkBuffer->curMark2);
                     higR=min( ((lpWorkBuffer->mark[crR]).d_m-(lpWorkBuffer->mark[crR]).g_m)+2,MAXLTRACE-1);
                     (lpWorkBuffer->lengthTraceR)=higR;
                     (lpWorkBuffer->traceR[0])=(lpWorkBuffer->mark[crR]).g_m;
                     rm=(lpWorkBuffer->mark[crR]).r_m;
                     if(rm<0) rm=-rm;
                     for(i=1;i<=higR;i++)  {
                             (lpWorkBuffer->traceR[i])=rm;
                             }
                     (lpWorkBuffer->lengthTraceR)=higR;
                     (lpWorkBuffer->flgTraceR)=0;
                     }
                   }

             }

NXT:
   (lpWorkBuffer->flgCut)=0;
   (lpWorkBuffer->linkRM[0])=2;
   (lpWorkBuffer->boundLeft)=0;
   lpWorkBuffer->flgUNP = 1;

//   if(strcmp(fieldName, "NUMBERS") == 0) {
//	   lpWorkBuffer->flgUNP = 2;
//	   printf("------------------------------------------------- new line \n\n");
//   }

//   printf("------------beforeSegmChars \n");
   if(strcmp(fieldName, "NUMBERS") == 0) {
       while( (nFr=SegmChars2(lpWorkBuffer,scal,wCharType,lpLwidth, fieldName, sizeW))==-100 );
   }else{
	   while( (nFr=SegmChars(lpWorkBuffer,scal,wCharType,lpLwidth, fieldName, sizeW))==-100 );
   }


SNT:;
   if((lpWorkBuffer->hndOrOCR) == 0 ) {
//  printf("------------SNT:; nFr= %d\n", nFr);
      if(nFr==-200) goto aBC;

       if(nFr>0 && nFr<100 && nFr!=3){

             if(lpWorkBuffer->traceL[0] + (lpWorkBuffer->lengthTraceL)  >=
               lpWorkBuffer -> begRect.y + lpWorkBuffer -> Height - 1)
                                    lpWorkBuffer->lengthTraceL = lpWorkBuffer -> begRect.y + lpWorkBuffer -> Height - lpWorkBuffer->traceL[0] -1;
             if(lpWorkBuffer->traceC[0] + (lpWorkBuffer->lengthTraceC)  >=
               lpWorkBuffer -> begRect.y + lpWorkBuffer -> Height - 1)
                                    lpWorkBuffer->lengthTraceC = lpWorkBuffer -> begRect.y + lpWorkBuffer -> Height - lpWorkBuffer->traceC[0] -1;
             if(lpWorkBuffer->traceL[0]  < lpWorkBuffer -> begRect.y)
                                    lpWorkBuffer->traceL[0] = lpWorkBuffer -> begRect.y;
             if(lpWorkBuffer->traceC[0]  < lpWorkBuffer -> begRect.y)
                                    lpWorkBuffer->traceC[0] = lpWorkBuffer -> begRect.y;


            i= sendToBuffer(lpWorkBuffer,lpExtractCh,wWidth,wHeight,wCharType);
//printf("------------sendToBuffer; i= %d\n", i);
            if(i!=0) nFr=i;
             }
       }


//?????????????        ONLY ONE SPACE
   if(nFr > 1000) {
              setEmptyBuff(lpExtractCh,wWidth,wHeight);
              nFr = 1001;
              }
   if((lpWorkBuffer -> segmType == 55) && (nFr == 45)) setEmptyBuff(lpExtractCh,wWidth,wHeight);

   return(nFr);

} /* of nextFieldR() */
//(lpWorkBuffer->line[indFr]).y1[j], (lpWorkBuffer->line[indFr]).y2[j]

int getLeftSideStartIdx(int *vHist, int h_size, int th){
	 int mass;
	 int i;
	   mass = 0;
	   for(i=3;i<h_size;i++){
		   mass = (vHist[i-2]+vHist[i-1]+vHist[i])/3;
		   if(mass > th){
			   return i;
			   }
		   }
	   return h_size;
}
int getRightSideStartIdx(int *vHist, int h_size, int th){
	 int mass;
	 int i;
	   mass = 0;
	   for(i=h_size-4;i>0;i--){
		   mass = (vHist[i+2]+vHist[i+1]+vHist[i])/3;
		   if(mass > th){
			   return i;
			   }
		   }
	   return 0;
}
void shiftLines(STRUCTWB *lpWorkBuffer, int idx, int idF){
	int j;
	 for(j=idx;j<(lpWorkBuffer->infFrame[idF]).nLines-1;j++) {
		 (lpWorkBuffer->line[idF]).y1[j] = (lpWorkBuffer->line[idF]).y1[j+1];
		 (lpWorkBuffer->line[idF]).y2[j] = (lpWorkBuffer->line[idF]).y2[j+1];
	 }
	 (lpWorkBuffer->infFrame[idF]).nLines--;
}
extern     int       gr_ar[];
void histHPtrPix(LPBYTE ptrPix, int widthImg, int hightImg, int x1, int x2, int y1, int y2, int *hHist) {
	  int sww, i ,j, dx, x1D8, dy, sum, ind, w_bt;
	  LONG nY;
	  BYTE  *hptr;
	  int  *pp;

	              w_bt=widthImg;
	              x1D8=x1/8;
	              pp=(int *)hHist;
	              dy=y2-y1+1;
	              dx=x2/8-x1D8-1;
	              *pp=dy;
	              pp++;i=0;
	              while(i<dy)  {
	                sum=0;
	                sww=y1+i;
	                sww=hightImg-1-sww;
	                nY=(LONG)sww*(LONG)w_bt;
	                hptr=ptrPix+nY+(LONG)x1D8;

	                ind=(int)(*hptr);
	                if(ind<0) {ind+=256;}
	                ind=255-ind;
	                sum+=FirstZer(ind,x1%8);
	                hptr++;
	                for(j=0;j<dx;j++)  {
	                        ind=(int)(*hptr);
	                        if(ind<0) {ind+=256;}
	                        ind=255-ind;
	                        sum+=gr_ar[ind];
	                        hptr++;
	                        }
	                if( (j+x1D8+1)<widthImg) {
	                  ind=(int)(*hptr);
	                  if(ind<0) {ind+=256;}
	                  ind=255-ind;
	                  sum+=LastZer(ind,8-x2%8);
	                  }
	//                hptr++;

	                *pp=sum;
	                i++;
	                pp++;
	                }

    }/* of histHPtrPix()*/
void histHTest(LPBYTE ptrPix, int widthImg, int hightImg,  int y1, int y2, int *hHist){
	  int sww, i ,j, dy, sum, ind, w_bt;
	  LONG nY;
	  BYTE  *hptr;
	  int  *pp;

	              w_bt=widthImg;
	              pp=(int *)hHist;
	              dy=y2-y1+1;
	              *pp=dy;
	              pp++;i=0;
	              while(i<dy)  {
	                sum=0;
	                sww=y1+i;
	                sww=hightImg-1-sww;
	                nY=(LONG)sww*(LONG)w_bt;
	                for(j = 0;j<w_bt-1;j++) {
	                    hptr=ptrPix+nY+j;
                        ind=(int)(*hptr);
                        if(ind<0) {ind+=256;}
                        ind=255-ind;
                        sum+=gr_ar[ind];
	                }
	                *pp=sum;
	                i++;
	                pp++;
                }

    }/* of histHPtrPix()*/
void histHTest_X1_X1(LPBYTE ptrPix, int widthImg, int hightImg,  int x1, int x2, int y1, int y2, int *hHist){
	  int sww, i ,j, dy, sum, ind, w_bt;
	  int xStart, xStop;
	  LONG nY;
	  BYTE  *hptr;
	  int  *pp;

	              w_bt=widthImg;
	              pp=(int *)hHist;
	              dy=y2-y1+1;
	              *pp=dy;
	              pp++;i=0;
	              xStart = x1/8;
	              xStop =  x2/8;
	              while(i<dy)  {
	                sum=0;
	                sww=y1+i;
	                sww=hightImg-1-sww;
	                nY=(LONG)sww*(LONG)w_bt;
	                for(j = xStart;j<=xStop;j++) {
	                    hptr=ptrPix+nY+j;
                        ind=(int)(*hptr);
                        if(ind<0) {ind+=256;}
                        ind=255-ind;
                        sum+=gr_ar[ind];
	                }
	                *pp=sum;
	                i++;
	                pp++;
                }

    }/* of histHPtrPix()*/
int maxBlackWidth(LPBYTE ptrPix, int widthImg, int hightImg,  int x1, int x2, int y1, int y2, int *hHist){
	  int sww, i ,j, dy, w_bt;
	  int xStart, xStop;
	  LONG nY;
	  BYTE  *hptr;
	  int max = 0;
	  int maxTmp = 0;
	  int  *pp;

	              w_bt=widthImg;
	              pp=(int *)hHist;
	              dy=y2-y1+1;
	              *pp=dy;
	              pp++;i=0;
	              xStart = x1/8;
	              xStop =  x2/8;
	              while(i<dy)  {
	                sww=y1+i;
	                sww=hightImg-1-sww;
	                nY=(LONG)sww*(LONG)w_bt;
	                maxTmp = 0;
	                max = 0;
	                for(j = xStart;j<=xStop;j++) {
	                    hptr=ptrPix+nY+j;
                        if((*hptr) == 0)
                              maxTmp++;
                        else{
                       	 if(max < maxTmp) max = maxTmp;
                       	 maxTmp = 0;
                        }
	                }
	                *pp=max;
	                i++;
	                pp++;
                }
          return max;
    }/* of maxBlackWidth()*/

void secondTestForLine(STRUCTWB *lpWorkBuffer, int idF, int sizeWImg){
#define	NUM_LINESTOFILTER 40
 int h_size = 1000;
 int h_size1 = 900;
 int th = 4;
 int vHist[h_size];
 int i1, i2;
 int  j, jj, start, stop;
 int idx[NUM_LINESTOFILTER];
 int k = 0, n;
 int av = 0;
 int thW = sizeWImg/2+20;

// char buffLog[500];

 for(j=0;j<10;j++){
	 idx[j] = -1;
 }

 for(j=0;j<(lpWorkBuffer->infFrame[idF]).nLines;j++) {

	 histVPtrPix(lpWorkBuffer->lpRBuffer, lpWorkBuffer -> widthImg, lpWorkBuffer -> hightImg, h_size1-50, 50, h_size1, (lpWorkBuffer->line[idF]).y1[j], (lpWorkBuffer->line[idF]).y2[j], vHist);
	 i1 = getLeftSideStartIdx(vHist, h_size1-50, th)-50;
	 start = lpWorkBuffer -> widthImg*8-h_size1;
	 stop = lpWorkBuffer -> widthImg*8-50;
	 histVPtrPix(lpWorkBuffer->lpRBuffer, lpWorkBuffer -> widthImg, lpWorkBuffer -> hightImg, h_size1-50, start, stop, (lpWorkBuffer->line[idF]).y1[j], (lpWorkBuffer->line[idF]).y2[j], vHist);
	 i2 = getRightSideStartIdx(vHist, h_size1-50, th);
     i2 = start + i2;

//     sprintf(buffLog," -----------------------------------  j= %d, i1= %d, i2= %d, i2-i1= %d, with= %d \n", j, i1, i2, i2-i1, lpWorkBuffer -> widthImg);
//     saveLog("", buffLog);
     if(i2-i1 < thW &&  k < NUM_LINESTOFILTER){
    	 idx[k] = j;
// sprintf(buffLog," -------------adding removal j= %d, k= %d, idx[k] = %d \n", j, k, idx[k]);
// saveLog("", buffLog);

    	 k++;
     }
// sprintf(buffLog," -----------------------------------  j= %d, i1= %d, i2= %d, i2-i1= %d \n", j, i1, i2, i2-i1);
// saveLog("", buffLog);

//  printf(" -----------------------------------  j= %d, i1= %d, i2= %d, i2-i1= %d \n", j, i1, i2, i2-i1);
 }
// for(j=0;j<k;j++){
//	 sprintf(buffLog," ---------------------removing  j= %d, idx[j]= %d \n", j, idx[j]);
//	 saveLog("", buffLog);

//	  printf(" ---------------------removing  j= %d, idx[j]= %d \n", j, idx[j]);
// }
 //remove lines
 j = 0;
 while(j<k){
	 if(idx[j] >= 0) {
// sprintf(buffLog," ---------------------shiftLines  j= %d, idx[j]= %d \n", j, idx[j]);
// saveLog("", buffLog);

		 shiftLines(lpWorkBuffer, idx[j], idF);
	     jj = j+1;
	     while(jj<k) {
	        idx[jj]--;
	        jj++;
	    }
	    j++;
     }
 }
 for(j=0;j<10;j++){
	 idx[j] = -1;
 }
// sprintf(buffLog," ---------------------lpWorkBuffer->infFrame[idF]).nLines= %d\n",(lpWorkBuffer->infFrame[idF]).nLines);
// saveLog("", buffLog);

 k = 0;
 for(j=0;j<(lpWorkBuffer->infFrame[idF]).nLines;j++) {
//	  histHPtrPix(lpWorkBuffer->lpRBuffer, lpWorkBuffer -> widthImg, lpWorkBuffer -> hightImg, 0, lpWorkBuffer -> widthImg-1,
//			     (lpWorkBuffer->line[idF]).y1[j], (lpWorkBuffer->line[idF]).y2[j], vHist);
	  histHTest(lpWorkBuffer->lpRBuffer, lpWorkBuffer -> widthImg, lpWorkBuffer -> hightImg, (lpWorkBuffer->line[idF]).y1[j], (lpWorkBuffer->line[idF]).y2[j], vHist);
      for(n=1;n<=vHist[0];n++){
    	  av += vHist[n];
//if(j == 2)
//  printf(" -----------------------------------  j= %d, n= %d, vHist[n]= %d \n", j, n, vHist[n]);
      }
      if(vHist[0] > 0)
    	  av /= vHist[0];
//  printf(" -----------------------------------  j= %d, av= %d \n", j, av);
      if(av < 30){
     	 idx[k] = j;
     	 k++;
      }
 }

 //remove lines
// sprintf(buffLog," ---------------------remove lines= %d\n",k);
// saveLog("", buffLog);

 for(j=0;j<k;j++){
	 if(idx[j] >= 0) {
		 shiftLines(lpWorkBuffer, idx[j], idF);
	     jj = j+1;
	     while(jj<k) {
	         idx[jj]--;
	         jj++;
	     }
	 }
 }


}

int histH_alpha2(LPBYTE buffPtr, int widthImg, int hightImg,
		          int x1, int y1, int x2, int y2, LONG alpha, float tG, int *histH, int *lenH)
{
  int   sww, i ,j, dx, x1D8, dy, sum, ind, w_bt;
  int   kDirect, sumP, swtP2, swtP, yC, xx, x28;
  LONG  nY;
  BYTE   *hptr;
  int  *pp;

              if((tG<EPSTG)&&(tG>(-EPSTG))) {
                swtP2=32000;
                swtP=0;
                }
              else {
                swtP2=(int)((float)1/tG);
                if(swtP2<0) {swtP2=-swtP2;}
                swtP=swtP2/2;
                }
              kDirect=1;
              if(alpha>0) {kDirect=-1;}
              w_bt= widthImg;
              x1D8=x1/8;
              dy=y2-y1+1;
              dx=x2/8-x1D8-1;
              *lenH=dy;
              pp=histH;
              xx=dx+x1D8+1;
              x28=8-x2%8;
              i=0;
              while(i<dy)  {
                sum=0;
                sww=y1+i;
                yC=sww;

                sww= hightImg-1-sww;
                nY=(LONG)sww*(LONG)w_bt;
                hptr=buffPtr+nY+(LONG)x1D8;

                ind=(int)(*hptr);
                if(ind<0) {ind+=256;}
                ind=255-ind;
                sumP=x1%8;
                sum+=FirstZer(ind, sumP);
                sumP=8-sumP;
                sumP+=swtP;
                hptr++;
                for(j=0;j<dx;j++)  {
                        if(sumP>swtP2) {
                           sumP=0;
                           yC+=kDirect;
                           if((yC<y1)||(yC>y2)) {break;}
                           hptr=hptr-(LONG)(kDirect*w_bt);
                           }
                        ind=(int)(*hptr);
                        if(ind<0) {ind+=256;}
                        ind=255-ind;
                        sum+=gr_ar[ind];
                        hptr++;
                        sumP+=8;
                        }
                if( xx<widthImg) {
                  ind=(int)(*hptr);
                  if(ind<0) {ind+=256;}
                  ind=255-ind;
                  sum+=LastZer(ind, x28);
                  }
                *pp=sum;
                i++;
                pp++;
                }
        return(0);

 }/* end of histH_alpha2() */

int getAngleFromHistH(LPBYTE buffPtr, int widthImg, int hightImg,
		              int x1, int y1, int x2, int y2,
		         float maxAlpha, float stepAlpha, float *ang){
  int   i, nH, lenHH, minH, wH, kD;
  float alP;

   angleHist[0]=0L;
   nH=1;
   kD=1;
   for(i=1;i<=MAXHIST/2;i++) {
        if(nH<MAXHIST) {
                alP=(float)i*stepAlpha;
                if(alP<=maxAlpha) {
// printf("------------ linesForFonts, alP= %e \n", alP);
                    angleHist[nH]=(LONG)(alP*(float)SCALEAN);
                    tGAngle[nH] = (float)tan((double)(alP*PI)/180.0);
                    nH++;
                    if(nH<MAXHIST) {
                        angleHist[nH]= -angleHist[nH-1];
                        tGAngle[nH]  = -tGAngle[nH-1];
                        nH++;
                        }
                    }
                else {break;}
                }
        }

//  printf("------------ linesForFonts, maxAlpha= %e nH= %d \n", maxAlpha, nH);
//  printf("------------ x1= %d, y1= %d, x2= %d, y2= %d \n", x1, y1, x2, y1);
   minH=32000;
   for(i=0;i<nH;i++) {
         lpHistHSk[i]=(int *)&histHSk[i][1];
         histH_alpha2(buffPtr, widthImg, hightImg,
       		          x1, y1, x2, y2,
                     angleHist[i],tGAngle[i],lpHistHSk[i],(int *)&lenHH);
         if(minH>lenHH) {minH=lenHH;}
         }
   wH=whichHist((WORD)(widthImg),(int  * *)lpHistHSk,(int)nH,(int)minH,
             (float *)&tGAngle[0]);

//   for(i=1;i<minH;i++) {
//        histH[i]=histHSk[wH][i];
//        }
//    if(minH >= (lpWorkBuffer->Height)) {minH = (lpWorkBuffer->Height)-1;}
//   *lenH     =minH;
//   *tG   = tGAngle[wH];
   *ang  = (float)angleHist[wH]/(float)SCALEAN;
//    printf("------------ tGAngle[wH]= %e \n", tGAngle[wH]);
//   printf("------------ wH= %d, angleHist[wH]= %ld, minH= %d \n", wH, angleHist[wH], minH);

   return(0);

}  /* getAngleFromHistH() */


