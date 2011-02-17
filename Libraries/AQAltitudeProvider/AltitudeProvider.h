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

#ifndef _AQ_ALTITUDE_PROVIDER_H_
#define _AQ_ALTITUDE_PROVIDER_H_
#include "WProgram.h"

class AltitudeProvider 
{
protected:
  float _groundPressure; // remove later
  float _groundTemperature; // remove later
  float _groundAltitude;  
  float _smoothFactor;
  double _rawAltitude;
  double _altitude; 
  
public:
  
  AltitudeProvider ();

  // **********************************************************************
  // The following function calls must be defined inside any new subclasses
  // **********************************************************************
  virtual void initialize(); 
  virtual void measure();
  
  // *********************************************************
  // The following functions are common between all subclasses
  // *********************************************************
  const float getData();
  const float getRawData();
  void setStartAltitude(float value);
  void measureGround();
  void setGroundAltitude(float value);
  const float getGroundAltitude();
  void setSmoothFactor(float value);
  const float getSmoothFactor();
};

#endif