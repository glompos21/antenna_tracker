//mot.h - Motor library.
//Copyright (c) 2015-2018 Julie VK3FOWL and Joe VK3YSP
//For more information please visit http://www.sarcnet.org
//Released under the GNU General Public License.
//Provides DC motor speed and direction control

#ifndef MOT_H
#define MOT_H

#include "fil.h"
#include <Arduino.h>


class Mot {
  public:
    Mot( float alpha, int gain, int pin1, int pin2);
    void halt();
    void drive(float err);
  private:
    int _type, _gain, _pin1, _pin2;
    unsigned long lastTime;
    Fil fil;
};
#endif
