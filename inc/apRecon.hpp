

#ifndef APRECON
#define APRECON


#include "apMCAE.hpp"
#include "qprocess.h"



#define     EMIN_BASE               0
#define     EMAX_BASE               10000
#define     CANTANILLOS_BASE        41
#define     SPAM_BASE               7
#define     MAXDIFANILLOS_BASE      40
#define     CANTANGULOS_BASE        248
#define     CANTRHOS_BASE           192
#define     MAXRHO_BASE             500
#define     MAXZ_BASE               200


#define     FOV_AXIAL_BASE          302.6
#define     MIN_DIF_CAB_BASE        1
#define     RADIO_FOV_BASE          300
#define     RADIO_PET_BASE          360
#define     ZONA_MUERTA_BASE        00


namespace ap {


    class Reconstructor: public MCAE
    {
        public:
            // Constructor
            Reconstructor();

            // Metodos
            bool Parsear();
            bool Reconstruir();

            // Set de paths
            void setPathAPIRL(QString par_string) {this->path_APIRL = par_string;}
            void setPathINTERFILES(QString par_string) {this->path_INTERFILES = par_string;}
            void setPathPARSER(QString par_string) {this->path_PARSER = par_string;}
            void setPathSalida(QString par_string) {this->path_INTERFILES = par_string;}
            // Get de paths
            QString getPathAPIRL() {return this->path_APIRL;}
            QString getPathINTERFILES() {return this->path_INTERFILES;}
            QString getPathPARSER() {return this->path_PARSER;}
            QString getPathSalida() {return this->path_Salida;}

            // Set de archivos
            void setArchRecon(QString par_string) {this->arch_recon = par_string;}
            void setArchInicial(QString par_string) {this->arch_ini = par_string;}
            void setArchSensib(QString par_string) {this->arch_sens = par_string;}
            void setArchCountSkimm(QString par_string) {this->arch_countskimm = par_string;}
            // Get de archivos
            QString getArchRecon() {return this->arch_recon;}
            QString getArchInicial() {return this->arch_ini;}
            QString getArchSensib() {return this->arch_sens;}
            QString getArchCountSkimm() {return this->arch_countskimm;}

            // Set de valores
            void setCant_anillos(double par_double){this->Cant_anillos = par_double ;}
            void setDif_anillos(double par_double){this->Dif_anillos = par_double ;}
            void setEmax(double par_double){this->Emax = par_double ;}
            void setEmin(double par_double){this->Emin = par_double ;}
            void setSpan(double par_double){this->Span = par_double ;}
            void setcant_ang(double par_double){this->cant_ang = par_double ;}
            void setcant_rhos(double par_double){this->cant_rhos = par_double ;}
            void setmax_Rho(double par_double){this->max_Rho = par_double ;}
            void setmax_Z(double par_double){this->max_Z  = par_double;}
            void setFOV_Axial(double par_double){this->FOV_Axial = par_double ;}
            void setMin_dif_cab(double par_double){this->Min_dif_cab = par_double ;}
            void setRadio_FOV(double par_double){this->Radio_FOV = par_double ;}
            void setRadio_PET(double par_double){this->Radio_PET = par_double ;}
            void setzona_muerta(double par_double){this->zona_muerta = par_double ;}
            // Get de valores
            double getCant_anillos(){return this->Cant_anillos ;}
            double getDif_anillos(){return this->Dif_anillos ;}
            double getEmax(){return this->Emax ;}
            double getEmin(){return this->Emin ;}
            double getSpan(){return this->Span ;}
            double getcant_ang(){return this->cant_ang ;}
            double getcant_rhos(){return this->cant_rhos ;}
            double getmax_Rho(){return this->max_Rho ;}
            double getmax_Z(){return this->max_Z ;}
            double getFOV_Axial(){return this->FOV_Axial ;}
            double getMin_dif_cab(){return this->Min_dif_cab ;}
            double getRadio_FOV(){return this->Radio_FOV ;}
            double getRadio_PET(){return this->Radio_PET ;}
            double getzona_muerta(){return this->zona_muerta ;}

            // Set procedimientos
            void setParsear(){this->parsear = 1;}
            void setReconstruir(){this->reconstruir = 1;}
            void resetParsear(){this->parsear = 0;}
            void resetReconstruir(){this->reconstruir = 0;}
            // Get Procedimientos
            bool getParsear(){return this->parsear;}
            bool getReconstruir(){return this->reconstruir;}

        private:

            // Paths a dependencias
            QString path_APIRL;
            QString path_INTERFILES;
            QString path_PARSER;
            QString path_Salida;

            // Archivos
            QString arch_recon;
            QString arch_ini;
            QString arch_sens;
            QString arch_countskimm;

            // Sinograma
            double Cant_anillos;
            double Dif_anillos;
            double Emax;
            double Emin;
            double Span;
            double cant_ang;
            double cant_rhos;
            double max_Rho;
            double max_Z;

            // Archivo de parametros
            double FOV_Axial;
            double Min_dif_cab;
            double Radio_FOV;
            double Radio_PET;
            double zona_muerta;

            // Procedimientos
            bool parsear;
            bool reconstruir;


    };






}


#endif // APRECON

