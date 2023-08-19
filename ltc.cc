// COmpile with: gcc -o ltc ltc.cc -lstdc++

#include <iostream>

class TimeCode {
 public:
  TimeCode(uint32_t s, uint32_t us ) { frame=(us*30l/1000000l)%30; sec=s%60; min=(s/60)%60; hour=(s/3600)%24;  };
  
  TimeCode(uint8_t h, uint8_t m, uint8_t s, uint8_t f)
      : hour(h), min(m), sec(s), frame(f){};
  uint32_t disp() {
    return frame + sec * 100l + min * 10000l + hour * 1000000l;
  }

 public:
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t frame;
};

class LTC {
 public:
  LTC( const TimeCode& time) { set(time); };
  
  void set(const TimeCode& time);

 public:
  uint8_t bits[10];
  uint8_t parity();
};

uint8_t LTC::parity(){
  uint8_t ret=0;

  for ( int i=0; i<=10 ; i++ ) {
    uint8_t data = bits[i];
    for ( int bit=0; bit<=8 ; bit++ ) {
      if ( data & 0x1 ) {
	ret++;
      }
      data = data >> 1;
    }
  }

  return ret%2;
}

class TransitionSet {
public:
  TransitionSet( const LTC& ltc );
public:
  static const int maxTransitions=161;
  uint16_t t[maxTransitions];
  uint8_t size; 
};

TransitionSet::TransitionSet( const LTC& ltc ){
  size=0;
  uint32_t time=0;
  // baud rate is 30 fps, 80 bits per frame , two transitions per bit for 1's = 2400 Hz for 1 
  uint32_t oneInc=1000000/2400; // compute from baud and clk rate. Rate for a 1 (2 tranition)
  uint32_t zeroInc=oneInc*2; // compute from baud and clk rate. Rate for a 1 (2 tranition)

  t[size++] = time;
  
   for ( int i=0; i<=10 ; i++ ) {
    uint8_t data = ltc.bits[i];
    for ( int bit=0; bit<=8 ; bit++ ) {
      if ( data & 0x1 ) {
	// encode a one
	time += oneInc;
	t[size++] = time;
	time += oneInc;
	t[size++] = time;
      } else {
	// encode a zero
	time += zeroInc;
	t[size++] = time;
      }      
      data = data >> 1;
    }
  }
  
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
  bits[8] = 0x3F;  // sync1
  bits[9] = 0xFE;  // sync2

  if ( parity() != 0 ) {
    // set polarity correction bit
    bits[3] |= 0x08; 
  }
}

int main(int argc, char* argv[]) {
  std::cout << "starting" << std::endl;

  TimeCode t1(11*60+22,500000l);
  std::cout << "t1=" << t1.disp() << std::endl;

  LTC ltc1( t1 );

  TransitionSet tSet( ltc1 );

  std::cout << "tSet size=" << (int)tSet.size << std::endl;
  for( int i=1; i < tSet.size; i++ ){
    std::cout << "t[" << i << "]=" << tSet.t[i] << "   " << tSet.t[i]- tSet.t[i-1] << std::endl;
  }
  
  return 0;
}
