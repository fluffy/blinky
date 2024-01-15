
#include "setting.h"


void settingInit(){
  setting.blinkMute = 1;       // mutes audio outout
  setting.blinkBlank = 1;      // causes LED to be off
  setting.blinkDispAudio = 0;  // caused audio latency to be displayed on LED
  setting.blinkHaveDisplay = 1;
   setting.blinkPPS =0;
}

void settingSetup(){
  // nothing to do here
}
