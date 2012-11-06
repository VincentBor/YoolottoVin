/*
 * Postprocessing.c
 *
 *  Created on: Sep 14, 2012
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

void saveLog(char *str1, char *str2);
void printfQuad(void);
void printLine(char *resultsStr, int iStart, int iStop);
char *Month_Names[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };


//        f.result = ticketDataPtr->strings.lpResult;
//        f.cand1  = ticketDataPtr->strings.lpCand1;
//        f.cand2  = ticketDataPtr->strings.lpCand2;
//        f.conf   = ticketDataPtr->flConf;
char        resultTmp[MAXCHARS+2];
//char        cand1Tmp[MAXCHARS+2];
char        cand2Tmp[MAXCHARS+2];
WORD        flConfTmp[MAXCHARS+2];
QUADRANGLE  flQuadTmp[MAXCHARS+2];
char result[200];
//char cand1[200];
char cand2[200];
WORD flConf[200];
QUADRANGLE flQuad[200];
int thConf = 20;

int getLine(char* results, int n, int* iStart, int* iStop){
   int i, len, cc;
   len = strlen(results);
   cc = 0;
   *iStart = 0;
   *iStop = -1;
   for(i=0;i<len;i++){
	   if(results[i] == 13){
		 cc++;
	     if(cc > n){
		   *iStop = i;
		   return 0;
	     }
	     if(i+1 >= len) return -1;
	     *iStart = i+1;
	   }
   }
   if(cc == n) {*iStop = len-1; return 0;}
   return -1;
}
int getNumChars(char *resultsStr, int iStart, int iStop){
		 int i, cc_Ch = 0;
	   for(i=iStart;i<=iStop;i++){
	        if(resultsStr[i] > 32) cc_Ch++;
		}
 	return cc_Ch;
 }
int getNumberOfGroups(char* results, int iStart, int iStop){
	   int i, cc;
	   cc = 0;
	   for(i=iStart;i<iStop;i++){
		   if(results[i] == 32) cc++;
	   }
       return cc;
}
void copyQuad(LPQUADRANGLE qO, LPQUADRANGLE qI, int iStart, int iStop){
	   int i, k;
	   k = 0;
	   for(i=iStart;i<=iStop;i++){
		   qO[k].p1X = qI[i].p1X;
		   qO[k].p1Y = qI[i].p1Y;
		   qO[k].p2X = qI[i].p2X;
		   qO[k].p2Y = qI[i].p2Y;
		   qO[k].p3X = qI[i].p3X;
		   qO[k].p3Y = qI[i].p3Y;
		   qO[k].p4X = qI[i].p4X;
		   qO[k].p4Y = qI[i].p4Y;
           k++;
	   }
}
void copyLine(LPFIELDSRESULTS lpFieldsResults, int iStart, int iStop){
	   int i, k;
	   k = 0;
	   for(i=iStart;i<=iStop;i++){
		   result[k] = lpFieldsResults->result[i];
		   cand2[k]  = lpFieldsResults->cand2[i];
		   flConf[k] = lpFieldsResults->flConf[i];
		   k++;
	   }
//	 printf("  lpFieldsResults->flQuad[1].p1X= %d, lpFieldsResults->flQuad[2].p2X= %d  \n", lpFieldsResults->flQuad[1].p1X, lpFieldsResults->flQuad[2].p2X);
	   copyQuad(flQuad, lpFieldsResults->flQuad, iStart, iStop);
//  printf("  flQuad[1].p1X= %d, flQuad[2].p2X= %d  \n", flQuad[1].p1X, flQuad[2].p2X);
	   result[k] = 0;
	   cand2[k] = 0;
}
void copyResultsBack(LPFIELDSRESULTS lpFieldsResults){
	   int i,  lenTmp;
	   lenTmp = strlen(resultTmp);
	   strcpy(lpFieldsResults->result, resultTmp);
	   strcpy(lpFieldsResults->cand2, cand2Tmp);
	   for(i=0;i<lenTmp;i++){
		   lpFieldsResults->flConf[i] = flConfTmp[i];
	   }
	   copyQuad(lpFieldsResults->flQuad, flQuadTmp, 0, lenTmp);
}
void addLineToTmp(void){
	   int i, k, len, lenTmp;
	   lenTmp = strlen(resultTmp);
	   len = strlen(result);
	   strcat(resultTmp, result);
//	   strcat(cand1Tmp, cand1);
	   strcat(cand2Tmp, cand2);
	   for(i=0;i<len;i++){
		   k = lenTmp +i;
		   flConfTmp[k] = flConf[i];
		   flQuadTmp[k].p1X = flQuad[i].p1X;
		   flQuadTmp[k].p1Y = flQuad[i].p1Y;
		   flQuadTmp[k].p2X = flQuad[i].p2X;
		   flQuadTmp[k].p2Y = flQuad[i].p2Y;
		   flQuadTmp[k].p3X = flQuad[i].p3X;
		   flQuadTmp[k].p3Y = flQuad[i].p3Y;
		   flQuadTmp[k].p4X = flQuad[i].p4X;
		   flQuadTmp[k].p4Y = flQuad[i].p4Y;
       }
}
void replaceFirstCharsTmp(LPFIELDSRESULTS lpFieldsResults, int iStart, int iStop, int n){
	  int i, k;
	  if(n > strlen(resultTmp)) return;
      k = iStart;
	  for(i=0;i<n;i++){
		  resultTmp[i]= lpFieldsResults->result[k];
		  cand2Tmp[i]=  lpFieldsResults->cand2[k];
	      flConfTmp[i] = lpFieldsResults->flConf[k];
	      k++;
	  }
	  copyQuad(flQuadTmp, lpFieldsResults->flQuad,  iStart, iStart+n-1);

}

void shiftQuadUp(LPQUADRANGLE qO, int idx, int lenQ, int lenSh){
	   int i, k;
	   for(i=lenQ;i>=idx;i--){
		   k = i+lenSh;
		   qO[k].p1X = qO[i].p1X;
		   qO[k].p1Y = qO[i].p1Y;
		   qO[k].p2X = qO[i].p2X;
		   qO[k].p2Y = qO[i].p2Y;
		   qO[k].p3X = qO[i].p3X;
		   qO[k].p3Y = qO[i].p3Y;
		   qO[k].p4X = qO[i].p4X;
		   qO[k].p4Y = qO[i].p4Y;
	   }
}
void shiftQuadDown(LPQUADRANGLE qO, int idx, int lenQ, int lenSh){
	   int i, k;
	   for(i=idx;i<lenQ;i++){
			k = i+lenSh;
		   qO[i].p1X = qO[k].p1X;
		   qO[i].p1Y = qO[k].p1Y;
		   qO[i].p2X = qO[k].p2X;
		   qO[i].p2Y = qO[k].p2Y;
		   qO[i].p3X = qO[k].p3X;
		   qO[i].p3Y = qO[k].p3Y;
		   qO[i].p4X = qO[k].p4X;
		   qO[i].p4Y = qO[k].p4Y;
	   }
}

void stretchResults(int idx, int lenStr){
	int i, k, len;
	   len = strlen(result);
	   for(i=len;i>=idx;i--){
		   k = i+lenStr;
		   result[k] = result[i];
//		   cand1[k]  = cand1[i];
		   cand2[k]  = cand2[i];
		   flConf[k] = flConf[i];
	   }
	   shiftQuadUp(flQuad, idx, len, lenStr);
}
void insResult(int idx, char *strT){
	int i, k, lenStr;
	lenStr = strlen(strT);
    k = 0;
	for(i=0;i<lenStr;i++){
		k = idx+i;
	    result[k] = strT[i];
//	    cand1[k]  = strT[i];
        cand2[k]  = strT[i];
	    flConf[k] = 100;
//	    copyQuad(flQuad, ticketDataPtr->flQuad, iStart, iStop);
	   }
}
void cutResults(int kMF){
	int k, len;
	len = strlen(result);
	k = len - kMF;
	if( k > 0) {
	   result[k] = 0;
	   cand2[k]  = 0;
	}
}
void shrinkResults(iStart, iStop){
	int i, k, len, shift;
	len = strlen(result);
	shift = iStop - iStart+1;
    k = 0;
	for(i=iStart;i<len;i++){
		k = i+shift;
		if(k > len) break;
	    result[i] = result[k];
//	    cand1[i]  = cand1[k];
        cand2[i]  = cand2[k];
	    flConf[i] = flConf[k];
	   }
     shiftQuadDown(flQuad, iStart, len, shift);

}
void replaceChars(int iStart, int iStop, char* strT){
   int  lenStr;
   lenStr = strlen(strT);
   if(lenStr < 1) return;
   if(iStop < iStart) {
// printf("--- iStop < iStart\n");
	   stretchResults(iStart, lenStr);
	   insResult(iStart, strT);
   }
   if(iStop >= iStart) {
//	   printf("iStop >= iStart\n");
       shrinkResults(iStart, iStop);
//  printf("---shrinkResults; iStart= %d, iStop= %d;  %s\n", iStart, iStop, result);
       stretchResults(iStart, lenStr);
//  printf("---shrinkResults; iStart= %d, lenStr= %d;  %s\n", iStart, lenStr, result);
       insResult(iStart, strT);
   }
}

void getFirstCharFromNumLine(int* iStart, int* iStop){
	int i, k;
	*iStart = 0;
	 k = -1;
	for(i=1;i<5;i++){
       if(result[i] != 32 && flConf[i]> thConf-10) {
    	   k = i-1;
    	   break;
       }
	}
	if(k> 0 && result[k] == 32)
		k--;
	*iStop = k;
}
int getFirstQPFromNumLine2(int* iStart, int* iStop){
	int i, k, k1, cc, len;
	len = strlen(result);
	*iStart = 0;
	 k = -1;
	 cc = 0;
	for(i=0;i<len;i++){
       if(result[i] == 32) cc++;
       if(cc > 4) {k = i;break;}
	}
    if(k < 0) return -1;
    k1 = -1;
	for(i=k;i<len;i++){
       if(flConf[i] < 20) {k1 = i;break;}
	}
    if(k1 < 0) return -1;
	*iStart = k1;
	k = -1;
	for(i=k1;i<len;i++){
       if(result[i] == 32) {k = i-1;break;}
	}
    if(k < 0) return -1;
    *iStop = k;
    return 0;
}
int getFirstQPFromNumLine(int* iStart, int* iStop){
	int i, k, cc, len;
//	int prev, d;
	int av, max, kMax, a;
	len = strlen(result);
	*iStart = 0;
	k = -1;
	av = 0;
	cc = 0;

	for(i=2;i<len-1;i++){
       if(result[i] == 32) {
    	   av += (flQuad[i+1].p1X - flQuad[i-1].p2X);
//printf("  result[i+1]= %c, result[i-1]= %c, dd= %d \n", result[i+1], result[i-1], flQuad[i+1].p1X - flQuad[i-1].p2X);
    	   cc++;
           if(cc > 3) {k = i;break;}
       }
	}
	if(cc>0)
	av /= cc;

    if(k < 0) return -1;
    max = 0;kMax = 0;
    if(k< len-1)
       a= (flQuad[k+1].p1X - flQuad[k-1].p2X);
    if(a > max) {max = a;kMax = k;}
    k++;
    while(result[k] != 32) k++;
    if(k< len-1)
       a= (flQuad[k+1].p1X - flQuad[k-1].p2X);
    if(a > max) {max = a;kMax = k;}

 //printf("  max= %d, 2*av= %d \n", max, 2*av);
    if(max > (av+av/2))
    	return -1;

   	k = -1;
   	cc = 0;
   	for(i=0;i<len;i++){
          if(result[i] == 32) cc++;
          if(cc > 5) {k = i;break;}
   	}

    while(result[k] == 32) k++;
    *iStart = k;

    while(result[k] != 32) k++;
    if(k+2 < len)
    	if(result[k+2] == 32) k += 2;
    k--;
    *iStop = k;
    return 0;
}
void printfQuad(void){
	int len = strlen(result);
	int i;
	for(i=0;i<len;i++){
   	 printf("  i= %d, flQuad[i].p1X= %d, flQuad[i].p1Y= %d  \n", i, flQuad[i].p1X, flQuad[i].p1Y);
   	 printf("  i= %d, flQuad[i].p2X= %d, flQuad[i].p2Y= %d  \n", i, flQuad[i].p2X, flQuad[i].p2Y);
   	 printf("  i= %d, flQuad[i].p3X= %d, flQuad[i].p3Y= %d  \n", i, flQuad[i].p3X, flQuad[i].p3Y);
   	 printf("  i= %d, flQuad[i].p4X= %d, flQuad[i].p4Y= %d  \n", i, flQuad[i].p4X, flQuad[i].p4Y);
	}
}

void getSecondQPFromNumLine2(int* iStart, int* iStop){
	int i, k, len, kStart = 0;
	len = strlen(result);
	*iStop = len-1;
	 k = len;
	for(i=1;i<6;i++){
	       if(result[len-i] > 32) {
               kStart = i;
	    	   break;
	       }
	}
	*iStop = len-kStart;
	for(i=kStart+1;i<6;i++){
       if(result[len-i] != 13 && result[len-i] != 32 && flConf[len-i] > thConf) {
    	   k = len-i+1;
    	   break;
       }
	}
// printf("--- k= %d; len= %d\n",k, len);
	if(k == len) {
		for(i=2;i<6;i++){
	       if(result[len-i] == 32) {
	    	   k = len-i+1;
	    	   break;
	       }
		}
	}
//printf("--- k= %d;\n",k);

	if(k < len && result[k] == 32)
		k++;
	*iStart = k;
}
int getSecondQPFromNumLine(int* iStart, int* iStop){
	int  k, i, cc, len;
	int iStop_QP1 = *iStop;
	len = strlen(result);
	k = len-1;
	while(result[k] <= 32) k--;
	*iStop = k;
	cc = 0;
    if(iStop_QP1 > 0){
    	for(i = iStop_QP1;i<k;i++)
    		if(result[i] <= 32) cc++;
//printf("--- iStop_QP1= %d, k= %d, cc= %d\n",iStop_QP1, k, cc);
    	if (cc < 2)
    	   return -1;
    }
	while(result[k] > 32) k--;
    if(k-2 > 0)
    	if(result[k-2] == 32)
    		   k -= 2;
    k++;
	*iStart = k;
	return 0;
//	printf("--- *iStart= %d, *iStop= %d\n",*iStart, *iStop);
//	printf("--- result[*iStart]= %c, result[*iStop]= %c\n",result[*iStart], result[*iStop]);
}

void printTmp();
//void printResult(LPTICKETDATA ticketDataPtr);

int testBeginCut(LPFIELDSRESULTS lpFieldsResults, int iStart, int iStop){
	   int cc;
	   cc = 0;
	   if(lpFieldsResults->result[iStart] > 32) cc++;
	   if(lpFieldsResults->result[iStart+1] > 32) cc++;
//  printf("--- lpFieldsResults->result[iStart]= %c, lpFieldsResults->result[iStart+1]= %c\n",lpFieldsResults->result[iStart], lpFieldsResults->result[iStart+1]);
	   if(cc == 2){
		   cc = 0;
		   if(lpFieldsResults->flConf[iStart] > 20) cc++;
           if(lpFieldsResults->flConf[iStart+1] > 20) cc++;
//printf("--- lpFieldsResults->flConf[iStart]= %d, lpFieldsResults->flConf[iStart+1]= %d\n",lpFieldsResults->flConf[iStart], lpFieldsResults->flConf[iStart+1]);
	   }
	   if(cc == 2)
		   return 1;
    return 0;

}
int testLineConf(LPFIELDSRESULTS lpFieldsResults, int iStart, int iStop){
	int i, cc, ccG, ccT;
	i = iStart;
	ccG = 0;ccT = 0;
	cc = 0;
	while(i < iStop) {
	      if(lpFieldsResults->result[i] > 32) cc++;
	      else cc = 0;
	      if(cc> 2) ccG++;
	      i++;
     }
//	printf(" ccG= %d\n",ccG);
	i = iStart;
	cc = 0;
	while(i < iStop) {
		  if((lpFieldsResults->result[i] > 32) && (lpFieldsResults->result[i] != '1')) ccT++;

		  if((lpFieldsResults->result[i] > 32) && (lpFieldsResults->result[i] != '1') &&
				  lpFieldsResults->flConf[i] < 20){ cc++;
// printf(" lpFieldsResults->result[i]= %c, lpFieldsResults->flConf[i]= %d\n",lpFieldsResults->result[i], lpFieldsResults->flConf[i]);
		  }
	      i++;
     }
//	printf(" ccLcnf= %d, ccT= %d\n",cc, ccT);
	if(cc > ccT/2 && ccG > 2) return -1;
	return 1;


}
void testLeftSideForLine(char *resultsStr, int *iStart, int iStop){
	int isT = *iStart;
	if(iStop - isT < 4) return;
	if(resultsStr[isT] == '1' && resultsStr[isT+1] == 32)
		isT += 2;
	*iStart = isT;
}
void cleanTicketNumbers(LPFIELDSRESULTS lpFieldsResults) {
	 char *resultsStr = lpFieldsResults->result;
	 int numGr, i, k;
	 int iStart, iStop;
	 char strT[10];
	 int ret;
	 int numLines = 0;

	 int thGroups = 5;
	 int numChars;
	 int beginCut = 0;
	 resultTmp[0] = 0;
	 cand2Tmp[0] = 0;
// test every line
//printf("\n\n");
//printf(resultsStr);
//printf("\n\n");
// get number of lines
	 i = 0;
	 while(1){
		if(getLine(resultsStr, i, &iStart, &iStop) < 0) break;
		numLines++;
		i++;
	 }
// test last lines
// printf("--- numLines= %d\n",numLines);

	 k = numLines-1;
	 while(k>0){
		 if(getLine(resultsStr, k, &iStart, &iStop) < 0) break;
		 if(testLineConf(lpFieldsResults, iStart, iStop) < 0)
			 numLines--;
		 else
			 break;
		 k--;
	 }

	 i = 0;
	 k = 0;
	 while(i < numLines){
		if(getLine(resultsStr, i, &iStart, &iStop) < 0) break;
           testLeftSideForLine(resultsStr, &iStart, iStop);
//printLine(resultsStr, iStart, iStop);

	    numGr = getNumberOfGroups(resultsStr, iStart, iStop);
	    beginCut = 0;
// printf("--- line i= %d; numGr= %d, iStart= %d, iStop= %d\n",i, numGr, iStart, iStop);
        numChars = getNumChars(resultsStr, iStart, iStop);
//printf("--- line i= %d; numGr= %d, numChars= %d\n",i, numGr, numChars);
        if(numGr == thGroups){
        	beginCut = testBeginCut(lpFieldsResults, iStart, iStop);
            if(beginCut > 0)
            	numGr++;
        }

//   printLine(resultsStr, iStart, iStop);

//   printf("--- line i= %d; numGr= %d, numChars= %d\n",i, numGr, numChars);
	    if(numGr > thGroups && strlen(resultsStr) > 15 && numChars < 20){
	       copyLine(lpFieldsResults, iStart, iStop);
//  printLine(resultsStr, iStart, iStop);

//    printfQuad();
//  printf("--- line i= %d; %s\n",i, result);
	       if(beginCut > 0){


//  printf("--- line i= %d; beginCut= %d\n",i, beginCut);

               strT[0] = 'A'+k;
               strT[1] = 32;
               strT[2] = 0;
               replaceChars(0, -1, strT);
	       }else{
              getFirstCharFromNumLine(&iStart, &iStop);

//  printfQuad();
//   printf("--- getFirstCharFromNumLine i= %d; iStart= %d, iStop= %d\n",i, iStart, iStop);

              if(numGr < 8 && iStart == 0 && iStop == 0 && result[1] != 32) {
                 strT[0] = 'A'+k;
                 strT[1] = 32;
                 strT[2] = 0;
                 replaceChars(0, -1, strT);
             }else{
//  	   printf("--- numGr= %d; iStart= %d, iStop= %d, result[1]= %d\n",numGr, iStart, iStop, result[1]);
                 strT[0] = 'A'+k;
                 strT[1] = 0;
                replaceChars(iStart, iStop, strT);
             }
	       }
           iStop = -1;
           ret = getFirstQPFromNumLine(&iStart, &iStop);
           if(ret >= 0) {
               replaceChars(iStart, iStop, "QP");
           }
           if( ret > 0 || numGr >6) {
              if(getSecondQPFromNumLine(&iStart, &iStop) >=0){
                 replaceChars(iStart, iStop, "QP");
              }
           }
	       addLineToTmp();
	       k++;
//printTmp();
	    }
	    i++;
	 }
	 copyResultsBack(lpFieldsResults);
}
void printTmp(){
	printf ("\n");
	printf ("\n");
    printf ("tmp recognition:-----------\n");
    printf (resultTmp);
    printf ("\n");
    printf ("tmp cand1:-----------\n");
    printf ("\n");
    printf ("tmp cand2:-----------\n");
    printf (cand2Tmp);
    printf ("\n");
    printf ("-----------------------\n");
    {int iCnf;
       for(iCnf = 0;iCnf<strlen(resultTmp);iCnf++){
    	if(resultTmp[iCnf] != 13)
    	   printf("%c,(%d)", resultTmp[iCnf], flConfTmp[iCnf]);
    	else
    	   printf("\n");
       }
    }

}
void printResult(LPTICKETDATA ticketDataPtr){
	printf ("\n");
	printf ("\n");
    printf ("recognition:-----------\n");
    printf (ticketDataPtr->strings.lpResult);
    printf ("\n");
//    printf ("cand1:-----------\n");
//    printf (ticketDataPtr->strings.lpCand1);
//    printf ("\n");
    printf ("cand2:-----------\n");
    printf (ticketDataPtr->strings.lpCand2);
    printf ("\n");
    printf ("-----------------------\n");
    {int iCnf;
       for(iCnf = 0;iCnf<strlen(ticketDataPtr->strings.lpResult);iCnf++){
    	if(ticketDataPtr->strings.lpResult[iCnf] != 13)
    	   printf("%c,(%d)", ticketDataPtr->strings.lpResult[iCnf], ticketDataPtr->flConf[iCnf]);
    	else
    	   printf("\n");
       }
    }
}
void printTicketType(int type, int cnf){
	switch(type){
	   case 0: printf("ticket type: Mega Million; confidence= %d", cnf); break;
	   case 1: printf("ticket type: Power Ball; confidence= %d", cnf); break;
	   default: printf("ticket type: ???"); break;
	}
}
void printField(LPFIELDSRESULTS lpFieldsResults){
	 printf ("-------------------------------------------\n");
      printf("Field: ");printf(lpFieldsResults->name);
  	  printf ("\n");
  	  printf ("\n");
      printf (lpFieldsResults->result);
      printf ("\n");
 	 printf ("-------------------------------------------\n");
/*
     {int iCnf;
        for(iCnf = 0;iCnf<strlen(lpFieldsResults->result);iCnf++){
     	if(lpFieldsResults->result[iCnf] > 32)
     	   printf("%c,(%d)", lpFieldsResults->result[iCnf], lpFieldsResults->flConf[iCnf]);
     	else
     	   printf("\n");
        }
     }
*/
}
void printResults_1(LPTICKETRESULTS ticketResults){
	int i;
   printf("Status= %d\n", ticketResults->status);
   printTicketType(ticketResults->type, ticketResults->typeCnf);
    for(i=0;i<NUMFIELDS_RESULTS;i++){
    	if(ticketResults->fieldsResults[i].name[0] != 0){
//    		if(strcmp(ticketResults->fieldsResults[i].name, "LOGO3") == 0) printField(&ticketResults->fieldsResults[i]);
    		if(strcmp(ticketResults->fieldsResults[i].name, "NUMBERS") == 0) printField(&ticketResults->fieldsResults[i]);
//    		if(strcmp(ticketResults->fieldsResults[i].name, "DATEA") == 0) printField(&ticketResults->fieldsResults[i]);
//    		if(strcmp(ticketResults->fieldsResults[i].name, "DATEN") == 0) printField(&ticketResults->fieldsResults[i]);
//    		else printField(&ticketResults->fieldsResults[i]);

//    		if(strcmp(ticketResults->fieldsResults[i].name, "NUMBERS") == 0) printField(&ticketResults->fieldsResults[i]);
//    		if(strcmp(ticketResults->fieldsResults[i].name, "DATE") == 0) printField(&ticketResults->fieldsResults[i]);
    	}
    }

}

void printResults(LPTICKETRESULTS ticketResults){
	int i;
   printf("Status= %d\n", ticketResults->status);
   printTicketType(ticketResults->type, ticketResults->typeCnf);
    for(i=0;i<NUMFIELDS_RESULTS;i++){
    	if(ticketResults->fieldsResults[i].name[0] != 0){
//  printf("ticketResults->fieldsResults[i].name= %s\n", ticketResults->fieldsResults[i].name);
    		if(strcmp(ticketResults->fieldsResults[i].name, "NUMBERS") == 0) printField(&ticketResults->fieldsResults[i]);
    		if(strcmp(ticketResults->fieldsResults[i].name, "PLAYER") == 0) printField(&ticketResults->fieldsResults[i]);
    		if(strcmp(ticketResults->fieldsResults[i].name, "DATE") == 0) printField(&ticketResults->fieldsResults[i]);
//    		else printField(&ticketResults->fieldsResults[i]);

//    		if(strcmp(ticketResults->fieldsResults[i].name, "NUMBERS") == 0) printField(&ticketResults->fieldsResults[i]);
//    		if(strcmp(ticketResults->fieldsResults[i].name, "DATE") == 0) printField(&ticketResults->fieldsResults[i]);
    	}
    }
}
int matchToStringOld(char *results, int iStart, int iStop, char *str){
	   int i, cc1, cc2;
	   int len = strlen(str);
	   if(len > iStop-iStart) len = iStop-iStart;
	   cc1 = 0;
	   for(i=0;i<len;i++){
		   if(results[i+iStart] == str[i]) cc1++;
	   }
	   cc2 = 0;
	   if(len < iStop-iStart) {
	      for(i=1;i<=len;i++){
			   if(results[i+iStart] == str[i]) cc2++;
	     }
	   }
       if(cc1<cc2) cc1 = cc2;
    return cc1;
}
int matchToString(char *results, int iStart, int iStop, char *str){
	   int i, k, n, cc, ccMax;
	   int len = strlen(str);

	   if(len > iStop-iStart) len = iStop-iStart;
	   ccMax = 0;
	   for(k=0;k<3;k++) {
	      cc = 0;
	      for(i=0;i<len;i++){
	    	  n = i+iStart+k;
	    	  if(n >= iStop) break;
		      if(results[n] == str[i]) cc++;
	      }
	      if(ccMax < cc) ccMax = cc;
	   }
    return ccMax;
}

int matchToStringEndOneLineIdx(char *results, char *str, int sh){
	   int i, k, cc, ccMax, maxIdx;
	   int iStart, iStop;
	   int dd;
	   int len = strlen(str);

	   iStart = 0; iStop = strlen(results)-1;
	   if(len > iStop-iStart) len = iStop-iStart;
	   cc = 0;ccMax = 0;maxIdx = iStop-4;
	   for(k=0;k<sh;k++){
		   cc = 0;
		   dd = iStop-len-k+1;
//  printf("-----------------------k= %d, dd= %d, iStop= %d\n", k, dd, iStop);
		   for(i=0;i<len;i++){
			   if(i+dd <= iStop) {
// printf("---i+dd= %d, results[i+dd]= %c, str[i]= %c\n", i+dd, results[i+dd], str[i]);
			     if(results[i+dd] == str[i]) cc++;
			   }
		   }
        if(cc > ccMax) {ccMax = cc;maxIdx=dd;}
//      printf("---ccMax= %d, maxIdx= %d\n", ccMax, maxIdx);
	   }
     return maxIdx;
}
int matchToStringEnd(char *results, int iStart, int iStop, char *str){
	   int i, k, cc, ccMax;
	   int dd;
	   int len = strlen(str);
	   if(len > iStop-iStart) len = iStop-iStart;
	   cc = 0;ccMax = 0;
	   for(k=0;k<3;k++){
		   cc = 0;
		   dd = iStop-len-k+1;
//printf("-----------------------k= %d\n", k);
		   for(i=0;i<len;i++){
			   if(i+dd <= iStop) {
// printf("---i+dd= %d, results[i+dd]= %c, str[i]= %c\n", i+dd, results[i+dd], str[i]);
			     if(results[i+dd] == str[i]) cc++;
			   }
		   }
        if(cc > ccMax) ccMax = cc;
	   }
     return ccMax;
}

void printLine(char *resultsStr, int iStart, int iStop){
	char line[100];
	int i, k;
	k = 0;
	for(i=iStart;i<iStop;i++){
		line[k] = resultsStr[i];
		k++;
	}
	line[k] = 0;
 printf("---line= %s\n", line);
}
void getConfidenceForTicketType(LPFIELDSRESULTS lpFieldsResults, int *lpcnfM, int *lpcnfP){
	 char *resultsStr = lpFieldsResults->result;
	 int cM, maxcM, cP, maxcP;
	 int i;
	 int iStart, iStop;

	 resultTmp[0] = 0;
	 cand2Tmp[0] = 0;
	 i = 0;
	 maxcM = 0;
	 maxcP = 0;
	 while(1){
		if(getLine(resultsStr, i, &iStart, &iStop) < 0) break;
//		printLine(resultsStr, iStart, iStop);
	    cM = matchToString(resultsStr, iStart, iStop, "MEGA");
//	    printf("---cM= %d\n", cM);
	    if(cM > maxcM) maxcM = cM;
	    cP = matchToString(resultsStr, iStart, iStop, "POWER");
//	    printf("---cP= %d\n", cP);
	    if(cP > maxcP) maxcP = cP;
	    i++;
	 }
	 *lpcnfM = maxcM;
	 *lpcnfP = maxcP;
}

void createPlierField(LPTICKETRESULTS ticketResults, int n){
	 FIELDSRESULTS fieldsResults = ticketResults->fieldsResults[n];
	 char *resultsStr = fieldsResults.result;
	 int cM, maxcM, cP, maxcP, lineM, lineP;
	 int i;
	 int iStart, iStop, line;
	 int ccYes, ccNo, maxYes, maxNo;

	 resultTmp[0] = 0;
	 cand2Tmp[0] = 0;
	 i = 0;
	 maxcM = 0;
	 maxcP = 0;
	 lineM = -1;
	 lineP = -1;
	 while(1){
		if(getLine(resultsStr, i, &iStart, &iStop) < 0) break;
//		printLine(resultsStr, iStart, iStop);
	    cM = matchToString(resultsStr, iStart, iStop, "MEGA");
//	    printf("---cM= %d\n", cM);
	    if(cM > maxcM) {maxcM = cM;lineM = i;}
	    cP = matchToString(resultsStr, iStart, iStop, "POWER");
//	    printf("---cP= %d\n", cP);
	    if(cP > maxcP) {maxcP = cP;lineP = i;}
	    i++;
	 }
	 line = -1;
	 ccYes = 0; ccNo = 0;
	 if(maxcM > maxcP && lineM >= 0) line = lineM;
	 if(maxcP > maxcM && lineP >= 0) line = lineP;
// printf("---maxcM= %d, lineM= %d, maxcP= %d, lineP= %d, line= %d\n", maxcM, lineM, maxcP, lineP, line);
	 maxYes = 0; maxNo = 0;
	 if(line >= 0) {
	         if(getLine(resultsStr, line, &iStart, &iStop) >= 0) {
//printLine(resultsStr, iStart, iStop);
	             ccYes = matchToStringEnd(resultsStr, iStart, iStop, "YES");
	             ccNo = matchToStringEnd(resultsStr, iStart, iStop, "NO");
//  printf("---ccYes= %d, ccNo= %d\n", ccYes, ccNo);
	          }
	 }else{
		 i = 0;
		 while(1){
			if(getLine(resultsStr, i, &iStart, &iStop) < 0) break;
			ccYes = matchToStringEnd(resultsStr, iStart, iStop, "YES");
//  printf("---i = %d, ccYes= %d\n", i, ccYes);
		    if(ccYes > maxYes) maxYes = ccYes;
            ccNo = matchToStringEnd(resultsStr, iStart, iStop, "NO");
//  printf("---i = %d, ccNo= %d\n", i, ccNo);
		    if(ccNo > maxNo) maxNo = ccNo;
//  printf("---i = %d, maxYes= %d, maxNo= %d\n", i, maxYes, maxNo);
		    i++;
		 }
		 ccYes = maxYes;
		 ccNo  = maxNo;
	 }
// printf("---ccYes= %d, ccNo= %d\n", ccYes, ccNo);
    {int nIdx = NUMFIELDS_RESULTS-1;
         strcpy(ticketResults->fieldsResults[nIdx].name, "PLAYER");
         strcpy(ticketResults->fieldsResults[nIdx].result, "???");
         if(ccYes > ccNo) {
	         strcpy(ticketResults->fieldsResults[nIdx].result, "YES");
         }
         if(ccYes < ccNo) {
	         strcpy(ticketResults->fieldsResults[nIdx].result, "NO");
         }
         for(i=0;i<strlen(ticketResults->fieldsResults[nIdx].result);i++){
        	 ticketResults->fieldsResults[nIdx].flConf[i] = 100;
         }

    }
}

int  calculateTypeCnf(int cnfT, int cnf2, int cnf3){
	int cnf = 0;
	 if(cnf2 >3 && cnf3 >3)
		 return 100;
	 if(cnf2 >3 || cnf3 >3)
		 return 80;
   	 cnf = cnfT +cnf2*5 + cnf3*5;
 	return cnf;
}

void  postProcessingLogo(LPTICKETRESULTS ticketResults){
	int i, d;
	int cnfM = 0, cnfP = 0;
	int cnfM2 = 0, cnfP2 = 0;
	int cnfM3 = 0, cnfP3 = 0;
	int cnfMT = 0, cnfPT = 0;
    for(i=0;i<NUMFIELDS_RESULTS;i++){
    	if(ticketResults->fieldsResults[i].name[0] != 0){
    		if(strcmp(ticketResults->fieldsResults[i].name, "LOGO2") == 0) getConfidenceForTicketType(&ticketResults->fieldsResults[i], &cnfM2, &cnfP2);
    		if(strcmp(ticketResults->fieldsResults[i].name, "LOGO3") == 0) {
    			getConfidenceForTicketType(&ticketResults->fieldsResults[i], &cnfM3, &cnfP3);
    			createPlierField(ticketResults, i);
    		}
    	}
    }
	switch(ticketResults->type){
	   case 0: cnfMT += 30; break;
	   case 1: cnfPT += 30; break;
	}

    cnfM = calculateTypeCnf(cnfMT, cnfM2, cnfM3);
// printf("---cnfM= %d, cnfMT= %d, cnfM2= %d, cnfM3= %d\n", cnfM, cnfMT, cnfM2, cnfM3);
    cnfP = calculateTypeCnf(cnfPT, cnfP2, cnfP3);
//printf("---cnfP= %d, cnfPT= %d, cnfP2= %d, cnfP3= %d\n", cnfP, cnfPT, cnfP2, cnfP3);
//  printf("---cnfMega= %d, cnfPower= %d\n", cnfM, cnfP);

    ticketResults->type = -1;
    d = cnfM - cnfP;
    if(d>10) ticketResults->type = 0;
    if(d<-10) ticketResults->type = 1;
    if(d<0) d = -d;
    ticketResults->typeCnf = d;
}
int getNumLowConfidentChars(LPFIELDSRESULTS lpFieldsResults, int tH){
	 char *resultsStr = lpFieldsResults->result;
	 WORD  *cnf = lpFieldsResults->flConf;
	 int len = strlen(resultsStr);
	 int i, cc_Lc = 0, cc_Ch = 0, cc_1 = 0;
	 for(i=0;i<len;i++){
        if(cnf[i] < tH) cc_Lc++;
        if(resultsStr[i] > 32) cc_Ch++;
        if(resultsStr[i] == '1') cc_1++;
	}
//printf("----------------cc_Lc= %d, cc_Ch= %d, cc_1= %d,\n", cc_Lc, cc_Ch, cc_1);
    if(cc_Ch == 0) return 0;
    if((float)cc_Lc/(float)cc_Ch > 0.1) return 0;
    if((float)cc_1/(float)cc_Ch > 0.5) return 0;
    return 1;
}

int  postProcessingNumbers(LPFIELDSRESULTS lpFieldsResults){
	int status = 0;
	cleanTicketNumbers(lpFieldsResults);
	status = getNumLowConfidentChars(lpFieldsResults, 20);
	return status;
//	printf("------------------------------------------------status= %d\n", status);
}
int matchToStringResult(char *str, int *kM){
	   int i, k, cc, ccMax, kMax;
	   int lenR = strlen(result);
	   int len = strlen(str);

	   if(len > lenR) len = lenR;
	   ccMax = 0; kMax = -1;
	   for(k=0;k<lenR-len;k++){
		   cc = 0;
	      for(i=0;i<len;i++){
		     if(result[i+k] == str[i]) cc++;
	      }
	      if(ccMax < cc) {ccMax = cc;kMax = k;}
	   }
	   *kM = kMax;
       return ccMax;
}
int matchChars(char *m, char *monthName){
	int i, cc;
	cc = 0;
	for(i=0;i<strlen(m);i++){
		if(m[i] == monthName[i]) cc++;
	}
	return cc;
}
void getMonth(char *m, char *mOut){
	int cc, ccMax, idxMax, cc2;
	int i;
	cc = 0;ccMax = 0;idxMax = -1;
	cc2 = 0;
	for(i=0;i<12;i++) {
	   cc = matchChars(m, Month_Names[i]);
	   if(ccMax == cc) cc2++;
	   else {
	      if(ccMax < cc){ccMax = cc;idxMax = i;cc2 = 0;}
	   }
	}
//printf("-------m=  %s, ccMax= %d, cc2= %d idxMax= %d\n", m, ccMax, cc2, idxMax);
//if(idxMax >= 0)
//	printf("-------Month_Names[idxMax]=  %s\n", Month_Names[idxMax]);

	if(cc2 == 0 && ccMax > 0) strcpy(mOut, Month_Names[idxMax]);
	else strcpy(mOut, m);
}
void cleanMonth(char *strInp){
	char m[64];
	char mOut[4];
	if(strlen(strInp) >= 64) return;
	strcpy(m, strInp);
	m[3] = 0;
	if(m[0] == '5') m[0] = 'S';
	getMonth(m, mOut);
	strInp[0] = mOut[0];strInp[1] = mOut[1];strInp[2] = mOut[2];
}

void cleanDateMega(){
	int kMF, ccF;
	int kMW, ccW;
// day
	ccF = matchToStringResult("FRI", &kMF);
	ccW = matchToStringResult("TUE", &kMW);
	if(ccF > ccW) {
	   if(kMF > 0) {shrinkResults(0, kMF-1);cutResults(kMF-1);}

// printf("-------result= %s \n", result);

	   result[0] = 'F';result[1] = 'R';result[2] = 'I';
	}
	if(ccF < ccW) {
	   if(kMW > 0) {shrinkResults(0, kMW-1);cutResults(kMW-1);}
	   result[0] = 'T';result[1] = 'U';result[2] = 'E';
	}
// month
	cleanMonth(&result[4]);

}
void cleanDatePower(){
	int kMF, ccF;
	int kMW, ccW;
// day
	ccF = matchToStringResult("SAT", &kMF);
	ccW = matchToStringResult("WED", &kMW);
	if(ccF > ccW) {
	   if(kMF > 0) {shrinkResults(0, kMF-1);cutResults(kMF-1);}
	   result[0] = 'S';result[1] = 'A';result[2] = 'T';
	}
	if(ccF < ccW) {
	   if(kMW > 0) {shrinkResults(0, kMW-1);cutResults(kMW-1);}
	   result[0] = 'W';result[1] = 'E';result[2] = 'D';
	}
	cleanMonth(&result[4]);

}

int matchDayMega(char *resultsStr, int iStart, int iStop){
	int match = 0;
	int ccF = 0;
	int ccT = 0;
	int ccD = 0;
	int i;
	ccF = matchToString(resultsStr, iStart, iStop, "FRI");
	ccT = matchToString(resultsStr, iStart, iStop, "TUE");
	if(ccF<ccT) ccF = ccT;
    ccD = 0;
    for(i=iStart;i<iStop;i++){
	   if(resultsStr[i] >= '0' && resultsStr[i] <= '9') ccD++;
    }
    if(ccF >1 && ccD > 3) match = 1;
	return match;
}
int matchDayPower(char *resultsStr, int iStart, int iStop){
	int match = 0;
	int ccF = 0;
	int ccT = 0;
	int ccD = 0;
	int i;
	ccF = matchToString(resultsStr, iStart, iStop, "SAT");
	ccT = matchToString(resultsStr, iStart, iStop, "WED");
	if(ccF<ccT) ccF = ccT;
    ccD = 0;
    for(i=iStart;i<iStop;i++){
	   if(resultsStr[i] >= '0' && resultsStr[i] <= '9') ccD++;
    }
    if(ccF >1 && ccD > 3) match = 1;
	return match;
}

int matchDayMegaA(char *resultsStr, int iStart, int iStop){
//	int match = 0;
	int ccF = 0;
	int ccT = 0;
//	int ccD = 0;
//	int i;
	ccF = matchToString(resultsStr, iStart, iStop, "FRI");
	ccT = matchToString(resultsStr, iStart, iStop, "TUE");
//printf("M-------ccF= %d, ccT= %d\n", ccF, ccT);
	if(ccF<ccT) ccF = ccT;
//    ccD = 0;
//    for(i=iStart;i<iStop;i++){
//	   if(resultsStr[i] >= '0' && resultsStr[i] <= '9') ccD++;
//    }
//    if(ccF >1 && ccD > 3) match = 1;
//	 if(ccF >1 ) match = 1;
	return ccF;
}
int matchDayPowerA(char *resultsStr, int iStart, int iStop){
//	int match = 0;
	int ccF = 0;
	int ccT = 0;
//	int ccD = 0;
//	int i;
	ccF = matchToString(resultsStr, iStart, iStop, "SAT");
	ccT = matchToString(resultsStr, iStart, iStop, "WED");

//printf("P-------ccF= %d, ccT= %d\n", ccF, ccT);
	if(ccF<ccT) ccF = ccT;
//    ccD = 0;
//    for(i=iStart;i<iStop;i++){
//	   if(resultsStr[i] >= '0' && resultsStr[i] <= '9') ccD++;
//    }
//    if(ccF >1 && ccD > 3) match = 1;
//    if(ccF > 1) match = 1;
	return ccF;
}

void  postProcessingDate(LPFIELDSRESULTS lpFieldsResults){
	 char *resultsStr = lpFieldsResults->result;
	 int i;
	 int iStart, iStop;
//	 char Date[20];

	 resultTmp[0] = 0;
	 cand2Tmp[0] = 0;
// test every line
	 i = 0;
	 while(1){
		if(getLine(resultsStr, i, &iStart, &iStop) < 0) break;
		if(matchDayMega(resultsStr, iStart, iStop) == 1){
		       copyLine(lpFieldsResults, iStart, iStop);
		       cleanDateMega();
		       addLineToTmp();
		       break;
		}
		if(matchDayPower(resultsStr, iStart, iStop) == 1){
		       copyLine(lpFieldsResults, iStart, iStop);
		       cleanDatePower();
		       addLineToTmp();
		       break;
		}
	    i++;
	 }
//	  strlen(resultTmp);
	 copyResultsBack(lpFieldsResults);

	 return;
}
int  postProcessingDateA(LPFIELDSRESULTS lpFieldsResults){
	 char *resultsStr = lpFieldsResults->result;
	 int i;
	 int iStart, iStop;
	 int line;
	 int ccM, ccP, ccMmax, ccPmax;
	 int lineMmax, linePmax;
//	 char Date[20];

	 resultTmp[0] = 0;
	 cand2Tmp[0] = 0;
// test every line
	 i = 0;
	 line = -1;
	 ccMmax= 0; ccPmax = 0;
	 lineMmax = -1; linePmax = -1;
	 while(1){
		if(getLine(resultsStr, i, &iStart, &iStop) < 0) break;

		ccM = matchDayMegaA(resultsStr, iStart, iStop);
		if(ccMmax < ccM) {ccMmax = ccM;lineMmax = i;}
		ccP = matchDayPowerA(resultsStr, iStart, iStop);
		if(ccPmax < ccP) {ccPmax = ccP;linePmax = i;}
//printLine(resultsStr, iStart, iStop);
//printf("-------ccM= %d, ccP= %d\n", ccM, ccP);
		i++;
	 }

		if(ccMmax > ccPmax) {
			if(getLine(resultsStr, lineMmax, &iStart, &iStop) >= 0) {
//printLine(resultsStr, iStart, iStop);
//printf("-------Mega= \n");
//			printLine(resultsStr, iStart, iStop);

		         copyLine(lpFieldsResults, iStart, iStop);
		         cleanDateMega();
		         addLineToTmp();
		         line = lineMmax;
			}
		}
		if(ccMmax < ccPmax) {
			if(getLine(resultsStr, linePmax, &iStart, &iStop) >= 0) {
//printf("-------Power= \n");
//printLine(resultsStr, iStart, iStop);
		       copyLine(lpFieldsResults, iStart, iStop);
		       cleanDatePower();
		       addLineToTmp();
		       line = linePmax;
			}
		}
//	    i++;
//	 }
//	  strlen(resultTmp);
//	 copyResultsBack(lpFieldsResults);
	 lpFieldsResults->name[strlen(lpFieldsResults->name)-1] = 0;

	 return line;
}

int  postProcessingDateD(LPFIELDSRESULTS lpFieldsResults){
	 char *resultsStr = lpFieldsResults->result;
	 int i;
	 int iStart, iStop;
	 int line;
	 int ccY, ccYmax;
	 int lineYmax;

	 resultTmp[0] = 0;
	 cand2Tmp[0] = 0;
// test every line
	 i = 0;
	 line = -1;
	 ccYmax= 0;
	 lineYmax = -1;
	 while(1){
		if(getLine(resultsStr, i, &iStart, &iStop) < 0) break;
		ccY = matchToStringEnd(resultsStr, iStart, iStop, "201");
		if(ccYmax < ccY) {ccYmax = ccY;lineYmax = i;}
		i++;
	 }

		if(ccYmax > 1) {
			if(getLine(resultsStr, lineYmax, &iStart, &iStop) >= 0) {
		         copyLine(lpFieldsResults, iStart, iStop);
		         cleanDateMega();
		         addLineToTmp();
		         line = lineYmax;
			}
		}
//	 lpFieldsResults->name[strlen(lpFieldsResults->name)-1] = 0;

	 return line;
}
int adjustLine(char *resultsStr, int line){
	int iStart, iStop;
	 int ccY, ccYmax;
	 int lineYmax;

    ccYmax= 0; lineYmax = line;
	if(line > 0){
         getLine(resultsStr, line-1, &iStart, &iStop);
 		 ccY = matchToStringEnd(resultsStr, iStart, iStop, "201");
 		 if(ccYmax < ccY) {ccYmax = ccY;lineYmax = line-1;}

	}
    getLine(resultsStr, line, &iStart, &iStop);
    ccY = matchToStringEnd(resultsStr, iStart, iStop, "201");
	if(ccYmax < ccY) {ccYmax = ccY;lineYmax = line;}

    if(getLine(resultsStr, line+1, &iStart, &iStop) >= 0){
 		 ccY = matchToStringEnd(resultsStr, iStart, iStop, "201");
 		 if(ccYmax < ccY) {ccYmax = ccY;lineYmax = line+1;}
    }
    return lineYmax;
}

int adjustLineA(char *resultsStr, int line){
	int iStart, iStop;
	 int lineOut = line;
	 int ccM, ccP, ccMmax, ccPmax;
	 int lineMmax, linePmax;

	 ccMmax= 0; ccPmax = 0;
	 lineMmax = -1; linePmax = -1;
	 if(line> 0) {
	    if(getLine(resultsStr, line-1, &iStart, &iStop) >= 0) {
		   ccM = matchDayMegaA(resultsStr, iStart, iStop);
		   if(ccMmax < ccM) {ccMmax = ccM;lineMmax = line-1;}
		   ccP = matchDayPowerA(resultsStr, iStart, iStop);
		   if(ccPmax < ccP) {ccPmax = ccP;linePmax = line-1;}
	    }
	 }
    if(getLine(resultsStr, line, &iStart, &iStop) >= 0) {
	   ccM = matchDayMegaA(resultsStr, iStart, iStop);
	   if(ccMmax < ccM) {ccMmax = ccM;lineMmax = line;}
	   ccP = matchDayPowerA(resultsStr, iStart, iStop);
	   if(ccPmax < ccP) {ccPmax = ccP;linePmax = line;}
    }
    if(getLine(resultsStr, line+1, &iStart, &iStop) >= 0) {
	   ccM = matchDayMegaA(resultsStr, iStart, iStop);
	   if(ccMmax < ccM) {ccMmax = ccM;lineMmax = line+1;}
	   ccP = matchDayPowerA(resultsStr, iStart, iStop);
	   if(ccPmax < ccP) {ccPmax = ccP;linePmax = line+1;}
    }
    if(ccPmax > ccMmax)	 lineOut = linePmax;
    if(ccPmax < ccMmax)	 lineOut = lineMmax;

    return lineOut;
}
int countNoChars(char *resultsStr){
	int i, cc, len;
	len = strlen(resultsStr);
	cc = 0;
	for(i=0;i<len-1;i++){
		if(resultsStr[i] <32) cc++;
	}
	if(cc == 0 && resultsStr[len-1] < 32)
		resultsStr[len-1] = 0;
	return cc;
}
void cleanDateField(LPTICKETRESULTS ticketResults){
	 LPFIELDSRESULTS lpFieldsResults;
	 char *resultsStr;
	 int i, cc;

   for(i=0;i<NUMFIELDS_RESULTS;i++){
	    	if(ticketResults->fieldsResults[i].name[0] != 0){
	    		if(strcmp(ticketResults->fieldsResults[i].name, "DATE") == 0) {

	    			lpFieldsResults = &ticketResults->fieldsResults[i];
	    			resultsStr = lpFieldsResults->result;
	    			cc = countNoChars(resultsStr);
	    			if(strlen(resultsStr) < 10) cc++;
	    			if(cc > 0)
	    				resultsStr[0] = 0;
	    			break;
	    		}

	    	}
	    }

}
void cleanYear(LPTICKETRESULTS ticketResults){
	 LPFIELDSRESULTS lpFieldsResults;
	 char *resultsStr;
	 int i, k;
//	 int iStart, iStop;

    for(i=0;i<NUMFIELDS_RESULTS;i++){
	    	if(ticketResults->fieldsResults[i].name[0] != 0){
	    		if(strcmp(ticketResults->fieldsResults[i].name, "DATE") == 0) {


 	    			lpFieldsResults = &ticketResults->fieldsResults[i];
 	    			resultsStr = lpFieldsResults->result;

//printf("-------Date = %s \n", resultsStr);
	    			k = matchToStringEndOneLineIdx(resultsStr, "201", 3);

//printf("-------k = %d, strlen(resultsStr)-2=%d \n", k, strlen(resultsStr)-2);
	    			if(k>0 && k<strlen(resultsStr)-2){
	    				resultsStr[k] = '2';resultsStr[k+1] = '0';resultsStr[k+2] = '1';
	    			}
	    			break;
	    		}

	    	}
	    }
}
void  postProcessingDateNew(LPTICKETRESULTS ticketResults){
	 LPFIELDSRESULTS lpFieldsResults;
//	 char *resultsStr = lpFieldsResults->result;
	 char *resultsStr;
	 int i, k, iA, line, len, sh;
	 int iStart, iStop, idxY, flgCut;
//	 char Date[20];
	 iA = -1;
	 line = -1;
     for(i=0;i<NUMFIELDS_RESULTS;i++){
	    	if(ticketResults->fieldsResults[i].name[0] != 0){
	    		if(strcmp(ticketResults->fieldsResults[i].name, "DATEA") == 0) {
	    			line = postProcessingDateA(&ticketResults->fieldsResults[i]);
	    			iA = i;
	    			break;
	    		}

	    	}
	    }
//	printf("-------resultTmp = %s, line= %d \n", resultTmp, line);

     for(i=0;i<NUMFIELDS_RESULTS;i++){
	    	if(ticketResults->fieldsResults[i].name[0] != 0){
	    		if(strcmp(ticketResults->fieldsResults[i].name, "DATEN") == 0) {
	    			 if(line >=0 && iA >= 0) {
	 	    			lpFieldsResults = &ticketResults->fieldsResults[i];
	 	    			 resultsStr = lpFieldsResults->result;
	 	    			 line = adjustLine(resultsStr, line);
	    				 getLine(resultsStr, line, &iStart, &iStop);
	    				 resultTmp[7] = 0;
	    				 cand2Tmp[7] = 0;
//	    		printf("-------resultTmp = %s \n", resultTmp);
	    				 copyLine(&ticketResults->fieldsResults[i], iStart, iStop);
	    				 len = strlen(result);
	    				 if(result[len-1] < 32) result[len-1] = 0;
	    				 len = strlen(result);
	    				 sh = 3;
	    				 if(result[len-2] == 32) sh = 5;
	    				 idxY = matchToStringEndOneLineIdx(result, "201", sh);
	    				 flgCut = 0;
	    				 if(sh == 5 && len - idxY > 4)
	    					 flgCut = 1;
//printLine(resultsStr, iStart, iStop);
//printf("-------results = %s \n", result);
//printf("-------idxY = %d, len - idxY = %d \n", idxY, len - idxY);
                         idxY -= 3;
	    				 if(idxY <= 0 || idxY >= (iStop - iStart)) idxY = 7;
	    				 k = iStart+idxY;
// printf("-------resultsStr[k] = %s \n", &resultsStr[k]);
	    				 if(k<iStop) {
	    			        copyLine(&ticketResults->fieldsResults[i], k, iStop);
		    				 len = strlen(result);
		    				 if(result[len-1] < 32) result[len-1] = 0;
		    				 len = strlen(result);
		    				 if(flgCut == 1 && result[len-2] == 32)
		    					 result[len-2] = 0;

//	    		printf("-------result = %s \n", result);
	    			        addLineToTmp();
//          		printf("-------resultTmp = %s \n", resultTmp);
	    				 }
	    				 copyResultsBack(&ticketResults->fieldsResults[iA]);
	    			 } else{
	    				 line = postProcessingDateD(&ticketResults->fieldsResults[i]);
		    			 if(line >=0 && iA >= 0) {
			 	    			lpFieldsResults = &ticketResults->fieldsResults[iA];
			 	    			resultsStr = lpFieldsResults->result;
			    				line = adjustLineA(resultsStr, line);
			    				getLine(resultsStr, line, &iStart, &iStop);
				    			idxY = matchToStringEndOneLineIdx(resultTmp, "201", 3);
				    			idxY -= 3;
				    			if(idxY <= 0 || idxY >= strlen(resultTmp)) idxY = 7;
//         		printf("-------idxY = %d \n", idxY);
			    				replaceFirstCharsTmp(lpFieldsResults, iStart, iStop, idxY);
			    				copyResultsBack(&ticketResults->fieldsResults[iA]);
		    			 }else
		    				 if(iA >= 0)
	    				       ticketResults->fieldsResults[iA].result[0] = 0;
	    			 }
	    			break;
	    		}
	    	}
	    }
        cleanYear(ticketResults);
        cleanDateField(ticketResults);
	 return;
}

void printTicketTypeToLog(int type, int cnf, char * buff){

	switch(type){
	   case 0: sprintf(buff,"ticket type: Mega Million; confidence= %d\n",cnf); break;
	   case 1: sprintf(buff,"ticket type: Power Ball; confidence= %d\n",cnf); break;
	   default: sprintf(buff,"ticket type: ???\n"); break;
	}
	 saveLog("", buff);
}
void printFieldToLog(LPFIELDSRESULTS lpFieldsResults, char *buff){

	  saveLog("-------------------------------------------", "\n");
	  sprintf(buff,"Field= %s\n",lpFieldsResults->name);
      saveLog("", buff);
      saveLog("\n", "\n");
      saveLog(lpFieldsResults->result, "\n");
      saveLog("", "\n");
	  saveLog("-------------------------------------------", "\n");
}

void printResultsToLog(LPTICKETRESULTS ticketResults){
	int i;
	char buff[500];

	 sprintf(buff,"Status= %d\n",ticketResults->status);
	 saveLog("", buff);
     printTicketTypeToLog(ticketResults->type, ticketResults->typeCnf, buff);
    for(i=0;i<NUMFIELDS_RESULTS;i++){
    	if(ticketResults->fieldsResults[i].name[0] != 0){
    		if(strcmp(ticketResults->fieldsResults[i].name, "NUMBERS") == 0) printFieldToLog(&ticketResults->fieldsResults[i], buff);
    		if(strcmp(ticketResults->fieldsResults[i].name, "PLAYER") == 0) printFieldToLog(&ticketResults->fieldsResults[i], buff);
    		if(strcmp(ticketResults->fieldsResults[i].name, "DATE") == 0) printFieldToLog(&ticketResults->fieldsResults[i], buff);
//    		if(strcmp(ticketResults->fieldsResults[i].name, "DATEN") == 0) printFieldToLog(&ticketResults->fieldsResults[i], buff);
//    		if(strcmp(ticketResults->fieldsResults[i].name, "DATEA") == 0) printFieldToLog(&ticketResults->fieldsResults[i], buff);
    	}
    }
}

void ticketPostProcessing(LPTICKETDATA ticketDataPtr, LPTICKETRESULTS ticketResults){
	int i;
//  printf ("Results before post-processing----------------------------------------\n");
//	printResults_1(ticketResults);
// printf ("-----------------------------------------------------------------------\n");
    for(i=0;i<NUMFIELDS_RESULTS;i++){
    	if(ticketResults->fieldsResults[i].name[0] != 0){
    		if(strcmp(ticketResults->fieldsResults[i].name, "LOGO") == 0) postProcessingLogo(ticketResults);
    		if(strcmp(ticketResults->fieldsResults[i].name, "NUMBERS") == 0) ticketResults->status = postProcessingNumbers(&ticketResults->fieldsResults[i]);
//    		if(strcmp(ticketResults->fieldsResults[i].name, "DATE") == 0) postProcessingDate(&ticketResults->fieldsResults[i]);
    		if(strcmp(ticketResults->fieldsResults[i].name, "DATEA") == 0) postProcessingDateNew(ticketResults);
    	}
    }
//    printf ("Results after post-processing----------------------------------------\n");
//	printResults(ticketResults);
//	 printf ("--------------------------------------------------------------------\n");

//	 printResultsToLog(ticketResults);
}

	void PrintOutputResults(LPTICKETRESULTS ticketResults){
		int i;
  printf ("Results after post-processing----------------------------------------\n");
	   printf("Status= %d\n", ticketResults->status);
	   printTicketType(ticketResults->type, ticketResults->typeCnf);
	    for(i=0;i<NUMFIELDS_RESULTS;i++){
	    	if(ticketResults->fieldsResults[i].name[0] != 0){
	    		if(strcmp(ticketResults->fieldsResults[i].name, "NUMBERS") == 0) printField(&ticketResults->fieldsResults[i]);
	    		if(strcmp(ticketResults->fieldsResults[i].name, "PLAYER") == 0) printField(&ticketResults->fieldsResults[i]);
	    		if(strcmp(ticketResults->fieldsResults[i].name, "DATE") == 0) printField(&ticketResults->fieldsResults[i]);
	    }
	}
    printf ("--------------------------------------------------------------------\n");
}

