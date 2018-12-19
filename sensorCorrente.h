#ifndef sensorCorrente_h
#define sensorCorrente_h

#include "Arduino.h"

enum sensorCorrente_type {T5B,T20A,T30A, SCT013A5, SCT013A10,SCT013A15,SCT013A20,SCT13A25,SCT013A30,SCT013A50, SCT013A60};


class sensorCorrente{

    public:
        sensorCorrente(int _pino,sensorCorrente_type modelo,char tipomedicao,char fase);

        int  getzero();
        float getTrueVcc();
        char getFase();

        float calculaRMS();
        float calculaCorrenteDC();
        float medir();

        void  calibrar();

        void printCalibrar();
        void printCorrente(float valor);





    private:

        int pino;
        float fatorConversao;
        int zero = 512;
        float trueVCC=5;
        char tipomedicao;
        char fase;


};


#endif
