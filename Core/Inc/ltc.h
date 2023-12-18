
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

#if 0
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
#endif

enum {  ltcMaxTransitions = 80 * 2 + 1 }; // use enum to get const integer for array size 

typedef struct {
  uint32_t transitionTimeUs[ltcMaxTransitions];
  uint16_t numTransitions;
  uint16_t nextTransition;
} LtcTransitionSet;


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
