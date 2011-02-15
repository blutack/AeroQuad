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

#ifndef _AQ_FLIGHT_ANGLE_PROCESSOR_KALMAN_FILTER_H_
#define _AQ_FLIGHT_ANGLE_PROCESSOR_KALMAN_FILTER_H_

#include "FlightAngleProcessor.h"

class FlightAngleKalmanFilter : public FlightAngleProcessor 
{
private:
    float x_angle[2]; 
    float x_bias[2];
    float P_00[2];
    float P_01[2];
    float P_10[2];
    float P_11[2];	
    float Q_angle;
    float Q_gyro;
    float R_angle;
    float y; 
    float S;
    float K_0;
    float K_1;

    float _calculate(byte axis, float newAngle, float newRate, float G_Dt);

public:
  FlightAngleKalmanFilter();
  
  void initialize(void);
  
  void calculate(float G_Dt);
  
  float getGyroUnbias(byte axis);

  void calibrate(void) {}
};

#endif