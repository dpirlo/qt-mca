
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

#include "apMCAE.hpp"

namespace ap {

    class AutoCalib: public MCAE
    {
        public:
            // Constructor
            AutoCalib(shared_ptr<MCAE> copy);
            // Calibracion Simple
            bool calibrar_simple();

            void setPMT_List(QList<int> checked_PMTs) {this->PMTs_List = checked_PMTs;}
            void setCab_List(QList<int> checked_Cab) {this->Cab_List = checked_Cab;}
            void setCanal_Obj(int Canal_Obj_par) {this->Canal_Obj = Canal_Obj_par;}


        private:
            // Pedir MCA
            void pedir_MCA_PMT(int Cabezal, int PMT, int canales);

        protected:
             QList<int> PMTs_List;
             QList<int> Cab_List;
             int  Canal_Obj;
    };
}

#endif // AUTOCALIB_H
