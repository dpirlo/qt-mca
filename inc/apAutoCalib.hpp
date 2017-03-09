
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

// Valor maximo de movimiento de canal
#define MAX_MOV_DIN 250
// Valor base de movimiento de canal
#define BASE_MOV_DIN 50


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
            double Buscar_Pico(double* Canales, int num_canales);

            // Plot de MCA actual
            void plot_MCA(QVector<double> hits, QCustomPlot *graph, QString graph_legend, QVector<int> param);

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

            // Memoria de los PMT
            double Acum_PMT[PMTs][CHANNELS];
            // Posiciones de los picos
            double Picos_PMT[PMTs];
            // Valor de los dinodos
            double Dinodos_PMT[PMTs];

            // Blancas Ajedres
            const double weisse[24]      = {1 , 3 , 5 , 7 , 10 , 12 , 14 , 16 , 17 , 19 , 21 , 23 , 26 , 28 , 30 , 32 , 33 , 35 , 37 , 39 , 42 , 44 , 46 , 48};
            const double schwarze[24]    = {2 , 4 , 6 , 8 , 9 , 11 , 13 , 15 , 18 , 20 , 22 , 24 , 25 , 27 , 29 , 31 , 34 , 36 , 38 , 40 , 41 , 43 , 45 , 47};

    };
}

#endif // AUTOCALIB_H
