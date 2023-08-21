// COmpile with: gcc -o ltc ltc.cc -lstdc++

#include <iostream>

class TimeCode {
 public:
  TimeCode() { valid = 0; };
  TimeCode(uint32_t s, uint32_t us) {
    frame = (us * 30l / 1000000l) % 30;
    sec = s % 60;
    min = (s / 60) % 60;
    hour = (s / 3600) % 24;
    valid = 1;
  };

  TimeCode(uint8_t h, uint8_t m, uint8_t s, uint8_t f)
      : hour(h), min(m), sec(s), frame(f){};
  uint32_t disp() {
    if (!valid) {
      return 0;
    };
    return frame + sec * 100l + min * 10000l + hour * 1000000l;
  }

 public:
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t frame;
  uint8_t valid;
};

class TransitionSet {
 public:
  TransitionSet() : size(0){};

 public:
  static const int maxTransitions = 161;
  uint32_t t[maxTransitions];
  uint8_t size;
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

  void encode(TransitionSet& tSet);
  void decode(const TransitionSet& tSet);

 public:
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

void LTC::encode(TransitionSet& tSet) {
  // TransitionSet::TransitionSet( const LTC& ltc ){
  tSet.size = 0;
  uint32_t time = 0;
  // baud rate is 30 fps, 80 bits per frame , two transitions per bit for 1's =
  // 2400 Hz for 1
  uint32_t oneInc =
      1000000 /
      2400;  // compute from baud and clk rate. Rate for a 1 (2 tranition)
  uint32_t zeroInc =
      oneInc * 2;  // compute from baud and clk rate. Rate for a 1 (2 tranition)

  tSet.t[tSet.size++] = time;

  for (int i = 0; i < 10; i++) {
    uint8_t data = bits[i];
    for (int bit = 0; bit <= 8; bit++) {
      if ( (data>>bit) & 0x1) {
        // encode a one
        time += oneInc;
        tSet.t[tSet.size++] = time;
        time += oneInc;
        tSet.t[tSet.size++] = time;
	
	 std::cout << "1";
      } else {
        // encode a zero
        time += zeroInc;
        tSet.t[tSet.size++] = time;

	 std::cout << "0";
      }
    }
    std::cout << "-";
  }
}

void LTC::decode(const TransitionSet& tSet) {
  valid = 0;

  uint8_t bitCount = 0;
  uint8_t byteCount = 0;

  for (int i = 0; i < 10; i++) {
    bits[i] = 0;
  }

  for (int i = 1; i < tSet.size; i++) {
      if (byteCount >= 10) {
        std::cout << " too many bytes i=" << i << " of " << (int)tSet.size
                  << std::endl;
        return;
      }

    uint32_t delta = tSet.t[i] - tSet.t[i - 1];

    if ((delta > 700) && (delta < 900)) {
      // found a zero
      std::cout << "0";

      // leave (1 << bitCount) in bits[byteCount] as zero 
      bitCount++;
      if (bitCount > 8) {
        bitCount = 0;
        byteCount++;
	 std::cout << "-";
      }
    

    } else if ((delta > 300) && (delta < 500)) {
      // start of 1
      if (i + 1 >= tSet.size) {
        std::cout << " missing last one i=" << i << " of " << (int)tSet.size
                  << std::endl;

        return;
      }
      uint32_t delta2 = tSet.t[i + 1] - tSet.t[i - 2];
      if ((delta > 300) && (delta < 500)) {
        // found a 1
        std::cout << "1";

        bits[byteCount] |= (1 << bitCount);
        bitCount++;
        if (bitCount > 8) {
          bitCount = 0;
          byteCount++;
	   std::cout << "-";
        }

        i++;  // skip 2nd transtion on  main loop
      } else {
        std::cout << " bad transition time i=" << i << " of " << (int)tSet.size
                  << std::endl;
        return;
      }
    } else {
      // error
      std::cout << " bad delta=" << delta << std::endl;
      return;
    }
  }

  if (parity() != 0) {
    std::cout << " bad parity" << std::endl;
    return;
  }
  if (bits[8] != 0x3F) {
    std::cout << " bad sync1" << std::endl;
    return;
  }
  if (bits[9] != 0xFE) {
    std::cout << " bad sync2" << std::endl;
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
  bits[8] = 0xFC; // 0x3F;  // sync1
  bits[9] = 0xBF; // 0xFE;  // sync2

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
  if (bits[8] != 0x3F) {
    return;
  }
  if (bits[9] != 0xF3) {
    return;
  }

  time.frame = (bits[1] & 0x03) * 10 + (bits[0] & 0x0F);
  time.sec = (bits[3] & 0x07) * 10 + (bits[2] & 0x0F);
  time.min = (bits[5] & 0x07) * 10 + (bits[4] & 0x0F);
  time.hour = (bits[7] & 0x03) * 10 + (bits[6] & 0x0F);

  time.valid = 1;
}

int main(int argc, char* argv[]) {
  std::cout << "LTC Test" << std::endl;

  TimeCode t1(7,0); // 11 * 60 + 22, 500000l);
  std::cout << "t1=" << t1.disp() << std::endl;

  LTC ltc1(t1);

  TransitionSet tSet;
  ltc1.encode(tSet);
 std::cout << std::endl << "done encode" << std::endl;

#if 0 
  std::cout << "tSet size=" << (int)tSet.size << std::endl;
  for( int i=1; i < tSet.size; i++ ){
    std::cout << "t[" << i << "]=" << tSet.t[i] << "   " << tSet.t[i]- tSet.t[i-1] << std::endl;
  }
#endif

  LTC ltc2;
  ltc2.decode(tSet);

  std::cout << std::endl << "done decode" << std::endl;

  TimeCode t2;
  ltc2.get(t2);

  std::cout << "t2=" << t2.disp() << std::endl;

  return 0;
}
