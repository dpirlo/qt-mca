
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


namespace ap {

    class AutoCalib: public MCAE
    {
        public:
            // Constructor
            AutoCalib();
            // Calibracion Simple
            bool calibrar_simple(QCustomPlot* plot_hand);

            // Plot de MCA actual
            void plot_MCA(QVector<double> hits, QCustomPlot *graph, QString graph_legend, QVector<int> param);

            // Set de lista de PMTs a calibrar
            void setPMT_List(QList<int> checked_PMTs) {this->PMTs_List = checked_PMTs;}
            // Set de lista de cabezales a calibrar
            void setCab_List(QList<int> checked_Cab) {this->Cab_List = checked_Cab;}
            // Set de canal objetivo
            void setCanal_Obj(int Canal_Obj_par) {this->Canal_Obj = Canal_Obj_par;}
            // Set de puerto serie
            void setPort_Name(QString port_name_par) {this->port_name = port_name_par;}


        private:
            // Pedir MCA
            void pedir_MCA_PMT(int Cabezal, int PMT, int canales, bool Calib);
            // Setear HV dinodo PMT
            void modificar_HV_PMT(int Cabezal, int PMT,  int val_dinodo);

            QList<int> PMTs_List;
            QList<int> Cab_List;
            int  Canal_Obj;
            QString port_name;
    };
}

#endif // AUTOCALIB_H
