void setup() {
  Serial.begin( 9600 );
}

void loop() {

  if( Serial.available() ){
    char c = 0;
    c = Serial.read();
    if (c == -1){
       Serial.print( " " );
    }else{
    Serial.print( c );
    }
  }
}
