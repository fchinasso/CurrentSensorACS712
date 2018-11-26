#include <sensorCorrente.h>

sensorCorrente s01(A0,T30A);

void setup() {
Serial.begin(9600);

}

void loop() {
    float I = s01.calculaRMS();
    I=s01.calculaCorrente(I);
   s01.

    Serial.println(String ("Corrente=")+ I + "RMS" );

    delay(1000);

}
