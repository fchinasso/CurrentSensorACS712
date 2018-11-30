
#ifndef sensorCorrente_h
#define sensorCorrente_h

#include "Arduino.h"

enum sensorCorrente_type {T5B,T20A,T30A};


class sensorCorrente{

    public:
        sensorCorrente(int _pino,sensorCorrente_type modelo);
        float calculaRMS();
        float calculaCorrenteDC();
        void  calibrar();
	      float calculaDigitalIpp();



    private:
        int pino;
        float fatorConversao;
        int zero = 512;


};


#endif
