#include <sensorCorrente.h>


sensorCorrente s01(A1,T30A);

void setup() {
Serial.begin(9600);
s01.calibrar();
Serial.println("Calibrado!");

}

void loop() {

    float I;


    I = s01.calculaCorrenteDC();
    Serial.println(I);


    delay(1000);

}
