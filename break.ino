



//-------------------------------------------------------------------------------------
// Thrustmaster T3PA brakeMod for loadcell
// Christopher Grimm 12.07.2018
//-------------------------------------------------------------------------------------

#include <HX711_ADC.h>
#include "Joystick.h"

// dout, sck
HX711_ADC LoadCell(5, 6);

long t;

float min_read = 0;
float max_read = -2700;
unsigned long time_now = 0;
int calibration_period_in_s = 60*5;
float scaling_value = 255;
bool recalibration = false;

bool FLAG_debugging = false;

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK, 0, 0,
  false, false, false, false, false, false,
  false, false, true, true, false);

// Set to true to test "Auto Send" mode or false to test "Manual Send" mode.
const bool testAutoSendMode = false;

const unsigned long gcCycleDelta = 1000;
const unsigned long gcButtonDelta = 500;
const unsigned long gcAnalogDelta = 25;
unsigned long gNextTime = 0;
unsigned int gCurrentStep = 0;

void setup() {
  Serial.begin(250000);
  LoadCell.begin();
  long stabilisingtime = 0; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  LoadCell.setCalFactor(696.0); // user set calibration factor (float)

  Joystick.setAcceleratorRange(0, 260);
  Joystick.setBrakeRange(0, scaling_value);
  
  if (testAutoSendMode)
  {
    Joystick.begin();
  }
  else
  {
    Joystick.begin(false);
  }
  
  pinMode(A0, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  
}

void loop() {

  // memory allocation
  float i;
  float ii;
  float bandwidth;
  
  // read value from loadcell
  LoadCell.update();
  i = LoadCell.getData();

  // read time
  time_now = millis();
  
  // adjust scaling factors during the first 10 seconds
  if (time_now < (calibration_period_in_s * 1000) )
  {
    if ( (time_now % 1000) > 500)
    {
      digitalWrite(LED_BUILTIN, HIGH); 
      }
      else
      {
        digitalWrite(LED_BUILTIN, LOW);
        }
    if ( i < min_read)
    {
      min_read = i;
      }
  
    if ( i > max_read)
    {
      max_read = i;
      }      
      
  }
  else
  {
    // do recalibration of scaling factors to ensure reaching the limits
    if (recalibration == true)
    {
      recalibration = false;
      bandwidth = max_read - min_read;
      min_read += 0.01 * bandwidth;
      max_read -= 0.01 * bandwidth;
      }
    
    digitalWrite(LED_BUILTIN, LOW);
    }

  // scale the value
  ii = (i - min_read) / ( max_read - min_read  ) * scaling_value;
  if (ii <= 0)
  {
    ii = 0;
    }
    if (ii >= scaling_value)
  {
    ii = scaling_value;
    }

  // for debugging: write to serial monitor
  if (FLAG_debugging == true)
  {
    Serial.print(  i );
    Serial.print("\t");
    Serial.print(  max_read );
    Serial.print("\t");              
    Serial.print( min_read );
    Serial.print("\t");              
    Serial.print( ii );          
    Serial.println();
  
    delay(20);
  }

  // write value to joystick api
  Joystick.setBrake( ii );

  if (testAutoSendMode == false)
  {
    Joystick.sendState();
  }

  

}
