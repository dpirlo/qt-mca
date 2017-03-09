#include "inc/apAutoCalib.hpp"

using namespace ap;


AutoCalib::AutoCalib()
{

}



bool AutoCalib::calibrar_simple(QCustomPlot* plot_hand)
{
    portConnect(port_name.toStdString().c_str());

    // Parametros del ploteo
    QVector<int> param(6);
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
        reset_Mem_Cab(Cab_actual);

        // Espero el tiempo indicado
        sleep(tiempo_adq);

        // LLeno los buffers de memoria de la clase
        //for (int i=0 ; i<PMTs ; i++)
        for (int i=0 ; i<4 ; i++)
        //int i = PMT_actual-1;
        {

            // Pido MCA de calibracion del PMT actual
            pedir_MCA_PMT(Cab_actual , i+1, CHANNELS, 1);

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
            Picos_PMT[i] = Buscar_Pico(Acum_PMT[i], CHANNELS);


            /*
            // Paso a Qvector y ploteo
            std::vector<double> auxVector;
            auxVector.assign(Acum_PMT[i], Acum_PMT[i] + CHANNELS);
            aux_hits.fromStdVector(auxVector);
            //plot_MCA(aux_hits, plot_hand , nombre_plot, param);
            plot_MCA(getHitsMCA(), plot_hand , nombre_plot, param);
            */


            // Leo el HV actual y lo guardo en memoria
            pedir_MCA_PMT(Cab_actual , i+1, 256, 0);
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
                    modificar_HV_PMT(Cab_actual , PMT_mover+1, (Dinodos_PMT[PMT_mover] + paso_dinodo[PMT_mover]));
                }
                else
                {
                    cout<< "Bajando PMT "<<PMT_mover+1<<" a "<< (Dinodos_PMT[PMT_mover] - paso_dinodo[PMT_mover])<<endl;
                    modificar_HV_PMT(Cab_actual , PMT_mover+1, Dinodos_PMT[PMT_mover] - paso_dinodo[PMT_mover]);
                }
            }
            else
            {
                cout<< "Modificando PMT "<<PMT_mover+1<<" a "<< (Dinodos_PMT[PMT_mover] + paso_dinodo[PMT_mover])<<endl;
                modificar_HV_PMT(Cab_actual , PMT_mover+1, (Dinodos_PMT[PMT_mover] + paso_dinodo[PMT_mover]));
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



        pedir_MCA_PMT(Cab_actual , 1, CHANNELS, 1);
        plot_MCA(getHitsMCA(), plot_hand , nombre_plot, param);




        //cout << "Enviando a cabezal "<<Cab_actual<<" PMT "<<PMT_actual<<endl;










        // Seteo HV de dinodo
        //modificar_HV_PMT(Cab_actual , PMT_actual, 600+10);






        // Ploteo
        //plot_MCA(getHitsMCA(), plot_hand , nombre_plot, param);

    }
    portDisconnect();

    return 1;
}


double AutoCalib::Buscar_Pico(double* Canales, int num_canales)
{
    int window_size = num_canales/20;
    int span = (window_size);
    int min_count_diff = 50;
    double Low_win, High_win;
    char Estados[4] = {0 , 0 , 0, 0};
    char Estado_aux;
    int ind_estado = 0;
    int outpoint = -1, low_extrema = -1;

    //for(int i=0; i<1024 ;i++) { cout<<Canales[i]<<","; }
    //cout<<endl;

    // Seteo el limite
    // Copio los canales a una matriz
    mat Canales_mat(Canales,num_canales,1);
    min_count_diff = (int) sum(sum(Canales_mat))*0.003;
    cout<<"minimooooo "<<min_count_diff<<endl;





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
                cout<<(i - (2*window_size) - span)<<endl;
                cout<<(i)<<endl;
                //outpoint = (i - (2*window_size) - span) + ((2*window_size) - span)/2;

                // Retorno lo que encontre
                low_extrema = (i - (2*window_size) - span);
                outpoint = i;
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
    if (outpoint == -1) {return -1;};
    if (low_extrema < window_size) {return -1;};



    // Una vez encontrado el pico de manera rudimentaria, le fiteo una gauss
    // y sale el armadillo


    // Me quedo solo con los canales que me interesan, los que estan dentro de las ventanas
    mat canales_peak;
    //canales_peak = Canales_mat.rows(low_extrema-window_size,outpoint+window_size);
    canales_peak = Canales_mat.rows(low_extrema-window_size,num_canales-1);

    // Maximo de la gausseana
    uword max_idx, std_idx;
    double Gauss_max = canales_peak.max(max_idx);

    // Busco el 68 % desde el lado de mas alta energia
    uvec mayores_std = find(canales_peak > 0.68*Gauss_max);
    mayores_std.max(std_idx);

    double Gauss_mean = max_idx;
    double Gauss_std = std_idx;

    // Calculo el vector de gauss para varias posiciones de media y distintos desvios
    int MOV_MEAN = 30, MOV_STD = 12;
    mat gauss_curve_aux(MOV_MEAN, canales_peak.n_elem);
    mat Dot_prods(MOV_MEAN,MOV_STD);
    double Gauss_mean_AUX, Gauss_std_AUX;

    for (int k=0 ; k < MOV_STD; k ++)
    {
        // Adquiero el valor de desvio estandar actual
        Gauss_std_AUX = Gauss_std - (MOV_STD/2) + k;

        for (int j=0 ; j < MOV_MEAN; j ++)
        {
            // Adquiero el valor de media actual
            Gauss_mean_AUX = Gauss_mean - (MOV_MEAN/2) + j;

            for (int i=0 ; i < canales_peak.size() ; i ++)
            {
                gauss_curve_aux(j,i) = (1/sqrt(2*Gauss_std_AUX*Gauss_std_AUX*3.1415))*exp(-( ( (i -Gauss_mean_AUX) * (i -Gauss_mean_AUX) ) / (2*Gauss_std_AUX*Gauss_std_AUX) ));
            }

            // Normalizo la curva calculada
            float aux_max_gen = gauss_curve_aux.row(j).max();
            for (int i=0 ; i < canales_peak.size() ; i ++)
            {
                gauss_curve_aux(j,i) = gauss_curve_aux(j,i) * (Gauss_max / aux_max_gen);
            }

            // Calculo la correlacion entre la campana propuesta y la medicion
            Dot_prods(j,k) = norm_dot(canales_peak,gauss_curve_aux.row(j));
        }
    }

    // Busco el máximo
    uword dot_max_idx = Dot_prods.index_max();
    uvec sub = ind2sub( arma::size(Dot_prods), dot_max_idx );

    // Lo paso a desvio y media
    Gauss_std_AUX = Gauss_std - (MOV_STD/2) + sub(1);
    Gauss_mean_AUX = Gauss_mean - (MOV_MEAN/2) + sub(0);


    /*
    cout << Gauss_std_AUX<<endl;
    cout << Gauss_mean_AUX<<endl;

    for (int i=0 ; i < canales_peak.size() ; i ++)
    {
        gauss_curve_aux(1,i) = (1/sqrt(2*Gauss_std_AUX*Gauss_std_AUX*3.1415))*exp(-( ( (i -Gauss_mean_AUX) * (i -Gauss_mean_AUX) ) / (2*Gauss_std_AUX*Gauss_std_AUX) ));
    }

    // normalizo
    float aux_max_gen = gauss_curve_aux.row(1).max();
    for (int i=0 ; i < canales_peak.size() ; i ++)
    {
        gauss_curve_aux(1,i) = gauss_curve_aux(1,i) * (Gauss_max / aux_max_gen);
    }

    for(int i=0; i<gauss_curve_aux.n_cols ;i++) { cout<<gauss_curve_aux(1,i)<<","; }
    cout<<endl;





    cout<<Gauss_mean<<endl;
    cout<<Gauss_std<<endl;
    cout<<Gauss_max<<endl;


    for(int i=0; i<canales_peak.size();i++) { cout<<canales_peak[i]<<","; }
    cout<<endl;

    for(int i=0; i<gauss_curve_aux.n_cols ;i++) { cout<<gauss_curve_aux(5,i)<<","; }
    cout<<endl;
    for(int i=0; i<gauss_curve_aux.n_cols ;i++) { cout<<gauss_curve_aux(15,i)<<","; }
    cout<<endl;
    for(int i=0; i<gauss_curve_aux.n_cols ;i++) { cout<<gauss_curve_aux(24,i)<<","; }
    cout<<endl;
    */

    // Retorno el valor de la media de gauss calculada
    //return low_extrema+window_size+Gauss_mean_AUX;
    return low_extrema-window_size+Gauss_mean_AUX;

}



/* -------------------------------------------------------------------
 * --------------------Funciones MCAE---------------------------------
 * -------------------------------------------------------------------
 */

void AutoCalib::pedir_MCA_PMT(int Cabezal, int PMT, int canales, bool Calib)
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

    }

    //cout<<msg<<endl;

    //cout << "Leyendo el buffer"<<endl;
    try{
             portReadBufferString(&msg_data,canales*6+16, port_name.toStdString().c_str());    //   msg_data = readBufferString(channels*6+16);
        }
        catch( Exceptions & ex ){
          cout << "No se leer... aparentemente... (pedir MCA)"<<endl;
             Exceptions exception_stop(ex.excdesc);
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
}



void AutoCalib::modificar_HV_PMT(int Cabezal, int PMT,  int val_dinodo)
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

    }
}



void AutoCalib::reset_Mem_Cab(int Cabezal)
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

    }
    cout << msg<<endl;
}



/* -------------------------------------------------------------------
 * --------------------Funciones Varias-------------------------------
 * -------------------------------------------------------------------
 */



void AutoCalib::plot_MCA(QVector<double> hits, QCustomPlot *graph, QString graph_legend, QVector<int> param)
{

    QVector<double> channels_ui = getChannels();

    graph->clearGraphs();


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
