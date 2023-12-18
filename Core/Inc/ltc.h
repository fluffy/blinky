
#include <stdint.h>

#if 0

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
      : hour(h), min(m), sec(s), frame(f), valid(true) {
    h = h % 24;
    m = m % 60;
    s = s % 60;
  };

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
#endif


enum {  ltcMaxTransitions = 80 * 2 + 1 }; // use enum to get const integer for array size 

typedef struct {
  uint32_t transitionTimeUs[ltcMaxTransitions];
  uint16_t numTransitions;
  uint16_t nextTransition;
} LtcTransitionSet;

void LtcTransitionSetClear( LtcTransitionSet* set );
void LtcTransitionSetAdd(LtcTransitionSet* set,uint32_t timeUs);
uint16_t LtcTransitionSetSize(LtcTransitionSet* set);
uint32_t  LtcTransitionSetDeltaUs(LtcTransitionSet* set, uint16_t i);

#if 0
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

#endif
