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



       // Read 1.1V reference against AVcc
       // set the reference to Vcc and the measurement to the internal 1.1V reference
       #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
         ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
       #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
          ADMUX = _BV(MUX5) | _BV(MUX0) ;
       #else
         ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
       #endif

       delay(2); // Wait for Vref to settle
       ADCSRA |= _BV(ADSC); // Start conversion
       while (bit_is_set(ADCSRA,ADSC)); // measuring

       uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
       uint8_t high = ADCH; // unlocks both

       long result = (high<<8) | low;

       result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
       trueVCC = result; // Vcc in millivolts


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
