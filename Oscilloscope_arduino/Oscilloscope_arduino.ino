int samples;
int Delay;

void setup() {
  Serial.begin(2000000);
  pinMode(A0, INPUT);
  
}

void loop() {
  if(Serial.available()){
    String incoming = Serial.readStringUntil('\n');
    if(incoming.startsWith("S")){
      samples = incoming.substring(1).toInt();
    }
    else if(incoming.startsWith("D")){
      Delay = incoming.substring(1).toInt();
      unsigned short reading[samples];
      for(int i = 0; i < samples; i++){
        reading[i] = analogRead(A0);
        delayMicroseconds(Delay);
      }
      for(int i = 0; i < samples; i++){
        delayMicroseconds(100);
        Serial.print(reading[i]);
        Serial.print("\n");
      }
    }
   }
}