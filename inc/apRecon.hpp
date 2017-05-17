

#ifndef APRECON
#define APRECON


#include "apMCAE.hpp"
#include "qprocess.h"
#include "qcustomplot.h"



#define     EMIN_BASE               0
#define     EMAX_BASE               10000
#define     CANTANILLOS_BASE        164
#define     SPAM_BASE               7
#define     MAXDIFANILLOS_BASE      82
#define     CANTANGULOS_BASE        496
#define     CANTRHOS_BASE           384
#define     MAXRHO_BASE             500
#define     MAXZ_BASE               200


#define     FOV_AXIAL_BASE          302.6
#define     MIN_DIF_CAB_BASE        1
#define     RADIO_FOV_BASE          300
#define     RADIO_PET_BASE          360
#define     ZONA_MUERTA_BASE        00
#define     ITERACIONES_BASE        30

#define     INST_PARSEO             4
#define     INST_RECON_LOCAL        1
#define     INST_RECON_SERVER       8
#define     INST_SENS_SERVER        2
#define     INST_MOSTRAR            1

namespace ap {


    class Reconstructor: public QObject
    {
        Q_OBJECT

        public:
            // Constructor
            Reconstructor();
            // Destructor
            ~Reconstructor();

            // Metodos
            bool Parsear();
            bool Reconstruir();
            bool Mostrar();


            // Lista de procesos
            bool SetearListasProcesos();
            bool ResetearListasProcesos();

            // Set de salida
            void setConsola(QPlainTextEdit *consola_par) {this->consola = consola_par;}

            // Set de paths
            void setPathAPIRL(QString par_string) {this->path_APIRL = par_string;}
            void setPathINTERFILES(QString par_string) {this->path_INTERFILES = par_string;}
            void setPathPARSER(QString par_string) {this->path_PARSER = par_string;}
            void setPathSalida(QString par_string) {this->path_INTERFILES = par_string;}
            void setNombre_archivo(QString par_string) {this->Nombre_archivo = par_string;}
            // Get de paths
            QString getPathAPIRL() {return this->path_APIRL;}
            QString getPathINTERFILES() {return this->path_INTERFILES;}
            QString getPathPARSER() {return this->path_PARSER;}
            QString getPathSalida() {return this->path_Salida;}
            QString getNombre_archivo() {return this->Nombre_archivo;}

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

            // set IP
            void setServerIP(QString par_string) {this->ip_SERVER = par_string;}
            // get IP
            QString getServerIP() {return this->ip_SERVER;}

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
            void setIteraciones(double par_double){this->iteraciones = par_double ;}
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
            double getIteraciones(){return this->iteraciones;}

            // Set procedimientos
            void setMostrar(){this->mostrar = 1;}
            void setParsear(){this->parsear = 1;}
            void setReconstruir(){this->reconstruir = 1;}
            void setMLEM(){this->reconMLEM = 1;}
            void setBackprojection(){this->reconBackprojection = 1;}
            void setReconServer(){this->reconServer = 1;}
            void setPreSensibilidad(){this->SensibilidadPrecalculada = 1;}
            void setAplicarCountSkimming(){this->AplicarCountSkimming = 1;}
            void setMuerto(){this->muerto = 1;}
            // Reset procedimientos
            void resetMostrar(){this->mostrar = 0;}
            void resetParsear(){this->parsear = 0;}
            void resetReconstruir(){this->reconstruir = 0;}
            void resetMLEM(){this->reconMLEM = 0;}
            void resetBackprojection(){this->reconBackprojection = 0;}
            void resetReconServer(){this->reconServer = 0;}
            void resetPreSensibilidad(){this->SensibilidadPrecalculada = 0;}
            void resetAplicarCountSkimming(){this->AplicarCountSkimming = 0;}
            void resetMuerto(){this->muerto = 0;}
            // Get Procedimientos
            bool getMostrar(){return this->mostrar;}
            bool getParsear(){return this->parsear;}
            bool getReconstruir(){return this->reconstruir;}
            bool getMLEM(){return this->reconMLEM;}
            bool getBackprojection(){return this->reconBackprojection;}
            bool getReconServer(){return this->reconServer;}
            bool getPreSensibilidad(){return this->SensibilidadPrecalculada ;}
            bool getAplicarCountSkimming(){return this->AplicarCountSkimming ;}
            bool getMuerto(){return this->muerto = 0;}

            // Procesos
            bool matar_procesos();

            // Signales
            QEventLoop loop_parser;
            QEventLoop loop_reconstruccion;

        private:

            // Paths a dependencias
            QString path_APIRL;
            QString path_INTERFILES;
            QString path_PARSER;
            QString path_Salida;
            QString Nombre_archivo;
            QString ip_SERVER;

            // Archivos
            QString arch_recon;
            QString arch_ini;
            QString arch_sens;
            QString arch_countskimm;
            QString arch_recon_orig;

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
            double iteraciones;

            // Procedimientos
            bool mostrar;
            bool parsear;
            bool reconstruir;
            bool reconServer;
            bool reconMLEM;
            bool reconBackprojection;
            bool SensibilidadPrecalculada;
            bool AplicarCountSkimming;

            // Procesos
            QProcess *proceso;
            QPlainTextEdit *consola;
            QString *programas;
            QStringList *listasparametros;
            int indice_ejecucion = 0;
            int limite_ejecucion = 0;
            int indice_armado_cola = 0;
            bool muerto = 0;

            // Server
            QString SERVER_BASE;
            QString SERVER_ENCABEZADOS;
            QString SERVER_ENTRADAS;
            QString SERVER_SALIDAS;
            QString SERVER_APIRL;



    private slots:
            void on_readyRead();
            void updateError();
            void on_procesoExit(int flag_exit, QProcess::ExitStatus qt_exit);

    signals:
            void signal_finParser();            // Señal de fin de parsear, necesaria para reconstruir en server
            void signal_finReconstruccion();    // Señal de fin de reconstruccion, necesaria para reconstruir en server


    };






}


#endif // APRECON

