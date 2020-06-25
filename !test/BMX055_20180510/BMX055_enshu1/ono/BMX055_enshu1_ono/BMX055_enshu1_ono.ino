//================================================================//
//  AE-BMX055             Arduino UNO                             //
//    VCC                    +5V                                  //
//    GND                    GND                                  //
//    SDA                    A4(SDA)                              //
//    SCL                    A5(SCL)                              //
//                                                                //
//   (JP6,JP4,JP5はショートした状態)                              //
//   http://akizukidenshi.com/catalog/g/gK-13010/                 //
//================================================================//

#include<Wire.h>
// BMX055　加速度センサのI2Cアドレス  
#define Addr_Accl 0x19  // (JP1,JP2,JP3 = Openの時)
// BMX055　ジャイロセンサのI2Cアドレス
#define Addr_Gyro 0x69  // (JP1,JP2,JP3 = Openの時)
// BMX055　磁気センサのI2Cアドレス
#define Addr_Mag 0x13   // (JP1,JP2,JP3 = Openの時)
#define YELLOW_LED (16)

// センサーの値を保存するグローバル関数
float xAccl = 0.00;
float yAccl = 0.00;
float zAccl = 0.00;
float xGyro = 0.00;
float yGyro = 0.00;
float zGyro = 0.00;
int   xMag  = 0;
int   yMag  = 0;
int   zMag  = 0;

int count = 0;
int flg = 0;

void setup()
{
  // Wire(Arduino-I2C)の初期化
  Wire.begin();
  // デバック用シリアル通信は9600bps
  Serial.begin(9600);
  //BMX055 初期化
  BMX055_Init();
  delay(300);
}

typedef enum flag{
  UNSET,
  SET,
}en_flag;

// 平均値の測定を開始するフラグ
en_flag enMsrStartFlg = UNSET;
// LEDの点灯を開始するフラグ
en_flag enLedOnFlg = UNSET;

//2(加速度センサとジャイロセンサ)×3(x, y, z)分の要素を確保する
float msrSum[2][3] = {{0,0,0},{0,0,0}};
float msrAve[2][3] = {{0,0,0},{0,0,0}};
float msrResult[2][3] = {{0,0,0},{0,0,0}};

float msr_jdgStartEnd(int *flg){
  int inputchar;
  int i,j;
  
  if ( Serial.available()>0 ){
        inputchar = Serial.read(); //シリアルモニタから文字を読み込む
        switch (inputchar){ //スイッチ文で、文字列Sを認識。Sの場合に平均値を計算開始。次にSが入力される(フラグがSET担っている状態)と、照度の平均を計算。
          case 'S':
            if ( enMsrStartFlg == UNSET){
              digitalWrite(YELLOW_LED, LOW);
              enMsrStartFlg = SET;
              Serial.println("MEARSER_START");
            }

            else {
              enMsrStartFlg = UNSET;
              Serial.println("MEARSER_END");
              Serial.print("Bright_Average. = ");
              for(i=0;i<2;i++){
                for(j=0;j<3;j++){
                  msrResult[i][j] = msrSum[i][j]/count; //照度の平均を計算
                  Serial.println (msrResult[i][j]);
                }
              }
              *flg=1;
              return msrResult;
              
            }
            break;
            default:
            break;
        }
   }
}

void loop()
{
  Serial.println("--------------------------------------"); 

  //BMX055 加速度の読み取り
  BMX055_Accl();
  Serial.print("Accl= ");
  Serial.print(xAccl);
  Serial.print(",");
  Serial.print(yAccl);
  Serial.print(",");
  Serial.print(zAccl);
  Serial.println(""); 
  
  //BMX055 ジャイロの読み取り
  BMX055_Gyro();
  Serial.print("Gyro= ");
  Serial.print(xGyro);
  Serial.print(",");
  Serial.print(yGyro);
  Serial.print(",");
  Serial.print(zGyro);
  Serial.println(""); 
  
  //BMX055 磁気の読み取り
  BMX055_Mag();
  Serial.print("Mag= ");
  Serial.print(xMag);
  Serial.print(",");
  Serial.print(yMag);
  Serial.print(",");
  Serial.print(zMag);
  Serial.println(""); 

  if(flg==0){ // フラグの立っていない状態(Sが押されていない状態)で、Sが押されたら平均を計算し始める。
    msrAve = msr_jdgStartEnd(&flg); 
  }
  //Serial.print("flg=");
  //Serial.print(flg);
  if(enMsrStartFlg==SET){ //Sが押された場合、もう一度Sが押されてフラグが下がるまで加算していく。
    //加速度
    msrSum[0][0] += xAccl;
    msrSum[0][1] += yAccl;
    msrSum[0][2] += zAccl;

    // 角速度
    msrSum[1][0] += xGyro;
    msrSum[1][1] += yGyro;
    msrSum[1][2] += zGyro;

    count++; //サンプル数をカウントアップ
  }
  if(flg==1){ // Sが押されて、平均値が計算できたあとに、平均と測定値の差を求める。
    msrResult[0][0]=xAccl-msrAve[0][0];
    msrResult[0][1]=yAccl-msrAve[0][1]; 
    msrResult[0][2]=zAccl-msrAve[0][2];
    
    //BMX055 ジャイロの読み取り
    BMX055_Gyro();
    msrResult[1][0]=xGyro-msrAve[1][0];
    msrResult[1][1]=yGyro-msrAve[1][1]; 
    msrResult[1][2]=zGyro-msrAve[1][2];

    digitalWrite(PIN_STB_LED, HIGH); 
  }else{
    digitalWrite(PIN_STB_LED, LOW); 
  }

  delay(1000);
}

//=====================================================================================//
void BMX055_Init()
{
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Accl);
  Wire.write(0x0F); // Select PMU_Range register
  Wire.write(0x03);   // Range = +/- 2g
  Wire.endTransmission();
  delay(100);
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Accl);
  Wire.write(0x10);  // Select PMU_BW register
  Wire.write(0x08);  // Bandwidth = 7.81 Hz
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Accl);
  Wire.write(0x11);  // Select PMU_LPW register
  Wire.write(0x00);  // Normal mode, Sleep duration = 0.5ms
  Wire.endTransmission();
  delay(100);
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Gyro);
  Wire.write(0x0F);  // Select Range register
  Wire.write(0x04);  // Full scale = +/- 125 degree/s
  Wire.endTransmission();
  delay(100);
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Gyro);
  Wire.write(0x10);  // Select Bandwidth register
  Wire.write(0x07);  // ODR = 100 Hz
  Wire.endTransmission();
  delay(100);
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Gyro);
  Wire.write(0x11);  // Select LPM1 register
  Wire.write(0x00);  // Normal mode, Sleep duration = 2ms
  Wire.endTransmission();
  delay(100);
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4B);  // Select Mag register
  Wire.write(0x83);  // Soft reset
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4B);  // Select Mag register
  Wire.write(0x01);  // Soft reset
  Wire.endTransmission();
  delay(100);
  //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4C);  // Select Mag register
  Wire.write(0x00);  // Normal Mode, ODR = 10 Hz
  Wire.endTransmission();
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x4E);  // Select Mag register
  Wire.write(0x84);  // X, Y, Z-Axis enabled
  Wire.endTransmission();
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x51);  // Select Mag register
  Wire.write(0x04);  // No. of Repetitions for X-Y Axis = 9
  Wire.endTransmission();
 //------------------------------------------------------------//
  Wire.beginTransmission(Addr_Mag);
  Wire.write(0x52);  // Select Mag register
  Wire.write(0x16);  // No. of Repetitions for Z-Axis = 15
  Wire.endTransmission();
}
//=====================================================================================//
void BMX055_Accl()
{
  int data[6];
  for (int i = 0; i < 6; i++)
  {
    Wire.beginTransmission(Addr_Accl);
    Wire.write((2 + i));// Select data register
    Wire.endTransmission();
    Wire.requestFrom(Addr_Accl, 1);// Request 1 byte of data
    // Read 6 bytes of data
    // xAccl lsb, xAccl msb, yAccl lsb, yAccl msb, zAccl lsb, zAccl msb
    if (Wire.available() == 1)
      data[i] = Wire.read();
  }
  // Convert the data to 12-bits
  xAccl = ((data[1] * 256) + (data[0] & 0xF0)) / 16;
  if (xAccl > 2047)  xAccl -= 4096;
  yAccl = ((data[3] * 256) + (data[2] & 0xF0)) / 16;
  if (yAccl > 2047)  yAccl -= 4096;
  zAccl = ((data[5] * 256) + (data[4] & 0xF0)) / 16;
  if (zAccl > 2047)  zAccl -= 4096;
  xAccl = xAccl * 0.0098; // renge +-2g
  yAccl = yAccl * 0.0098; // renge +-2g
  zAccl = zAccl * 0.0098; // renge +-2g
}
//=====================================================================================//
void BMX055_Gyro()
{
  int data[6];
  for (int i = 0; i < 6; i++)
  {
    Wire.beginTransmission(Addr_Gyro);
    Wire.write((2 + i));    // Select data register
    Wire.endTransmission();
    Wire.requestFrom(Addr_Gyro, 1);    // Request 1 byte of data
    // Read 6 bytes of data
    // xGyro lsb, xGyro msb, yGyro lsb, yGyro msb, zGyro lsb, zGyro msb
    if (Wire.available() == 1)
      data[i] = Wire.read();
  }
  // Convert the data
  xGyro = (data[1] * 256) + data[0];
  if (xGyro > 32767)  xGyro -= 65536;
  yGyro = (data[3] * 256) + data[2];
  if (yGyro > 32767)  yGyro -= 65536;
  zGyro = (data[5] * 256) + data[4];
  if (zGyro > 32767)  zGyro -= 65536;

  xGyro = xGyro * 0.0038; //  Full scale = +/- 125 degree/s
  yGyro = yGyro * 0.0038; //  Full scale = +/- 125 degree/s
  zGyro = zGyro * 0.0038; //  Full scale = +/- 125 degree/s
}
//=====================================================================================//
void BMX055_Mag()
{
  int data[8];
  for (int i = 0; i < 8; i++)
  {
    Wire.beginTransmission(Addr_Mag);
    Wire.write((0x42 + i));    // Select data register
    Wire.endTransmission();
    Wire.requestFrom(Addr_Mag, 1);    // Request 1 byte of data
    // Read 6 bytes of data
    // xMag lsb, xMag msb, yMag lsb, yMag msb, zMag lsb, zMag msb
    if (Wire.available() == 1)
      data[i] = Wire.read();
  }
  // Convert the data
  xMag = ((data[1] <<8) | (data[0]>>3));
  if (xMag > 4095)  xMag -= 8192;
  yMag = ((data[3] <<8) | (data[2]>>3));
  if (yMag > 4095)  yMag -= 8192;
  zMag = ((data[5] <<8) | (data[4]>>3));
  if (zMag > 16383)  zMag -= 32768;
}
