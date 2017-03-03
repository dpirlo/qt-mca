
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
            AutoCalib(QList<int> checked_PMTs, QList<int> checked_Cab, float Canal_Obj_par);
            // Calibracion Simple
            bool calibrar_simple();


        private:
            // Pedir MCA
            void pedir_MCA_PMT(int Cabezal, int PMT, int canales);

        protected:
             QList<int> PMTs_List;
             QList<int> Cab_List;
             float  Canal_Obj;
    };
}

#endif // AUTOCALIB_H
