#include "inc/apAutoCalib.hpp"

using namespace ap;


AutoCalib::AutoCalib()
{

    // Checkeo la RAM para geder...

    FILE *meminfo = fopen("/proc/meminfo", "r");

    int ram = 0;

    char line[256];
    while(fgets(line, sizeof(line), meminfo))
    {

        if(sscanf(line, "MemTotal: %d kB", &ram) == 1)
        {
            break;
        }
    }


    cout<<"Ram: "<<ram<<endl;

    if (ram < HighMemDevice)
    {
        IsLowRAM = 1;
        cout<<"No hay multi-calibracion para ti mi pequeño padawan..."<<endl;

    }
    else
    {
        IsLowRAM = 0;
        cout<<"Apa... ¿sos Copernico de casualidad?"<<endl;
    }


    // Preparo las estructuras de datos
    for(int i = 0 ; i < CANTIDADdEcABEZALES ; i ++)
    {
        E_prom_PMT[i].set_size(CANTIDADdEpMTS,CANTIDADdEpMTS);
        E_prom_PMT[i].zeros(CANTIDADdEpMTS,CANTIDADdEpMTS);
        desv_temp_media_central[i].set_size(CANTIDADdEpMTS,CANTIDADdEpMTS);
        desv_temp_media_central[i].zeros(CANTIDADdEpMTS,CANTIDADdEpMTS);
        almohadon[i].set_size(BinsAlmohadon,BinsAlmohadon);
        almohadon[i].zeros(BinsAlmohadon,BinsAlmohadon);
        count_skimm[i].set_size(BinsAlmohadon,BinsAlmohadon);
        count_skimm[i].zeros(BinsAlmohadon,BinsAlmohadon);
        count_skimm_inter_cab[i].set_size(BinsAlmohadon,BinsAlmohadon);
        count_skimm_inter_cab[i].zeros(BinsAlmohadon,BinsAlmohadon);


        param_cab[i][0]=rand()%245+10;//R
        param_cab[i][1]=rand()%245+10;//G
        param_cab[i][2]=rand()%245+10;//B
        param_cab[i][3]=rand()%5+1; //LineStyle
        param_cab[i][4]=rand()%14+1;//ScatterShape
        param_cab[i][5]=rand()/(double)RAND_MAX*2+1;//setWidthF
    }


    // Seteo el default de salida
    path_salida = "Salidas/";

    // Seteo el default de entrada
    path_entrada = "Salidas/";


    // Mando esto a la nada para evitar errores
    //QString nombre_Log = "/dev/null";
    //QFile file(nombre_Log);
    //stream.setDevice(&file);


}



bool AutoCalib::calibrar_simple(QCustomPlot* plot_hand)
{
    portConnect(port_name.toStdString().c_str());

    // Parametros del ploteo
    int param[6];
    param[0]=0;//R
    param[1]=61;//G
    param[2]=245;//B
    param[3]=5+1; //LineStyle
    param[4]=14+1;//ScatterShape
    param[5]=1/(double)RAND_MAX*2+1;//setWidthF

    // Paso inicial del hv dinodo
    int paso_dinodo[PMTs];
    fill_n(paso_dinodo, PMTs, BASE_MOV_DIN);

    // Armo el vector de canal objetivo
    double Canal_Obj_vec[PMTs];
    fill_n(Canal_Obj_vec, PMTs, Canal_Obj);
    double Canal_Obj_dif[PMTs];

    // Memoria del paso previo
    double Picos_PMT_ant[PMTs];
    double Dinodos_PMT_ant[PMTs];

    // Parametros de linealización de paso
    double A_param[PMTs];
    double B_param[PMTs];



    int PMT_index = 0;
    int Cab_index = 0;

    int PMT_actual = PMTs_List[PMT_index];
    int Cab_actual = Cab_List[Cab_index];

    QString nombre_plot;
    nombre_plot = "PMT "+ QString::number(PMT_actual);




    // Loop de calibracion
    int iter_actual = 0;
    while(1)
    {


        // Borro memoria de la clase
        for (int j=0 ; j < PMTs ; j++)
        {
            for (int i=0 ; i < CHANNELS ; i++)
            {
                Acum_PMT[j][i] = 0;
            }
            Picos_PMT[j] = 0;
        }

        // Reseteo la memoria de SP6
        if (!reset_Mem_Cab(Cab_actual)) return -1;


        // Espero el tiempo indicado
        sleep(tiempo_adq);

        // LLeno los buffers de memoria de la clase
        for (int i=0 ; i<PMTs ; i++)
        //for (int i=0 ; i<4 ; i++)
        //int i = PMT_actual-1;
        {

            // Pido MCA de calibracion del PMT actual
            if (!pedir_MCA_PMT(Cab_actual , i+1, CHANNELS, 1)) return -1;

            // Leo los hits y los paso a double
            QVector<double> aux_hits;
            aux_hits = getHitsMCA();
            double aux_double_hits[CHANNELS];
            for (int j = 0 ; j < CHANNELS ; j++)
            {
              aux_double_hits[j] = aux_hits[j];
            }

            // Acumulo en mi memoria
            for (int j=0 ; j < CHANNELS ; j++)
            {
                Acum_PMT[i][j] = Acum_PMT[i][j] +  aux_double_hits[j];
            };

            // Busco el pico
            struct Pico_espectro aux;
            aux = Buscar_Pico(Acum_PMT[i], CHANNELS);
            Picos_PMT[i] = aux.canal_pico;


            /*
            // Paso a Qvector y ploteo
            std::vector<double> auxVector;
            auxVector.assign(Acum_PMT[i], Acum_PMT[i] + CHANNELS);
            aux_hits.fromStdVector(auxVector);
            //plot_MCA(aux_hits, plot_hand , nombre_plot, param);
            plot_MCA(getHitsMCA(), plot_hand , nombre_plot, param);
            */


            // Leo el HV actual y lo guardo en memoria
            if (!pedir_MCA_PMT(Cab_actual , i+1, 256, 0)) return -1;
            Dinodos_PMT[i] = getHVMCA();

            cout<<"PMT: "<< i+1 << "--" <<Picos_PMT[i]<<"---"<<Dinodos_PMT[i]<<endl;

        }


        // Comparo la posición actual con la objetivo
        // usando armadillo
        mat Canal_Obj_vec_arma(Canal_Obj_vec, PMTs, 1);
        mat Canal_Obj_dif_arma(Canal_Obj_dif, PMTs, 1);
        mat Picos_PMT_arma(Picos_PMT, PMTs, 1);

        // Diferencia
        Canal_Obj_dif_arma = (Canal_Obj_vec_arma - Picos_PMT_arma);
        // Diferencia cuadratica
        mat Canal_Obj_dif_arma_cuad = Canal_Obj_dif_arma % Canal_Obj_dif_arma;


        // Ignoro los PMT no seleccionados
        bool adentro = false;
        for (int j = 0 ;j < PMTs ; j++)
        {
            adentro = false;
            for(int i = 0 ; i < PMTs_List.length() ; i++)
            {
                if ((PMTs_List[i]-1) == j)
                {
                    adentro = true;
                }
            }
            if (!adentro)
            {
                Canal_Obj_dif_arma_cuad[j] = 0;
                Canal_Obj_dif_arma[j] = 0;
                Picos_PMT[j] == -1;
            }
        }



        // Calculo el step linealizando

        if (iter_actual > 0 )
        {
            for (int j = 0 ; j < PMTs ; j++)
            {

                // Si falle al encontrar el pico, no me muevo
                if (Picos_PMT[j] == -1)
                {
                    paso_dinodo[j] = 0;
                }
                else
                {
                    //Calculo la recta entre picos

                    A_param[j] = (Picos_PMT_ant[j] - Picos_PMT[j])/(Dinodos_PMT_ant[j]-Dinodos_PMT[j]);
                    B_param[j] = Picos_PMT[j] - A_param[j] * Dinodos_PMT[j];

                    // Checkeo que debido a ruido en la posicion del pico no este haciendo fruta
                    if (A_param[j] > 0 && A_param[j] < 100000)
                    {

                        // Calculo la posicion final del dinodo para llegar al objetivo
                        paso_dinodo[j] = Dinodos_PMT[j] - ( (Canal_Obj_vec_arma[j] - B_param[j])/ A_param[j] );

                        // Peso el valor del dinodo con el coeficiente de PMT centroide a total de energia
                        paso_dinodo[j] = paso_dinodo[j] * 0.65;

                        // Checkeo que debido a una pendiente muy baja no me mande al diablo y saturo
                        if (paso_dinodo[j]*paso_dinodo[j] > MAX_MOV_DIN*MAX_MOV_DIN)
                        {
                            if (paso_dinodo[j] < 0)
                            {
                                paso_dinodo[j] = -MAX_MOV_DIN;
                            }
                            else
                            {
                                paso_dinodo[j] = MAX_MOV_DIN;
                            }
                        }
                    }
                    else
                    {
                        if (Canal_Obj_dif_arma[j] > 0)
                        {
                            paso_dinodo[j] = BASE_MOV_DIN;
                        }
                        else if (Canal_Obj_dif_arma[j] < 0)
                        {
                            paso_dinodo[j] = -BASE_MOV_DIN;
                        }
                        else
                        {
                            paso_dinodo[j] = 0;
                        }
                    }
                }
            }


        }



        // Puntero al color del paso actual
        const double *color_tablero;


        /*
        cout<<paso_dinodo[0]<<" - "<<paso_dinodo[1]<<" - "<<paso_dinodo[2]<<" - "<<paso_dinodo[3]<<endl;

        for(int i=0; i<Canal_Obj_dif_arma_cuad.size();i++) { cout<<Canal_Obj_dif_arma_cuad[i]<<","; }
        cout<<endl;
        for(int i=0; i<Canal_Obj_dif_arma.size();i++) { cout<<Canal_Obj_dif_arma[i]<<","; }
        cout<<endl;
        for(int i=0; i<Picos_PMT_arma.size();i++) { cout<<Picos_PMT_arma[i]<<","; }
        cout<<endl;

        /*


        // --- Modo tablero de ajedrez
        /*
        // Busco el que está mas lejamo
        int ind_max_dif = Canal_Obj_dif_arma_cuad.index_max();
        cout<<ind_max_dif<<endl;


        // Paso los colores a matrices
        mat blancas(weisse, PMTs/2, 1);
        mat negras(schwarze, PMTs/2, 1);

        // Me fijo en que color quedo el más lejano
        uvec fins_salida = find(blancas == (ind_max_dif));
        for(int i=0; i<fins_salida.size();i++) { cout<<fins_salida(i)<<","; }
        cout<<endl;


        if (sum(fins_salida) > 0)
        {
            cout<<"blancas.."<<endl;
            color_tablero = weisse;
        }
        else
        {
            cout<<"negras.."<<endl;
            color_tablero = schwarze;
        }
        */

        // Busco el de mayor diferencia
        int ind_max_dif = -1;
        double aux_maximo= 0;
        for (int i=0 ; i < Canal_Obj_dif_arma_cuad.size() ; i++)
        {
            if (Canal_Obj_dif_arma_cuad[i] > aux_maximo)
            {
                ind_max_dif = i;
                aux_maximo = Canal_Obj_dif_arma_cuad[i];
            }
        }
        cout<<ind_max_dif<<endl;

        // Busco el color
        for (int i = 0 ; i < PMTs/2 ; i++)
        {
          if (weisse[i] == ind_max_dif+1)
          {
            cout<<"blancas.."<<endl;
            color_tablero = weisse;
          }
        }
        for (int i = 0 ; i < PMTs/2 ; i++)
        {
          if (schwarze[i] == ind_max_dif+1)
          {
            cout<<"negras.."<<endl;
            color_tablero = schwarze;
          }
        }


        // Recorro y modifico todos los PMT del color
        for (int i = 0 ; i < PMTs/2 ; i++)
        {
            int PMT_mover = color_tablero[i]-1;

            if (iter_actual == 0)
            {
                if (Canal_Obj_dif_arma(PMT_mover) > 0)
                {
                    cout<< "Subiendo PMT "<<PMT_mover+1<<" a "<< (Dinodos_PMT[PMT_mover] + paso_dinodo[PMT_mover])<<endl;
                    if (!modificar_HV_PMT(Cab_actual , PMT_mover+1, (Dinodos_PMT[PMT_mover] + paso_dinodo[PMT_mover]))) return -1;
                }
                else
                {
                    cout<< "Bajando PMT "<<PMT_mover+1<<" a "<< (Dinodos_PMT[PMT_mover] - paso_dinodo[PMT_mover])<<endl;
                    if (!modificar_HV_PMT(Cab_actual , PMT_mover+1, Dinodos_PMT[PMT_mover] - paso_dinodo[PMT_mover])) return -1;
                }
            }
            else
            {
                cout<< "Modificando PMT "<<PMT_mover+1<<" a "<< (Dinodos_PMT[PMT_mover] + paso_dinodo[PMT_mover])<<endl;
                if (!modificar_HV_PMT(Cab_actual , PMT_mover+1, (Dinodos_PMT[PMT_mover] + paso_dinodo[PMT_mover]))) return -1;
            }

        }


       // paso_dinodo = paso_dinodo - (paso_dinodo/10);




        for (int i=0 ; i< PMTs ; i++)
        {
            Picos_PMT_ant[i] =  Picos_PMT[i];
            Dinodos_PMT_ant[i] = Dinodos_PMT[i];
        }

        iter_actual++;


        // Pido el total del cabezal y ploteo
        //plot_MCA(getHitsMCA(), plot_hand , nombre_plot, param);



        if (!pedir_MCA_PMT(Cab_actual , 1, CHANNELS, 1)) return -1;
        plot_MCA(getHitsMCA(), getChannels(), plot_hand , nombre_plot, param, 1);




        //cout << "Enviando a cabezal "<<Cab_actual<<" PMT "<<PMT_actual<<endl;










        // Seteo HV de dinodo
        //modificar_HV_PMT(Cab_actual , PMT_actual, 600+10);






        // Ploteo
        //plot_MCA(getHitsMCA(), plot_hand , nombre_plot, param);

    }
    portDisconnect();

    return 1;
}






/* -------------------------------------------------------------------
 * --------------------Calibración Fina-------------------------------
 * -------------------------------------------------------------------
 */


bool AutoCalib::calibrar_fina(void)
{
    // Para todos los cabezales que se seleccionaron
    for(int i = 0 ; i < Cab_List.length() ; i++)
    {

        cout<<Cab_List[i]<<endl;
        // Si es un cabezal lo calibro
        if (Cab_List[i] != 7)
        {
            // Para count skimming
            int cant_archivos = 1;

            // Convierto de numero de cabezal a indce (-1)
            int cab_num_act = Cab_List[i]-1;

            // Abro un archivo de log
            QString nombre_cab = QString::number(cab_num_act+1);
            time_t rawtime;
            struct tm * timeinfo;
            char buffer[80];
            time (&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(buffer,sizeof(buffer),"%d_%m_%Y_%I_%M_%S",timeinfo);
            std::string str(buffer);
            //QString nombre_Log = path_salida+"Log_Cabezal_"+nombre_cab+"_"+QString::fromStdString( str )+".txt";
            QString nombre_Log = path_salida+"Log_Cabezal_"+nombre_cab+".txt";
            cout<<"Creando log: "<<nombre_Log.toStdString()<<endl;
            QFile file(nombre_Log);
            if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
            {
                cout<<"Error al abrir log"<<endl;
                return -1;
            }

            //QTextStream stream_open(&file);
            stream.setDevice(&file);

            stream<<"Log iniciado: "<<asctime(timeinfo)<<endl;

            string nombre_archivo = adq_cab[cab_num_act];
            string path_archivo = nombre_archivo.substr(0, nombre_archivo.find_last_of("\\/"));
            QDir directorio(QString::fromStdString(path_archivo));
            QStringList todos_archivos = directorio.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);//(QDir::Filter::Files,QDir::SortFlag::NoSort)
            if(Count_skim_calib)
            {

                stream<<"ACHTUNG!!!!        Modo de count skimming, se van a utilizar todos los archivos de la carpeta."<<endl;
                cant_archivos = todos_archivos.length();
                stream<<"Se leeran "<<cant_archivos<<" archivos"<<endl;
            }


            // Cargo el cabezal actual en memoria
            LevantarArchivo_Planar(cab_num_act);

            // Busco eventos promedio y calculo la posición del pico
            preprocesar_info_planar(cab_num_act, 1);

            // Calculo un paso previo de calibración donde ajusto el espectro individual del PMT centroide
            Pre_calibrar_aleta(cab_num_act);

            // Calibro energía
            calibrar_fina_energia(cab_num_act);

            // Calibro en tiempos
            calibrar_fina_tiempos(cab_num_act);

            // Calibro en posicion
            calibrar_fina_posiciones(cab_num_act);

            // Libero la memoria del cabezal actual
            if (IsLowRAM)
            {
                // No se como borrar asi que los transformo en una matriz de un elemento
                Energia_calib[cab_num_act].set_size(1, 1);
                Tiempos_calib[cab_num_act].set_size(1, 1);
                Tiempos_full_calib[cab_num_act].set_size(1, 1);
                TimeStamp_calib[cab_num_act].set_size(1, 1);
            }

            if(Count_skim_calib)
            {
                for (int skim_i = 0 ; skim_i < cant_archivos ;  skim_i++)
                {
                    stream<<"----------------Calculando planares "<<skim_i+1<<" de "<<cant_archivos<<endl;
                    cout<<"----------------Calculando planares "<<skim_i+1<<" de "<<cant_archivos<<endl;

                    if(Count_skim_calib)
                    {
                        adq_cab[cab_num_act] = path_archivo;
                        adq_cab[cab_num_act].append("/");
                        adq_cab[cab_num_act].append(todos_archivos.at(skim_i).toStdString());
                    }

                    // Cargo el cabezal actual en memoria
                    LevantarArchivo_Planar(cab_num_act);

                    // Busco eventos promedio y calculo la posición del pico
                    preprocesar_info_planar(cab_num_act, 0);


                    calcular_almohadon(cab_num_act);

                    // No se como borrar asi que los transformo en una matriz de un elemento
                    Energia_calib[cab_num_act].set_size(1, 1);
                    Tiempos_calib[cab_num_act].set_size(1, 1);
                    Tiempos_full_calib[cab_num_act].set_size(1, 1);
                    TimeStamp_calib[cab_num_act].set_size(1, 1);
                }

                // Calibro el count skimming
                calibrar_count_skimming(cab_num_act);

                // Muestro el nuevo almohadon
                mostrar_almohadon(cab_num_act,1,1);



            }

            // Guardo
            bool tipo[5] = {1,1,1,0,0};
            if(Count_skim_calib) tipo[3] = 1;
            guardar_tablas(cab_num_act, tipo);

            stream<<"Log finalizado: "<<asctime(timeinfo)<<endl;

        }
        // Sino calibro tiempos
        else
        {
            // Abro un archivo de log
            time_t rawtime;
            struct tm * timeinfo;
            char buffer[80];
            time (&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(buffer,sizeof(buffer),"%d_%m_%Y_%I_%M_%S",timeinfo);
            std::string str(buffer);
            QString nombre_Log = path_salida+"Log_Coincidencia.txt";
            QFile file(nombre_Log);
            if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
            {
                cout<<"Error al abrir log"<<endl;
                return -1;
            }

            stream.setDevice(&file);

            stream<<"Log iniciado: "<<asctime(timeinfo)<<endl;

            // Calibro
            calibrar_tiempo_intercabezal();

            stream<<"Log finalizado: "<<asctime(timeinfo)<<endl;
        }

    }

    if (Count_skim_total_calib && Count_skim_calib)
    {
        // Abro un archivo de log
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];
        time (&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer,sizeof(buffer),"%d_%m_%Y_%I_%M_%S",timeinfo);
        std::string str(buffer);
        QString nombre_Log = path_salida+"Log_Inter_count_skimming.txt";
        QFile file(nombre_Log);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
            cout<<"Error al abrir log"<<endl;
            return -1;
        }

        stream<<"Log iniciado: "<<asctime(timeinfo)<<endl;

        // Aplico el inter skimming
        calibrar_inter_count_skimming();

        bool tipo[5] = {0,0,0,0,1};
        for (int cab_num_act = 0 ; cab_num_act < CANTIDADdEcABEZALES ; cab_num_act++)
            guardar_tablas(cab_num_act, tipo);

        stream<<"Log finalizado: "<<asctime(timeinfo)<<endl;

    }




}


bool AutoCalib::preprocesar_info_planar(int cab_num_act,  bool plotear)
{
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------INICIO DE: preprocesar_info_planar ------------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;

    // Saco la suma de los canales del evento
    rowvec Suma_canales = sum( Energia_calib[cab_num_act], 0);

    // Logueo numero de eventos totales
    stream<<"Eventos limpios: "<<Suma_canales.n_elem<<endl;

    // Creo el vector de centros para el histograma
    vec centros_hist = linspace<vec>(0,8000,BinsHist);

    // Calculo el histograma
    urowvec espectro_suma_crudo = hist(Suma_canales, centros_hist);




    // Calculo el FWHM
    struct Pico_espectro pico_sin_calib;
    double aux_espectro[BinsHist];
    for (int i=0 ; i < BinsHist ; i++)
    {
        aux_espectro[i] = espectro_suma_crudo(i);
    }
    pico_sin_calib = Buscar_Pico(aux_espectro, BinsHist);

    cout<<"Sin Calibrar:"<<endl;
    cout<<"FWHM: "<<pico_sin_calib.FWHM*100<<" %"<<endl;

    stream<<"FWHM sin Calibrar: "<<pico_sin_calib.FWHM*100<<"%"<<endl;
    stream<<"   Datos pico: "<<endl;
    stream<<"   FWHM: "<<pico_sin_calib.FWHM*100<<" % - "<<centros_hist(pico_sin_calib.limites_FWHM[0])<<" ; "<<centros_hist(pico_sin_calib.limites_FWHM[1])<<endl;
    stream<<"   FWTM: "<<pico_sin_calib.FWTM*100<<" % - "<<centros_hist(pico_sin_calib.limites_FWTM[0])<<" ; "<<centros_hist(pico_sin_calib.limites_FWTM[1])<<endl;
    stream<<"   Canal pico: "<<centros_hist(pico_sin_calib.canal_pico)<<endl;
    vec vec_log = arma::conv_to<vec>::from(espectro_suma_crudo);
    stream<<"   Espect_Crudo_Vec ="<<guardar_vector_stream(vec_log)<<endl;


    // ----------------------- Ploteo
    // Paso los vectores a Qvector para plotear
    QVector<double> aux_qvec_cent(BinsHist);
    for (int i=0 ; i < BinsHist ; i++){aux_qvec_cent[i] = centros_hist(i);}
    QVector<double> aux_qvec(BinsHist);
    for (int i=0 ; i < BinsHist ; i++){aux_qvec[i] = espectro_suma_crudo(i);}
    QString nombre_plot;
    nombre_plot = "Espectro crudo cabezal "+ QString::number(cab_num_act+1)+" FWHM = "+ QString::number(pico_sin_calib.FWHM*100) + "%";
    // Parametros del ploteo
    int param[6];
    if (plotear)
    {
        plot_MCA(aux_qvec, aux_qvec_cent,&Espectro_emergente_crudo, nombre_plot, param_cab[cab_num_act], 0);
        Espectro_emergente_crudo.show();
        Espectro_emergente_crudo.resize(1000,500);
    }
    qApp->processEvents();



    // Conservo solo los eventos dentro del FWTM
    mat Energia_calib_FWHM;
    mat Tiempo_calib_FWHM;
    uvec indices_aux = find(Suma_canales > centros_hist(pico_sin_calib.limites_FWTM[0]));
    rowvec suma_aux = Suma_canales.elem(indices_aux).t();
    Energia_calib_FWHM = Energia_calib[cab_num_act].cols(indices_aux);
    Tiempo_calib_FWHM = Tiempos_full_calib[cab_num_act].cols(indices_aux);
    indices_aux = find(suma_aux < centros_hist(pico_sin_calib.limites_FWTM[1]));
    suma_aux = suma_aux.elem(indices_aux).t();
    Energia_calib_FWHM = Energia_calib_FWHM.cols(indices_aux);
    Tiempo_calib_FWHM = Tiempo_calib_FWHM.cols(indices_aux);


    stream<<"Eventos en FWTM: "<<Energia_calib_FWHM.n_cols<<endl;
    rowvec suma_FWTM = sum( Energia_calib_FWHM, 0);
    vec_log.set_size(0,0);
    vec_log = arma::conv_to<vec>::from(hist(suma_FWTM, centros_hist));
    stream<<"   FWTM_Vec = "<<guardar_vector_stream(vec_log)<<endl;





    double canal_norm[CANTIDADdEpMTS];


    urowvec indices_maximo_PMT;
    mat Eventos_max_PMT;
    mat Tiempos_max_PMT;
    rowvec Fila_max_PMT;
    double maximo_abs_PMT;
    double limite_actual;
    int eventos_centroide;
    vec Tasa_PMT(CANTIDADdEpMTS);
    vec Porcentaje_PMT(CANTIDADdEpMTS);
    // Busco los elementos centroides de cada PMT
    for (int index_PMT_cent = 0 ; index_PMT_cent < CANTIDADdEpMTS ; index_PMT_cent ++)
    {
        stream<<endl;
        stream<<"---------------------- EVENTOS POR PMT ------------------------------"<<endl;
        stream<<endl;

        // Extraigo los eventos en los cuales el PMT fue maximo
        indices_maximo_PMT = index_max( Energia_calib_FWHM, 0 );
        //cout<<indices_maximo_PMT.n_elem<<endl;
        indices_aux = find(indices_maximo_PMT == index_PMT_cent);
        //cout<<indices_aux.n_elem<<endl;
        Eventos_max_PMT =  Energia_calib_FWHM.cols(indices_aux);
        Fila_max_PMT = Eventos_max_PMT.row(index_PMT_cent);


        Porcentaje_PMT(index_PMT_cent) = ((double)Fila_max_PMT.n_elem/(double)Energia_calib_FWHM.n_cols)*100;
        Tasa_PMT(index_PMT_cent) = Fila_max_PMT.n_elem / Tiempo_medicion[cab_num_act];

        stream<<"       Eventos maximo PMT "<<index_PMT_cent+1<<": "<<Fila_max_PMT.n_elem<<"   ---   "<< Porcentaje_PMT(index_PMT_cent) <<" % del total en FWHM"<<endl;
        centros_hist = linspace<vec>(0000,2000,BinsHist);
        vec_log.set_size(0,0);
        vec_log = arma::conv_to<vec>::from(hist(Fila_max_PMT, centros_hist));
        stream<<"       Eventos_maximo_PMT_"<<index_PMT_cent+1<<"_Vec ="<<guardar_vector_stream(vec_log)<<endl;
        centros_hist = linspace<vec>(0,8000,BinsHist);
        vec_log.set_size(0,0);
        rowvec suma_log = sum( Eventos_max_PMT, 0);
        vec_log = arma::conv_to<vec>::from(hist(suma_log, centros_hist));
        stream<<"       Eventos_maximo_Suma_PMT_"<<index_PMT_cent+1<<"_Vec = "<<guardar_vector_stream(vec_log)<<endl;


        //Saco tiempos
        Tiempos_max_PMT =  Tiempo_calib_FWHM.cols(indices_aux);


        // Calculo el maximo valor de energia encontrado en este subset
        maximo_abs_PMT = Fila_max_PMT.max();
        limite_actual = maximo_abs_PMT;


        // Itero hasta conseguir la cantidad deseada
        eventos_centroide = 0;

        while (eventos_centroide < NUM_EVENT_CENTRO)
        {
            // Cuento cuantos eventos encontre hasta el punto actual
            indices_aux = find(Fila_max_PMT > limite_actual);
            eventos_centroide = indices_aux.n_elem;


            // Actualizo el limite
            limite_actual = limite_actual - (maximo_abs_PMT*0.01);

        }

        // Me quedo con los eventos en el centroide
        Eventos_max_PMT =  Eventos_max_PMT.cols(indices_aux);
        Tiempos_max_PMT =  Tiempos_max_PMT.cols(indices_aux);


        // Color y marker random
        param[0]=rand()%245+10;//R
        param[1]=rand()%245+10;//G
        param[2]=rand()%245+10;//B
        param[3]=rand()%5+1; //LineStyle
        param[4]=rand()%14+1;//ScatterShape
        param[5]=rand()/(double)RAND_MAX*2+1;//setWidthF

        centros_hist = linspace<vec>(0,8000,BinsHist);

        // Ploteo el histograma de suma para este PMT
        suma_aux = sum( Eventos_max_PMT,  0);
        espectro_suma_crudo = hist(suma_aux, centros_hist);
        // ----------------------- Ploteo
        // Paso los vectores a Qvector para plotear
        for (int i=0 ; i < BinsHist ; i++){aux_qvec_cent[i] = centros_hist(i);}
        for (int i=0 ; i < BinsHist ; i++){aux_qvec[i] = espectro_suma_crudo(i);}
        nombre_plot = "PMT Nº "+ QString::number(index_PMT_cent+1);
        if (plotear)
        {
            plot_MCA(aux_qvec, aux_qvec_cent,&Espectro_PMT_emergente[cab_num_act], nombre_plot, param, 0);
            Espectro_PMT_emergente[cab_num_act].show();
            Espectro_PMT_emergente[cab_num_act].resize(1000,500);
        }
        qApp->processEvents();


        stream<<"       --------Centroides usados: "<<indices_aux.n_elem<<endl;
        centros_hist = linspace<vec>(0000,2000,BinsHist);
        vec_log.set_size(0,0);
        vec_log = arma::conv_to<vec>::from(hist(Eventos_max_PMT.row(index_PMT_cent),centros_hist));
        stream<<"               Aleta_PMT_"<<index_PMT_cent+1<<"_Vec ="<<guardar_vector_stream(vec_log)<<endl;
        centros_hist = linspace<vec>(0,8000,BinsHist);
        vec_log.set_size(0,0);
        vec_log = arma::conv_to<vec>::from(espectro_suma_crudo);
        stream<<"               Aleta_Suma_PMT_"<<index_PMT_cent+1<<"_Vec ="<<guardar_vector_stream(vec_log)<<endl;



        // Calculo la energia promedio de todos los PMT para este centroide
        E_prom_PMT[cab_num_act].row(index_PMT_cent) = mean(Eventos_max_PMT,1).t();


        // Calculo la diferencia de tiempo entre el PMT actual y todo el resto.
        for (int i = 0 ; i < CANTIDADdEpMTS ; i ++)
        {
            // Le resto a todos los PMT la referencia actual
            rowvec dist_aux = Tiempos_max_PMT.row(i)-Tiempos_max_PMT.row(index_PMT_cent);

            // Me quedo solo con los eventos con energía superior a una fracción del pico medio observado
            double porc_ener_aux = PORCENTUAL_ENERGIA_VECINO;
            uvec indices_keep = find(Eventos_max_PMT.row(i) >= (porc_ener_aux/100)*mean(Eventos_max_PMT.row(index_PMT_cent)) );

            // Calculo la media del mismo
            double desv_temp_media = mean(dist_aux);

            // Calculo el desvio
            double desv_temp_std = stddev(dist_aux);

            if (indices_keep.n_elem > 0)
            {
                /*
                // Un poco de integridad estadistica (TEST)
                if (   ((desv_temp_std/sqrt(dist_aux.n_elem))/desv_temp_media) > 0.5    )
                    desv_temp_media_central[cab_num_act](i,index_PMT_cent) = mean(dist_aux.elem(indices_keep));
                else
                    desv_temp_media_central[cab_num_act](i,index_PMT_cent) = datum::nan;
                */
                desv_temp_media_central[cab_num_act](i,index_PMT_cent) = mean(dist_aux.elem(indices_keep));

            }
            else
            {
                desv_temp_media_central[cab_num_act](i,index_PMT_cent) = datum::nan;
            }


        }

        //desv_temp_media_central[cab_num_act].row(index_PMT_cent) = mean(Tiempos_max_PMT,1).t();





        // TEST
        // Calculo el Ce de normalizacion de aleta
        // Busco el cruze de la aleta con un punto al 2% del numero de cuentas necesarias
        centros_hist = linspace<vec>(0000,2000,BinsHist);
        espectro_suma_crudo = hist(Eventos_max_PMT.row(index_PMT_cent), centros_hist);

        espectro_suma_crudo = (espectro_suma_crudo - (NUM_EVENT_CENTRO/PORC_ALETA));
        espectro_suma_crudo %= espectro_suma_crudo;
        canal_norm[index_PMT_cent] = espectro_suma_crudo.index_min();

    }

    // Guardo las tasas
    stream<<"               Porcentaje_PMT = "<<guardar_vector_stream(Porcentaje_PMT)<<endl;
    stream<<"               Tasa_PMT = "<<guardar_vector_stream(Tasa_PMT)<<endl;



    stream<<"               Mat_prom_inicial = "<<guardar_matriz_stream(E_prom_PMT[cab_num_act])<<endl;
    stream<<"               Mat_tiempos_inicial = "<< guardar_matriz_stream(desv_temp_media_central[cab_num_act])<<endl;
    stream<<"               Canales_a_normalizar = ["<< canal_norm[0];
    for (int i_log = 1 ; i_log < CANTIDADdEpMTS ; i_log++) stream<<" , "<< canal_norm[i_log];
    stream<<"];"<<endl;



    // TEST
    // Calculo el Ce loco
    double canal_medio_zona[3] = {0, 0, 0};
    for (int index_PMT_cent = 0 ; index_PMT_cent < CANTIDADdEpMTS ; index_PMT_cent ++)
    {
        // Esquina
        if ( (index_PMT_cent == 0 || index_PMT_cent == 7 || index_PMT_cent == 40 || index_PMT_cent == 47 ) )
        {
            canal_medio_zona[0] += canal_norm[index_PMT_cent]/4;
        }
        // Borde
        else if (    (index_PMT_cent == 1 || index_PMT_cent == 2 || index_PMT_cent == 3 || index_PMT_cent == 4 || index_PMT_cent == 5 || index_PMT_cent == 6||
                index_PMT_cent == 8 || index_PMT_cent == 16 || index_PMT_cent == 24 || index_PMT_cent == 32 ||
                index_PMT_cent == 15 || index_PMT_cent == 23 || index_PMT_cent == 31 || index_PMT_cent == 39||
                index_PMT_cent == 41 || index_PMT_cent == 42 || index_PMT_cent == 43 || index_PMT_cent == 44 || index_PMT_cent == 45 || index_PMT_cent == 46) )
        {
            canal_medio_zona[1] += canal_norm[index_PMT_cent]/20;
        }
        // Centro
        else
        {
            canal_medio_zona[2] += canal_norm[index_PMT_cent]/24;
        }
    }
    stream<<"Canales Normalizados: "<<canal_medio_zona[0]<<" - "<<canal_medio_zona[1]<<" - "<<canal_medio_zona[2]<<endl;
    for (int index_PMT_cent = 0 ; index_PMT_cent < CANTIDADdEpMTS ; index_PMT_cent ++)
    {
        // Esquina
        if ( (index_PMT_cent == 0 || index_PMT_cent == 7 || index_PMT_cent == 40 || index_PMT_cent == 47 ) )
        {
            Ce_pre[cab_num_act][index_PMT_cent] = canal_medio_zona[0] / canal_norm[index_PMT_cent];
        }
        // Borde
        else if (    (index_PMT_cent == 1 || index_PMT_cent == 2 || index_PMT_cent == 3 || index_PMT_cent == 4 || index_PMT_cent == 5 || index_PMT_cent == 6||
                index_PMT_cent == 8 || index_PMT_cent == 16 || index_PMT_cent == 24 || index_PMT_cent == 32 ||
                index_PMT_cent == 15 || index_PMT_cent == 23 || index_PMT_cent == 31 || index_PMT_cent == 39||
                index_PMT_cent == 41 || index_PMT_cent == 42 || index_PMT_cent == 43 || index_PMT_cent == 44 || index_PMT_cent == 45 || index_PMT_cent == 46) )
        {
            Ce_pre[cab_num_act][index_PMT_cent] = canal_medio_zona[1] / canal_norm[index_PMT_cent];
        }
        // Centro
        else
        {
            Ce_pre[cab_num_act][index_PMT_cent] = canal_medio_zona[2] / canal_norm[index_PMT_cent];
        }

    }

    stream<<"               Ce_inicial = ["<< Ce_pre[cab_num_act][0];
    for (int i_log = 1 ; i_log < CANTIDADdEpMTS ; i_log++) stream<<" , "<< Ce_pre[cab_num_act][i_log];
    stream<<"];"<<endl;

    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------SALIDA DE: preprocesar_info_planar ------------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
}


bool AutoCalib::Pre_calibrar_aleta(int cab_num_act)
{

    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------INICIO DE: Pre_calibrar_aleta ------------------------------------------------------------ "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;


    colvec Ce_arma(Ce_pre[cab_num_act], CANTIDADdEpMTS);

    // Paso toda la matriz de eventos a energia calibrada
    mat Energia_calib_aux = Energia_calib[cab_num_act];
    Energia_calib_aux.each_col() %= Ce_arma;



    // Saco la suma de los canales del evento
    rowvec Suma_canales = sum( Energia_calib_aux, 0);


    // Creo el vector de centros para el histograma
    vec centros_hist = linspace<vec>(0,8000,BinsHist);

    // Calculo el histograma
    urowvec espectro_suma_crudo = hist(Suma_canales, centros_hist);


    // Calculo el FWHM
    struct Pico_espectro pico_sin_calib;
    double aux_espectro[BinsHist];
    for (int i=0 ; i < BinsHist ; i++)
    {
        aux_espectro[i] = espectro_suma_crudo(i);
    }
    pico_sin_calib = Buscar_Pico(aux_espectro, BinsHist);

    cout<<"Pre Calibrado:"<<endl;
    cout<<"FWHM: "<<pico_sin_calib.FWHM*100<<"%"<<endl;


    stream<<"FWHM pre calibrado: "<<pico_sin_calib.FWHM*100<<"%"<<endl;
    stream<<"   Datos pico: "<<endl;
    stream<<"   FWHM: "<<pico_sin_calib.FWHM*100<<" % - "<<centros_hist(pico_sin_calib.limites_FWHM[0])<<" ; "<<centros_hist(pico_sin_calib.limites_FWHM[1])<<endl;
    stream<<"   FWTM: "<<pico_sin_calib.FWTM*100<<" % - "<<centros_hist(pico_sin_calib.limites_FWTM[0])<<" ; "<<centros_hist(pico_sin_calib.limites_FWTM[1])<<endl;
    stream<<"   Canal pico: "<<centros_hist(pico_sin_calib.canal_pico)<<endl;
    vec vec_log = arma::conv_to<vec>::from(espectro_suma_crudo);
    stream<<"   Espect_Pre_Cal_Vec ="<<guardar_vector_stream(vec_log)<<endl;


    QVector<double> aux_qvec_cent(BinsHist);
    QVector<double> aux_qvec(BinsHist);
    QString nombre_plot;
    /*
    // ----------------------- Ploteo
    // Paso los vectores a Qvector para plotear
    for (int i=0 ; i < BinsHist ; i++){aux_qvec_cent[i] = centros_hist(i);}
    for (int i=0 ; i < BinsHist ; i++){aux_qvec[i] = espectro_suma_crudo(i);}
    nombre_plot = "Espectro Pre calibrado cabezal "+ QString::number(cab_num_act+1)+" FWHM = "+ QString::number(pico_sin_calib.FWHM*100) + "%";
    plot_MCA(aux_qvec, aux_qvec_cent,&Espectro_emergente, nombre_plot, param_cab[cab_num_act], 0);
    Espectro_emergente.show();
    Espectro_emergente.resize(1000,500);
    qApp->processEvents();
    */



    // Conservo solo los eventos dentro del FWTM
    mat Energia_calib_FWHM;
    mat Tiempo_calib_FWHM;
    uvec indices_aux = find(Suma_canales > centros_hist(pico_sin_calib.limites_FWTM[0]));
    rowvec suma_aux = Suma_canales.elem(indices_aux).t();
    Energia_calib_FWHM = Energia_calib_aux.cols(indices_aux);
    Tiempo_calib_FWHM = Tiempos_full_calib[cab_num_act].cols(indices_aux);
    indices_aux = find(suma_aux < centros_hist(pico_sin_calib.limites_FWTM[1]));
    suma_aux = suma_aux.elem(indices_aux).t();
    Energia_calib_FWHM = Energia_calib_FWHM.cols(indices_aux);
    Tiempo_calib_FWHM = Tiempo_calib_FWHM.cols(indices_aux);


    stream<<"Eventos en FWTM: "<<Energia_calib_FWHM.n_cols<<endl;
    rowvec suma_FWTM = sum( Energia_calib_FWHM, 0);
    vec_log.set_size(0,0);
    vec_log = arma::conv_to<vec>::from(hist(suma_FWTM, centros_hist));
    stream<<"   FWTM_pre_Cal_Vec = "<<guardar_vector_stream(vec_log)<<endl;


    // Borro espectros individuales anteriores
    Espectro_PMT_emergente[cab_num_act].clearGraphs();

    urowvec indices_maximo_PMT;
    mat Eventos_max_PMT;
    mat Tiempos_max_PMT;
    rowvec Fila_max_PMT;
    double maximo_abs_PMT;
    double limite_actual;
    int eventos_centroide;
    vec Tasa_PMT(CANTIDADdEpMTS);
    vec Porcentaje_PMT(CANTIDADdEpMTS);
    // Busco los elementos centroides de cada PMT
    for (int index_PMT_cent = 0 ; index_PMT_cent < CANTIDADdEpMTS ; index_PMT_cent ++)
    {
        stream<<endl;
        stream<<"---------------------- EVENTOS POR PMT ------------------------------"<<endl;
        stream<<endl;


        // Extraigo los eventos en los cuales el PMT fue maximo
        indices_maximo_PMT = index_max( Energia_calib_FWHM, 0 );
        //cout<<indices_maximo_PMT.n_elem<<endl;
        indices_aux = find(indices_maximo_PMT == index_PMT_cent);
        //cout<<indices_aux.n_elem<<endl;
        Eventos_max_PMT =  Energia_calib_FWHM.cols(indices_aux);
        Fila_max_PMT = Eventos_max_PMT.row(index_PMT_cent);

        Porcentaje_PMT(index_PMT_cent) = ((double)Fila_max_PMT.n_elem/(double)Energia_calib_FWHM.n_cols)*100;
        Tasa_PMT(index_PMT_cent) = Fila_max_PMT.n_elem / Tiempo_medicion[cab_num_act];

        stream<<"       Eventos maximo PMT "<<index_PMT_cent+1<<": "<<Fila_max_PMT.n_elem<<"   ---   "<< ((double)Fila_max_PMT.n_elem/(double)Energia_calib_FWHM.n_cols)*100<<" % del total en FWHM"<<endl;
        centros_hist = linspace<vec>(0000,2000,BinsHist);
        vec_log.set_size(0,0);
        vec_log = arma::conv_to<vec>::from(hist(Fila_max_PMT, centros_hist));
        stream<<"       Eventos_maximo_pre_cal_PMT_"<<index_PMT_cent+1<<"_Vec ="<<guardar_vector_stream(vec_log)<<endl;
        centros_hist = linspace<vec>(0,8000,BinsHist);
        vec_log.set_size(0,0);
        rowvec suma_log = sum( Eventos_max_PMT, 0);
        vec_log = arma::conv_to<vec>::from(hist(suma_log, centros_hist));
        stream<<"       Eventos_maximo_Suma_pre_cal_PMT_"<<index_PMT_cent+1<<"_Vec = "<<guardar_vector_stream(vec_log)<<endl;


        //Saco tiempos
        Tiempos_max_PMT =  Tiempo_calib_FWHM.cols(indices_aux);


        // Calculo el maximo valor de energia encontrado en este subset
        maximo_abs_PMT = Fila_max_PMT.max();
        limite_actual = maximo_abs_PMT;


        // Itero hasta conseguir la cantidad deseada
        eventos_centroide = 0;

        while (eventos_centroide < NUM_EVENT_CENTRO)
        {
            // Cuento cuantos eventos encontre hasta el punto actual
            indices_aux = find(Fila_max_PMT > limite_actual);
            eventos_centroide = indices_aux.n_elem;


            // Actualizo el limite
            limite_actual = limite_actual - (maximo_abs_PMT*0.01);

        }

        // Me quedo con los eventos en el centroide
        Eventos_max_PMT =  Eventos_max_PMT.cols(indices_aux);
        Tiempos_max_PMT =  Tiempos_max_PMT.cols(indices_aux);


        // Color y marker random
        int param[6];
        param[0]=rand()%245+10;//R
        param[1]=rand()%245+10;//G
        param[2]=rand()%245+10;//B
        param[3]=rand()%5+1; //LineStyle
        param[4]=rand()%14+1;//ScatterShape
        param[5]=rand()/(double)RAND_MAX*2+1;//setWidthF

        centros_hist = linspace<vec>(0,8000,BinsHist);

        // Ploteo el histograma de suma para este PMT
        suma_aux = sum( Eventos_max_PMT,  0);
        espectro_suma_crudo = hist(suma_aux, centros_hist);
        // ----------------------- Ploteo
        // Paso los vectores a Qvector para plotear
        for (int i=0 ; i < BinsHist ; i++){aux_qvec_cent[i] = centros_hist(i);}
        for (int i=0 ; i < BinsHist ; i++){aux_qvec[i] = espectro_suma_crudo(i);}
        nombre_plot = "PMT Nº "+ QString::number(index_PMT_cent+1);
        plot_MCA(aux_qvec, aux_qvec_cent,&Espectro_PMT_emergente[cab_num_act], nombre_plot, param, 0);
        Espectro_PMT_emergente[cab_num_act].show();
        Espectro_PMT_emergente[cab_num_act].resize(1000,500);
        qApp->processEvents();


        stream<<"       --------Centroides usados: "<<indices_aux.n_elem<<endl;
        centros_hist = linspace<vec>(0000,2000,BinsHist);
        vec_log.set_size(0,0);
        vec_log = arma::conv_to<vec>::from(hist(Eventos_max_PMT.row(index_PMT_cent),centros_hist));
        stream<<"               Aleta_pre_cal_PMT_"<<index_PMT_cent+1<<"_Vec ="<<guardar_vector_stream(vec_log)<<endl;
        centros_hist = linspace<vec>(0,8000,BinsHist);
        vec_log.set_size(0,0);
        vec_log = arma::conv_to<vec>::from(espectro_suma_crudo);
        stream<<"               Aleta_Suma_pre_cal_PMT_"<<index_PMT_cent+1<<"_Vec ="<<guardar_vector_stream(vec_log)<<endl;



        // Calculo la energia promedio de todos los PMT para este centroide
        E_prom_PMT[cab_num_act].row(index_PMT_cent) = mean(Eventos_max_PMT,1).t();


        // Calculo la diferencia de tiempo entre el PMT actual y todo el resto.
        for (int i = 0 ; i < CANTIDADdEpMTS ; i ++)
        {
            // Le resto a todos los PMT la referencia actual
            rowvec dist_aux = Tiempos_max_PMT.row(i)-Tiempos_max_PMT.row(index_PMT_cent);

            // Me quedo solo con los eventos con energía superior a una fracción del pico medio observado
            double porc_ener_aux = PORCENTUAL_ENERGIA_VECINO;
            uvec indices_keep = find(Eventos_max_PMT.row(i) >= (porc_ener_aux/100)*mean(Eventos_max_PMT.row(index_PMT_cent)) );

            // Calculo la media del mismo
            double desv_temp_media = mean(dist_aux);

            // Calculo el desvio
            double desv_temp_std = stddev(dist_aux);

            if (indices_keep.n_elem > 0)
            {
                /*
                // Un poco de integridad estadistica (TEST)
                if (   ((desv_temp_std/sqrt(dist_aux.n_elem))/desv_temp_media) > 0.5    )
                    desv_temp_media_central[cab_num_act](i,index_PMT_cent) = mean(dist_aux.elem(indices_keep));
                else
                    desv_temp_media_central[cab_num_act](i,index_PMT_cent) = datum::nan;
                */
                desv_temp_media_central[cab_num_act](i,index_PMT_cent) = mean(dist_aux.elem(indices_keep));

            }
            else
            {
                desv_temp_media_central[cab_num_act](i,index_PMT_cent) = datum::nan;
            }


        }



    }


    // Guardo las tasas
    stream<<"               Porcentaje_PMT = "<<guardar_vector_stream(Porcentaje_PMT)<<endl;
    stream<<"               Tasa_PMT = "<<guardar_vector_stream(Tasa_PMT)<<endl;



    stream<<"               Mat_prom_inicial = "<<guardar_matriz_stream(E_prom_PMT[cab_num_act])<<endl;
    stream<<"               Mat_tiempos_inicial = "<< guardar_matriz_stream(desv_temp_media_central[cab_num_act])<<endl;



    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------SALIDA DE: Pre_calibrar_aleta ------------------------------------------------------------ "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;


    return 1;
}


bool AutoCalib::calibrar_fina_energia(int cab_num_act)
{

    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------INICIO DE: calibrar_fina_energia --------------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;

    // Creo el vector de energía objetivo
    colvec Ener_obj;
    Ener_obj.set_size(CANTIDADdEpMTS,1);
    Ener_obj.zeros(CANTIDADdEpMTS,1);
    Ener_obj = Ener_obj + 511;

    // Paso todos los eventos a energía pre calirbada
    colvec Ce_arma_ant(Ce_pre[cab_num_act], CANTIDADdEpMTS);

    stream<<endl<<"Ce_inicial = ["<< Ce_pre[cab_num_act][0];
    for (int i_log = 1 ; i_log < CANTIDADdEpMTS ; i_log++) stream<<" , "<< Ce_pre[cab_num_act][i_log];
    stream<<"];"<<endl;
    stream<<endl;

    // Calculo el primer paso de calibracion en energia
    // E_prom\Ener_obj  == solve(E_prom,Ener_obj)
    colvec Ce_arma_mat_prom = solve(E_prom_PMT[cab_num_act],Ener_obj);
    vec vec_log = arma::conv_to<vec>::from(Ce_arma_mat_prom);
    stream<<"Ce_aux_ini = "<<guardar_vector_stream(vec_log)<<endl;

    colvec Ce_arma = Ce_arma_mat_prom % Ce_arma_ant;

     vec_log = arma::conv_to<vec>::from(Ce_arma);
    stream<<"Ce_ini = "<<guardar_vector_stream(vec_log)<<endl;
    colvec Ce_iter;


    // Paso toda la matriz de eventos a energia calibrada
    mat Energia_calib_aux = Energia_calib[cab_num_act];
    Energia_calib_aux.each_col() %= Ce_arma;

    // Calculo la suma
    rowvec Energia_calib_suma = sum(Energia_calib_aux,0);

    // Creo el vector de centros para el histograma
    colvec centros_hist = linspace<vec>(0,1024,BinsHist);

    // Calculo el espectro
    urowvec espectro_suma = hist(Energia_calib_suma, centros_hist);


    // Busco el pico
    struct Pico_espectro pico_calib;
    double aux_espectro[BinsHist];
    // Calculo el FWHM
    for (int i=0 ; i < BinsHist ; i++)
    {
        aux_espectro[i] = espectro_suma(i);
    }
    pico_calib = Buscar_Pico(aux_espectro, BinsHist);
    cout<<"Primer paso Calibrar:  "<<pico_calib.FWHM*100<<" %"<<endl;
    stream<<"Primer paso Calibrar:  "<<pico_calib.FWHM*100<<" %"<<endl;

    // Guardo los parametros iniciales
    double FWHM_mejor = pico_calib.FWHM;
    colvec Ce_mejor = Ce_arma;


    // Itero para mejorar el FWHM
    for(int iter_act = 0 ; iter_act < MAX_ITER_ENERGIA ; iter_act ++)
    {

        // Recorto los eventos dentro del FWTM de la matriz de eventos pre-calibrada
        mat Energia_calib_FWHM;
        uvec indices_aux = find(Energia_calib_suma > centros_hist(pico_calib.limites_FWTM[0]));
        rowvec suma_aux = Energia_calib_suma.elem(indices_aux).t();
        Energia_calib_FWHM = Energia_calib_aux.cols(indices_aux);
        indices_aux = find(suma_aux < centros_hist(pico_calib.limites_FWTM[1]));
        suma_aux = suma_aux.elem(indices_aux).t();
        Energia_calib_FWHM = Energia_calib_FWHM.cols(indices_aux);

        // Re-calculo la matrix de energias promedios
        mat E_prom_PMT_aux;
        E_prom_PMT_aux.set_size(CANTIDADdEpMTS,CANTIDADdEpMTS);
        urowvec indices_maximo_PMT;
        mat Eventos_max_PMT;
        rowvec Fila_max_PMT;
        double maximo_abs_PMT;
        double limite_actual;
        int eventos_centroide;
        // Busco los elementos centroides de cada PMT
        for (int index_PMT_cent = 0 ; index_PMT_cent < CANTIDADdEpMTS ; index_PMT_cent ++)
        {
            // Extraigo los eventos en los cuales el PMT fue maximo
            indices_maximo_PMT = index_max( Energia_calib_FWHM, 0 );
            indices_aux = find(indices_maximo_PMT == index_PMT_cent);
            Eventos_max_PMT =  Energia_calib_FWHM.cols(indices_aux);
            Fila_max_PMT = Eventos_max_PMT.row(index_PMT_cent);

            // Calculo el maximo valor de energia encontrado en este subset
            maximo_abs_PMT = Fila_max_PMT.max();
            limite_actual = maximo_abs_PMT;

            // Itero hasta conseguir la cantidad deseada
            eventos_centroide = 0;

            while (eventos_centroide < NUM_EVENT_CENTRO)
            {
                // Cuento cuantos eventos encontre hasta el punto actual
                indices_aux = find(Fila_max_PMT > limite_actual);
                eventos_centroide = indices_aux.n_elem;

                // Actualizo el limite
                limite_actual = limite_actual - (maximo_abs_PMT*0.01);
            }

            // Me quedo con los eventos en el centroide
            Eventos_max_PMT =  Eventos_max_PMT.cols(indices_aux);

            // Calculo la energia promedio de todos los PMT para este centroide
            E_prom_PMT_aux.row(index_PMT_cent) = mean(Eventos_max_PMT,1).t();

        }



        // Re calculo el nuevo Ce
        Ce_iter = solve(E_prom_PMT_aux,Ener_obj);
        // Actualizo al Ce recien calculado
        Ce_arma = Ce_arma%Ce_iter;

        // Paso todos los eventos a eventos en energía calibrada
        Energia_calib_aux.each_col() %= Ce_iter;
        // Calculo la suma
        Energia_calib_suma = sum(Energia_calib_aux,0);
        // Calculo el espectro
        espectro_suma = hist(Energia_calib_suma, centros_hist);
        // Calculo el FWHM
        for (int i=0 ; i < BinsHist ; i++)
        {
            aux_espectro[i] = espectro_suma(i);
        }
        pico_calib = Buscar_Pico(aux_espectro, BinsHist);


        if (FWHM_mejor > pico_calib.FWHM)
        {
            FWHM_mejor = pico_calib.FWHM;
            Ce_mejor = Ce_arma;
        }
        else if ((FWHM_mejor - pico_calib.FWHM)*(FWHM_mejor - pico_calib.FWHM) < 0.001*0.001 || pico_calib.FWHM > FWHM_mejor)
        {
            break;
        }

        cout<<"Paso "<<iter_act<<": "<<pico_calib.FWHM*100<<"%"<<endl;
        stream<<"       Paso "<<iter_act<<": "<<pico_calib.FWHM*100<<"%"<<endl;
        vec_log = arma::conv_to<vec>::from(Ce_arma);
        stream<<"       Ce_paso = "<<guardar_vector_stream(vec_log)<<endl;

        // que no se apague la pantalla
        qApp->processEvents();


    }


    for (int i = 0 ; i < CANTIDADdEpMTS ; i ++)
    {
        Ce[cab_num_act][i] = Ce_mejor(i);
    }

    cout<<"Final: "<<pico_calib.FWHM*100<<"%"<<endl;
    stream<<"Final: "<<pico_calib.FWHM*100<<"%"<<endl;
    vec_log = arma::conv_to<vec>::from(Ce_mejor);
    stream<<"       Ce_final = ["<<guardar_vector_stream(vec_log)<<endl;


    // ----------------------- Ploteo
    // Paso los vectores a Qvector para plotear
    QVector<double> aux_qvec_cent(BinsHist);
    for (int i=0 ; i < BinsHist ; i++){aux_qvec_cent[i] = centros_hist(i);}
    QVector<double> aux_qvec(BinsHist);
    for (int i=0 ; i < BinsHist ; i++){aux_qvec[i] = espectro_suma(i);}

    QString nombre_plot = "Espectro calibrado cabezal "+ QString::number(cab_num_act+1)+" FWHM = "+ QString::number(FWHM_mejor*100) + "%";
    plot_MCA(aux_qvec, aux_qvec_cent,&Espectro_emergente, nombre_plot, param_cab[cab_num_act], 0);
    Espectro_emergente.show();
    Espectro_emergente.resize(1000,500);
    qApp->processEvents();


    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------SALIDA DE: calibrar_fina_energia --------------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;

}





bool AutoCalib::calibrar_fina_tiempos(int cab_num_act)
{
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------INICIO DE: calibrar_fina_tiempos --------------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;

    stream<<endl;
    stream<<"No hay mucho que loguear aca..."<<endl;
    stream<<endl;


    //cout<<desv_temp_media_central[cab_num_act]<<endl;
    // Bienvenido a la calibracion en tiempo, usted esta a punto de presenciar una funcion
    // recursiva, que la fuerza lo acompañe.


    // PMT de referencia para la calibración
    int PMT_Ref = 20-1;

    // Preparo el inicio de la funcion recursiva.

    rowvec Correccion_Temporal;
    Correccion_Temporal.set_size(1,CANTIDADdEpMTS);
    Correccion_Temporal.zeros(1,CANTIDADdEpMTS);

    rowvec Corregido;
    Corregido.set_size(1,CANTIDADdEpMTS);
    Corregido.zeros(1,CANTIDADdEpMTS);

    rowvec Distancia;
    Distancia.set_size(1,CANTIDADdEpMTS);
    Distancia.zeros(1,CANTIDADdEpMTS);
    Distancia = Distancia + 9999;

    Corregido(PMT_Ref) = 1;
    Distancia(PMT_Ref) = 0;

    // ¡Inicio la recursividad!
    struct tiempos_recursiva Tiempos_finales = tiempos_a_vecino( PMT_Ref,  Correccion_Temporal,  Corregido,  Distancia,  desv_temp_media_central[cab_num_act] );


    // Para tener todos coeficientes todos positivos vuelvo a ajustar los
    // tiempos pero ahora con respecto al PMT mas rápido. Esto es necesario para
    // la correccion dentro del cabezal.
    double time_corr = min(Tiempos_finales.Correccion_Temporal_out);

    Tiempos_finales.Correccion_Temporal_out = round(Tiempos_finales.Correccion_Temporal_out + (-time_corr));

    for (int i = 0 ; i < CANTIDADdEpMTS ; i ++)
    {
        Ct[cab_num_act][i] = Tiempos_finales.Correccion_Temporal_out(i);
    }

    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------SALIDA DE: calibrar_fina_tiempos --------------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;


    return 1;
}


struct tiempos_recursiva AutoCalib::tiempos_a_vecino(int PMT_Ref, rowvec Correccion_Temporal, rowvec Corregido, rowvec Distancia, mat desv_temp_max_hist )
{

    struct tiempos_recursiva salida_act;

    // Busco los vecinos inmediatos (NO en diagonales)
    // El caso de ser un borde es particular, ya que por ejemplo el 40 y el
    // 41 no serian vecinos, recalculo los vecinos.
    int *Vecinos_aux;
    int N_Vecinos_aux = 0;
    if (PMT_Ref == 8-1 || PMT_Ref == 16-1 || PMT_Ref == 24-1 || PMT_Ref == 32-1 || PMT_Ref == 40-1)
    {
        Vecinos_aux = new int[3];
        Vecinos_aux [0] = PMT_Ref-1;
        Vecinos_aux [1] = PMT_Ref+8;
        Vecinos_aux [2] = PMT_Ref-8;
        N_Vecinos_aux = 3;
    }
    else if (PMT_Ref == 9-1 || PMT_Ref == 17-1 || PMT_Ref == 25-1 || PMT_Ref == 33-1 || PMT_Ref == 41-1)
    {
        Vecinos_aux = new int[3];
        Vecinos_aux [0] = PMT_Ref+1;
        Vecinos_aux [1] = PMT_Ref+8;
        Vecinos_aux [2] = PMT_Ref-8;
        N_Vecinos_aux = 3;
    }
    else
    {
        Vecinos_aux = new int[4];
        Vecinos_aux [0] = PMT_Ref+1;
        Vecinos_aux [1] = PMT_Ref-1;
        Vecinos_aux [2] = PMT_Ref+8;
        Vecinos_aux [3] = PMT_Ref-8;
        N_Vecinos_aux = 4;
    }


    // Elimino los vecinos que no existan
    int N_vecinos = 0;
    for(int i = 0 ; i < N_Vecinos_aux ; i++ )
    {
        if (Vecinos_aux[i] >= 0 && Vecinos_aux[i] < CANTIDADdEpMTS)
        {
            N_vecinos++ ;
        }
    }
    int* Vecinos;
    Vecinos = new int[N_vecinos];
    int cuenta_pasa = 0;
    for(int i = 0 ; i < N_Vecinos_aux ; i++ )
    {
        if (Vecinos_aux[i] >= 0 && Vecinos_aux[i] < CANTIDADdEpMTS)
        {
            Vecinos[cuenta_pasa] = Vecinos_aux[i];
            cuenta_pasa++;
        }
    }

    delete(Vecinos_aux);


    // Elimino los vecinos cuya distancia a la referencia sea mejor que la
    // mia, ya que estaria volviendo hacia atras.
    int kill_count = 0;
    for ( int i=0 ; i < N_vecinos ; i++ )
    {
        if (Distancia(Vecinos[i]) < Distancia(PMT_Ref))
        {
            kill_count ++;
        }
    }
    int* label_matar;
    int* Vecinos_finales;
    if (kill_count > 0)
    {
        int N_vecinos_finales = N_vecinos - kill_count;
        int kill_count_aux = kill_count;

        label_matar = new int[kill_count];
        for ( int i=0 ; i < N_vecinos ; i++ )
        {
            if (Distancia(Vecinos[i]) < Distancia(PMT_Ref))
            {
                label_matar[kill_count-1] = i;
                kill_count-- ;
            }
        }


        Vecinos_finales = new int[N_vecinos_finales];
        bool matar = 0;
        cuenta_pasa = 0;
        for (int i = 0 ; i < N_vecinos ; i++)
        {
            matar = 0;
            for (int j = 0 ; j <kill_count_aux ; j++)
            {
                if (i == label_matar[j])
                    matar = 1;
            }
            if (!matar)
            {
                Vecinos_finales[cuenta_pasa] = Vecinos[i];
                cuenta_pasa++ ;
            }

        }

        delete(Vecinos);
        Vecinos = Vecinos_finales;
        N_vecinos = N_vecinos_finales;


    }

    // Proceso si quedaron vecinos
    if (N_vecinos > 0 )
    {


        // Ajusto todos los vecinos a la referencia
        for (int i=0 ; i < N_vecinos ; i++)
        {
            // Ajusto en los siguientes casos:

            // Nunca fue corregido
            if ( !Corregido(Vecinos[i]) )
            {
                // Asigno la corrección temporal y le sumo la correccion del
                // PMT de referencia.
                Correccion_Temporal(Vecinos[i]) = -desv_temp_max_hist(Vecinos[i],PMT_Ref) + Correccion_Temporal(PMT_Ref);
                // Marco el PMT como correjido
                Corregido(Vecinos[i]) = 1;
                // Le asigno la distancia al PMT de referencia original.
                Distancia(Vecinos[i]) = Distancia(PMT_Ref)+1;

            }
            // Estoy en una iteracion cuya corrección es mas cercana a la
            // referencia original que cuando se le asigno la corrección
            // ya existente.
            else if (Distancia(Vecinos[i]) > (Distancia(PMT_Ref)+1))
            {
                // idem caso anterior
                Correccion_Temporal(Vecinos[i]) = -desv_temp_max_hist(Vecinos[i],PMT_Ref) + Correccion_Temporal(PMT_Ref);
                Corregido(Vecinos[i]) = 1;
                Distancia(Vecinos[i]) = Distancia(PMT_Ref)+1;
            }
        }


        // Ejecuto nuevamente la funcion para los vecinos encontrados,
        // siendo ahora estos mismos el PMT de referencia, ya que quedaron
        // corregidos por esta llamada.

        // Matriz que va a contener las salidas
        mat Correcciones_vecinos;
        Correcciones_vecinos.set_size(N_vecinos,CANTIDADdEpMTS);
        Correcciones_vecinos.zeros(N_vecinos,CANTIDADdEpMTS);

        mat Corregido_aux;
        Corregido_aux.set_size(N_vecinos,CANTIDADdEpMTS);
        Corregido_aux.zeros(N_vecinos,CANTIDADdEpMTS);

        mat Distancia_aux;
        Distancia_aux.set_size(N_vecinos,CANTIDADdEpMTS);
        Distancia_aux.zeros(N_vecinos,CANTIDADdEpMTS);
        Distancia_aux = Distancia_aux + 9999;


        // Ejecuto recursividad
        for (int i=0 ; i < N_vecinos ; i++)
        {

                struct tiempos_recursiva Tiempos_aux = tiempos_a_vecino(Vecinos[i], Correccion_Temporal, Corregido, Distancia, desv_temp_max_hist);


                // Guardo los resultados del camino de este vecino
                Correcciones_vecinos.row(i) = Tiempos_aux.Correccion_Temporal_out;
                Corregido_aux.row(i) = Tiempos_aux.Corregido_out;
                Distancia_aux.row(i) = Tiempos_aux.Distancia_out;

        }



        // Creo salidas
        salida_act.Correccion_Temporal_out.set_size(1,CANTIDADdEpMTS);
        salida_act.Correccion_Temporal_out.zeros(1,CANTIDADdEpMTS);

        salida_act.Corregido_out.set_size(1,CANTIDADdEpMTS);
        salida_act.Corregido_out.zeros(1,CANTIDADdEpMTS);

        salida_act.Distancia_out.set_size(1,CANTIDADdEpMTS);
        salida_act.Distancia_out.zeros(1,CANTIDADdEpMTS);
        salida_act.Distancia_out = salida_act.Distancia_out + 9999;


        // Checkeo de correccion, para saber que PMTs ya fueron corregidos
        // al menos una vez
        for (int i=0 ; i < N_vecinos ; i++)
        {
            for (int j=0 ; j < CANTIDADdEpMTS ; j++)
            {
                if (Corregido_aux(i,j) == 1 || Corregido(j) == 1)
                    salida_act.Corregido_out(j) = 1;
            }

        }


        // Compilo las salidas de los vecinos en una unica salida
        // conservando los resultados con la menor distancia a la
        // referencia.
        for (int i=0 ; i < CANTIDADdEpMTS ; i++)
        {
            // Solo opero si fue corrgido alguna vez.
            if (salida_act.Corregido_out(i))
            {
                rowvec distancia_check;
                distancia_check.set_size(1,N_vecinos);
                distancia_check.zeros(1,N_vecinos);

                // Levanto la distancia de correccion del PMT actual que
                // medieron los caminos de los vecinos.
                for ( int j=0 ; j< N_vecinos ; j++ )
                {
                    distancia_check(j) = Distancia_aux(j,i);
                }



                // Busco la minima de todas las distancias
                int index_nivel = distancia_check.index_min();
                double nivel = distancia_check.min();

                // Asigno a la salida la distancia menor.
                salida_act.Distancia_out(i) = nivel;
                salida_act.Correccion_Temporal_out(i) = Correcciones_vecinos(index_nivel,i);
            }

        }





    }
    else
    {
        // Si estoy en una punta/PMT sin salida/Vecinos, retorno lo que
        // calcule (Punta de salida de la recursividad).

        // Creo salidas
        salida_act.Correccion_Temporal_out = Correccion_Temporal;
        salida_act.Corregido_out = Corregido;
        salida_act.Distancia_out =  Distancia;


    }



    // Retorno y rezo...
    return salida_act;


    /* Y aqui termina esta odisea programatica en C, espero que halla
     * recorrido sus estados con entusiasmo. Finalmente querria
     * agradecerle a usted por leer este codigo, ¿Que? ¿yo agradecerle
     * a usted? Si, esta en lo cierto, ya que disfrute tanto codiando
     * estas lineas como usted en leerlas.
     * */

}





bool AutoCalib::calibrar_fina_posiciones(int cab_num_act)
{

    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------INICIO DE: calibrar_fina_posiciones ------------------------------------------------------ "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;

    double Lado_X_Cabezal = (Lado_PMT*PMTs_X)/2;
    double Lado_Y_Cabezal = (Lado_PMT*PMTs_Y)/2;

    // Convierto la matriz de medias en cuentas a medias en energía
    mat E_prom_PMT_keV = E_prom_PMT[cab_num_act];
    rowvec Ce_aux(Ce[cab_num_act], CANTIDADdEpMTS);
    rowvec Ce_aux_pre(Ce_pre[cab_num_act], CANTIDADdEpMTS);
    Ce_aux /= Ce_aux_pre;
    //cout<<Ce_aux<<endl;
    E_prom_PMT_keV = E_prom_PMT_keV.each_row()%Ce_aux;
    stream<<"E_prom_PMT_keV = "<<guardar_matriz_stream(E_prom_PMT_keV)<<endl;



    // Configuro los vectores de posición objetivo

    double *vec_y;
    double step_act = -(Lado_Y_Cabezal-(Lado_PMT/2));
    int elem_vec = 1;
    while ((Lado_Y_Cabezal-(Lado_PMT/2)) > step_act)
    {
        step_act += Lado_PMT;
        elem_vec++;
    }
    vec_y = new double[elem_vec];
    vec_y[0] = -(Lado_Y_Cabezal-(Lado_PMT/2));
    for (int i=1 ; i < elem_vec ; i++)
    {
        vec_y[i] = vec_y[i-1] + Lado_PMT;
    }

    double *vec_x;
    step_act = -(Lado_X_Cabezal-(Lado_PMT/2));
    elem_vec = 1;
    while ((Lado_X_Cabezal-(Lado_PMT/2)) > step_act)
    {
        step_act += Lado_PMT;
        elem_vec++;
    }
    vec_x = new double[elem_vec];
    vec_x[0] = -(Lado_X_Cabezal-(Lado_PMT/2));
    for (int i=1 ; i < elem_vec ; i++)
    {
        vec_x[i] = vec_x[i-1] + Lado_PMT;
    }


    double Fx[CANTIDADdEpMTS], Fy[CANTIDADdEpMTS];

    for (int j = 0 ; j < PMTs_Y ; j++)
    {
        for (int i = 0 ; i < PMTs_X ; i++)
        {
            Fx[i+((j)*PMTs_X)] = vec_x[i];
            Fy[i+((j)*PMTs_X)] = vec_y[j];
        }
    }

    colvec Fx_arma(Fx, CANTIDADdEpMTS);
    colvec Fy_arma(Fy, CANTIDADdEpMTS);

    // Despejo el par de ecuaciones
    //   E_prom * Cx = Fx
    //   E_prom * Cy = Fy
    colvec Cx_arma = solve(E_prom_PMT_keV, Fx_arma);
    colvec Cy_arma = solve(E_prom_PMT_keV, Fy_arma);

    // Paso a double
    for (int i = 0 ; i < CANTIDADdEpMTS ; i ++)
    {
        Cx[cab_num_act][i] = Cx_arma(i);
        Cy[cab_num_act][i] = Cy_arma(i);
    }


    // Calculo el alhoadon
    calcular_almohadon(cab_num_act);

    // Dibujo el almohadon en pantalla
    mostrar_almohadon(cab_num_act,1,0);


    vec vec_log = arma::conv_to<vec>::from(Cx_arma);
    stream<<"       Cx = "<<guardar_vector_stream(vec_log)<<endl;
    vec_log = arma::conv_to<vec>::from(Cy_arma);
    stream<<"       Cy = "<<guardar_vector_stream(vec_log)<<endl;


    delete(vec_y);
    delete(vec_x);

    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------SALIDA DE: calibrar_fina_posiciones ------------------------------------------------------ "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;

    return 1;
}


bool AutoCalib::calcular_almohadon(int cab_num_act)
{
    colvec Cx_arma(Cx[cab_num_act], CANTIDADdEpMTS);
    colvec Cy_arma(Cy[cab_num_act], CANTIDADdEpMTS);

    // Calculo la posición de todos los eventos (para el amohadon)



    // Paso todos los eventos a energía calibrada
    // Paso toda la matriz de eventos a energia calibrada
    mat Energia_calib_aux = Energia_calib[cab_num_act];
    colvec Ce_arma(Ce[cab_num_act], CANTIDADdEpMTS);
    Energia_calib_aux.each_col() %= Ce_arma;


    // Calculo la suma
    rowvec Energia_calib_suma = sum(Energia_calib_aux,0);

    // Creo el vector de centros para el histograma
    colvec centros_hist = linspace<vec>(0,1024,BinsHist);

    // Calculo el espectro
    urowvec espectro_suma = hist(Energia_calib_suma, centros_hist);


    // Busco el pico
    struct Pico_espectro pico_calib;
    double aux_espectro[BinsHist];
    // Calculo el FWHM
    for (int i=0 ; i < BinsHist ; i++)
    {
        aux_espectro[i] = espectro_suma(i);
    }
    pico_calib = Buscar_Pico(aux_espectro, BinsHist);

    // Recorto los eventos dentro del FWTM de la matriz de eventos pre-calibrada
    mat Energia_calib_FWHM;
    uvec indices_aux = find(Energia_calib_suma > centros_hist(pico_calib.limites_FWHM[0]));
    rowvec suma_aux = Energia_calib_suma.elem(indices_aux).t();
    Energia_calib_FWHM = Energia_calib_aux.cols(indices_aux);
    indices_aux = find(suma_aux < centros_hist(pico_calib.limites_FWHM[1]));
    suma_aux = suma_aux.elem(indices_aux).t();
    Energia_calib_FWHM = Energia_calib_FWHM.cols(indices_aux);





    // Calculo la posición de los mismos
    mat Posiciones_estimadas(Energia_calib_FWHM.n_cols, 2);
    for (int ind_evento = 0 ; ind_evento < Energia_calib_FWHM.n_cols ; ind_evento++)
    {
        mat aux = Energia_calib_FWHM.col(ind_evento).t()*Cx_arma;
        Posiciones_estimadas(ind_evento, 0) =  aux(0);
        aux = Energia_calib_FWHM.col(ind_evento).t()*Cy_arma;
        Posiciones_estimadas(ind_evento, 1) =  aux(0);
    }





    // Armo la grilla objetivo
    double Largo_Cab = Lado_PMT*PMTs_X;
    double Alto_Cab = Lado_PMT*PMTs_Y;

    // Paso entre bines
    double paso_x = Largo_Cab/ BinsAlmohadon;
    double paso_y = Alto_Cab/BinsAlmohadon;

    // Reservo memoria para los vectores de X e Y
    double *vec_x, *vec_y;
    vec_x = new double[BinsAlmohadon];
    vec_y = new double[BinsAlmohadon];

    vec_x[0] = -Largo_Cab/2;
    vec_y[0] = -Alto_Cab/2;

    // Los lleno
    for(int i = 1 ; i < BinsAlmohadon ; i++ )
    {
        vec_x[i] = vec_x[i-1] + paso_x;
        vec_y[i] = vec_y[i-1] + paso_y;
    }



    // Voy ubicando los eventos
    for (int ind_evento = 0 ; ind_evento < Posiciones_estimadas.n_rows ; ind_evento++)
    {
        double pos_evento_x = Posiciones_estimadas(ind_evento,0);
        double pos_evento_y = Posiciones_estimadas(ind_evento,1);

        // Busco el indice de x
        double x_step = vec_x[0];
        int x_indice = 0;
        while(x_step < pos_evento_x && x_indice < BinsAlmohadon-1)
        {
            x_indice++;
            x_step = vec_x[x_indice];
        }

        double y_step = vec_y[0];
        int y_indice = 0;
        while(y_step < pos_evento_y && y_indice < BinsAlmohadon-1)
        {
            y_indice++;
            y_step = vec_y[y_indice];
        }

        almohadon[cab_num_act](x_indice, y_indice) = almohadon[cab_num_act](x_indice, y_indice) + 1;


    }





    delete(vec_y);
    delete(vec_x);

    return 1;
}








bool AutoCalib::calibrar_count_skimming(int cab_num_act)
{
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------INICIO DE: calibrar_count_skimming ------------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;

    // Copio el almohadon a la matriz de count skimming
    count_skimm[cab_num_act] = almohadon[cab_num_act];


    // Armo la grilla objetivo
    double Largo_Cab = Lado_PMT*PMTs_X;
    double Alto_Cab = Lado_PMT*PMTs_Y;

    // Paso entre bines
    double paso_x = Largo_Cab/ BinsAlmohadon;
    double paso_y = Alto_Cab/BinsAlmohadon;

    // Reservo memoria para los vectores de X e Y
    double *vec_x, *vec_y;
    vec_x = new double[BinsAlmohadon];
    vec_y = new double[BinsAlmohadon];

    vec_x[0] = -Largo_Cab/2;
    vec_y[0] = -Alto_Cab/2;

    // Los lleno
    for(int i = 1 ; i < BinsAlmohadon ; i++ )
    {
        vec_x[i] = vec_x[i-1] + paso_x;
        vec_y[i] = vec_y[i-1] + paso_y;
    }

    // Calculo los limites con el recorte de PMT
    double lim_inf_x = vec_x[0] + PMT_Recortado*Lado_PMT;
    double lim_sup_x = vec_x[BinsAlmohadon-1] - PMT_Recortado*Lado_PMT;
    double lim_inf_y = vec_y[0] + PMT_Recortado*Lado_PMT;
    double lim_sup_y = vec_y[BinsAlmohadon-1] - PMT_Recortado*Lado_PMT;


    // Recorto los eventos de los bordes (aquellos que no puedo posicionar)
    for (int ind_x = 0 ; ind_x < BinsAlmohadon ; ind_x++)
    {
        for (int ind_y = 0 ; ind_y < BinsAlmohadon ; ind_y++)
        {
            if ( vec_x[ind_x] < lim_inf_x || vec_x[ind_x] > lim_sup_x || vec_y[ind_y] < lim_inf_y || vec_y[ind_y] > lim_sup_y)
                count_skimm[cab_num_act](ind_x, ind_y) = 0;
        }
    }


    // Saco los eventos que no son 0
    uvec px_activos_ind = find(count_skimm[cab_num_act] > 0);
    vec px_activos = count_skimm[cab_num_act].elem(px_activos_ind);

    // Me quedo con el minimo valor de eventos
    double px_base =  min(px_activos);
    double px_pico =  max(px_activos);
    double px_total =  sum(px_activos);

    stream<<"Cuentas en pixel base: "<<px_base<<endl;
    stream<<"Cuentas en pixel pico: "<<px_pico<<endl;
    stream<<"Cuentas totales en area util: "<<px_total<<endl;


    // Le resto a todos los elementos el valor de este pixel y dejo los bordes en 0
    for (int ind_x = 0 ; ind_x < BinsAlmohadon ; ind_x++)
    {
        for (int ind_y = 0 ; ind_y < BinsAlmohadon ; ind_y++)
        {
            count_skimm[cab_num_act](ind_x, ind_y) = px_base/count_skimm[cab_num_act](ind_x, ind_y);

            if ( vec_x[ind_x] < lim_inf_x || vec_x[ind_x] > lim_sup_x || vec_y[ind_y] < lim_inf_y || vec_y[ind_y] > lim_sup_y)
                count_skimm[cab_num_act](ind_x, ind_y) = 0;
        }
    }




    delete(vec_y);
    delete(vec_x);


    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------SALIDA DE: calibrar_count_skimming ------------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;



    return 1;
}


bool AutoCalib::calibrar_inter_count_skimming()
{
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------INICIO DE: calibrar_inter_count_skimming ------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;

    vec px_base(CANTIDADdEcABEZALES);
    vec px_pico(CANTIDADdEcABEZALES);

    for(int cab_num_act ; cab_num_act < CANTIDADdEcABEZALES ; cab_num_act++)
    {
        // Saco los eventos que no son 0 en el count skimming
        uvec px_activos_ind = find(count_skimm[cab_num_act] > 0);
        vec px_activos = almohadon[cab_num_act].elem(px_activos_ind);

        // Me quedo con el minimo valor de eventos
        if (px_activos_ind.n_elem > 0)
        {
            px_base(cab_num_act) =  min(px_activos);
            px_pico(cab_num_act) =  max(px_activos);
        }
        else
        {
            px_base(cab_num_act) =  9e99;
            px_pico(cab_num_act) =  0;
        }

    }

    // Saco el minimo de los 6
    double px_min_total = min(px_base);
    stream<<"Minimo global de eventos: "<<px_min_total<<" (Cabezal :"<<px_base.index_min()+1<<")"<<endl;
    double px_max_total = max(px_pico);
    stream<<"Maximo global de eventos: "<<px_max_total<<" (Cabezal :"<<px_base.index_max()+1<<")"<<endl;

    for(int cab_num_act ; cab_num_act < CANTIDADdEcABEZALES ; cab_num_act++)
    {
        // Calculo el proporcional del count skimming individual
        double px_min_prop = px_min_total/px_base(cab_num_act);

        // Calculo el count skimming inter cabezal
        count_skimm_inter_cab[cab_num_act] = count_skimm[cab_num_act] * px_min_prop;
    }


    cout<<"Finalizado Inter-Count Skimming"<<endl;

    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------SALIDA DE: calibrar_inter_count_skimming ------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;



    return 1;
}


bool AutoCalib::calibrar_tiempo_intercabezal()
{

    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------INICIO DE: calibrar_tiempo_intercabezal -------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;

    // Recupero el nombre del archivo
    stream<<"Leyendo: "<<QString::fromStdString(adq_coin)<< endl;

    // Levanto el archivo
    Tiempos_inter_cab.load(adq_coin, raw_ascii);
    stream<<"Archivo leido, matriz de "<<Tiempos_inter_cab.n_rows<<" por "<<Tiempos_inter_cab.n_cols<<endl;

    // Calculo el FWHM de las energías de los eventos, tanto en el evento 1 como el evento 2
    colvec Energias_1 = Tiempos_inter_cab.col(Coin_Calib_Energia_1);
    colvec Energias_2 = Tiempos_inter_cab.col(Coin_Calib_Energia_2);

    // Creo el vector de centros para el histograma
    vec centros_hist = linspace<vec>(0,1024,BinsHist);

    // Calculo el histograma
    ucolvec espectro_1 = hist(Energias_1, centros_hist);
    ucolvec espectro_2 = hist(Energias_2, centros_hist);

    vec vec_log;
    vec_log.set_size(0,0);
    vec_log = arma::conv_to<vec>::from(espectro_1);
    stream<<"               Pico_1"<<guardar_vector_stream(vec_log)<<endl;
    vec_log.set_size(0,0);
    vec_log = arma::conv_to<vec>::from(espectro_2);
    stream<<"               Pico_2"<<guardar_vector_stream(vec_log)<<endl;

    // Calculo el FWHM
    struct Pico_espectro pico_1, pico_2;
    double aux_espectro_1[BinsHist];
    double aux_espectro_2[BinsHist];
    for (int i=0 ; i < BinsHist ; i++)
    {
        aux_espectro_1[i] = espectro_1(i);
        aux_espectro_2[i] = espectro_2(i);
    }
    pico_1 = Buscar_Pico(aux_espectro_1, BinsHist);
    pico_2 = Buscar_Pico(aux_espectro_2, BinsHist);

    cout<<"FWHM eventos 1: "<<pico_1.FWHM*100<<"%"<<endl;
    stream<<"FWHM eventos 1: "<<pico_1.FWHM*100<<"%"<<endl;
    cout<<"FWHM eventos 2: "<<pico_2.FWHM*100<<"%"<<endl;
    stream<<"FWHM eventos 2: "<<pico_2.FWHM*100<<"%"<<endl;

    // Me quedo con los extremos del FWHM
    double FWHM_lim[2];

    if (pico_1.limites_FWHM[0] < pico_2.limites_FWHM[0]) FWHM_lim[0] = pico_1.limites_FWHM[0];
    else FWHM_lim[0] = pico_2.limites_FWHM[0];

    if (pico_1.limites_FWHM[1] > pico_2.limites_FWHM[1]) FWHM_lim[1] = pico_1.limites_FWHM[1];
    else FWHM_lim[1] = pico_2.limites_FWHM[1];

    stream<<"Limites FWHM: "<<centros_hist(FWHM_lim[0])<<" - "<<centros_hist(FWHM_lim[1])<<endl;

    // Filtro por la ventana de energía
    mat Tiempos_inter_cab_FWHM;
    uvec indices_aux = find(Energias_1 > centros_hist(FWHM_lim[0]));
    rowvec suma_aux = Energias_1.elem(indices_aux).t();
    Tiempos_inter_cab_FWHM = Tiempos_inter_cab.rows(indices_aux);
    indices_aux = find(suma_aux < centros_hist(FWHM_lim[1]));
    suma_aux = Energias_1.elem(indices_aux).t();
    Tiempos_inter_cab_FWHM = Tiempos_inter_cab_FWHM.rows(indices_aux);
    // lo mismo pero para la energía 2
    Energias_2 = Tiempos_inter_cab_FWHM.col(Coin_Calib_Energia_2);
    indices_aux = find(Energias_2 > centros_hist(FWHM_lim[0]));
    suma_aux = Energias_2.elem(indices_aux).t();
    Tiempos_inter_cab_FWHM = Tiempos_inter_cab_FWHM.rows(indices_aux);
    indices_aux = find(suma_aux < centros_hist(FWHM_lim[1]));
    suma_aux = Energias_2.elem(indices_aux).t();
    Tiempos_inter_cab_FWHM = Tiempos_inter_cab_FWHM.rows(indices_aux);



    stream<<"Eventos en FWHM: "<<Tiempos_inter_cab_FWHM.n_elem<<endl;

    // Para identificar cabezales
    //colvec cabezal_1 = Tiempos_inter_cab_FWHM.col(Coin_Calib_Iden_cabezal_1);
    //colvec cabezal_2 = Tiempos_inter_cab_FWHM.col(Coin_Calib_Iden_cabezal_2);

    // Ahora me fijo las relaciones de tiempo entre cabezales
    mat desvios_medios, desvios_desvio;
    desvios_medios.set_size(CANTIDADdEcABEZALES,CANTIDADdEcABEZALES);
    desvios_medios.zeros(CANTIDADdEcABEZALES,CANTIDADdEcABEZALES);
    desvios_desvio.set_size(CANTIDADdEcABEZALES,CANTIDADdEcABEZALES);
    desvios_desvio.zeros(CANTIDADdEcABEZALES,CANTIDADdEcABEZALES);



    // Loopeo por par de cabezales
    for (int iter_i = 0 ; iter_i < CANTIDADdEcABEZALES ; iter_i++)
    {
        for (int iter_j = 0 ; iter_j < CANTIDADdEcABEZALES ; iter_j++)
        {
            bool invert_coef = 0;

            // Extraigo eventos compartidos
            mat eventos_compartidos;
            uvec ind_cab_1, ind_cab_2;
            ind_cab_1 = find(Tiempos_inter_cab_FWHM.col(Coin_Calib_Iden_cabezal_1) == iter_i+1);
            eventos_compartidos = Tiempos_inter_cab_FWHM.rows(ind_cab_1);
            ind_cab_2 = find(eventos_compartidos.col(Coin_Calib_Iden_cabezal_2) == iter_j+1);
            eventos_compartidos = eventos_compartidos.rows(ind_cab_2);

            if (eventos_compartidos.n_elem <= 0)  // Checkeo para los dos lados
            {
                ind_cab_1 = find(Tiempos_inter_cab_FWHM.col(Coin_Calib_Iden_cabezal_1) == iter_j+1);
                eventos_compartidos = Tiempos_inter_cab_FWHM.rows(ind_cab_1);
                ind_cab_2 = find(eventos_compartidos.col(Coin_Calib_Iden_cabezal_2) == iter_i+1);
                eventos_compartidos = eventos_compartidos.rows(ind_cab_2);

                invert_coef = 1;
            }

            if (eventos_compartidos.n_elem > 0)
            {
                // Calculo la diferencia de tiempos
                colvec DeltaT;
                colvec aux_DeltaT;
                if (invert_coef)
                    DeltaT = eventos_compartidos.col(Coin_Calib_tiempos_2)-eventos_compartidos.col(Coin_Calib_tiempos_1);
                else
                    DeltaT = eventos_compartidos.col(Coin_Calib_tiempos_1)-eventos_compartidos.col(Coin_Calib_tiempos_2);

                // Eliminno aquellos con un tiempo extraño
                uvec a = find(abs(DeltaT) > 1000);
                if (a.n_elem > 0 )
                {

                    aux_DeltaT.set_size(DeltaT.n_elem-a.n_elem);
                    aux_DeltaT.zeros(DeltaT.n_elem-a.n_elem);
                    int j = 0;
                    for (int i = 0 ; i < DeltaT.n_elem ; i++)
                    {
                        if (abs(DeltaT(i)) < 1000)
                        {
                            aux_DeltaT(j) = DeltaT(i);
                            j++;
                        }
                    }

                    DeltaT = aux_DeltaT;
                }

                // Cantidad de bines
                centros_hist = linspace<vec>(-125,125,BinsHist);

                // Calculo el histograma
                ucolvec bin_val = hist(DeltaT, centros_hist);

                vec_log.set_size(0,0);
                vec_log = arma::conv_to<vec>::from(bin_val);
                stream<<"               Hist_"<<iter_i+1<<"_"<<iter_j+1<<" = "<<guardar_vector_stream(vec_log)<<endl;

                // Calculo la media y el desvio
                double media = centros_hist(bin_val.index_max());
                double desvio = stddev(DeltaT);

                // Guardo
                desvios_medios(iter_i,iter_j) = media;
                desvios_desvio(iter_i,iter_j) = desvio;
            }

        }
        cout<<"Finalizado pivote "<<iter_i+1<<" de 6"<<endl;
    }

    stream<<"desvios_medios = "<< guardar_matriz_stream(desvios_medios)<<endl;
    stream<<"desvios_desvio = "<< guardar_matriz_stream(desvios_desvio)<<endl;


    // Parto tomando al primer cabezal como correcto
    vec correciones;
    correciones.set_size(CANTIDADdEcABEZALES);
    correciones.zeros(CANTIDADdEcABEZALES);

    // Corrijo el que esta en frente
    correciones(3) = desvios_medios(0,3) + correciones(0);
    // Del que esta en frente corrijo los dos costados
    correciones(5) = desvios_medios(3,5) + correciones(3);
    correciones(1) = desvios_medios(3,1) + correciones(3);
    // Finalmente de los costados voy a los que ellos tienen en frente
    correciones(4) = desvios_medios(1,4) + correciones(1);
    correciones(2) = desvios_medios(5,2) + correciones(5);

    stream<<"Ci_crudo = "<< guardar_vector_stream(correciones)<<endl;
    vec coeficientes = correciones + abs(min(correciones));
    stream<<"Ci = "<< guardar_vector_stream(coeficientes)<<endl;

    for (int i = 0 ; i < CANTIDADdEcABEZALES ; i++)
    {
        Ci[i] = coeficientes(i);
    }

    // Guardo

    QString nombre_inter_cab = path_salida+"Coef_tiempo_inter_Cabezal"+".txt";

    QFile file(nombre_inter_cab);

    if (file.open(QIODevice::ReadWrite|QIODevice::Truncate))
    {
        QTextStream filestream(&file);
        for (int i = 0 ; i < CANTIDADdEcABEZALES ; i++)
        {
            filestream << round(Ci[i])  << endl;
        }
    }






    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------SALIDA DE: calibrar_tiempo_intercabezal -------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;

    return 1;
}






/* -------------------------------------------------------------------
 * --------------------Visualizar Planar------------------------------
 * -------------------------------------------------------------------
 */


bool AutoCalib::visualizar_planar(void)
{
    // Para todos los cabezales que se seleccionaron
    for(int i = 0 ; i < Cab_List.length() ; i++)
    {
        // Convierto de numero de cabezal a indce (-1)
        int cab_num_act = Cab_List[i]-1;


        // Abro un archivo de log
        QString nombre_cab = QString::number(cab_num_act+1);
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];
        time (&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer,sizeof(buffer),"%d_%m_%Y_%I_%M_%S",timeinfo);
        std::string str(buffer);
        //QString nombre_Log = path_salida+"Log_Visualizacion_Cabezal_"+nombre_cab+"_"+QString::fromStdString( str )+".txt";
        QString nombre_Log = path_salida+"Log_Visualizacion_Cabezal_"+nombre_cab+".txt";
        cout<<"Creando log: "<<nombre_Log.toStdString()<<endl;
        QFile file(nombre_Log);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
            cout<<"Error al abrir log"<<endl;
            return -1;
        }

        //QTextStream stream_open(&file);
        stream.setDevice(&file);

        stream<<"Log iniciado: "<<asctime(timeinfo)<<endl;

        // Vector de tablas a cargar
        bool carga[5] = {0 , 0 , 0 , 0, 0};

        // Recupero las calibraciones necesarias
        for(int i = 0 ; i < Vis_List.length() ; i++)
        {
            // Energia
            if (Vis_List[i] == 1) carga[0] = 1;
            // Posicion
            if (Vis_List[i] == 2)
            {
                carga[0] = 1;
                carga[2] = 1;
            }
            // skimm
            if (Vis_List[i] == 3)
            {
                carga[0] = 1;
                carga[2] = 1;
                carga[3] = 1;
            }
        }

        // Cargo las tablas de este cabezal
        cargar_tablas(cab_num_act, carga);

        // Primero levanto el archivo
        LevantarArchivo_Planar(cab_num_act);

        // Calculo las posiciones de los eventos si asi fuera necesario
        if (carga[2])
        {
            // Borro el almohadon anterior si lo hubiera
            almohadon[cab_num_act].set_size(BinsAlmohadon,BinsAlmohadon);
            almohadon[cab_num_act].zeros(BinsAlmohadon,BinsAlmohadon);

            calcular_almohadon(cab_num_act);
        }


        // Para todos las visualizaciones seleccionadas
        for(int i = 0 ; i < Vis_List.length() ; i++)
        {
            switch (Vis_List[i])
            {
                case 0:    // Mostrar espectro crudo
                    Espectro_emergente_crudo.clearGraphs();
                    plotear_espectro(cab_num_act, 0);
                    qApp->processEvents();
                    break;
                case 1:    // Mostrar espectro calibrado
                    Espectro_emergente.clearGraphs();
                    plotear_espectro(cab_num_act, 1);
                    qApp->processEvents();
                    break;
                case 2:    // Mostrar planar
                    mostrar_almohadon(cab_num_act,0,0);
                    qApp->processEvents();
                    break;
                case 3:    // Mostrar planar con count skimming
                    mostrar_almohadon(cab_num_act,0,1);
                    qApp->processEvents();
                    break;
                default:
                    cout<<"no se como llegue a querer visualizar el metodo inexistente nro: "<<Vis_List[i]<<endl;
                    return -1;
                    break;
            }
        }

        // Borro los archivos de ram
        Energia_calib[cab_num_act].set_size(1, 1);
        Tiempos_calib[cab_num_act].set_size(1, 1);
        Tiempos_full_calib[cab_num_act].set_size(1, 1);
        TimeStamp_calib[cab_num_act].set_size(1, 1);


        stream<<"Log finalizado: "<<asctime(timeinfo)<<endl;

    }


}








/* -------------------------------------------------------------------
 * --------------------Funciones Datos---------------------------------
 * -------------------------------------------------------------------
 */





Pico_espectro AutoCalib::Buscar_Pico(double* Canales, int num_canales)
{
    struct Pico_espectro Pico_calculado;
    Pico_calculado.canal_pico = -1;
    Pico_calculado.limites_FWHM[0] = 0;
    Pico_calculado.limites_FWHM[1] = 0;
    Pico_calculado.limites_FWTM[0] = 0;
    Pico_calculado.limites_FWTM[1] = 0;
    Pico_calculado.limites_Pico[0] = 0;
    Pico_calculado.limites_Pico[1] = 0;

    int window_size = num_canales/20;
    int span = (window_size);
    int min_count_diff = 50;
    double Low_win, High_win;
    char Estados[4] = {0 , 0 , 0, 0};
    char Estado_aux;
    int ind_estado = 0;
    int outpoint = -1, low_extrema = -1;

    // Seteo el limite
    // Copio los canales a una matriz
    mat Canales_mat(Canales,num_canales,1);
    min_count_diff = (int) (sum(sum(Canales_mat))*0.003);

    // La busqueda arranca en el ultimo canal
    for (int i = num_canales ; i >= window_size ; i--)
    {
        // Reseteo las ventanas
        Low_win = 0;
        High_win = 0;

        // Calculo los valores de las ventanas
        for (int j = 0 ; j < window_size ; j ++)
        {
            High_win += Canales[i - j];
        }
        High_win = High_win/window_size;
        for (int j = 0 ; j < window_size ; j ++)
        {
            Low_win += Canales[i - window_size - span - j];
        }
        Low_win = Low_win/window_size;

        // Me fijo la direccion
        if ((Low_win - High_win) > min_count_diff)
        {
            Estado_aux = 1;
        }
        else if (-(Low_win - High_win) > min_count_diff)
        {
            Estado_aux = -1;
        }
        else
        {
            Estado_aux = 0;
        }

        // Si el estado cambio lo actualizo
        if (Estados[ind_estado] != Estado_aux)
        {
            ind_estado++;
            Estados[ind_estado] = Estado_aux;
        }

        // Checkeo si encontre pico
        if (ind_estado == 3)
        {
            if( Estados[0] == 1 && Estados[1] == 0 && Estados[2] == -1 &&  (Estados[3] == 1 || Estados[3] == 0 ) )
            {
                // Retorno lo que encontre
                low_extrema = (i - (2*window_size) - span);
                outpoint = i;

                Pico_calculado.limites_Pico[0] = low_extrema;
                Pico_calculado.limites_Pico[1] = outpoint;

                break;
            }
            else
            {
                // Si no encontre nada corro la ventana de estados
                Estados[0] =  Estados[1];
                Estados[1] =  Estados[2];
                Estados[2] =  Estados[3];
                Estados[3] =  0;
                ind_estado = 2;
            }
        }

    }
    if (outpoint == -1) {return Pico_calculado;};
    if (low_extrema < window_size) {return Pico_calculado;};



    // Fitteo la curva gausseana por newton-gauss by Juan
    int NP = 3;

    // Me quedo solo con los eventos en el limite encontrado
    //int ndatos = Pico_calculado.limites_Pico[1] - Pico_calculado.limites_Pico[0];
    int ndatos = (num_canales-1) - (low_extrema-window_size);
    double* x;
    double* y;
    x = new double[ndatos];
    y = new double[ndatos];
    for (int i=0 ; i<ndatos ; i++)
    {
        x[i] = (low_extrema-window_size) + i;
        int aux = x[i];
        y[i] = Canales[aux];
    }


/*
    time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);


    QString path_salida = "Salidas/";

    if ( 1)
    {
        strftime(buffer,sizeof(buffer),"%d_%m_%Y_%I_%M_%S",timeinfo);
        std::string str(buffer);
        QString nombre_ener = path_salida +QString::fromStdString( str );

        QFile file(nombre_ener);

        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file);
            for (int i = 0 ; i < ndatos ; i++)
            {
                stream << y[i]  << endl;
            }
        }

    }
*/


    // Defino los parametros
    // Una vez encontrado el pico de manera rudimentaria, le fiteo una gauss
    // y sale el armadillo


    // Me quedo solo con los canales que me interesan, los que estan dentro de las ventanas
    mat canales_peak(y,ndatos,1);

    // Maximo de la gausseana
    uword max_idx, std_idx;
    double Gauss_max = canales_peak.max(max_idx);
    double Gauss_mean = max_idx;
    // Busco el 68 % desde el lado de mas alta energia
    uvec mayores_std = find(canales_peak > 0.68*Gauss_max);
    mayores_std.max(std_idx);
    double Gauss_std = std_idx;

    double p[NP];
    p[0] = 1;
    p[1] = Gauss_mean+(low_extrema-window_size);
    //p[2] = Gauss_std*2*3.14;
    //p[2] = p[1]/10;
    p[2] = ((p[1]*0.08)/2)*2;
    double paso = 0.01;
    int len = 100;

    estadogna estado;

    if(inicializargna(&estado,ndatos,NP))
    {
        fprintf(stderr,"Sin memoria suficiente\n");
        Pico_calculado.canal_pico = -1;
        Pico_calculado.limites_FWHM[0] = 0;
        Pico_calculado.limites_FWHM[1] = 0;
        Pico_calculado.limites_FWTM[0] = 0;
        Pico_calculado.limites_FWTM[1] = 0;
        Pico_calculado.limites_Pico[0] = 0;
        Pico_calculado.limites_Pico[1] = 0;
        return Pico_calculado;
    }

    double tmp;
    for(int i=0;i<ndatos;i++)
        {
            if(!i) tmp = y[i];
            if(i && y[i]>tmp) tmp=y[i];
        }
    for(int i=0;i<ndatos;i++)  y[i]/=tmp;


    //for (int i = 0 ; i < ndatos ; i++) cout<<y[i]<<endl;
    //for (int i = 0 ; i < ndatos ; i++) cout<<f_gauss(x[i],p)<<endl;



    double sseprev=0.0;
    for(int i=0,sseprev=0.0;i<len;i++)
    {
        double sse = sumacuadrados(x,y,ndatos,p,&AutoCalib::f_gauss);
        printf("(%d)\tsse: %f\n",i+1,sse);
        if(gaussnewton(x,y,ndatos,p,NP,paso,&AutoCalib::f_gauss,&AutoCalib::df_gauss,&estado)<0) break;
        if(sse>sseprev && i) break;
        sseprev=sse;
    }
    //printf("\n\n");
    //printf("ym: %f\n",tmp);
    //for(int i=0;i<NP;i++)
    //{
    //    printf("p%d: %f\n",i,p[i]);
    //}
    //printf("%0.01f\n",p[1]+(double)min);
    liberargna(&estado);
    delete(x);
    delete(y);


    // Recupero el canal del pico
    Pico_calculado.canal_pico = p[1];

    // Calculo el FWTM usando la función fiteada
    int i = 0;
    while (Pico_calculado.limites_FWTM[0] == 0)
    {
        if (0.1 >=  f_gauss(round(Pico_calculado.canal_pico) - i , p)  )
        {
           Pico_calculado.limites_FWTM[0] =  round(Pico_calculado.canal_pico) - i +1;
        }
        i++;
    }
    i = 0;
    while (Pico_calculado.limites_FWTM[1] == 0)
    {
        if (0.1 >=  f_gauss(round(Pico_calculado.canal_pico) + i , p)  )
        {
           Pico_calculado.limites_FWTM[1] =  round(Pico_calculado.canal_pico) + i -1;
        }
        i++;
    }
    Pico_calculado.FWTM = (Pico_calculado.limites_FWTM[1]-Pico_calculado.limites_FWTM[0])/round(Pico_calculado.canal_pico);
/*
    // Calculo el FWHM usando la función fiteada
    i = 0;
    while (Pico_calculado.limites_FWHM[0] == 0)
    {
        if (0.5 >=  f_gauss(round(Pico_calculado.canal_pico) - i , p)  )
        {
           Pico_calculado.limites_FWHM[0] =  round(Pico_calculado.canal_pico) - i +1;
        }
        i++;
    }
    i = 0;
    while (Pico_calculado.limites_FWHM[1] == 0)
    {
        if (0.5 >=  f_gauss(round(Pico_calculado.canal_pico) + i , p) )
        {
           Pico_calculado.limites_FWHM[1] =  round(Pico_calculado.canal_pico) + i -1;
        }
        i++;
    }
    Pico_calculado.FWHM = (Pico_calculado.limites_FWHM[1]-Pico_calculado.limites_FWHM[0])/round(Pico_calculado.canal_pico);
*/





    //double maximo_pico = canales_peak.max();
    double maximo_pico = Canales_mat(round(Pico_calculado.canal_pico));
    // Calculo el FWHM usando el espectro
    i = 0;
    while (Pico_calculado.limites_FWHM[0] == 0)
    {
        if (maximo_pico*0.5 >=  Canales_mat(round(Pico_calculado.canal_pico - i))  )
        {
           Pico_calculado.limites_FWHM[0] =  Pico_calculado.canal_pico - i +1;
        }
        i++;
    }
    i = 0;
    while (Pico_calculado.limites_FWHM[1] == 0)
    {
        if (maximo_pico*0.5 >=  Canales_mat(round(Pico_calculado.canal_pico + i))  )
        {
           Pico_calculado.limites_FWHM[1] =  Pico_calculado.canal_pico + i -1;
        }
        i++;
    }
    Pico_calculado.FWHM = (Pico_calculado.limites_FWHM[1]-Pico_calculado.limites_FWHM[0])/Pico_calculado.canal_pico;
/*
    // Calculo el FWTM usando el espectro
    i = 0;
    while (Pico_calculado.limites_FWTM[0] == 0)
    {
        if (maximo_pico*0.1 >=  Canales_mat(round(Pico_calculado.canal_pico - i))  )
        {
           Pico_calculado.limites_FWTM[0] =  Pico_calculado.canal_pico - i +1;
        }
        i++;
    }
    i = 0;
    while (Pico_calculado.limites_FWTM[1] == 0)
    {
        if (maximo_pico*0.1 >=  Canales_mat(round(Pico_calculado.canal_pico + i))  )
        {
           Pico_calculado.limites_FWTM[1] =  Pico_calculado.canal_pico + i -1;
        }
        i++;
    }
    Pico_calculado.FWTM = (Pico_calculado.limites_FWTM[1]-Pico_calculado.limites_FWTM[0])/Pico_calculado.canal_pico;

*/







    return Pico_calculado;

}

















/* -------------------------------------------------------------------
 * --------------------Funciones MCAE---------------------------------
 * -------------------------------------------------------------------
 */

bool AutoCalib::pedir_MCA_PMT(int Cabezal, int PMT, int canales, bool Calib)
{


    string msg, msg_data;
    size_t bytes_transfered = 0;
    string sended;
    QVector<double> canales_pmt, hits_pmt;

    QString Cabezal_str, PMT_str;
    Cabezal_str = QString::number(Cabezal);
    PMT_str = QString::number(PMT);

    if (Calib == true)
    {
        setHeader_MCAE(getHead_MCAE() + Cabezal_str.toStdString() + getFunCHead());
    }
    else
    {
        setHeader_MCAE(getHead_MCAE() + Cabezal_str.toStdString() + getFunCSP3());
    }

    setMCAEStream(PMT_str.toStdString(), canales*6+16, getData_MCA(), "");

    sended = getTrama_MCAE() + getEnd_MCA();

    portFlush();

    //cout << "Enviando a cabezal "<<Cabezal_str.toStdString()<<" PMT "<<PMT_str.toStdString()<<endl;
    cout<<"Get MCA PMT"<<endl;
    cout<<sended<<endl;


    try
    {
        bytes_transfered = portWrite(&sended, port_name.toStdString().c_str());
    }
    catch(boost::system::system_error e)
    {
      cout << "No se puede acceder al puerto serie. (pedir MCA)"<<endl;
        Exceptions exception_serial_port((string("No se puede acceder al puerto serie. Error: ")+string(e.what())).c_str());
        return -1;
    }

    //cout << "Leyendo"<<endl;
    try
    {
         portReadString(&msg,'\r', port_name.toStdString().c_str());                  //     msg = readString();
    }
    catch( Exceptions & ex )
    {
      cout << "No se puede leer. (pedir MCA)"<<endl;
         Exceptions exception_stop(ex.excdesc);
         return -1;

    }

    //cout<<msg<<endl;

    //cout << "Leyendo el buffer"<<endl;
    try{
             portReadBufferString(&msg_data,canales*6+16, port_name.toStdString().c_str());    //   msg_data = readBufferString(channels*6+16);
        }
        catch( Exceptions & ex ){
          cout << "No se leer... aparentemente... (pedir MCA)"<<endl;
             Exceptions exception_stop(ex.excdesc);
             return -1;
        }

    //cout << "Leyendo los datos"<<endl;
    getMCASplitData(msg_data, canales);

    //cout << "Obteniendo channels"<<endl;
    canales_pmt = getChannels();
    //cout << "Obteniendo hits"<<endl;
    hits_pmt = getHitsMCA();

/*
    cout<<"Canales:"<<endl;
    for(int i=0; i<canales_pmt.length();i++) { cout<<canales_pmt[i]<<","; }
    cout<<endl<<"Hits:"<<endl;
    for(int i=0; i<hits_pmt.length();i++) { cout<<hits_pmt[i]<<","; }
    cout<<endl;
*/

    return 1;
}



bool AutoCalib::modificar_HV_PMT(int Cabezal, int PMT,  int val_dinodo)
{
    string msg;
    size_t bytes_transfered = 0;
    string sended;

    QString Cabezal_str, PMT_str, val_dinodo_str;
    Cabezal_str = QString::number(Cabezal);
    PMT_str = QString::number(PMT);
    val_dinodo_str = QString::number(val_dinodo);

    setHeader_MCAE(getHead_MCAE() + Cabezal_str.toStdString() + getFunCSP3());


    setMCAEStream(PMT_str.toStdString(), 0, getSetHV_MCA(), val_dinodo_str.toStdString());

    sended = getTrama_MCAE() + getEnd_MCA();

    portFlush();

    //cout << "Enviando a cabezal "<<Cabezal_str.toStdString()<<" PMT "<<PMT_str.toStdString()<<endl;
    cout<<"Set HV dinodo"<<endl;
    cout<<sended<<endl;


    try
    {
        bytes_transfered = portWrite(&sended, port_name.toStdString().c_str());
    }
    catch(boost::system::system_error e)
    {
      cout << "No se puede acceder al puerto serie. (modif HV)"<<endl;
        Exceptions exception_serial_port((string("No se puede acceder al puerto serie. Error: ")+string(e.what())).c_str());
        return -1;
    }

    //cout << "Leyendo"<<endl;
    try
    {
         portReadString(&msg,'\r', port_name.toStdString().c_str());                  //     msg = readString();
    }
    catch( Exceptions & ex )
    {
      cout << "No se puede leer. (modif HV)"<<endl;
         Exceptions exception_stop(ex.excdesc);
         return -1;

    }

    return 1;
}



bool AutoCalib::reset_Mem_Cab(int Cabezal)
{
    string msg;
    size_t bytes_transfered = 0;
    string sended;

    QString Cabezal_str, PMT_str, val_dinodo_str;
    Cabezal_str = QString::number(Cabezal);
    val_dinodo_str = "150";
    PMT_str = "00";

    setHeader_MCAE(getHead_MCAE() + Cabezal_str.toStdString() + getFunCHead());


    setMCAEStream(PMT_str.toStdString(), 0, getSetHV_MCA(), val_dinodo_str.toStdString());

    sended = getTrama_MCAE() + getEnd_MCA();

    portFlush();

    //cout << "Enviando a cabezal "<<Cabezal_str.toStdString()<<" PMT "<<PMT_str.toStdString()<<endl;
    cout<<"Reset Cabezal"<<endl;
    cout<<sended<<endl;


    try
    {
        bytes_transfered = portWrite(&sended, port_name.toStdString().c_str());
    }
    catch(boost::system::system_error e)
    {
      cout << "No se puede acceder al puerto serie. (Reset Cab)"<<endl;
        Exceptions exception_serial_port((string("No se puede acceder al puerto serie. Error: ")+string(e.what())).c_str());
        return -1;
    }

    //cout << "Leyendo"<<endl;
    try
    {
         portReadString(&msg,'\r', port_name.toStdString().c_str());                  //     msg = readString();
    }
    catch( Exceptions & ex )
    {
      cout << "No se puede leer. (Reset Cab)"<<endl;
         Exceptions exception_stop(ex.excdesc);
         return -1;

    }
    cout << msg<<endl;

    return 1;
}




































/* -------------------------------------------------------------------
 * --------------------Funciones Varias-------------------------------
 * -------------------------------------------------------------------
 */

bool AutoCalib::plotear_espectro(int cab_num_act,  bool calibrado)
{
    // Recupero la constante
    colvec Ce_arma(Ce[cab_num_act], CANTIDADdEpMTS);

    // Matriz auxiliar para plotear
    mat Energia_calib_aux = Energia_calib[cab_num_act];

    colvec centros_hist;
    if(calibrado)
    {
        // Paso todos los eventos a eventos en energía calibrada
        Energia_calib_aux.each_col() %= Ce_arma;

        // Creo el vector de centros para el histograma
        centros_hist = linspace<vec>(0,1024,BinsHist);
    }
    else
    {
        // Creo el vector de centros para el histograma
        centros_hist = linspace<vec>(0,8000,BinsHist);
    }



    // Busco el pico
    struct Pico_espectro pico_calib;
    double aux_espectro[BinsHist];

    // Calculo la suma
    rowvec Energia_calib_suma = sum(Energia_calib_aux,0);
    // Calculo el espectro
    urowvec espectro_suma = hist(Energia_calib_suma, centros_hist);
    // Calculo el FWHM
    for (int i=0 ; i < BinsHist ; i++)
    {
        aux_espectro[i] = espectro_suma(i);
    }
    pico_calib = Buscar_Pico(aux_espectro, BinsHist);


    // ----------------------- Ploteo
    // Paso los vectores a Qvector para plotear
    QVector<double> aux_qvec_cent(BinsHist);
    for (int i=0 ; i < BinsHist ; i++){aux_qvec_cent[i] = centros_hist(i);}
    QVector<double> aux_qvec(BinsHist);
    for (int i=0 ; i < BinsHist ; i++){aux_qvec[i] = espectro_suma(i);}

    if(calibrado)
    {
        QString nombre_plot = "Espectro calibrado cabezal "+ QString::number(cab_num_act+1)+" FWHM = "+ QString::number(pico_calib.FWHM*100) + "%";
        plot_MCA(aux_qvec, aux_qvec_cent,&Espectro_emergente, nombre_plot, param_cab[cab_num_act], 0);
        Espectro_emergente.show();
        Espectro_emergente.resize(1000,500);
        qApp->processEvents();
    }
    else
    {
        QString nombre_plot = "Espectro crudo cabezal "+ QString::number(cab_num_act+1)+" FWHM = "+ QString::number(pico_calib.FWHM*100) + "%";
        plot_MCA(aux_qvec, aux_qvec_cent,&Espectro_emergente_crudo, nombre_plot, param_cab[cab_num_act], 0);
        Espectro_emergente_crudo.show();
        Espectro_emergente_crudo.resize(1000,500);
        qApp->processEvents();
    }


    return 1;
}



bool AutoCalib::mostrar_almohadon(int cab_num_act, bool calib, bool skimm)
{
    // Almohadon auxiliar para presentar
    mat almohadon_aux = almohadon[cab_num_act];

    // Aplico el skimming de ser solicitado
    if (skimm) almohadon_aux %= count_skimm[cab_num_act];

    // Recta de escala de colores
    double val_max = almohadon_aux.max();
    double val_mid = val_max/2;
    double val_min = 0;
    double Y1, X1, Y2, X2;





    X1 = val_min;
    Y1 = 255;
    X2 = val_mid;
    Y2 = 0;
    double a_b = ((Y2-Y1)/(X2-X1)) , b_b = Y1 - X1 * ((Y2-Y1)/(X2-X1));

    X1 = val_min;
    Y1 = 0;
    X2 = val_mid;
    Y2 = 255;
    double a_g_1 = ((Y2-Y1)/(X2-X1)) , b_g_1 = Y1 - X1 * ((Y2-Y1)/(X2-X1));

    X1 = val_mid;
    Y1 = 255;
    X2 = val_max;
    Y2 = 0;
    double a_g_2 = ((Y2-Y1)/(X2-X1)) , b_g_2 = Y1 - X1 * ((Y2-Y1)/(X2-X1));

    X1 = val_mid;
    Y1 = 0;
    X2 = val_max;
    Y2 = 255;
    double a_r = ((Y2-Y1)/(X2-X1)) , b_r = Y1 - X1 * ((Y2-Y1)/(X2-X1));


    cube aux_save;
    aux_save.set_size(BinsAlmohadon,BinsAlmohadon,3);

    for(int i = 0 ; i<BinsAlmohadon ; i++)
    {
        for(int j = 0 ; j<BinsAlmohadon ; j++)
        {
            aux_save(i,j,0) = almohadon_aux(i,j)*a_r + b_r;

            double green_aux = almohadon_aux(i,j)*a_g_1 + b_g_1 ;
            if (green_aux > 255 || green_aux < 0)
            {
                aux_save(i,j,1) = almohadon_aux(i,j)*a_g_2 + b_g_2 ;
            }
            else
            {
                aux_save(i,j,1) = green_aux;
            }

            aux_save(i,j,2) = almohadon_aux(i,j)*a_b + b_b;
        }
    }


    // Guardo las imagenes
    QString nombre_cab = QString::number(cab_num_act+1);
    QString nombre_almohadon;
    QString path_salida_aux;
    if (!calib)
    {
        path_salida_aux = path_salida+".temp_";
    }
    else
    {
        path_salida_aux = path_salida;
    }

    if (skimm) nombre_almohadon = path_salida_aux+"Almohadon_Cabezal_"+nombre_cab+"_skimmed.pgm";
    else nombre_almohadon = path_salida_aux+"Almohadon_Cabezal_"+nombre_cab+".pgm";
    almohadon_aux.save(nombre_almohadon.toStdString(), pgm_binary);
    if (skimm) nombre_almohadon = path_salida_aux+"Almohadon_Cabezal_"+nombre_cab+"_skimmed.ppm";
    else nombre_almohadon = path_salida_aux+"Almohadon_Cabezal_"+nombre_cab+".ppm";
    aux_save.save(nombre_almohadon.toStdString(), ppm_binary);

    // Las muestro
    QPixmap file(nombre_almohadon);
    if (skimm)
    {
        Almohadon_emergente_skimmed[cab_num_act].setPixmap(file);
        Almohadon_emergente_skimmed[cab_num_act].show();
        Almohadon_emergente_skimmed[cab_num_act].setScaledContents(true);
    }
    else
    {
        Almohadon_emergente[cab_num_act].setPixmap(file);
        Almohadon_emergente[cab_num_act].show();
        Almohadon_emergente[cab_num_act].setScaledContents(true);
    }




}





void AutoCalib::guardar_tablas(int cab_num_act, bool* tipo)
{
    QString nombre_cab = QString::number(cab_num_act+1);

    // Energías
    if (tipo[0] == 1)
    {
        QString nombre_ener = path_salida+"Coef_Energia_Cabezal_"+nombre_cab+".txt";

        QFile file(nombre_ener);

        if (file.open(QIODevice::ReadWrite|QFile::Truncate))
        {
            QTextStream filestream(&file);
            for (int i = 0 ; i < CANTIDADdEpMTS ; i++)
            {
                filestream << round(Ce[cab_num_act][i]*1000)  << endl;
            }
        }

    }

    // Tiempos
    if (tipo[1] == 1)
    {
        QString nombre_tiempo = path_salida+"Tiempos_Cabezal_"+nombre_cab+".txt";

        QFile file(nombre_tiempo);

        if (file.open(QIODevice::ReadWrite|QFile::Truncate))
        {
            QTextStream filestream(&file);
            for (int i = 0 ; i < CANTIDADdEpMTS ; i++)
            {
                filestream << Ct[cab_num_act][i]  << endl;
            }
        }

    }

    // Posicion
    if (tipo[2] == 1)
    {
        QString nombre_pos_x = path_salida+"Cx_Cabezal_"+nombre_cab+".txt";

        QFile file_x(nombre_pos_x);

        if (file_x.open(QIODevice::ReadWrite|QFile::Truncate))
        {
            QTextStream filestream(&file_x);
            for (int i = 0 ; i < CANTIDADdEpMTS ; i++)
            {
                // Antes de grabar lo saturo en 10 bit
                int aux_c;
                aux_c = round(Cx[cab_num_act][i]*1000);
                int c_sat = std::min(std::max(aux_c, -511), 511);
                filestream <<c_sat<< endl;
            }
        }

        QString nombre_pos_y = path_salida+"Cy_Cabezal_"+nombre_cab+".txt";

        QFile file_y(nombre_pos_y);

        if (file_y.open(QIODevice::ReadWrite|QFile::Truncate))
        {
            QTextStream filestream(&file_y);
            for (int i = 0 ; i < CANTIDADdEpMTS ; i++)
            {
                // Antes de grabar lo saturo en 10 bit
                int aux_c;
                aux_c = round(Cy[cab_num_act][i]*1000);
                int c_sat = std::min(std::max(aux_c, -511), 511);
                filestream <<c_sat<< endl;
            }
        }

    }

    // Count skimming
    if (tipo[3] == 1)
    {
        QString nombre_skimm = path_salida+"CountSkimm_Cab_"+nombre_cab+".txt";

        count_skimm[cab_num_act].save(nombre_skimm.toStdString(), raw_ascii);
    }

    // Count skimming inter cabezal
    if (tipo[4] == 1)
    {
        QString nombre_skimm_inter = path_salida+"CountSkimm_Inter_Cab_"+nombre_cab+".txt";
        count_skimm_inter_cab[cab_num_act].save(nombre_skimm_inter.toStdString(), raw_ascii);
    }

}

void AutoCalib::cargar_tablas(int cab_num_act, bool* tipo)
{
    QString nombre_cab = QString::number(cab_num_act+1);


    // Energías
    if (tipo[0] == 1)
    {
        QString nombre_ener = path_entrada+"Coef_Energia_Cabezal_"+nombre_cab+".txt";

        QFile file(nombre_ener);

        if (file.open(QIODevice::ReadOnly|QFile::Truncate))
        {
            QTextStream filestream(&file);
            for (int i = 0 ; i < CANTIDADdEpMTS ; i++)
            {
                Ce[cab_num_act][i] = filestream.readLine().toDouble()/1000;
            }
        }
        else
        {
            cout<<"No puedo leer archivo de calibracion de energia ("<<nombre_ener.toStdString()<<")"<<endl;
        }

    }

    // Tiempos
    if (tipo[1] == 1)
    {
        QString nombre_tiempo = path_entrada+"Tiempos_Cabezal_"+nombre_cab+".txt";

        QFile file(nombre_tiempo);

        if (file.open(QIODevice::ReadOnly|QFile::Truncate))
        {
            QTextStream filestream(&file);
            for (int i = 0 ; i < CANTIDADdEpMTS ; i++)
            {
                Ct[cab_num_act][i] = filestream.readLine().toDouble();
            }
        }
        else
        {
            cout<<"No puedo leer archivo de calibracion de tiempos ("<<nombre_tiempo.toStdString()<<")"<<endl;
        }


    }

    // Posicion
    if (tipo[2] == 1)
    {
        QString nombre_pos_x = path_entrada+"Cx_Cabezal_"+nombre_cab+".txt";

        QFile file_x(nombre_pos_x);

        if (file_x.open(QIODevice::ReadOnly|QFile::Truncate))
        {
            QTextStream filestream(&file_x);
            for (int i = 0 ; i < CANTIDADdEpMTS ; i++)
            {
                Cx[cab_num_act][i] = filestream.readLine().toDouble()/1000;
            }
        }
        else
        {
            cout<<"No puedo leer archivo de calibracion en posicion X ("<<nombre_pos_x.toStdString()<<")"<<endl;
        }


        QString nombre_pos_y = path_entrada+"Cy_Cabezal_"+nombre_cab+".txt";

        QFile file_y(nombre_pos_y);

        if (file_y.open(QIODevice::ReadOnly|QFile::Truncate))
        {
            QTextStream filestream(&file_y);
            for (int i = 0 ; i < CANTIDADdEpMTS ; i++)
            {
                Cy[cab_num_act][i] = filestream.readLine().toDouble()/1000;
            }
        }
        else
        {
            cout<<"No puedo leer archivo de calibracion en posicion Y ("<<nombre_pos_y.toStdString()<<")"<<endl;
        }

    }

    // Count skimming
    if (tipo[3] == 1)
    {
        QString nombre_skimm = path_entrada+"CountSkimm_Cab_"+nombre_cab+".txt";
        count_skimm[cab_num_act].load(nombre_skimm.toStdString(), raw_ascii);
    }

    // Count skimming inter cabezal
    if (tipo[4] == 1)
    {
        QString nombre_skimm_inter = path_entrada+"CountSkimm_Inter_Cab_"+nombre_cab+".txt";
        count_skimm_inter_cab[cab_num_act].load(nombre_skimm_inter.toStdString(), raw_ascii);
    }

}

QString AutoCalib::guardar_vector_stream(vec guardar)
{
    stringstream salida;
    salida<<"["<<guardar(0);
    for (int log_ind = 1 ; log_ind < guardar.n_elem ; log_ind++)
        salida<<" , "<< guardar(log_ind);
    salida<<"];"<<endl;

    QString out_salida = QString::fromStdString(salida.str());

    return out_salida;
}


QString AutoCalib::guardar_matriz_stream(mat guardar)
{
    stringstream salida;
    salida<<"[";
    for (int log_ind_2 = 0 ; log_ind_2 < guardar.n_rows ; log_ind_2++)
    {
        for (int log_ind = 0 ; log_ind < guardar.n_cols ; log_ind++)
        {
            salida<< guardar(log_ind_2, log_ind)<<" ,";
        }
        salida.seekp(-1, std::ios_base::end);
        salida<<";";
    }
    salida.seekp(-1, std::ios_base::end);
    salida<<"];"<<endl;

    QString out_salida = QString::fromStdString(salida.str());

    return out_salida;
}


void AutoCalib::plot_MCA(QVector<double> hits, QVector<double> channels_ui, QCustomPlot *graph, QString graph_legend, int *param, bool clear )
{

    if (clear)
    {
        graph->clearGraphs();
    }



    graph->addGraph();
    graph->graph()->setName(graph_legend);
    graph->graph()->setData(channels_ui,hits);
    graph->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(param[4])));
    QPen graphPen;
    graphPen.setColor(QColor(param[0], param[1], param[2]));
    graphPen.setWidthF(param[5]);
    graph->graph()->setPen(graphPen);
    graph->legend->setVisible(true);
    graph->legend->setWrap(4);
    graph->legend->setRowSpacing(1);
    graph->legend->setColumnSpacing(2);
    graph->rescaleAxes();
    graph->replot();





}


bool AutoCalib::LevantarArchivo_Planar(int cab_num_act)
{

    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------INICIO DE: LevantarArchivo_Planar -------------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;




        /*
        // Reading size of file
        FILE * file = fopen("input.txt", "r+");
        if (file == NULL) return;
        fseek(file, 0, SEEK_END);
        long int size = ftell(file)
        */

    // Calculo los tamaños de la trama
    int CANTIDAdEbYTESeNERGIAyTIEMPO    =   ((CANTIDADdEpMTS*12)/8)+CANTIDADdEbYTEStIEMPO;
    int CANTiNFO                        =   CANTIDAdEbYTESeNERGIAyTIEMPO + CANTIDADdEbYTEStIMEsTAMP;


    // Recupero el nombre del archivo
    string nombre_adq = adq_cab[cab_num_act];
    stream << "Levantando archivo: "<<QString::fromStdString(nombre_adq)<< endl;


    // abro archivo
    FILE * archivo = fopen(nombre_adq.c_str(), "r");

    if (archivo == NULL)
    {
        cout<<"ahhh exploto el archivooooo"<< endl;
        stream <<"ahhh exploto el archivooooo"<< endl;
        return -1;
    }

    // Checkeo el tiempo de modificacion
    struct stat attr;
    stat(nombre_adq.c_str(), &attr);
    uint64_t access       = attr.st_atime;
    uint64_t status       = attr.st_ctime;
    uint64_t modification = attr.st_mtime;

    // Recupero el tiempo de creación
    size_t fin_adq_nom = adq_cab[cab_num_act].find_last_of(".raw");
    double anio = QString::fromStdString(adq_cab[cab_num_act].substr(fin_adq_nom - 18, 4)).toDouble();
    double mes = QString::fromStdString(adq_cab[cab_num_act].substr(fin_adq_nom - 14, 2)).toDouble();
    double dia = QString::fromStdString(adq_cab[cab_num_act].substr(fin_adq_nom - 12, 2)).toDouble();
    double hora = QString::fromStdString(adq_cab[cab_num_act].substr(fin_adq_nom - 9, 2)).toDouble();
    double min = QString::fromStdString(adq_cab[cab_num_act].substr(fin_adq_nom - 7, 2)).toDouble();
    double seg = QString::fromStdString(adq_cab[cab_num_act].substr(fin_adq_nom - 5, 2)).toDouble();

    struct tm t = {0};  // Initalize to all 0's
    t.tm_year = anio-1900;  // This is year-1900, so 112 = 2012
    t.tm_mon = mes-1;
    t.tm_mday = dia;
    t.tm_hour = hora;
    t.tm_min = min;
    t.tm_sec = seg;
    time_t timeSinceEpoch = mktime(&t);


    // Calculo el tamaño del archivo
    fseek(archivo, 0, SEEK_END);
    long int BytesLeer = ftell(archivo);
    rewind(archivo);
    stream <<"Se leeran "<<BytesLeer<<" Bytes"<< endl;


    // Leo todo a memoria
    unsigned char * entrada = (unsigned char *) malloc(BytesLeer);
    int bytes_leidos = fread(entrada, sizeof(unsigned char), BytesLeer, archivo);

    stream <<"Se leyeron "<<bytes_leidos<<" Bytes"<< endl;

    // Cierro archivo
    fclose(archivo);


    // Parseo el archivo
    int cantidadDeTramaSalida;                  /*Cantidad de bytes la trama de salida Valida */
    unsigned char *vectorSalida;                /*para la funcion */
    vectorSalida =  Trama(entrada,BytesLeer,&cantidadDeTramaSalida);


    // Calculo la cantidad de eventos leidos
    int num_columnas = cantidadDeTramaSalida/CANTiNFO;

    stream <<"Total de eventos: "<<num_columnas<<endl;

    // Configuro las matrices de los cabezales
    Energia_calib[cab_num_act].set_size(CANTIDADdEpMTS, num_columnas);
    Tiempos_calib[cab_num_act].set_size(CANTIDADdEpMTS, num_columnas);
    Tiempos_full_calib[cab_num_act].set_size(CANTIDADdEpMTS, num_columnas);
    TimeStamp_calib[cab_num_act].set_size(1, num_columnas);


    // Parseo todas las tramas y las guardo en las matrices
    //int trama_actual = 0;
    for (int trama_actual = 0 ; trama_actual < num_columnas ; trama_actual ++)
    {
        // Saco la trama parseada e invierto el orden
        unsigned char vectorSalidaInv[CANTiNFO];

        for (int i=CANTiNFO-1 ; i >= 0 ; i--)
        {
            vectorSalidaInv[CANTiNFO - 1 - i] = vectorSalida[i + (trama_actual*CANTiNFO)];
        }

        // Recupero el time stamp del evento
        double aux_very_high = vectorSalidaInv[0]<<24   & 0xFF000000;
        double aux_high = vectorSalidaInv[1]<<16        & 0x00FF0000;
        double aux_mid = vectorSalidaInv[2]<<8          & 0x0000FF00;
        double aux_low = vectorSalidaInv[3]             & 0x000000FF;;
        TimeStamp_calib[cab_num_act](trama_actual) = aux_very_high + aux_high + aux_mid + aux_low;

        // Me quedo con la energia y el tiempo
        unsigned char Energia_Tiempo[CANTIDAdEbYTESeNERGIAyTIEMPO];

        for (int i=0 ; i < CANTIDAdEbYTESeNERGIAyTIEMPO ; i++)
        {
            Energia_Tiempo[i] = vectorSalidaInv[CANTIDADdEbYTEStIMEsTAMP+i];
        }


        // Saco la energía por PMT
        for (int i=0 ; i < CANTIDADdEpMTS ; i++)
        {
            aux_high = Energia_Tiempo[i*(CANTIDAdEbYTESeNERGIAyTIEMPO/CANTIDADdEpMTS)] << 4;
            aux_low =  (Energia_Tiempo[ (i*(CANTIDAdEbYTESeNERGIAyTIEMPO/CANTIDADdEpMTS))+1 ] & 0xF0 ) >> 4 ;

            Energia_calib[cab_num_act](i,trama_actual) =  aux_high + aux_low ;
        }

        // Saco el tiempo por PMT
        for (int i=0 ; i < CANTIDADdEpMTS ; i++)
        {
            aux_high = (Energia_Tiempo[i*(CANTIDAdEbYTESeNERGIAyTIEMPO/CANTIDADdEpMTS) + 1] & 0x0F) << 16;
            aux_mid =  Energia_Tiempo[i*(CANTIDAdEbYTESeNERGIAyTIEMPO/CANTIDADdEpMTS) + 2] << 8;
            aux_low = Energia_Tiempo[i*(CANTIDAdEbYTESeNERGIAyTIEMPO/CANTIDADdEpMTS) + 3]  ;

            Tiempos_full_calib[cab_num_act](i,trama_actual) =  aux_high + aux_mid + aux_low ;
            Tiempos_calib[cab_num_act](i,trama_actual) = floor(Tiempos_full_calib[cab_num_act](i,trama_actual) / 64 );

        }

        // Filtro segun el time stamp del centroide (máximo)

        // busco el maximo
        int index_max = Energia_calib[cab_num_act].col(trama_actual).index_max();
        // me fijo el time stamp del mismo
        double tiempo_max = Tiempos_calib[cab_num_act](index_max,trama_actual);
        // Elimino aquellos fuera de la ventana de tiempo
        for (int i=0 ; i < CANTIDADdEpMTS ; i++)
        {
            double aux = Tiempos_calib[cab_num_act](i,trama_actual) - tiempo_max;

            if ( aux*aux > VENTANAdEtIEMPOeNTREeNERGIAS*VENTANAdEtIEMPOeNTREeNERGIAS)
            {
                Energia_calib[cab_num_act](i,trama_actual) = 0;
            }
        }



        /**************************************** HEREDADO DEL MATLAB DE AUTOR DESCONOCIDO  ***********************************************/
        // PARCHE, PARA QUE LA FUNCION DE BUSQUEDA DE COINCIDENCIAS NO SE
        // CONFUNDA, DEBO BORRAR LAS MARCAS DE TIEMPO QUE ESTAN MAL PORQUE NO
        // HAY CARRY ENTRE LAS SPARTAN Y LA VIRTEX. LA MARCAS DE TIEMPO CRECEN
        // CON EL TIEMPO, PERO CUANDO UNA CRECE DEMAS Y LA SIGUIENTE ES MENOR,
        // ES PORQUE HUBO UN DESBORDE DE LA SPARTAN, QUE NO SE PROPAGO HACIA LA
        // VIRTEX. ENTONCES ESAS MARCAS DE TIEMPO SON BORRADAS CON LA DERIVADA,
        // ESO SE DA CUANDO LA DERIVADA ES NEGATIVA.
        // 1) Llevo la marca de tiempo al inicio, porque la virtex la pone
        // despues de recibir las tramas de la spartan.
        TimeStamp_calib[cab_num_act](trama_actual) = TimeStamp_calib[cab_num_act](trama_actual) - CANTIADADdEcLOKS_1useg;
        // 2) Cuento los OverFlows de la virtex
        double cantidadDeOverFlows = TimeStamp_calib[cab_num_act](trama_actual)  /   pow(2,CANTIDADdEbITSeNTEROSsPARTAN);
        cantidadDeOverFlows = floor(cantidadDeOverFlows);
        // 3) Concateno los overflows al final de los bits de la spartan.
        TimeStamp_calib[cab_num_act](trama_actual) = cantidadDeOverFlows * pow(2,CANTIDADdEbITSeNTEROSsPARTAN) + tiempo_max;
    }



    Tiempo_medicion[cab_num_act] = modification - timeSinceEpoch;

    // Informo el tiempo de adquisicion
    stream<<"Tiempo de adquisicion: "
            ""<<Tiempo_medicion[cab_num_act]<<endl;

    free(vectorSalida);
    free(entrada);
    stream<<"Memoria de buffer de entrada liberada"<<endl;


    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;
    stream<<"----------------------------------SALIDA DE: LevantarArchivo_Planar -------------------------------------------------------- "<<endl;
    stream<<"---------------------------------------------------------------------------------------------------------------------------- "<<endl;



}


// Parseo de trama inspirado en un donante anónimo de código
unsigned char * AutoCalib::Trama(unsigned char *tramaEntrada,int tamanioTramaEntrada,int * tamanioTramaSalidaPointer)
{
    int byteDeTrama;       /*para el for */
    int Trama;
    int offset;
    int byteDato;          /*para el for */
    unsigned char *tramaDeSalida;
    unsigned char datosTrama[CANTIDADdEbYTESdATOS]; /* CANTIDADdEbYTESdATOS = (CANTIDADdEbYTEStIMEsTAMP+CANTIDADdEbYTESpMTS+CANTIDADdEbYTEStIEMPO+CANTIDADdEbYTESnUMERO) */
    unsigned short xorCheckSum;
    unsigned short xorCheckSum_AUX;
    unsigned short CheckSumTrama;
    unsigned short header;
    int a;
    /* Reservo memoria para la trama de salida */
    tramaDeSalida=(unsigned char *)malloc((size_t)tamanioTramaEntrada*(size_t)sizeof(unsigned char) );

    /* Reseteo el contador*/
    (*tamanioTramaSalidaPointer)=0;

    /* mexPrintf("Sigo vivo \n"); */
    /*for(byteDeTrama=0;byteDeTrama<tamanioTramaEntrada-CANTIDADtOTALbYTES+1;byteDeTrama++)*/
    for(Trama=0;Trama<tamanioTramaEntrada/CANTIDADtOTALbYTES-1;Trama++)
        {
        offset = Trama*CANTIDADtOTALbYTES;
        /* mexPrintf("Entre al for byte=%d\n   tamanio=%d \n",byteDeTrama,tamanioTramaEntrada); */
        header=(unsigned short)tramaEntrada[offset]+((unsigned short)tramaEntrada[offset+1])*256;

        if( (HEADER_1==header) || (HEADER_2==header))
            {

            /* mexPrintf("Entre al IF Header:%d \n",header);*/


            xorCheckSum=0;      /*Reseteo el contador de checksum*/

            /* Cargo los datos */
            /*for(byteDato=POSICIONpRIMERdATO;byteDato<(POSICIONdELuLTIMOdATO+1);byteDato++)*/
            for(byteDato=POSICIONpRIMERdATO;byteDato<(POSICIONdELuLTIMOdATO+1);byteDato++)
                {
                datosTrama[byteDato-POSICIONpRIMERdATO]=tramaEntrada[offset + byteDato];
                xorCheckSum+=(unsigned short)datosTrama[byteDato-POSICIONpRIMERdATO];
                }

            /*xorCheckSum=(unsigned short)xorCheckSum;*/
            /* se comprovo e checksum, saco los datos, los guardo en el vector de salida, y adelanto el for */
            /* mexPrintf("TamanioDeSalida:%d  TamanioDeEntrada:%d \n",*tamanioTramaSalidaPointer,tamanioTramaEntrada);   */
            CheckSumTrama=256*((unsigned short)tramaEntrada[offset + POSICIONdELcHECKsUM] )+(unsigned short)tramaEntrada[offset + POSICIONdELcHECKsUM+1];
            xorCheckSum_AUX=xorCheckSum-CheckSumTrama;

            /* OJO, cambien el checkum por la suma */
         if(xorCheckSum_AUX==0)
       /* if (1) */
                {
                for(byteDato=0;byteDato<CANTIDADdEbYTESdATOS;byteDato++)
                   {
                    tramaDeSalida[byteDato+(*tamanioTramaSalidaPointer)]=datosTrama[CANTIDADdEbYTESdATOS-1-byteDato];
                    }
                (*tamanioTramaSalidaPointer)=(*tamanioTramaSalidaPointer)+CANTIDADdEbYTESdATOS;
                }
         /*else
             mexPrintf("Trama:%d    Checksum:%d   Calculado:%d \n",Trama-1,CheckSumTrama,xorCheckSum);*/
            }
        }

    return(tramaDeSalida);
}



/* -------------------------------------------------------------------
 * --------------------Newton-Gauss Juan -----------------------------
 * -------------------------------------------------------------------
 */


double AutoCalib::f_gauss(double X,double *P)
{
    return P[0]*exp(-pow((X-P[1])/P[2],2));
}

double AutoCalib::df_gauss(double X, double *P, int nP)
{
    double ret = 0.0;
        switch(nP)
        {
            case 0: ret=exp(-pow((X-P[1])/P[2],2)); break;
            case 1: ret=P[0]*(2.0*(X-P[1])/pow(P[2],2))*exp(-pow((X-P[1])/P[2],2));break;
            case 2: ret=P[0]*2.0*pow(X-P[1],2)*pow(P[2],-3)*exp(-pow((X-P[1])/P[2],2)); break;
        }
    return ret;
}



double AutoCalib::sumacuadrados(double *x,  double *y, int ndatos, double *P,double (AutoCalib::*fx)(double X, double *P))
{
    int i;
    double cuadrados=0;
    for(i=0;i<ndatos;i++)
        cuadrados+=pow(y[i]-(this->*fx)(x[i],P),2);
    return cuadrados;
}

void AutoCalib::jacobiano(double *matriz, double *x, int ndatos, double *P, int nparam, double (AutoCalib::*dfx)(double X, double *P, int nP))
{
    int i,j;
    for(i=0; i<ndatos; i++)
        for(j=0; j<nparam; j++)
        setelemento(matriz,i,j,ndatos,nparam,(this->*dfx)(x[i],P,j));
}

int AutoCalib::inicializargna(estadogna *estado, int ndatos, int nparam)
{
    estado->Z0 = (double*) malloc(sizeof(double)*ndatos*nparam);
    estado->Z0T= (double*) malloc(sizeof(double)*nparam*ndatos);
    estado->Z0C= (double*) malloc(sizeof(double)*nparam*nparam);
    estado->Z0I= (double*) malloc(sizeof(double)*nparam*nparam);
    estado->D  = (double*) malloc(sizeof(double)*ndatos*1);
    estado->ZTD= (double*) malloc(sizeof(double)*nparam*1);
    estado->A  = (double*) malloc(sizeof(double)*nparam*1);
    if(!estado->Z0)		return -1;
    if(!estado->Z0T) 	return -1;
    if(!estado->Z0C)	return -1;
    if(!estado->Z0I)	return -1;
    if(!estado->D)		return -1;
    if(!estado->ZTD)	return -1;
    if(!estado->A)		return -1;
    return 0;
}

void AutoCalib::liberargna(estadogna *estado)
{
    free(estado->Z0);
    free(estado->Z0T);
    free(estado->Z0C);
    free(estado->Z0I);
    free(estado->D);
    free(estado->ZTD);
    free(estado->A);
}

int AutoCalib::gaussnewton(	double	*x,
            double	*y,
            int 	ndatos,
            double	*p,
            int 	nparam,
            double 	paso,
            double 	(AutoCalib::*fx)(double X, double *P),
            double (AutoCalib::*dfx)(double X, double *P,int nP),
            estadogna *estado)
{
    int i,j;
    jacobiano(estado->Z0,x,ndatos,p,nparam,dfx);
    trasponerMatriz(estado->Z0,ndatos,nparam,estado->Z0T);
    multiplicarMatrices(estado->Z0T,nparam,ndatos,estado->Z0,ndatos,nparam,estado->Z0C);
    matrizInversa(estado->Z0C,nparam,estado->Z0I);
    for(i=0;i<nparam;i++)
        for(j=0;j<nparam;j++)
            if(isnan(getelemento(estado->Z0I,i,j,nparam,nparam)))
                return -1;
    for(i=0;i<ndatos;i++) setelemento(estado->D,i,0,ndatos,1,y[i]-(this->*fx)(x[i],p));
    multiplicarMatrices(estado->Z0T,nparam,ndatos,estado->D,ndatos,1,estado->ZTD);
    multiplicarMatrices(estado->Z0I,nparam,nparam,estado->ZTD,nparam,1,estado->A);
    productoporescalarMatriz(estado->A,nparam,1,estado->A,paso);
    sumarMatrices(estado->A,nparam,1,p,p);
    return 0;
}

void AutoCalib::acotarparametros(double *p, double *pmax, double *pmin, int nparam)
{
    int i;
    for(i=0;i<nparam;i++)
    {
        if(p[i]>pmax[i]) p[i]=pmax[i];
        if(p[i]<pmin[i]) p[i]=pmin[i];
    }
}




double AutoCalib::getelemento(double *matriz, int fila, int col, int nFila, int nCol)
{
    return matriz[fila*nCol+col];
}

void AutoCalib::setelemento(double *matriz, int fila, int col, int nFila, int nCol,double val)
{
    matriz[fila*nCol+col]=val;
}

void AutoCalib::imprimirMatriz(double *matriz, int nFilas, int nCols,int decimales,char formato)
{
    char buf[16];
    int i,j;

    memset(buf,0,16);
    sprintf(buf,"%%0.0%d%c\t",decimales,formato);
    for(i=0;i<nFilas;i++)
    {
        for(j=0;j<nCols;j++)
            printf(buf,getelemento(matriz,i,j,nFilas,nCols));
        printf("\n");
    }
}

double* AutoCalib::llenarMatriz(double *matriz,int nFilas,int nCols, double val)
{
    int i,j;
    for(i=0;i<nFilas;i++)
        for(j=0;j<nCols;j++)
            setelemento(matriz,i,j,nFilas,nCols,val);
    return matriz;
}

double* AutoCalib::identidadMatriz(double *matriz,int n, double val)
{
    int i,j;
    for(i=0;i<n;i++)
        for(j=0;j<n;j++)
            if(i==j)
                setelemento(matriz,i,j,n,n,val);
            else
                setelemento(matriz,i,j,n,n,0.0);
    return matriz;
}

double* AutoCalib::productoporescalarMatriz(double *morg,int nFilas, int nCols, double *mdst, double val)
{
    int i,j;
    for(i=0;i<nFilas;i++)
        for(j=0;j<nCols;j++)
            setelemento(mdst,i,j,nFilas,nCols,getelemento(morg,i,j,nFilas,nCols)*val);
    return mdst;
}

double* AutoCalib::trasponerMatriz(double *morg,int nFilasOrg, int nColsOrg, double *mdst)
{
    int i,j;
    for(i=0;i<nFilasOrg;i++)
        for(j=0;j<nColsOrg;j++)
            setelemento(mdst,j,i,nColsOrg,nFilasOrg,getelemento(morg,i,j,nFilasOrg,nColsOrg));
    return mdst;
}

double* AutoCalib::sumarMatrices(double *mop1,int nFilasOrg, int nColsOrg, double *mop2, double *mdst)
{
    int i,j;
    for(i=0;i<nFilasOrg;i++)
        for(j=0;j<nColsOrg;j++)
            setelemento(mdst,i,j,nFilasOrg,nColsOrg,getelemento(mop1,i,j,nFilasOrg,nColsOrg)+getelemento(mop2,i,j,nFilasOrg,nColsOrg));
    return mdst;
}

double* AutoCalib::restarMatrices(double *mop1,int nFilasOrg, int nColsOrg, double *mop2, double *mdst)
{
    int i,j;
    for(i=0;i<nFilasOrg;i++)
        for(j=0;j<nColsOrg;j++)
            setelemento(mdst,i,j,nFilasOrg,nColsOrg,getelemento(mop1,i,j,nFilasOrg,nColsOrg)-getelemento(mop2,i,j,nFilasOrg,nColsOrg));
    return mdst;
}

double* AutoCalib::multiplicarMatrices(double *mop1,int nFilasop1, int nColsop1, double *mop2, int nFilasop2, int nColsop2, double *mdst)
{
    int i,j,k;
    double tmp;
    if(nColsop1!=nFilasop2) return NULL;

    for(i=0;i<nFilasop1;i++)
        for(j=0;j<nColsop2;j++)
        {
            tmp = 0.0;
            for(k=0;k<nColsop1;k++)
                tmp+=getelemento(mop1,i,k,nFilasop1,nColsop1)*getelemento(mop2,k,j,nFilasop2,nColsop2);
            setelemento(mdst,i,j,nFilasop1,nColsop2,tmp);
        }
    return mdst;
}

double* AutoCalib::permutarMatriz(double *matriz,int fila1, int fila2, int nFilas, int nCols)
{
    int i;
    double *filaswap=  (double*) malloc(sizeof(double)*nCols);
    if(!filaswap) return NULL;
    for(i=0;i<nCols;i++)
    {
        setelemento(filaswap,0,i,1,nCols,getelemento(matriz,fila1,i,nFilas,nCols));
        setelemento(matriz,fila1,i,nFilas,nCols,getelemento(matriz,fila2,i,nFilas,nCols));
        setelemento(matriz,fila2,i,nFilas,nCols,getelemento(filaswap,0,i,1,nCols));

    }
    free(filaswap);
    return matriz;
}

double AutoCalib::determinante(double *matriz, int n)
{
    int i,j,k,s;
    double r;
    double d;
    double *aux =  (double*) malloc(sizeof(double)*n*n);
    if(!aux) return 0.0;
    memcpy(aux,matriz,sizeof(double)*n*n);

    //La eliminación trula con un cero en la diagonal. Permuto filas.
    for(i=0,s=0;i<n;i++)
        if(fabs(getelemento(aux,i,i,n,n))<1e-20)
            for(j=i+1;j<n;j++)
                if(fabs(getelemento(aux,j,i,n,n))>=1e-20)
                {
                    permutarMatriz(aux,i,j,n,n);
                    s^=1;
                    break;
                }
    for(i=0;i<n;i++)
            for(j=0;j<n;j++)
                    if(j>i)
            {
                r = getelemento(aux,j,i,n,n)/getelemento(aux,i,i,n,n);
                        for(k=0;k<n;k++)
                    setelemento(aux,j,k,n,n,getelemento(aux,j,k,n,n)-r*getelemento(aux,i,k,n,n));
            }
    d=1.0;
    for(i=0;i<n;i++)
        d*=getelemento(aux,i,i,n,n);
    free(aux);
    return (s)?-d:d;
}

double* AutoCalib::matrizmenorMatriz(double *matriz, int nofila, int nocol, int nFilas, int nCols,double *mdst)
{
    int i,j,k,l;
    for(i=0,k=0;i<nFilas;i++)
        if(i!=nofila)
        {
            for(j=0,l=0;j<nCols;j++)
                if(j!=nocol)
                    setelemento(mdst,k,l++,nFilas-1,nCols-1,getelemento(matriz,i,j,nFilas,nCols));
            k++;
        }

    return mdst;
}

double* AutoCalib::matrizInversa(double *matriz,int n, double *inversa)
{
    int i,j;
    double tmp;
    double *min =  (double*) malloc(sizeof(double)*(n-1)*(n-1));
    double det=determinante(matriz,n);
    if(fabs(det)<1e-20 || !min) return NULL;
    for(i=0;i<n;i++)
        for(j=0;j<n;j++)
        {
            tmp = determinante(matrizmenorMatriz(matriz,i,j,n,n,min),n-1);
            if(((i+j)%2)) tmp = -tmp;
            tmp/=det;
            setelemento(inversa,j,i,n,n,tmp);
        }
    free(min);
    return inversa;
}
