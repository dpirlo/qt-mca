#include "inc/apRecon.hpp"

using namespace ap;


Reconstructor::Reconstructor()
{
    // Paths a dependencias
    path_APIRL = "../../../apirl-code-pet/build/cmd/";
    path_INTERFILES = "../../../interfiles/";


    // Sinograma
    Cant_anillos = CANTANILLOS_BASE;
    Dif_anillos = MAXDIFANILLOS_BASE;
    Emax = EMAX_BASE;
    Emin = EMIN_BASE;
    Span = SPAM_BASE;
    cant_ang = CANTANGULOS_BASE;
    cant_rhos = CANTRHOS_BASE;
    max_Rho = MAXRHO_BASE;
    max_Z = MAXZ_BASE;

    // Archivo de parametros
    FOV_Axial = FOV_AXIAL_BASE;
    Min_dif_cab = MIN_DIF_CAB_BASE;
    Radio_FOV = RADIO_FOV_BASE;
    Radio_PET = RADIO_PET_BASE;
    zona_muerta = ZONA_MUERTA_BASE;

}
