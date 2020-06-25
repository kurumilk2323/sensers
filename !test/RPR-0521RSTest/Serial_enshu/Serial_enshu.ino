//  照度センサ用のLED出力PIN(GPIO 04はPWM出力が可能)
#define PIN_BRIGHT_LED (16)

void setup() {
  Serial.begin( 115200 );
  pinMode(PIN_BRIGHT_LED, OUTPUT);
}
int count=0;

void loop() {

  if( Serial.available() ){ //シリアルポートでの入力を受ける。
    char c = 0;
    c = Serial.read(); //char型を読み込む。１文字ずつ。
    if (c == -1){ 
       Serial.print( " " );
    }else{
      Serial.print( c );
      //Serial.println("total 文字数");
      Serial.print( count );
      count++;
      if(count>=10){ //入力が10個以上だと、警告として点灯するように応用
          digitalWrite(PIN_BRIGHT_LED,HIGH);
      }
      else {
          digitalWrite(PIN_BRIGHT_LED,LOW);
      }
    }
    
    }
}
