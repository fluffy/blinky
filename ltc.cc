// COmpile with: gcc -o ltc ltc.cc -lstdc++

#include <iostream>

class LTC;

class TimeCode {
  friend class LTC;

 public:
  TimeCode() { valid = false; };
  TimeCode(uint32_t s, uint32_t us) {
    frame = (us * 30l / 1000000l) % 30;
    sec = s % 60;
    min = (s / 60) % 60;
    hour = (s / 3600) % 24;
    valid = true;
  };

  TimeCode(uint8_t h, uint8_t m, uint8_t s, uint8_t f)
    : hour(h), min(m), sec(s), frame(f), valid(true) { h=h%24; m=m%60; s=s%60; };

  uint32_t seconds() const {
    if (!valid) return 0;
    return (uint32_t)sec + (uint32_t)min * 60l + (uint32_t)hour * 3600l;
  };
  uint32_t microSeconds() const {
    if (!valid) return 0;
    return (uint32_t)frame * 1000000l / 30l;
  };

  uint32_t disp() const {
    if (!valid) {
      return 0;
    };
    return frame + sec * 100l + min * 10000l + hour * 1000000l;
  }

  bool isValid() const { return valid; }
  
 private:
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t frame;
  uint8_t valid;
};

class TransitionSet {
  // circular buffer that holds 80 bits worth of transitions
 public:
  TransitionSet() : numTransitions(0){};
  void add(uint32_t t) {
    time[numTransitions % maxTransitions] = t;
    numTransitions++;
  };
  uint32_t delta(uint16_t i) const {
    if ((i < 1) || (i >= numTransitions)) return 0;
    uint32_t curr = i % maxTransitions;
    uint32_t prev = (i - 1) % maxTransitions;
    return time[curr] - time[prev];
  };
  uint16_t size() const { return numTransitions; };
  void clear() { numTransitions = 0; };

 private:
  static const uint16_t maxTransitions = 80 * 2 + 1;
  uint32_t time[maxTransitions];
  uint16_t numTransitions;
};

class LTC {
 public:
  LTC() { valid = 0; };
  LTC(const TimeCode& time) {
    set(time);
    valid = 1;
  };

  void set(const TimeCode& time);
  void get(TimeCode& time);

  void encode(TransitionSet& tSet, uint8_t fps = 30);
  void decode(const TransitionSet& tSet, uint8_t fps = 30);

 private:
  uint8_t bits[10];
  uint8_t parity();
  uint8_t valid;
};

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

        //std::cout << "1";
      } else {
        // encode a zero
        time += zeroInc;
        tSet.add(time);

	// std::cout << "0";
      }
    }
    //std::cout << "-";
  }

  //std::cout << " totalTime=" << time / 1000 << "ms" << std::endl;
}

void LTC::decode(const TransitionSet& tSet, uint8_t fps ) {
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
      //std::cout << "mnot enough transitions" << std::endl;
      return;
    }

    uint32_t delta = tSet.delta(setIndex);

    if ((delta > zeroInc - 50) && (delta < zeroInc + 50)) {
      // found a zero
      //std::cout << "0";
      setIndex--;
    } else if ((delta > oneInc - 50) && (delta < oneInc + 50)) {
      // found a start of 1
      if (setIndex < 1) {
	//std::cout << "mnot enough transitions 2nd half of one" << std::endl;
        return;
      }
      uint32_t delta2 = tSet.delta(setIndex - 1);
      if ((delta > oneInc - 50) && (delta < oneInc + 50)) {
        // found a 1
	//std::cout << "1";

        bits[byteCount] |= (1 << bitCount);
      } else {
	//std::cout << "  2nd half 1 missing" << std::endl;
        return;
      }
      setIndex -= 2;
    } else {
      //std::cout << "  bad delta=" << delta << " at setIndex=" << (int)setIndex << std::endl;
      return;
    }

    if (bitCount == 0) {
      bitCount = 7;
      if (byteCount == 0) {
        done = 1;
      } else {
        byteCount--;
        //std::cout << "-";
      }
    } else {
      bitCount--;
    }

    if ((byteCount == 7) && (bitCount == 7)) {
      if (bits[8] != 0xFC) {
        //std::cout << " bad sync1" << std::endl;
        return;
      }
    }

    if ((byteCount == 8) && (bitCount == 7)) {
      if (bits[9] != 0xBF) {
	//std::cout << " bad sync2" << std::endl;
        return;
      }
    }
  }

  if (parity() != 0) {
    //std::cout << " bad parity" << std::endl;
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

#if 0
 std::cout << "tSet size=" << (int)tSet.size() << std::endl;
 for( int i=1; i < tSet.size(); i++ ){
   std::cout << "delta[" << i << "]=" << tSet.delta(i) << std::endl;
  }
#endif

  std::cout << "start decode" << std::endl;

  LTC ltc2;
  ltc2.decode(tSet);

  std::cout << "done decode" << std::endl;

  TimeCode t2;
  ltc2.get(t2);

  std::cout << "t2=" << t2.disp() << std::endl;

  return 0;
}
