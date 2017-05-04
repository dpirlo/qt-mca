#include "inc/apRecon.hpp"

using namespace ap;


Reconstructor::Reconstructor()
{
    // Paths a dependencias
    path_APIRL = "../../../apirl-code-pet/build/cmd/";
    path_INTERFILES = "../../../interfiles/";
    path_PARSER = "../../../Parser/";
    path_Salida = ".Salidas/";

    // Archivos
    arch_recon = "-";
    arch_ini = "-";
    arch_sens = "-";
    arch_countskimm = "-";



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



bool Reconstructor::Parsear()
{
    // Paso 1, parseo



    // Armo la linea al programa
    QString programa = path_INTERFILES+"AnalizoCoincidenciasPET.pl";

    // Armo los argumentos
    QString parametros = "--Emin "+QString::number(Emin)+" --Emax "+QString::number(Emax)+" --in "+path_Salida+"out_parser.lor --out "+path_Salida+"coincidencias.dat";

    return 1;
}
bool Reconstructor::Reconstruir()
{
    return 1;
}
