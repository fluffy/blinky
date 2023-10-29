// Copyright (c) 2023 Cullen Jennings

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#include "detect.h"

#define maxCycleLen 20
static float dataMin;
static float dataMax;
static float dataSum[maxCycleLen];
static float dataAvg100;
static float maxSum;
static float maxSumTime;

void detectInit( int cycleLen ){
  assert( cycleLen <= maxCycleLen );
  
  for ( int i=0; i<maxCycleLen; i++ ) {
     dataSum[i]=0.0;
  }
  dataMin=1e6;
  dataMax=-1e6;
  dataAvg100 = 0.0;
}

void detectUpdate( uint32_t* pData, int len , bool invert ){
   assert( len <= maxCycleLen );

   for ( int i=0; i<len; i++ ) {
     uint32_t vInt = pData[i];
     float f = vInt;
     if ( f < dataMin ) { dataMin = f ; }
     if ( f > dataMax ) { dataMax = f ; }

     dataAvg100 = dataAvg100 * 0.99 + 0.01 * f;
     float v = f - dataAvg100;

     if ( invert ) {
       dataSum[i] = dataSum[i] *0.95 - v;
     } else {
       dataSum[i] = dataSum[i] *0.95 + v;
     }
   }
}

void detectUpdateMlp( uint32_t time ){
  for ( int i=0; i< maxCycleLen; i++ ) {
    float sum = (dataSum[i] >= 0.0 ) ?   (dataSum[i]) : ( - dataSum[i]) ;
    if ( sum > maxSum ) {
      maxSum = sum;
      maxSumTime = time; 
    }
  }
}

void detectGetMlpTime( uint32_t* timeP, float* valP ) {
  *timeP = maxSumTime;
  *valP = maxSum;
}

void detectResetMlp(){
  maxSum=0.0;
  maxSumTime=0;
}
