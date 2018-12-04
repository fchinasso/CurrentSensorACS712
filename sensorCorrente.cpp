//#include "Arduino.h"
#include <sensorCorrente.h>


sensorCorrente::sensorCorrente(int _pino,sensorCorrente_type modelo){     //METODO CONSTRUTOR

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

void sensorCorrente::calibrar(){
    int n = 0;

    for(int i =0;i<10;i++){

      n += analogRead(pino);
    }
     zero = (int) n/10;



     long result;
// Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  trueVCC = result/1000;


}



int sensorCorrente::getzero(){

    return(zero);
}

long sensorCorrente::getTrueVcc(){

      return (trueVCC);
}

float sensorCorrente::calculaDigitalIpp(){

      float periodo = (float) 1/60;
      int x;
      int maior=zero;
      int menor=zero;
      unsigned long inicio = millis();

      while(millis()-inicio < 10 * periodo){
        x=analogRead(pino)-zero;

          if(x<menor)
            menor=x;

          if(x>maior)
            maior=x;
         }

         return (maior-menor);
}

float sensorCorrente::calculaRMS(){

       float periodo= (float) 1000000/60;
       float soma=0;
       float M;
       unsigned long inicio = micros();
       int N=0;

        for( N=0 ; micros()-inicio < 10*periodo; N++){

           M=analogRead(pino)-zero;
           soma += M*M;
        }
        return (sqrt(soma/N)/1024*trueVCC)/fatorConversao;


}
float sensorCorrente::calculaCorrenteDC(){

 float A=0;

    for(int n=0; n<10; n++){
        A += analogRead(pino)-zero;
    }

    A = A/10;

	return (((float) (trueVCC*A))/1024)/fatorConversao;
}
