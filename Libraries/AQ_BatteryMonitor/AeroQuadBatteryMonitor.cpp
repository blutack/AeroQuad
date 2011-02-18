/*
  AeroQuad v2.2 - Feburary 2011
  www.AeroQuad.com
  Copyright (c) 2011 Ted Carancho.  All rights reserved.
  An Open Source Arduino based multicopter.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "AeroQuadBatteryMonitor.h"

#if defined (__AVR_ATmega328P__)
  #define BUZZERPIN 12
  #define LED2PIN 12
  #define LED3PIN 12
#else
  #define BUZZERPIN 49
  #define LED2PIN 4
  #define LED3PIN 31
#endif

#define LEDPIN 13


AeroQuadBatteryMonitor::AeroQuadBatteryMonitor()
{
}

void AeroQuadBatteryMonitor::initialize() 
{
  float R1   = 15000;
  float R2   =  7500;
  float Aref =     5.0;
  _batteryScaleFactor = ((Aref / 1024.0) * ((R1 + R2) / R2));    
  _diode = 0.9; // measured with DMM
  analogReference(DEFAULT);
  pinMode(BUZZERPIN, OUTPUT); // connect a 12V buzzer to pin 49
  digitalWrite(BUZZERPIN, LOW);
  _previousTime = millis();
  _state = LOW;
  _firstAlarm = OFF;
  _currentTime = 0;
  
}

void AeroQuadBatteryMonitor::lowBatteryEvent(byte level,int throttle) 
{
  long currentTime = millis()- _previousTime;
  if (level == OK) 
  {
    digitalWrite(BUZZERPIN, LOW);
    _autoDescent = 0;
  }
  if (level == WARNING) 
  {
    if ((_autoDescent == 0) && (_currentTime > 1000)) 
    {
      _autoDescent = -50;
    }
    if (_currentTime > 1100) 
    {
      _autoDescent = 50;
      digitalWrite(LED2PIN, HIGH);
      digitalWrite(BUZZERPIN, HIGH);
    }
    if (_currentTime > 1200) 
    {
      _previousTime = millis();
      _autoDescent = 0;
      digitalWrite(LED2PIN, LOW);
      digitalWrite(BUZZERPIN, LOW);
    }
  }
  if (level == ALARM) 
  {
    if (_firstAlarm == OFF) 
    {
      _autoDescent = 0; // intialize autoDescent to zero if first time in ALARM state
    }
    _firstAlarm = ON;
    digitalWrite(BUZZERPIN, HIGH); // enable buzzer
    if ((_currentTime > 500) && (throttle > 1400)) 
    {
      _autoDescent -= 1; // auto descend quad
      _previousTime = millis();
      if (_state == LOW) 
      {
        _state = HIGH;
      }
      else
      { 
        _state = LOW;
      }
      digitalWrite(LEDPIN, _state);
      digitalWrite(LED2PIN, _state);
      digitalWrite(LED3PIN, _state);
    }
  }
}

const float AeroQuadBatteryMonitor::readBatteryVoltage(byte channel) 
{
  return (analogRead(channel) * _batteryScaleFactor) + _diode;
}