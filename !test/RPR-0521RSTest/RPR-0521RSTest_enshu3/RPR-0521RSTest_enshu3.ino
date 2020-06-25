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

#define LEDC_CHANNEL_0 0
// 256(8bit)段階でDuty比を変化させることが出来る
#define LEDC_DUTY_BIT 8
// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 12800

typedef enum flag{
  UNSET,
  SET,
}en_flag;

// 平均値の測定を開始するフラグ
en_flag enMsrBrightStartFlg = UNSET;
// LEDの点灯を開始するフラグ
en_flag enLedOnFlg = UNSET;

int count = 0;
float msrSum = 0;
float msrAve = 0;

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) 
{
    // calculate duty, 8191 from 2 ^ 13 - 1
    uint32_t duty = (8191 / valueMax) * min(value, valueMax);

    // write duty to LEDC
    ledcWrite(channel, duty);
}
void setup() 
{
  byte rc;

  Serial.begin(9600);
  while (!Serial);

  Wire.begin(PIN_SDA, PIN_SCL);
  rc = rpr0521rs.init();
  pinMode(PIN_BRIGHT_LED, OUTPUT);

  // 使用するチャネルと周波数を設定
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_DUTY_BIT);
  // ピンをチャネル0へ接続
  ledcAttachPin(PIN_BRIGHT_LED, LEDC_CHANNEL_0);
}

void loop() 
{
  int error;
  unsigned short ps_val;
  float als_val;

  int inputchar;
  int x;
  
  error = rpr0521rs.get_psalsval(&ps_val, &als_val);
  if(error == 0) {
    // �ߐڃZ���T�[
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

// 計測開始と終了の判断部

   msr_jdgStartEnd();

  if ( enMsrBrightStartFlg == SET){
      msrSum += als_val;
      count += 1;
    }

  // 平均値の計測が完了したら、LEDを点灯させる
  if ( enLedOnFlg == SET){
    x = msrAve-als_val;
    if (x<=0){
      x=0;
    }
    ledcAnalogWrite(LEDC_CHANNEL_0, 255*x/msrAve);
  }
  delay(1000);
}
// 初期化
void msr_init(){
  count = 0;
  msrSum = 0;
  enMsrBrightStartFlg = UNSET;
}

void msr_jdgStartEnd(){

    int inputchar;
  
  if ( Serial.available()>0 ){
        inputchar = Serial.read();
        switch (inputchar){
          case 'S':
            if ( enMsrBrightStartFlg == UNSET){
              enLedOnFlg = UNSET;
              enMsrBrightStartFlg = SET;
              Serial.println("MEARSER_START");
            }
            else {
              enMsrBrightStartFlg = UNSET;
              enLedOnFlg = SET;
              Serial.println("MEARSER_END");
              Serial.print("Bright_Average. = ");
              msrAve = msrSum/count;
              Serial.println ( msrAve );
              msr_init();
            }
            break;
          default:
            break;
        }
   }
}
