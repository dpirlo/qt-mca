
/**
 * @class ap::AutoCalib
 *
 * @brief Clase para calibracion automatico
 *
 * Esta clase provee métodos y propiedades para el
 * calibrado de cabezales automaticamente.
 *
 * @note Clase heredada de MCAE
 *
 * @author Ramiro G. Rodriguez Colmeiro
 *
 * @version $Version
 *
 * Contacto: rodriguezcolmeiro@cae.cnea.gov.ar
 *
 */

#ifndef AUTOCALIB_H
#define AUTOCALIB_H

#include "qcustomplot.h"
#include "apMCAE.hpp"
#include <armadillo>
#include <sys/sysinfo.h>


// Valor maximo de movimiento de canal
#define MAX_MOV_DIN 250
// Valor base de movimiento de canal
#define BASE_MOV_DIN 50

// Memoria para mantener los 6 cabezales en RAM (Suponiendo un archivo de 1 GB de calibracion)
#define     HighMemDevice               10*6*1024*1024

// Definiciones para levantar archivos
#define     BYTES                       8
#define     CANTIDADdEpMTS              48
#define     CANTIDADdEbYTEShEADER   	2
#define     CANTIDADdEbYTEStIMEsTAMP    4
#define     CANTIDADdEbYTESpMTS     	(CANTIDADdEpMTS*12)/BYTES
#define     CANTIDADdEbYTEStIEMPO   	(CANTIDADdEpMTS*20)/BYTES
#define     CANTIDADdEbYTESnUMERO   	0
#define     CANTIDADdEbYTEScHECKs   	2
#define     CANTIDADtOTALbYTES      	(CANTIDADdEbYTEShEADER+CANTIDADdEbYTEStIMEsTAMP+CANTIDADdEbYTESpMTS+CANTIDADdEbYTEStIEMPO+CANTIDADdEbYTESnUMERO+CANTIDADdEbYTEScHECKs)

#define     HEADER_1                	0xFFFF
#define     HEADER_2                	0xFEFE

#define     CANTIDADdEbYTESdATOS    	(CANTIDADdEbYTEStIMEsTAMP+CANTIDADdEbYTESpMTS+CANTIDADdEbYTEStIEMPO+CANTIDADdEbYTESnUMERO)
#define     POSICIONpRIMERdATO      	CANTIDADdEbYTEShEADER
#define     POSICIONdELuLTIMOdATO   	(CANTIDADtOTALbYTES-CANTIDADdEbYTEScHECKs-1)
#define     POSICIONdELcHECKsUM     	(CANTIDADtOTALbYTES-CANTIDADdEbYTEScHECKs)



#define     VENTANAdEtIEMPOeNTREeNERGIAS        2
#define     CANTIADADdEcLOKS_1useg              40
#define     CANTIDADdEbITSeNTEROSsPARTAN        14


#define     BinsHist                            256
#define     NUM_EVENT_CENTRO                    5000

using namespace arma;

namespace ap {

    class AutoCalib: public MCAE
    {
        public:
            // Constructor
            AutoCalib();
            // Calibracion Simple
            bool calibrar_simple(QCustomPlot* plot_hand);

            // Busqueda de pico sobre vector
            struct Pico_espectro Buscar_Pico(double* Canales, int num_canales);

            // Plot de MCA actual
            void plot_MCA(QVector<double> hits, QVector<double> channels_ui, QCustomPlot *graph, QString graph_legend, QVector<int> param, bool clear);

            // Set de lista de PMTs a calibrar
            void setPMT_List(QList<int> checked_PMTs) {this->PMTs_List = checked_PMTs;}
            // Set de lista de cabezales a calibrar
            void setCab_List(QList<int> checked_Cab) {this->Cab_List = checked_Cab;}
            // Set de canal objetivo
            void setCanal_Obj(int Canal_Obj_par) {this->Canal_Obj = Canal_Obj_par;}
            // Set de tiempo adquisicion
            void setTiempo_adq(int tiempo_adq_par) {this->tiempo_adq = tiempo_adq_par;}
            // Set de puerto serie
            void setPort_Name(QString port_name_par) {this->port_name = port_name_par;}

            // Set de archivos de configuración
            void setAdq_Cab_1(string par_string) {this->adq_cab[0] = par_string;}
            void setAdq_Cab_2(string par_string) {this->adq_cab[1] = par_string;}
            void setAdq_Cab_3(string par_string) {this->adq_cab[2] = par_string;}
            void setAdq_Cab_4(string par_string) {this->adq_cab[3] = par_string;}
            void setAdq_Cab_5(string par_string) {this->adq_cab[4] = par_string;}
            void setAdq_Cab_6(string par_string) {this->adq_cab[5] = par_string;}
            void setAdq_Coin(string par_string) {this->adq_coin = par_string;}

            // Calibración fina
            bool calibrar_fina(void);


        private:
            // Pedir MCA
            void pedir_MCA_PMT(int Cabezal, int PMT, int canales, bool Calib);
            // Setear HV dinodo PMT
            void modificar_HV_PMT(int Cabezal, int PMT,  int val_dinodo);
            // Reseteo memoria cabezal (SP6)
            void reset_Mem_Cab(int Cabezal);

            // Datos calibracion
            QList<int> PMTs_List;
            QList<int> Cab_List;
            int  Canal_Obj, tiempo_adq;

            // Puerto serie
            QString port_name;

            // Ventanas emergentes
            QCustomPlot Espectro_emergente[6];
            QCustomPlot Espectro_PMT_emergente[6];


            // Memoria de los PMT
            double Acum_PMT[PMTs][CHANNELS];
            // Posiciones de los picos
            double Picos_PMT[PMTs];
            // Valor de los dinodos
            double Dinodos_PMT[PMTs];

            // Blancas Ajedres
            const double weisse[24]      = {1 , 3 , 5 , 7 , 10 , 12 , 14 , 16 , 17 , 19 , 21 , 23 , 26 , 28 , 30 , 32 , 33 , 35 , 37 , 39 , 42 , 44 , 46 , 48};
            const double schwarze[24]    = {2 , 4 , 6 , 8 ,  9 , 11 , 13 , 15 , 18 , 20 , 22 , 24 , 25 , 27 , 29 , 31 , 34 , 36 , 38 , 40 , 41 , 43 , 45 , 47};

            // Archivos de adquisición
            string adq_cab[6];
            string adq_coin;

            // Archivos parseados
            mat Energia_calib[6];
            mat Tiempos_calib[6];
            mat Tiempos_full_calib[6];
            mat TimeStamp_calib[6];

            // Matriz de energia promedio en centroides
            mat E_prom_PMT;

            // Calibración fina de eneregía
            bool calibrar_fina_energia(int cab_num_act);

            // Levantar archivo de calibración
            bool LevantarArchivo_Planar(int cab_num_act);
            // Parseo de trama
            unsigned char * Trama(unsigned char *tramaEntrada,int tamanioTramaEntrada,int * tamanioTramaSalidaPointer);

            // Flag de RAM
            bool IsLowRAM;

    };

    struct Pico_espectro {
        double canal_pico;
        double limites_FWHM[2];
        double limites_FWTM[2];
        double FWHM;
        double FWTM;
        double limites_Pico[2];
    };
}

#endif // AUTOCALIB_H
