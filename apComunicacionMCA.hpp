#ifndef COMUNICACIONMCA_H
#define COMUNICACIONMCA_H

#include <QtSerialPort>


namespace ap {

    class ComunicacionMCA
    {
    public:
        ComunicacionMCA();
        void Abort();

    public:
        struct FrameMetaData
        {
            double TotalTime;    //Tiempo total de la medición
            int HV;              //HV del pmt
            int ADC_Offset;      //Offset del ADC
            int ADC_Var;         //Varianza del ADC
            int Temp;            //Temperatura del PMT
        };
        enum CALLBACKCODES
        {
            START = 0,
            WAIT = 1,
            FINISH = 2,
            TEXT = 3
        };

    private:
        bool Cancelar = false;                              //Cancela un procedimiento asincrónico


    public:        

      };

}

#endif // COMUNICACIONMCA_H
