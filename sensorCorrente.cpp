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

      case SCT013A5
      fatorConversao = 0.2;
      break;

      case SCT013A10
      fatorConversao = 0.1;
      break;

      case SCT013A15
      fatorConversao = 0.066;
      break;


      case SCT013A20:
      fatorConversao = 0.05;
      break;


      case SCT13A25
      fatorConversao = 0.04;
      break;

      case SCT013A30
      fatorConversao = 0.033;
      break;

      case SCT013A50
      fatorConversao = 0.02;
      break;

      case SCT013A60
      fatorConversao = 0.016;
      break;
}

}

void sensorCorrente::calibrar(){
    float n = 0;

    for(int i =0;i<10;i++){

      n =  n + analogRead(pino);

    }

     zero = n/10;



     long result;
// Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  trueVCC = (float) result/1000;


}



int sensorCorrente::getzero(){

    return(zero);
}

float sensorCorrente::getTrueVcc(){

      return (trueVCC);
}

float sensorCorrente::calculaDigitalIpp(){

      float periodo = (float) 1000/60;
      int x;
      int maior=0;
      int menor=0;
      unsigned long inicio = millis();

      while(millis()-inicio < periodo){
        x=analogRead(pino);

          if(x<menor)
            menor=x;

          if(x>maior)
            maior=x;
         }

         return (maior);
}
float mediaTempo(int time){
  unsigned long inicio = millis();
  unsigned int soma=0;
  int i=0;

  while(millis()-inicio < time){
    soma=sensorCorrente.calculaRMS();
    i++;
  }
return soma/i;

}


float sensorCorrente::calculaRMS(){

       float periodo= (float) 1000/60;
       float soma=0;
       float M;
       uint32_t inicio = millis();
       int N=0;

        while (millis()-inicio < 2 * periodo){

           M=analogRead(pino)-zero;
           N++;
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
