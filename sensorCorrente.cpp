#include "Arduino.h"
#include <sensorCorrente.h>


sensorCorrente::sensorCorrente(int _pino,sensorCorrente_type modelo){     //Construction method

    pino=_pino;

    switch(modelo){

      case T5B:
      fatorConversao = 0.185;
      break;

      case T20A:
      fatorConversao = 0.100;
      break;

      case T30A:
      fatorConversao = 0.066;
      break;

    }
}
void sensorCorrente::calibrar(){ //calibrates ADC reference
    int n = 0;

    for(int i =0;i<10;i++){

      n += analogRead(pino);
    }
     zero = n/10;

}
float sensorCorrente::calculaDigitalIpp(){ //calculates lowest and highest value for Ipp

      float periodo = (float) 1000/60; //60Hz
      int x;
      int maior=zero;
      int menor=zero;
      unsigned long inicio = millis();

      while(millis()-inicio < 10 * periodo){ //need to test how many periods works best
        x=analogRead(pino)-zero;

          if(x<menor)
            menor=x;

          if(x>maior)
            maior=x;
         }

         return (maior-menor);
}

float sensorCorrente::calculaRMS(){ //takes samples for RMS calculating of AC signal

       float periodo= (float) 1000/60;
       float soma=0;
       float M;
       unsigned long inicio = millis();
       int N=0;

        for( N=0 ; millis()-inicio < periodo; N++){
             M=analogRead(pino)-zero;
             soma += M*M;
        }
        return sqrt(soma/N);


}
float sensorCorrente::calculaCorrente(int A){ //receives a value and converts it according to the sensor conversion value


       A = (A*(5/1024))/fatorConversao;

       return A;
}
