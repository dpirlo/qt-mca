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

#include <fstream>
#include <math.h>

#include <ctime>


// Valor maximo de movimiento de canal
#define MAX_MOV_DIN 250
// Valor base de movimiento de canal
#define BASE_MOV_DIN 50

// Memoria para mantener los 6 cabezales en RAM (Suponiendo un archivo de 1 GB de calibracion)
#define     HighMemDevice               10*6*1024*1024

// Definiciones para levantar archivos
#define     BYTES                       8
#define     CANTIDADdEpMTS              48
#define     CANTIDADdEcABEZALES         6
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
#define     BinsAlmohadon                       256
#define     NUM_EVENT_CENTRO                    2500
#define     PORC_ALETA                          100
#define     MAX_ITER_ENERGIA                    10

#define     PORCENTUAL_ENERGIA_VECINO           40

#define     PMTs_X                              8
#define     PMTs_Y                              6

#define     Lado_PMT                            53.5

#define     PMT_Recortado                       1

// Archivo de coincidencias parseado, posiciones en la matriz
#define     Coin_Calib_Iden_cabezal_1           2
#define     Coin_Calib_tiempos_1                3
#define     Coin_Calib_X_1                      4
#define     Coin_Calib_Y_1                      5
#define     Coin_Calib_Z_1                      6
#define     Coin_Calib_Energia_1                7
#define     Coin_Calib_Iden_cabezal_2           8
#define     Coin_Calib_tiempos_2                9
#define     Coin_Calib_X_2                      10
#define     Coin_Calib_Y_2                      11
#define     Coin_Calib_Z_2                      12
#define     Coin_Calib_Energia_2                13

using namespace arma;

namespace ap {

    // Retorno del buscador de pico
    struct Pico_espectro {
        double canal_pico;
        int limites_FWHM[2];
        int limites_FWTM[2];
        double FWHM;
        double FWTM;
        int limites_Pico[2];
    };

    // Retorno de la exploracion de tiempos entre PMT
    struct tiempos_recursiva {
        rowvec Correccion_Temporal_out;
        rowvec Corregido_out;
        rowvec Distancia_out;
    };


    // Parte del newton-gauss de juan
    typedef struct {
            double *Z0;
            double *Z0T;
            double *Z0C;
            double *Z0I;
            double *D;
            double *ZTD;
            double *A;
        }estadogna;


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
            void plot_MCA(QVector<double> hits, QVector<double> channels_ui, QCustomPlot *graph, QString graph_legend, int* param, bool clear);

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
            // Set de Count skimming
            void setCount_skimming() {this->Count_skim_calib = 1;}
            // Set de Count skimming inter-cabezal
            void setCount_skimming_total() {this->Count_skim_total_calib = 1;}
            // Set de lista de visuzalizaciones
            void setVis_List(QList<int> checked_Cab) {this->Vis_List = checked_Cab;}

            // Set de archivos de configuración
            void setAdq_Cab_1(string par_string) {this->adq_cab[0] = par_string;}
            void setAdq_Cab_2(string par_string) {this->adq_cab[1] = par_string;}
            void setAdq_Cab_3(string par_string) {this->adq_cab[2] = par_string;}
            void setAdq_Cab_4(string par_string) {this->adq_cab[3] = par_string;}
            void setAdq_Cab_5(string par_string) {this->adq_cab[4] = par_string;}
            void setAdq_Cab_6(string par_string) {this->adq_cab[5] = par_string;}
            void setAdq_Coin(string par_string) {this->adq_coin = par_string;}
            void setPathSalida(QString par_string) {this->path_salida = par_string;}
            void setPathEntrada(QString par_string) {this->path_entrada = par_string;}


            // Calibración fina
            bool calibrar_fina(void);

            // Visualizacion planar
            bool visualizar_planar(void);



        private:
            // Pedir MCA
            bool pedir_MCA_PMT(int Cabezal, int PMT, int canales, bool Calib);
            // Setear HV dinodo PMT
            bool modificar_HV_PMT(int Cabezal, int PMT,  int val_dinodo);
            // Reseteo memoria cabezal (SP6)
            bool reset_Mem_Cab(int Cabezal);

            // Datos calibracion
            QList<int> PMTs_List;
            QList<int> Cab_List;
            QList<int> Vis_List;
            int  Canal_Obj, tiempo_adq;

            // Puerto serie
            QString port_name;

            // Parametros del ploteo de espectros
            int param_cab[CANTIDADdEcABEZALES][6];

            // Ventanas emergentes
            QCustomPlot Espectro_emergente;
            QCustomPlot Espectro_emergente_crudo;
            QCustomPlot Espectro_PMT_emergente[CANTIDADdEcABEZALES];
            QLabel Almohadon_emergente[CANTIDADdEcABEZALES];
            QLabel Almohadon_emergente_skimmed[CANTIDADdEcABEZALES];


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
            string adq_cab[CANTIDADdEcABEZALES];
            string adq_coin;
            QString path_salida;
            QString path_entrada;

            // Archivos parseados
            mat Energia_calib[CANTIDADdEcABEZALES];
            mat Tiempos_calib[CANTIDADdEcABEZALES];
            mat Tiempos_full_calib[CANTIDADdEcABEZALES];
            mat TimeStamp_calib[CANTIDADdEcABEZALES];
            mat Tiempos_inter_cab;
            double Tiempo_medicion[CANTIDADdEcABEZALES];

            // Flag de count skimming
            bool Count_skim_calib = 0;

            // Flag de count skimming inter cabezal
            bool Count_skim_total_calib = 0;

            // Matriz de energia promedio en centroides
            mat E_prom_PMT[CANTIDADdEcABEZALES];

            // Coeficientes de energía
            double Ce[CANTIDADdEcABEZALES][CANTIDADdEbYTESpMTS];
            double Ce_pre[CANTIDADdEcABEZALES][CANTIDADdEbYTESpMTS];
            // Matriz de desvios de tiempos en centroide
            mat desv_temp_media_central[CANTIDADdEcABEZALES];
            // Coeficientes de Tiempo
            double Ct[CANTIDADdEcABEZALES][CANTIDADdEbYTESpMTS];
            // Coeficientes de posicion
            double Cx[CANTIDADdEcABEZALES][CANTIDADdEbYTESpMTS];
            double Cy[CANTIDADdEcABEZALES][CANTIDADdEbYTESpMTS];
            // Almohadones
            mat almohadon[CANTIDADdEcABEZALES];
            // Count skimmings
            mat count_skimm[CANTIDADdEcABEZALES];
            // Count skimmings inter cabezales
            mat count_skimm_inter_cab[CANTIDADdEcABEZALES];
            // Coeficientes inter cabezal
            double Ci[CANTIDADdEcABEZALES];

            // Preprocesamiento de datos planar
            bool preprocesar_info_planar(int cab_num_act, bool plotear);
            // Pre calibrar usando la aleta
            bool Pre_calibrar_aleta(int cab_num_act);
            // Calibración fina de eneregía
            bool calibrar_fina_energia(int cab_num_act);
            // Calibración fina de tiempos
            bool calibrar_fina_tiempos(int cab_num_act);
            // Calibración fina de posiciones
            bool calibrar_fina_posiciones(int cab_num_act);
            // Recursiva tiempo
            struct tiempos_recursiva tiempos_a_vecino(int PMT_Ref, rowvec Correccion_Temporal, rowvec Corregido, rowvec Distancia, mat desv_temp_max_hist );
            // Calibración tiempos inter-cabezal
            bool calibrar_tiempo_intercabezal();
            // Guardar tablas
            void guardar_tablas(int cab_num_act, bool* tipo);
            // Cargar tablas
            void cargar_tablas(int cab_num_act, bool* tipo);


            // Calcular alohadon
            bool calcular_almohadon(int cab_num_act);
            // Mostrar almohadon
            bool mostrar_almohadon(int cab_num_act, bool calib, bool skimm);

            // Calcular Count skimming
            bool calibrar_count_skimming(int cab_num_act);
            // Calcular Inter Count skimming
            bool calibrar_inter_count_skimming();

            // Mostrar espectro
            bool plotear_espectro(int cab_num_act,  bool calibrado);

            // Guardar vector de armadillo en archivo
            QString guardar_vector_stream(vec guardar);
            // Guardar matriz de armadillo en archivo
            QString guardar_matriz_stream(mat guardar);
            // Puntero al archivo de log
            QTextStream stream;


            // Levantar archivo de calibración
            bool LevantarArchivo_Planar(int cab_num_act);
            // Parseo de trama
            unsigned char * Trama(unsigned char *tramaEntrada,int tamanioTramaEntrada,int * tamanioTramaSalidaPointer);

            // Flag de RAM
            bool IsLowRAM;






            // Newton-Gauss de juan
            double f_gauss(double X,double *P);
            double df_gauss(double X, double *P, int nP);
            // -- Matrices
            double 	getelemento(double *matriz, int fila, int col, int nFila, int nCol);
            void 	setelemento(double *matriz, int fila, int col, int nFila, int nCol,double val);
            void 	imprimirMatriz(double *matriz, int nFilas, int nCols,int decimales,char formato);
            double*	llenarMatriz(double *matriz,int nFilas,int nCols, double val);
            double*	identidadMatriz(double *matriz,int n, double val);
            double*	productoporescalarMatriz(double *morg,int nFilas, int nCols, double *mdst, double val);
            double*	trasponerMatriz(double *morg,int nFilasOrg, int nColsOrg, double *mdst);
            double*	sumarMatrices(double *mop1,int nFilasOrg, int nColsOrg, double *mop2, double *mdst);
            double*	restarMatrices(double *mop1,int nFilasOrg, int nColsOrg, double *mop2, double *mdst);
            double*	multiplicarMatrices(double *mop1,int nFilasop1, int nColsop1, double *mop2, int nFilasop2, int nColsop2, double *mdst);
            double*	permutarMatriz(double *matriz,int fila1, int fila2, int nFilas, int nCols);
            double	determinante(double *matriz, int n);
            double*	matrizmenorMatriz(double *matriz, int nofila, int nocol, int nFilas, int nCols,double *mdst);
            double*	matrizInversa(double *matriz,int n, double *inversa);
            // -- GNA
            double sumacuadrados(double *x,  double *y, int ndatos, double *P,double (AutoCalib::*fx)(double X, double *P));
            void jacobiano(double *matriz, double *x, int ndatos, double *P, int nparam, double (AutoCalib::*dfx)(double X, double *P, int nP));
            int  inicializargna(estadogna *estado,int ndatos, int nparam);
            void liberargna(estadogna *estado);
            void acotarparametros(double *p, double *pmax, double *pmin, int nparam);
            int gaussnewton(	double	*x,
                        double	*y,
                        int 	ndatos,
                        double	*P,
                        int 	nparam,
                        double 	paso,
                        double 	(AutoCalib::*fx)(double X, double *P),
                        double (AutoCalib::*dfx)(double X, double *P,int nP),
                        estadogna *estado);

    };






}

#endif // AUTOCALIB_H
