/*
 * Functions.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Wicek
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include  "../headers/Image.h"
#include  "../headers/Prototype.h"


#include <ctype.h>
#include <string.h>

void convertInp24BMPtoMonochrome(LPIMGDATA img, LPIMGDATA imgOut);
LPBYTE LoadBMPFile (LPIMGDATA img);
LPBYTE LoadRGBAPFile (LPIMGDATA img);
//void convertBMPTo_mono_BMP(LPIMGDATA img, LPIMGDATA imgOutp, int tH);
void convertBMPTo_mono_BMP(LPIMGDATA img, LPIMGDATA imgOutp, int tH1, int tH2, int tH3);
void convertDPI_in_BMP(LPIMGDATA img, int dpiX_Otp, int dpiY_Otp);
void saveBMP (LPIMGDATA img);
void saveBMPbuffer (LPBYTE buff, char* name, char* path);

int close_file(FILE *f);
int testBMP (LPBYTE ptr, size_t fSize, LPIMGDATA img);
void printBMPData (LPIMGDATA img);
void copyHeadersBMP(LPBYTE ptr, LPIMGDATA img);
void printLines(LPIMGDATA img, int h, int n);
int isItBMP(LPIMGDATA img);
void setLeftBytes (LPBYTE ptr, int n, int v);
void setPixel(LPIMGDATA img, int xn, int yn, int value);
unsigned int setColorRGB ( BYTE R, BYTE G, BYTE B);
void saveFile (LPIMGDATA img, char *szIMGName, LPBYTE ptr, int len);
void testColorArray(LPIMGDATA img);
void set4Bytes(LPBYTE out, int inp);
void setOnePixelWBFromRGBA(LPIMGDATA imgOutp, int xn, int yn, int v);
void convertRGBAToBMP(LPIMGDATA img);
void convertRGBAcolor_to_RGBAwb(unsigned char *ptr, int imgWidth, int imgHeight);


int setHeaderDataForRGBA (LPBYTE ptr, size_t fSize, LPIMGDATA img);
int getLeftBytes(int* ptr, int num_bytes);
int getWB_value(RGBA *rgba, int tH);
int radonMainG(LPRADON radonStruct);
int radMT(LPRADON radonStruct);
int radDTL(LPRADON radonStruct);
int radDT(LPRADON radonStruct);

void recalculateAllocBuffer(LPDWORD size, int modulo);
int reallocMemoryBuffer (LPHLPBYTE hPtr, DWORD size);
void freeh(LPHLPBYTE hPtr);
int reallocMemoryBufferTest (LPHLPBYTE hPtr, DWORD size);
int recMargin(int* p,  int n, int* pStart,  int * pEnd);
int LoadPrototype (LPTICKETDATA* pticketDataPtr, int pathMode);
int ticketRecognitionInit(LPIMGDATA img, LPRADON radonStruct, RECTANGLE* area, int IDVersion, HLPBYTE* workAreaOutPtr, LPBYTE* iDStructOutPtr,
                          int * pAngle, LPPHI  pPhi);

int ticketRecognitionStruct(LPIMGDATA img, LPRADON radonStruct, LPBYTE pTicketDescr, HLPBYTE* pWorkAreaPtr, LPBYTE* pTicketStruct,
		int iAngle, int tgpA);

int allignArrays
(LPRADON radonStruct,
		LPBYTE pWork, WORD* p0, WORD* p1, WORD n0, WORD n1, DWORD* pDist, WORD* pAllign, WORD* pPos0, WORD* pPosEnd);
int extractTicketsField(LPIMGDATA pImg, LPBYTE lpFieldDescr, WORD wDpiX0, WORD wDpiY0, LPBYTE pZoneStruct,
                        int iAngle,  LPQUADRANGLE pFieldQuad, IMAGEH* pFieldH, LPTICKETRESULTS ticketResults, char * fName);
int  createZoneStructCall (LPIMGDATA img, LPTICKETDATA ticketDataPtr, WORD wZoneNo, LPRADON radonStruct);
int createZoneStruct(LPIMGDATA pImg, LPBYTE pTDescr, HLPBYTE* pWorkArea,  LPBYTE pFormStruct,
		             int* pAngle,  WORD wZoneNo,  LPBYTE* pZoneStruct, LPQUADRANGLE pZoneQuad, LPRADON radonStruct);
int  recalculateCharsPoints(LPIMGDATA pImg, LPQUADRANGLE pFieldQuad,
                                          LPSTRINGS pResults, LPQUADRANGLE pPattern);
int  map(WORD x, WORD y,  DWORD * pX, DWORD * pY, int angle, BYTE *lpStruct);
int mapQ(LPQUADRANGLE pQuad, int angle, LPBYTE lpStruct, int margin);
int grid( WORD Width, WORD Height, RECT *Prot, RECT *gTabProt, STRUCT4POINTS  *gTabForm,
          int *nOfGrid, int gridX, int gridY, int angle, LPBYTE lpZoneStruct, long Scale);
int linearCoef(RECT *Prot, STRUCT4POINTS  *Form, COEF *Coef);
int putImgF(int  xx, int yy, long Scale, BYTE *lpPointer,
            WORD Width, WORD Height, int epsNX, int epsNY);
int removeFieldBack(BYTE *lpPointer, WORD Width, WORD Height, int angle, LPBYTE lpZoneStruct,
                    RECT *lpFieldRect, BYTE *BackStruct);
int desKew(BYTE * pFormBuffer, WORD widthForm, WORD heightForm, int originX, int originY,
           long cosTeta, long sinTeta, BYTE * pFieldBuffer, WORD width, WORD height, WORD extractMargin);
int  extractField(LPBYTE pFormBufferA, WORD widthFormA, WORD heightFormA,
		          int ax, int ay, int bx, int by, int dx, int dy,
		          LPHLPBYTE hpFieldBufferAInp, WORD *pwidth, WORD *pheight, WORD extractMargin);
//int Recognition (LPTICKETDATA ticketDataPtr);
int RecognitionCall (LPIMGDATA img, LPTICKETDATA ticketDataPtr);
int ProcessField (LPIMGDATA img, LPTICKETDATA ticketDataPtr, LPRADON radonStruct, LPTICKETRESULTS ticketResults);
int extractTicketsFieldcall(LPIMGDATA img, LPTICKETDATA ticketDataPtr, LPFIELDDATA field, LPRADON radonStruct,
		LPTICKETRESULTS ticketResults, char * fName);


int recField(LPTICKETDATA ticketDataPtr, IMAGEL* lpImageLA, SEGMENT * lpSegmentA, LINEREM * lpLineRem, TEXTLINE  * lpTextLineA,
 SPECK *lpSpeckA, IGNSUBSIGNS  * lpIgnSubSigns, CONTEXT  * lpContext, int iTypeField, int iAlphaIndx,
 LPBYTE lpAlphaMapA, int iConfThreshold, int iWidthFrame, int iHeightFrame, LPBYTE lpWorkBufferA,
 EXTRACTCHAR * lpExtractChar, RECCHAR * lpRecChar, int iRecMode, int iRecContinue,
 STRINGS * lpStrings, WORD * pwConf0, COORDINATES * lpCoordinates);

int distanceFD(BYTE * pNormBuffer, BYTE* pNNBSample, WORD sqSize, long* pDist);
int recNext(LPTICKETDATA ticketDataPtr);
int readHeader (unsigned char * pDataBuffer,  unsigned char * pHeader);
void initLib (LPTICKETDATA ticketDataPtr);
unsigned char *loadLibrary_G(LPTICKETDATA ticketDataPtr, int index, int *lpLibSize);
int recPictM(LPBYTE buffer, WORD width, WORD height, short *pResult, int index);
int wordpos(char* lpStr);
int getCoordinates1(void);
int getCoordinates(void);
int defineSubsets(char* a, LPBYTE mapA);
int pictWidthHeightSizeDeltaFactor (WORD widthPar, WORD heightPar, WORD sizePar, WORD deltaPar, WORD factorPar);
int normalize(LPPATT lpPatt, LPPATT lpNormPatt, int delta, int factor, int mapVersion, int normVersion);
int getMass(LPPATT lpPatt);
int setMapVers ( WORD mapVersionPar);
int setNormVers ( WORD normVersionPar);
int pictFromNormal (BYTE * pCharBuffer, BYTE * pNormBuffer);
int getMoments ( BYTE* pCharBuffer, int	width, int height, LPMOMENTS pMoments, WORD	 mapVersion);
int normalizeM(LPPATT lpPatt, LPPATT lpNormPatt, int delta, int factor, int mapVersion, int normVersion);
int getMomentsFD(LPPATT lpPatt,  LPMOMENTSFD lpMoments, int mapVersion);
int  initFieldR(char* fieldName, int sizeWImg, LPIMAGEL  lpDataIm, int iTypeField, int iCharType,  int  iSegmType, int angle,
                int skewHist, PREPROCINFO *lpPreprocInfo, TEXTLINE *lpTextLine, int HLRecog,
                int VLRecog, SEGMENT *lpSegment, SPECK *lpSpeck, STRUCTWB  *lpWorkBuffer, int smSignPer, RECCHARSTRUCT  *lpRecCharStruct,
                LPBYTE   lpAlphaMapA, int scale);
void addFilesToTheLib(char *path);
int whatIsThisPreP(BYTE  *lpPointer, int  wImg, int wImgHeight, int xx, int yy);
double calculateDarknessMonoCenter(LPIMGDATA img);
double calculateDarknessMonoTop(LPIMGDATA img);
double calculateDarknessMonoBottom(LPIMGDATA img);
int changeLowDark(int tH, double mass);
void convert24BMPtoMonochromeIter(LPIMGDATA img, LPIMGDATA imgOut);
void convertRGBAtoMonochromeIter(LPIMGDATA img, LPIMGDATA imgOut);
int Recognition(LPIMGDATA fieldImg, LPTICKETDATA ticketDataPtr, BYTE* pFieldDescr, WORD wDpiX0,  WORD wDpiY0,  WORD wCorrectionLevel,
		        STRINGS * pResults, WORD* pwConf, QUADRANGLE* pPattern);
int  nextFieldR(char* fieldName, int sizeW, BYTE *lpBuffer, STRUCTWB *lpWorkBuffer, int *lpLwidth, LPBYTE lpExtractCh,
		        WORD wWidth, WORD  wHeight, WORD wCharType, int scal);

void saveLinesTest(LPBYTE testFieldBufferBMP, STRUCTWB *lpWorkBuffer, char* imgName);
void ClearZones (LPTICKETDATA ticketDataPtr);

void blackMarginRemoval(LPIMGDATA img, char* name);
void cleanTicketNumbers(LPFIELDSRESULTS lpFieldsResults);
void printResult(LPTICKETDATA ticketDataPtr);
void saveField(LPIMGDATA img, LPTICKETDATA ticketDataPtr);
double getFieldMass(LPTICKETDATA ticketDataPtr);
void copyFieldResults(LPTICKETRESULTS ticketResults, FIELD* f, int idx, char *name);
void copyQuad(LPQUADRANGLE qO, LPQUADRANGLE qI, int iStart, int iStop);
void setTicketTypeFromMass(LPTICKETRESULTS ticketResults, double fMass);
void ticketPostProcessing(LPTICKETDATA ticketDataPtr, LPTICKETRESULTS ticketResults);
int getAngleFromHistH(LPBYTE buffPtr, int widthImg, int hightImg, int x1, int y1, int x2, int y2, float maxAlpha, float stepAlpha, float *ang);
void printBMPSize (LPIMGDATA img);
void rotate (LPIMGDATA img, int angle);
void pre_ProcessColorImageBMP(LPIMGDATA img, LPIMGDATA imgOut);
void convert24BMPtoMonochromeGrid(LPIMGDATA img, LPIMGDATA imgOut, int *lpImgMap_m, int *lpImgMap_s, int grid);
void histHTest_X1_X1(LPBYTE ptrPix, int widthImg, int hightImg,  int x1, int x2, int y1, int y2, int *hHist);
void ReleasePrototypes (LPTICKETDATA ticketDataPtr);
int  LoadPrototypes (LPTICKETDATA ticketDataPtr);
int identifyTicket(LPRADON radonStruct, WORD numDescr, LPBYTE* ppIDStruct0, int IDVersion, LPBYTE pIDStructInp, HLPBYTE* workAreaOutPtr,
                   WORD * pwDescrNo, WORD * pwFlip, WORD * pwConfidence, WORD tgp, WORD wPar, WORD wFlags, int * pwOut);

int IdentifyTicketCall (LPRADON radonStruct, HLPBYTE* pWorkAreaPtr, LPTICKETDATA ticketDataPtr, LPBYTE iDStructPtr);
int extractFieldAgain(LPIMGDATA pImg, LPFIELDDATA field, LPTICKETDATA ticketDataPtr);
int maxBlackWidth(LPBYTE ptrPix, int widthImg, int hightImg,  int x1, int x2, int y1, int y2, int *hHist);
int getLogoType(LPTICKETDATA ticketDataPtr);
int getAngle(LPBYTE pFormBuffer, WORD widthF, WORD  heightF, WORD beginX, WORD beginY, WORD widthR, WORD heightR,
		int angMin, int angMax, int angStep);

#if IOS
// These functions are not part of the ANSI standard and are not available in the standard
// library of Apple or Linux platforms.
char *strlwr(char *str);
char *strupr(char *str);

#endif

#endif /* FUNCTIONS_H_ */
