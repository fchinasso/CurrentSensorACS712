#ifndef sensorCorrente_h
#define sensorCorrente_h

#include "Arduino.h"

enum sensorCorrente_type {T5B,T20A,T30A, SCT013A5, SCT013A10,SCT013A15,SCT013A20,SCT13A25,SCT013A30,SCT013A50, SCT013A60};


class sensorCorrente{

    public:
        sensorCorrente(int _pino,sensorCorrente_type modelo,char tipomedicao);
        float calculaRMS();
        float calculaCorrenteDC();
        void  calibrar();
        int  getzero();
        float getTrueVcc();
        float mediaTempo(float time,int intervalo);
        float medir();
        void printCalibrar();




    private:
        int pino;
        float fatorConversao;
        int zero = 512;
        float trueVCC=5;
        char tipomedicao = 'A';


};


#endif
