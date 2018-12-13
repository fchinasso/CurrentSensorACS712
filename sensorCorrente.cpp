//#include "Arduino.h"
#include <sensorCorrente.h>


sensorCorrente::sensorCorrente(int _pino,sensorCorrente_type modelo,char _tipomedicao){ //construction method for the different types of sensors

    pino=_pino;
    tipomedicao =_tipomedicao;

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

      case SCT013A5:
      fatorConversao = 0.2;
      break;

      case SCT013A10:
      fatorConversao = 0.1;
      break;

      case SCT013A15:
      fatorConversao = 0.066;
      break;


      case SCT013A20:
      fatorConversao = 0.05;
      break;


      case SCT13A25:
      fatorConversao = 0.04;
      break;

      case SCT013A30:
      fatorConversao = 0.033;
      break;

      case SCT013A50:
      fatorConversao = 0.02;
      break;

      case SCT013A60:
      fatorConversao = 0.016;
      break;
}

}

void sensorCorrente::calibrar(){  //calibrates the sensor (no current must flow throught it)
    float n = 0;

    for(int i =0;i<10;i++){  //calibrates de offset 2,5 V

      n =  n + analogRead(pino);

    }

     zero = n/10;


                                          //calibrates VCC
     long result;

  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result;
  trueVCC = (float) result/1000;


}



int sensorCorrente::getzero(){

    return(zero);
}

float sensorCorrente::getTrueVcc(){

      return (trueVCC);
}



float sensorCorrente::calculaRMS(){ //Calculates RMS value

       float periodo= (float) 1000/60;
       float soma=0;
       float M;
       uint32_t inicio = millis();
       int N=0;

        while (millis()-inicio < periodo){

           M=analogRead(pino)-zero;
           N++;
           soma += M*M;
           }


       return (sqrt(soma/N)/1024*trueVCC)/fatorConversao;

}
float sensorCorrente::calculaCorrenteDC(){ //calculates DC value

    float A=0;

    for(int n=0; n<5; n++){
        A += analogRead(pino)-zero;
    }

    A = A/5;

	return (((float) (trueVCC*A))/1024)/fatorConversao;
}

float sensorCorrente::medir(){//simples function for measuring a sensor value

    float I;

     if(tipomedicao == 'A')
      I = calculaRMS();

     if(tipomedicao == 'D')
      I = calculaCorrenteDC();

    return I;

}
void sensorCorrente::printCalibrar(){


  Serial.println("Calibrado!");
  Serial.println(String("Valor de calibracao:") + getzero());
  Serial.println(String("Valor de Vcc:") + getTrueVcc());
  Serial.println();

}

float sensorCorrente::mediaTempo(float time,int intervalo){ // receives excursion time for means calculation


    float soma=0;
    int N=0;
    uint32_t inicio = millis();

    time = time * 60 * 1000;
    intervalo = intervalo *1000;


    while(millis()-inicio < time){

        soma = soma + medir();


        N++;

        delay(intervalo);
    }
    return (float)(soma/N);


}
