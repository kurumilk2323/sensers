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
#define LEDC_CHANNEL_0 0
// 256(8bit)段階でDuty比を変化させることが出来る
#define LEDC_DUTY_BIT 8
// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 12800


typedef enum flag{
  UNSET,
  SET,
}en_flag;

en_flag enMsrBrightStartFlg = UNSET;

int count = 0;
float msrSum = 0;
float msrAve = 0;
float msrAve2 = 0;
float bright = 0;
int flg = 0;

void setup() 
{
  byte rc;

  Serial.begin(115200);
  while (!Serial);

  Wire.begin(PIN_SDA, PIN_SCL);

  rc = rpr0521rs.init();
  pinMode(YELLOW_LED, OUTPUT);

    // 使用するチャネルと周波数を設定
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_DUTY_BIT);
  // ピンをチャネル0へ接続
  ledcAttachPin(YELLOW_LED, LEDC_CHANNEL_0);
}

void msr_init(){
  count = 0;
  msrSum = 0;
  enMsrBrightStartFlg = UNSET;
}

float msr_jdgStartEnd(int *flg){
  int inputchar;
  
  if ( Serial.available()>0 ){
        inputchar = Serial.read(); //シリアルモニタから文字を読み込む
        switch (inputchar){ //スイッチ文で、文字列Sを認識。Sの場合に平均値を計算開始。次にSが入力される(フラグがSET担っている状態)と、照度の平均を計算。
          case 'S':
            if ( enMsrBrightStartFlg == UNSET){
              digitalWrite(YELLOW_LED, LOW);
              enMsrBrightStartFlg = SET;
              Serial.println("MEARSER_START");
            }

            else {
              enMsrBrightStartFlg = UNSET;
              Serial.println("MEARSER_END");
              Serial.print("Bright_Average. = ");
              msrAve = msrSum/count; //照度の平均を計算
              Serial.println ( msrAve ); 
              msr_init();
              *flg=1;
              return msrAve;
              
            }
            break;
            default:
            break;
        }
   }
}

void ledAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
    // calculate duty, 8191 from 2 ^ 13 - 1
    uint32_t duty = (8191 / valueMax) * min(value, valueMax);
    // write duty to LEDC
    ledcWrite(channel, duty);
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
  }
  else {
    Serial.println("[Error] cannot get sensor data.");
  }
  if(flg==0){ // 平均を一回計算する。
    msrAve2 = msr_jdgStartEnd(&flg);
  }
  //Serial.print("flg=");
  //Serial.print(flg);
  if(enMsrBrightStartFlg==SET){
    msrSum += als_val; //照度を加算していく。　平均を求める関数内でmsrSumを使用し、msrAveを計算している。
    count++; //照度のサンプル数をカウントアップ
  }
  if(flg==1){ // Sが押されて、平均値が計算できたあとに、平均と測定値の差を求める。
    bright = msrAve2 - als_val;
    Serial.print("bright=");
    Serial.print(bright);
    if(bright<=0){
        bright=0;
    }
    //Serial.print("bright="); //bright 確認用
    //Serial.print(bright);
    //Serial.print("ON");
    ledAnalogWrite(LEDC_CHANNEL_0, 255*bright/msrAve2); // 256×Duty比  
  }

  //delay(1000);
}
