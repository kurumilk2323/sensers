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
#define YELLOW_LED (16) //黄色LEDのピン
#define BRIGHT_THRESHOLD (50)
void setup() 
{
  byte rc;

  Serial.begin(115200);
  while (!Serial);

  Wire.begin(PIN_SDA, PIN_SCL);

  rc = rpr0521rs.init();
  pinMode(YELLOW_LED, OUTPUT);
}

void loop() 
{
  int error;
  unsigned short ps_val; //照度変数
  float als_val; //近接値変数

  error = rpr0521rs.get_psalsval(&ps_val, &als_val); //センサー値を取得
  if(error == 0) { //エラーチェック。エラーデなければセンサー値出力
    // �ߐڃZ���T�[
    Serial.print("Prox. = ");
    Serial.print(ps_val);
    Serial.print("\t||\t");
    Serial.print("Bright. = ");
    Serial.print(als_val);
    Serial.println();

    if(als_val<=BRIGHT_THRESHOLD){ //もし、照度センサーの値が閾値以下、つまり暗い場所ならば、ライトをつける
        digitalWrite(YELLOW_LED,HIGH); //
        Serial.println("ON");
    }
    else{
        digitalWrite(YELLOW_LED,LOW); //
    }
  }
  else {
    Serial.println("[Error] cannot get sensor data.");
  }

  delay(1000);
}
