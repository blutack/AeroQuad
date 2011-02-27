/*
  AeroQuad v2.3 - February 2011
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

// Classes updated by jihlein

class Gyro {
public:
  float gyroScaleFactor;
  float smoothFactor;
  float gyroVector[3];
  int   gyroZero[3];
  int   gyroRaw[3];
  
  Gyro(void){
    gyroZero[ROLL]  = readFloat(GYRO_ROLL_ZERO_ADR);
    gyroZero[PITCH] = readFloat(GYRO_PITCH_ZERO_ADR);
    gyroZero[YAW]   = readFloat(GYRO_YAW_ZERO_ADR);
    smoothFactor    = readFloat(GYROSMOOTH_ADR);
  }
  
  const float getNonDriftCorrectedRate(byte axis) {
    return gyroVector[axis];
  }
  
  const float getSmoothFactor(void) {
    return smoothFactor;
  }
  
  void setSmoothFactor(float value) {
    smoothFactor = value;
  }
};

////////////////////////////////////////////////////////////////////////////////
// AeroQuad Mega v2.0 Gyro (ITG-3200)
////////////////////////////////////////////////////////////////////////////////

class Gyro_AeroQuadMega_v2 : public Gyro {
private:
  int address;
  
public:
  Gyro_AeroQuadMega_v2() : Gyro() {
    gyroScaleFactor = DEG_2_RAD(1.0 / 14.375);  //  ITG3200 14.375 LSBs per °/sec
    //address = 0xD0;
    address = 0x69;
  }
  
////////////////////////////////////////////////////////////////////////////////
// Initialize AeroQuad v2.0 Gyro
////////////////////////////////////////////////////////////////////////////////

  void initialize(void) {
    updateRegisterI2C(address, 0x3E, 0x80); // send a reset to the device
    updateRegisterI2C(address, 0x16, 0x1D); // 10Hz low pass filter
    updateRegisterI2C(address, 0x3E, 0x01); // use internal oscillator 
  }
  
////////////////////////////////////////////////////////////////////////////////
// Measure AeroQuad v2.0 Gyro
////////////////////////////////////////////////////////////////////////////////

  void measure(void) {
    sendByteI2C(address, 0x1D);
    Wire.requestFrom(address, 6);
    
    // The following 3 lines read the gyro and assign it's data to gyroRaw
    // in the correct order and phase to suit the standard shield installation
    // orientation.  See TBD for details.  If your shield is not installed in this
    // orientation, this is where you make the changes.
    gyroRaw[ROLL]  = ((Wire.receive() << 8) | Wire.receive())  - gyroZero[ROLL];
    gyroRaw[PITCH] = gyroZero[PITCH] - ((Wire.receive() << 8) | Wire.receive());
    gyroRaw[YAW]   = gyroZero[YAW]   - ((Wire.receive() << 8) | Wire.receive());

    for (byte axis = ROLL; axis < LASTAXIS; axis++) {
      gyroVector[axis] = filterSmooth(gyroRaw[axis] * gyroScaleFactor, gyroVector[axis], smoothFactor);
    }
  }
  
////////////////////////////////////////////////////////////////////////////////
// Calibrate AeroQuad v2.0 Gyro
////////////////////////////////////////////////////////////////////////////////

  void calibrate() {
    autoZero();
    writeFloat(gyroZero[ROLL],  GYRO_ROLL_ZERO_ADR);
    writeFloat(gyroZero[PITCH], GYRO_PITCH_ZERO_ADR);
    writeFloat(gyroZero[YAW],   GYRO_YAW_ZERO_ADR);
  }
  
  void autoZero() {
    int findZero[FINDZERO];
    
    for (byte calAxis = ROLL; calAxis < LASTAXIS; calAxis++) {
      for (int i=0; i<FINDZERO; i++) {
        sendByteI2C(address, (calAxis * 2) + 0x1D);
        findZero[i] = readWordI2C(address);
        delay(10);
      }
      gyroZero[calAxis] = findMedian(findZero, FINDZERO);
    }
  }
  
////////////////////////////////////////////////////////////////////////////////
// Zero AeroQuad v2.0 Gyro
////////////////////////////////////////////////////////////////////////////////

  void zero() {
    // Not required for AeroQuad 2.0 Gyro
  }
      
  const int getFlightData(byte axis) {
    if (axis == PITCH)
      return -gyroRaw[PITCH] >> 3;
    else
      return gyroRaw[axis] >> 3;
  }
};

////////////////////////////////////////////////////////////////////////////////
//  APM Gyro
////////////////////////////////////////////////////////////////////////////////
#if defined(APM)
class Gyro_APM : public Gyro {
private:

public:
  Gyro_APM() : Gyro() {
    gyroScaleFactor = DEG_2_RAD((3.3/4096) / 0.002);  // IDG/IXZ500 sensitivity = 2mV/(deg/sec)
  }
  
////////////////////////////////////////////////////////////////////////////////
// Initialize APM Gyro
////////////////////////////////////////////////////////////////////////////////

void initialize(void) {
  initializeApmADC();  // this is needed for both gyros and accels, done once in this class
  }
  
////////////////////////////////////////////////////////////////////////////////
// Measure APM Gyro
////////////////////////////////////////////////////////////////////////////////

void measure(void) {
  // The following 3 lines marked with ** read the gyro and assign it's data to
  // gyroRaw in the correct order and phase to suit the standard shield installation
  // orientation.  See TBD for details.  If your shield is not installed in this
  // orientation, this is where you make the changes.
  gyroRaw[ROLL]  = readApmADC(ROLL);
  if (gyroRaw[ROLL] > 500)
    gyroRaw[ROLL] = gyroRaw[ROLL] - gyroZero[ROLL];     // **
    
  gyroRaw[PITCH] = readApmADC(PITCH);
  if (gyroRaw[PITCH] > 500)
    gyroRaw[PITCH] = gyroZero[PITCH] - gyroRaw[PITCH];  // **
    
  gyroRaw[YAW] = readApmADC(YAW);
  if (gyroRaw[YAW] > 500)
    gyroRaw[YAW] = gyroZero[YAW] - gyroRaw[YAW];        // **
    
  for (byte axis = ROLL; axis < LASTAXIS; axis++) {
    gyroVector[axis] = smooth(gyroRaw[axis] * gyroScaleFactor, gyroVector[axis], smoothFactor);
  }  
}

////////////////////////////////////////////////////////////////////////////////
// Calibrate APM Gyro
////////////////////////////////////////////////////////////////////////////////

  void calibrate() {
    autoZero();
    writeFloat(gyroZero[ROLL],  GYRO_ROLL_ZERO_ADR);
    writeFloat(gyroZero[PITCH], GYRO_PITCH_ZERO_ADR);
    writeFloat(gyroZero[YAW],   GYRO_YAW_ZERO_ADR);
  }
  
  void autoZero() {
    int findZero[FINDZERO];
    
    for (byte calAxis = ROLL; calAxis < LASTAXIS; calAxis++) {
      for (int i=0; i<FINDZERO; i++) {
        findZero[i] = readApmADC(calAxis);
        delay(10);
      }
      gyroZero[calAxis] = findMode(findZero, FINDZERO);
    }
  }
  
////////////////////////////////////////////////////////////////////////////////
// Zero APM Gyro
////////////////////////////////////////////////////////////////////////////////

  void zero() {
    for (byte n = 0; n < 4; n++) {
      adc_value[n] = 0;
      adc_counter[n] = 0;
    }
  }
   
  const int getFlightData(byte axis) {
    return gyroRaw[axis] >> 3;
  }
  
  const int getFlightData(byte axis) {
    return getNonDriftCorrectedRate(axis);
  }
};
#endif

////////////////////////////////////////////////////////////////////////////////
// Wii Gyro
////////////////////////////////////////////////////////////////////////////////
#if defined(AeroQuad_Wii) || defined(AeroQuadMega_Wii)
class Gyro_Wii : public Gyro {
private:

public:
  Gyro_Wii() : Gyro(){
    gyroScaleFactor = DEG_2_RAD(0.06201166);
  }

////////////////////////////////////////////////////////////////////////////////
// Initialize Wii Gyro
////////////////////////////////////////////////////////////////////////////////

void initialize(void) {
  //Init WM+ and Nunchuk
  updateRegisterI2C(0x53, 0xFE, 0x05);
  delay(100);
  updateRegisterI2C(0x53, 0xF0, 0x55);
  delay(100);
};

////////////////////////////////////////////////////////////////////////////////
// Measure Wii Gyro
////////////////////////////////////////////////////////////////////////////////

void measure() {
  readWii(1);
}

////////////////////////////////////////////////////////////////////////////////
// Calibrate Wii Gyro
////////////////////////////////////////////////////////////////////////////////

  void calibrate() {
    autoZero();
    writeFloat(gyroZero[ROLL],  GYRO_ROLL_ZERO_ADR);
    writeFloat(gyroZero[PITCH], GYRO_PITCH_ZERO_ADR);
    writeFloat(gyroZero[YAW],   GYRO_YAW_ZERO_ADR);
  }
  
  void autoZero() {
    int findZero[FINDZERO];
  
    for (byte calAxis = ROLL; calAxis < LASTAXIS; calAxis++) {
      for (int i=0; i<FINDZERO; i++) {
        readWii(0);
        findZero[i] = gyroRaw[calAxis];
        delay(10);
      }
      gyroZero[calAxis] = findMode(findZero, FINDZERO);
    }
  }
  
////////////////////////////////////////////////////////////////////////////////
// Zero Wii Gyro
////////////////////////////////////////////////////////////////////////////////

  void zero() {
    // Not required for Wii Gyro
  }

  const int getFlightData(byte axis) {
    return getNonDriftCorrectedRate(axis);
  }
};
#endif
