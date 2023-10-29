// Copyright (c) 2023 Cullen Jennings

#ifndef __DETECT_H
#define __DETECT_H

#ifdef __cplusplus
extern "C" {
#endif

  void detectInit( int cycleLen );
  void detectUpdatePos( uint32_t* pData, int len );
  void detectUpdateNeg( uint32_t* pData, int len );
  
#ifdef __cplusplus
}
#endif

#endif /* __DETECT_H */
