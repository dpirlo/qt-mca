#ifndef COMUNICACIONMSA_H
#define COMUNICACIONMSA_H

#include <QtSerialPort>


namespace ap {

    class ComunicacionMSA
    {
    public:
        ComunicacionMSA();
     //   void Init(string port);
        bool InitVars();
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
/*        int CantCanales;
        int MINCANAL;
        int MAXCANAL;
        const int HVMAXVALUE = 4096;
        int LengthEncabezadoMCA;
        int LengthDatosMCA;
        int LevelOpen = 0;
        QSerialPort SerialPortMCA;*/

    public:        
 /*       int pmtCount = 0;          // Cantidad de PMTs
        const string CMD_HARDRESET = "@00690?";
       // UInt32[][] Multicanal;
       // UInt32[] LastCanal;
       // double MulticanalTiempoMedicion = 0;
       // int[] pmtTemp;
       // int[] pmtHV;
       // int[] pmtPick;
       // float[] pmtTarget;
        string OpcionesCalibracionTipo = "ctromasa";         //Opciones para la Calibración
        int OpcionesCalibracionTiempo = 10000;
        int OpcionesCalibracionPMT;
        int OpcionesCalibracionTargetCh;
        int OpcionesCalibracionCtroMasa;
        int OpcionesCalibracionMesureTime;
        bool CalibracionSuccess;
        int OpcionesMedirEspectroTiempo = 20000;
        int OpcionesMedirEspectroPMTini = 0;
        int OpcionesMedirEspectroPMTfin = 0;
        string OpcionesMedirEspectroDirOut = "";
        bool MedicionEspectroSuccess;
        string sErrorLevel = "";
        string LastError;                                 //Variable pública para indicar el texto del último error
        int ErrorLevel = 0;                                  //Indica el error con un número.*/

    };

}

#endif // COMUNICACIONMSA_H
