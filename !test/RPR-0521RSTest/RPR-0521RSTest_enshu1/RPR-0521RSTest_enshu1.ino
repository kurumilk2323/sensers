/*****************************************************************************
  RPR-0521RS.ino
 Copyright (c) 2016 ROHM Co.,Ltd.
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
******************************************************************************/
#include <Wire.h>
#include <RPR-0521RS.h>

RPR0521RS rpr0521rs;

#define PIN_SDA SDA
#define PIN_SCL SCL

//  照度センサ用のLED出力PIN(GPIO 04はPWM出力が可能)
#define PIN_BRIGHT_LED (4)

//  照度センサ用LEDの点灯閾値 単位[Lx(ルクス)]
#define BRIGHT_LED_ON_THR (50)


void setup() 
{
	byte rc;

	Serial.begin(9600);
	while (!Serial);

	Wire.begin(PIN_SDA, PIN_SCL);

	rc = rpr0521rs.init();

  pinMode(PIN_BRIGHT_LED, OUTPUT);
}

void loop() 
{
	int error;
	unsigned short ps_val;
	float als_val;

	error = rpr0521rs.get_psalsval(&ps_val, &als_val);
	if(error == 0) {
		// �ߐڃZ���T�[
    if ( als_val <= BRIGHT_LED_ON_THR){
      digitalWrite(PIN_BRIGHT_LED,HIGH);
    }
    else {
      digitalWrite(PIN_BRIGHT_LED,LOW);
    }
   
		Serial.print("Prox. = ");
		Serial.print(ps_val);
		Serial.print("\t||\t");
		// �Ɠx�Z���T�[
		Serial.print("Bright. = ");
		Serial.print(als_val);

		Serial.println();
	}
	else {
		Serial.println("[Error] cannot get sensor data.");
	}

	delay(1000);
}
