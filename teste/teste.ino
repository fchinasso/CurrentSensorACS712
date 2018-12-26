#include <sensorCorrente.h>


sensorCorrente s01(A3,T30A,'D','1');
sensorCorrente s02(A4,T30A,'D','2');
sensorCorrente s03(A5,T30A,'D','3');

int intervalo = 2;
float tempo = 0.5;







void setup() {

Serial.begin(9600);

/*s01.calibrar();
s02.calibrar();
s03.calibrar();

s01.printCalibrar();
s02.printCalibrar();
s03.printCalibrar();*/

intervalo=intervalo*1000;
tempo=tempo*1000*60;


}

void loop() {

uint32_t inicio = millis();
uint32_t timer = millis();

int N=0;

float I1=0;
float I2=0;
float I3=0;


    while( millis() - inicio < tempo){

        if(millis() - timer >= intervalo ){

          I1+=s01.medir();
          I2+=s02.medir();
          I3+=s03.medir();

          timer = millis();
          N++;
        }

     }

      I1 = I1/N;
      I2 = I2/N;
      I3 = I3/N;

    s01.printCorrente(I1);
    s02.printCorrente(I2);
    s03.printCorrente(I3);


}
