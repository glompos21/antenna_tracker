// mot.cpp - Motor library.
// Copyright (c) 2015-2018 Julie VK3FOWL and Joe VK3YSP
// For more information please visit http://www.sarcnet.org
// Released under the GNU General Public License.
// Provides DC motor speed and direction control

#include "mot.h"

// Public methods

Mot::Mot(float alpha, int gain, int pin1, int pin2)
  : fil(alpha) {
  // Constructor
  _gain = gain;
  _pin1 = pin1;
  _pin2 = pin2;
  pinMode(_pin1, OUTPUT);
  pinMode(_pin2, OUTPUT);
  analogWrite(_pin1, 0);
  analogWrite(_pin2, 0);
}

void Mot::halt() {
  drive(0.0);
}

// void Mot::drive(float err) {
//   if (abs(err) < 0.5) {
//     digitalWrite(_pin1, LOW);
//     digitalWrite(_pin2, LOW);
//   } else {
//     if (err > 0) {
//       digitalWrite(_pin1, HIGH);
//       digitalWrite(_pin2, LOW);
//     } else {
//       digitalWrite(_pin1, LOW);
//       digitalWrite(_pin2, HIGH);
//     }
//   }
// }

void Mot::drive(float err) {
  int speed = 255;
  int no_movement_under = 5.;
if (err < 10.0) {
    speed = 120;
  } else if (err < 20.0) {
    speed = 180;
  } else {
    speed = 255;
  }

  if (abs(err) < no_movement_under) {
    analogWrite(_pin1, 0);
    analogWrite(_pin2, 0);
  } else {
    if (err > 0) {
      analogWrite(_pin1, speed);
      analogWrite(_pin2, 0);
    } else {
      analogWrite(_pin1, 0);
      analogWrite(_pin2, speed);
    }
  }
}
