// Compile with: gcc -o ltc ltc.cc -lstdc++
//  Descrption of LTC can be found at https://en.wikipedia.org/wiki/Linear_timecode
// Official specification in SMPTE 12M TODO


#include "ltc.h"


void LtcTransitionSetClear( LtcTransitionSet* set ) {
  set->numTransitions = 0;
  set->nextTransition = 0;
}

void LtcTransitionSetAdd(LtcTransitionSet* set,uint32_t timeUs){
  set->transitionTimeUs[ (set->numTransitions) % ltcMaxTransitions ] = timeUs;
  set->numTransitions++;
}

uint16_t LtcTransitionSetSize(LtcTransitionSet* set){
  return set->numTransitions;
}

uint32_t  LtcTransitionSetDeltaUs(LtcTransitionSet* set, uint16_t i){
    if ((i < 1) || (i >= set->numTransitions)) return 0;
    uint32_t curr = i % ltcMaxTransitions;
    uint32_t prev = (i - 1) % ltcMaxTransitions;
    return set->transitionTimeUs[curr] - set->transitionTimeUs[prev];
}



void LtcTimeCodeClear(LtcTimeCode* set){
  set->valid = 0; 
}

void LtcTimeCodeSet(LtcTimeCode* set, uint32_t s, uint32_t us){
  set->frame = (us * 30l / 1000000l) % 30;
  set->sec = s % 60;
  set->min = (s / 60) % 60;
  set->hour = (s / 3600) % 24;
  set->valid = 1;
}

void LtcTimeCodeSetHMSF(LtcTimeCode* set, uint8_t h, uint8_t m, uint8_t s, uint8_t f){
  set->frame =f % 30;
  set->sec = s % 60;
  set->min = m % 60;
  set->hour = h % 24;
  set->valid = 1;
}

uint32_t  LtcTimeCodeSeconds(LtcTimeCode* set){
   if (!set->valid) return 0;
   return (uint32_t)(set->sec) + (uint32_t)(set->min) * 60l + (uint32_t)(set->hour) * 3600l;
}

uint32_t  LtcTimeCodeMicroSeconds(LtcTimeCode* set){
    if (!set->valid) return 0;
    return (uint32_t)(set->frame) * 1000000l / 30l;
}

uint32_t  LtcTimeCodeDisp(LtcTimeCode* set){
   if (! set->valid) {
      return 0;
    };
    return set->frame + set->sec * 100l + set->min * 10000l + set->hour * 1000000l;
}

int LtcTimeCodeIsValid(LtcTimeCode* set){
  return set->valid;
}

  









#if 0

uint8_t LTC::parity() {
  uint8_t ret = 0;

  for (int i = 0; i <= 10; i++) {
    uint8_t data = bits[i];
    for (int bit = 0; bit <= 8; bit++) {
      if (data & 0x1) {
        ret++;
      }
      data = data >> 1;
    }
  }

  return ret % 2;
}

void LTC::encode(TransitionSet& tSet, uint8_t fps) {
  tSet.clear();

  uint32_t time = 0;
  // baud rate is 30 fps, 80 bits per frame , two transitions per bit for 1's =
  // 2400 Hz for 1
  uint32_t baud = fps * 80 /* bits per frame */;
  uint32_t zeroInc = 1000000 / baud;  // one time in micro seconds
  uint32_t oneInc = zeroInc / 2;

  tSet.add(time);

  for (int i = 0; i < 10; i++) {
    uint16_t data = bits[i];
    for (uint16_t bit = 0; bit < 8; bit++) {
      // std::cout << " data=" << data << " dataShift=" << (data>>bit) << "
      // bit=" << bit << " "  <<std::endl;
      if ((data >> bit) & 0x1) {
        // encode a one
        time += oneInc;
        tSet.add(time);
        time += oneInc;
        tSet.add(time);

        // std::cout << "1";
      } else {
        // encode a zero
        time += zeroInc;
        tSet.add(time);

        // std::cout << "0";
      }
    }
    // std::cout << "-";
  }

  // std::cout << " totalTime=" << time / 1000 << "ms" << std::endl;
}

void LTC::decode(const TransitionSet& tSet, uint8_t fps) {
  uint32_t baud = fps * 80 /* bits per frame */;
  uint32_t zeroInc = 1000000 / baud;  // zero time in micro seconds
  uint32_t oneInc = zeroInc / 2;

  // clear out the data before debcoding
  valid = 0;
  for (int i = 0; i < 10; i++) {
    bits[i] = 0;
  }

  // decode in reverse direction
  uint8_t bitCount = 7;
  uint8_t byteCount = 9;
  uint16_t setIndex = tSet.size() - 1;

  uint8_t done = 0;
  while (!done) {
    if (setIndex == 0) {
      // std::cout << "mnot enough transitions" << std::endl;
      return;
    }

    uint32_t delta = tSet.delta(setIndex);

    if ((delta > zeroInc - 50) && (delta < zeroInc + 50)) {
      // found a zero
      // std::cout << "0";
      setIndex--;
    } else if ((delta > oneInc - 50) && (delta < oneInc + 50)) {
      // found a start of 1
      if (setIndex < 1) {
        // std::cout << "mnot enough transitions 2nd half of one" << std::endl;
        return;
      }
      uint32_t delta2 = tSet.delta(setIndex - 1);
      if ((delta > oneInc - 50) && (delta < oneInc + 50)) {
        // found a 1
        // std::cout << "1";

        bits[byteCount] |= (1 << bitCount);
      } else {
        // std::cout << "  2nd half 1 missing" << std::endl;
        return;
      }
      setIndex -= 2;
    } else {
      // std::cout << "  bad delta=" << delta << " at setIndex=" <<
      // (int)setIndex << std::endl;
      return;
    }

    if (bitCount == 0) {
      bitCount = 7;
      if (byteCount == 0) {
        done = 1;
      } else {
        byteCount--;
        // std::cout << "-";
      }
    } else {
      bitCount--;
    }

    if ((byteCount == 7) && (bitCount == 7)) {
      if (bits[8] != 0xFC) {
        // std::cout << " bad sync1" << std::endl;
        return;
      }
    }

    if ((byteCount == 8) && (bitCount == 7)) {
      if (bits[9] != 0xBF) {
        // std::cout << " bad sync2" << std::endl;
        return;
      }
    }
  }

  if (parity() != 0) {
    // std::cout << " bad parity" << std::endl;
    return;
  }
  valid = 1;
}

void LTC::set(const TimeCode& time) {
  bits[0] = time.frame % 10;
  bits[1] = time.frame / 10;
  bits[2] = time.sec % 10;
  bits[3] = time.sec / 10;
  bits[4] = time.min % 10;
  bits[5] = time.min / 10;
  bits[6] = time.hour % 10;
  bits[7] = time.hour / 10;
  bits[8] = 0xFC;  // sync1
  bits[9] = 0xBF;  // sync2

  if (parity() != 0) {
    // set polarity correction bit
    bits[3] |= 0x08;
  }
}

void LTC::get(TimeCode& time) {
  time.valid = 0;

  if (!valid) {
    return;
  }
  if (bits[8] != 0xFC) {
    return;
  }
  if (bits[9] != 0xBF) {
    return;
  }

  time.frame = (bits[1] & 0x03) * 10 + (bits[0] & 0x0F);
  time.sec = (bits[3] & 0x07) * 10 + (bits[2] & 0x0F);
  time.min = (bits[5] & 0x07) * 10 + (bits[4] & 0x0F);
  time.hour = (bits[7] & 0x03) * 10 + (bits[6] & 0x0F);

  time.frame %= 30;
  time.sec %= 60;
  time.min %= 60;
  time.hour %= 24;

  time.valid = 1;
}

#endif 


#if 0
#include <iostream>

int main(int argc, char* argv[]) {
  std::cout << "LTC Test" << std::endl;

  TimeCode t1(11 * 60 + 22, 500000l);
  std::cout << "t1=" << t1.disp() << " " << t1.microSeconds() << std::endl;
  assert(t1.seconds() == 682);
  assert(t1.microSeconds() == 500000l);

  LTC ltc1(t1);

  TransitionSet tSet;
  ltc1.encode(tSet);
  ltc1.encode(tSet);
  ltc1.encode(tSet);
  std::cout << "done encode" << std::endl;

  if (0 ) {
 std::cout << "tSet size=" << (int)tSet.size() << std::endl;
 for( int i=1; i < tSet.size(); i++ ){
   std::cout << "delta[" << i << "]=" << tSet.delta(i) << std::endl;
  }
  }

  std::cout << "start decode" << std::endl;

  LTC ltc2;
  ltc2.decode(tSet);

  std::cout << "done decode" << std::endl;

  TimeCode t2;
  ltc2.get(t2);

  std::cout << "t2=" << t2.disp() << std::endl;

  return 0;
}

#endif
