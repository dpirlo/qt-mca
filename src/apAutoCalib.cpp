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



    E_prom_PMT.set_size(CANTIDADdEpMTS,CANTIDADdEpMTS);


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
            // Convierto de numero de cabezal a indce (-1)
            int cab_num_act = Cab_List[i]-1;

            // Cargo el cabezal actual en memoria
            //LevantarArchivo_Planar(cab_num_act);

            // Calibro energía
            //calibrar_fina_energia(cab_num_act);





            E_prom_PMT.load("/home/rgrodriguez/Desktop/lala.mat", raw_ascii);

            // Calculo el primer paso de calibracion en energia
            // E_prom\Ener_obj  == solve(E_prom,Ener_obj)
            colvec Ener_obj;
            Ener_obj.set_size(CANTIDADdEpMTS,1);
            //Ener_obj = Ener_obj + 511;
            Ener_obj.zeros(CANTIDADdEpMTS,1);
            Ener_obj = Ener_obj + 511;
            //cout<<Ener_obj.n_rows<<" - "<<Ener_obj.n_cols<<endl;



            mat L, U, P, Y;

            lu(L, U, P, E_prom_PMT);
/*
            int m = E_prom_PMT.n_rows;

            U=E_prom_PMT;




            L.eye(m,m);

            mat pivee;
            pivee.set_size(48,1);
            double piv;
            double mult;
            for (int i=0 ; i < m-1 ; i++)
            {
                piv = U(i,i);
                pivee(i) = piv;

                      for (int k=i+1 ; k<m ; k++)
                      {
                          mult = U(k,i)/piv;

                          U.row(k) = -mult*U.row(i) + U.row(k);
                          L(k,i) = mult;

                      }
            }

            //cout<<L<<endl;
            L.save("/home/rgrodriguez/Desktop/lala_L.mat", raw_ascii);
            pivee.save("/home/rgrodriguez/Desktop/lala_P.mat", raw_ascii);
*/



            // perform forwards substitution
            int n = CANTIDADdEpMTS-1;
            mat x;
            x.set_size(CANTIDADdEpMTS,1);
            x.zeros();
            x(0)=Ener_obj(0)/L(0,0);

            for (int i=1 ; i<=n ; i++)
            {

                mat aux = ( Ener_obj(i)   -  ( L.row(i).cols(0,i-1) * x.rows(0,i-1) )  )    /    L(i,i);
                x(i)= aux(0);



                /*
                double lal = 0;
                for (int r = 0 ; r<= (i-1) ; r++)
                {
                    cout<<L(i,r) * x(r)<<endl;
                    lal = lal + L(i,r) * x(r)   ;
                }
                cout<<i<<endl;

                x(i) = ( Ener_obj(i)   -  ( lal )  )    /    L(i,i);
                */

                /*
                mat lal, aux;
                lal.set_size(i,1);
                for (int r = 0 ; r<= (i-1) ; r++)
                {
                    lal(r) = L(i,r) * x(r)   ;
                    cout<<L(i,r)<<endl;
                    cout<<x(r)<<endl;
                    cout<<lal(r)<<endl;

                }
                aux = sum(sort(lal));
                cout<<"aux"<<aux<<endl;

                x(i) = ( Ener_obj(i)   -  ( aux(0) )  )    /    L(i,i);
                cout<<Ener_obj(i)<<endl;
                cout<<L(i,i)<<endl;
                cout<<"xi"<<x(i)<<endl;
                */
            }
            Y = x;

            Y.save("/home/rgrodriguez/Desktop/lala_Y.mat", raw_ascii);

            cout<<"pa delante"<<endl;

            cout<<x<<endl;




            // perform backwards substitution
            x.set_size(CANTIDADdEpMTS,1);
            x.zeros();

            x(n)=Y(n)/U(n,n);
            for (int i=n-1 ; i >=0 ; i--)
            {
                   //x(i)=(Y(i)-U(i,i+1:n)*x(i+1:n))/U(i,i);
                //mat aux = ( Y(i)   -   U( span(i,i), span(i+1,n) ) *x.cols(i+1,n)   )    /    U(i,i);
                mat aux = ( Y(i)   -  ( U.row(i).cols(i+1,n) * x.rows(i+1,n) )  )    /    U(i,i);
                x(i)= aux(0);

/*
                double lal = 0;
                for (int r = 0 ; r<= (i+1) ; r++)
                {
                    lal = lal + U(i,r) * x(r)   ;
                }
                x(i) = ( Y(i)   -  ( lal )  )    /    U(i,i);
*/
/*
                mat lal, aux;
                lal.set_size(i,1);
                for (int r = 0 ; r<= (i+1) ; r++)
                {
                    lal(r) = U(i,r) * x(r)   ;
                }
                aux = sum(sort(lal));
                x(i) = ( Y(i)   -  ( aux(0) )  )    /    U(i,i);
*/
            }
            cout<<"pa tra"<<endl;

            colvec Ce = x;







            cout<<Ce<<endl;

            Y = solve(trimatl(L),Ener_obj);
            colvec Ce2 = solve(trimatu(U),Y);
            cout<<Ce2<<endl;

            colvec Ce3 = solve(E_prom_PMT,Ener_obj);
            cout<<Ce3<<endl;













            // Libero la memoria del cabezal actual
            if (IsLowRAM)
            {
                // No se como borrar asi que los transformo en una matriz de un elemento
                Energia_calib[cab_num_act].set_size(1, 1);
                Tiempos_calib[cab_num_act].set_size(1, 1);
                Tiempos_full_calib[cab_num_act].set_size(1, 1);
                TimeStamp_calib[cab_num_act].set_size(1, 1);
            }
        }
        // Sino calibro tiempos
        else
        {
            // TO DO
        }

    }

}




bool AutoCalib::calibrar_fina_energia(int cab_num_act)
{
    // Saco la suma de los canales del evento
    rowvec Suma_canales = sum( Energia_calib[cab_num_act], 0);

    // Creo el vector de centros para el histograma
    vec centros_hist = linspace<vec>(0,8000,BinsHist);

    // Calculo el histograma
    urowvec espectro_suma_crudo = hist(Suma_canales, centros_hist);

    // ----------------------- Ploteo
    // Paso los vectores a Qvector para plotear
    QVector<double> aux_qvec_cent(BinsHist);
    for (int i=0 ; i < BinsHist ; i++){aux_qvec_cent[i] = centros_hist(i);}
    QVector<double> aux_qvec(BinsHist);
    for (int i=0 ; i < BinsHist ; i++){aux_qvec[i] = espectro_suma_crudo(i);}
    QString nombre_plot;
    nombre_plot = "Espectro crudo cabezal "+ QString::number(cab_num_act+1);
    // Parametros del ploteo
    QVector<int> param(6);
    param[0]=0;//R
    param[1]=61;//G
    param[2]=245;//B
    param[3]=5+1; //LineStyle
    param[4]=14+1;//ScatterShape
    param[5]=1/(double)RAND_MAX*2+1;//setWidthF
    plot_MCA(aux_qvec, aux_qvec_cent,&Espectro_emergente[cab_num_act], nombre_plot, param, 1);
    Espectro_emergente[cab_num_act].show();
    Espectro_emergente[cab_num_act].resize(1000,500);
    qApp->processEvents();


    // Calculo el FWHM
    struct Pico_espectro pico_sin_calib;
    double aux_espectro[BinsHist];
    for (int i=0 ; i < BinsHist ; i++)
    {
        aux_espectro[i] = espectro_suma_crudo(i);
    }
    pico_sin_calib = Buscar_Pico(aux_espectro, BinsHist);

    cout<<"Sin Calibrar:"<<endl;
    cout<<"FWHM: "<<pico_sin_calib.FWHM*100<<"%"<<endl;
    /*
    cout<<pico_sin_calib.FWHM<<" - "<<centros_hist(pico_sin_calib.limites_FWHM[0])<<" ; "<<centros_hist(pico_sin_calib.limites_FWHM[1])<<endl;
    cout<<pico_sin_calib.FWTM<<" - "<<centros_hist(pico_sin_calib.limites_FWTM[0])<<" ; "<<centros_hist(pico_sin_calib.limites_FWTM[1])<<endl;
    cout<<centros_hist(pico_sin_calib.canal_pico)<<endl;
    */


    // Conservo solo los eventos dentro del FWTM
    uvec indices_aux = find(Suma_canales > centros_hist(pico_sin_calib.limites_FWTM[0]));
    rowvec suma_aux = Suma_canales.elem(indices_aux).t();
    Energia_calib[cab_num_act] = Energia_calib[cab_num_act].cols(indices_aux);
    indices_aux = find(suma_aux < centros_hist(pico_sin_calib.limites_FWTM[1]));
    suma_aux = suma_aux.elem(indices_aux).t();
    Energia_calib[cab_num_act] = Energia_calib[cab_num_act].cols(indices_aux);



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
        indices_maximo_PMT = index_max( Energia_calib[cab_num_act], 0 );
        //cout<<indices_maximo_PMT.n_elem<<endl;
        indices_aux = find(indices_maximo_PMT == index_PMT_cent);
        //cout<<indices_aux.n_elem<<endl;
        Eventos_max_PMT =  Energia_calib[cab_num_act].cols(indices_aux);
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

        // Ploteo el histograma de suma para este PMT
        suma_aux = sum( Eventos_max_PMT,  0);
        espectro_suma_crudo = hist(suma_aux, centros_hist);

        // ----------------------- Ploteo
        // Paso los vectores a Qvector para plotear
        for (int i=0 ; i < BinsHist ; i++){aux_qvec_cent[i] = centros_hist(i);}
        for (int i=0 ; i < BinsHist ; i++){aux_qvec[i] = espectro_suma_crudo(i);}
        nombre_plot = "PMT Nº "+ QString::number(index_PMT_cent+1);
        // Color y marker random
        param[0]=rand()%245+10;//R
        param[1]=rand()%245+10;//G
        param[2]=rand()%245+10;//B
        param[3]=rand()%5+1; //LineStyle
        param[4]=rand()%14+1;//ScatterShape
        param[5]=rand()/(double)RAND_MAX*2+1;//setWidthF
        plot_MCA(aux_qvec, aux_qvec_cent,&Espectro_PMT_emergente[cab_num_act], nombre_plot, param, 0);
        Espectro_PMT_emergente[cab_num_act].show();
        Espectro_PMT_emergente[cab_num_act].resize(1000,500);
        qApp->processEvents();

        // Calculo la energia promedio de todos los PMT para este centroide
        E_prom_PMT.row(index_PMT_cent) = mean(Eventos_max_PMT,1).t();
        //cout<<mean(Eventos_max_PMT,1).t()<<";"<<endl;


    }

    E_prom_PMT.save("/home/rgrodriguez/Desktop/lala.mat", raw_ascii);


    // Calculo el primer paso de calibracion en energia
    // E_prom\Ener_obj  == solve(E_prom,Ener_obj)
    colvec Ener_obj(CANTIDADdEpMTS);
    Ener_obj = Ener_obj + 511;
    //cout<<Ener_obj.n_rows<<" - "<<Ener_obj.n_cols<<endl;

    // Parche
    /*
    mat E_prom_PMT_aux = E_prom_PMT;
    for (int i=0 ; i < CANTIDADdEpMTS ; i++ )
    {
        for (int j=0 ; j < CANTIDADdEpMTS ; j++ )
        {
            if (i != j)
            {
                E_prom_PMT_aux(i,j) = (E_prom_PMT(i,j) + E_prom_PMT(j,i))/2;
            }

        }
    }
    E_prom_PMT = E_prom_PMT_aux;
    */

    //colvec Ce = solve(E_prom_PMT,Ener_obj);
    //colvec Ce = inv(E_prom_PMT)*Ener_obj;
    //colvec Ce = qr_solve2(E_prom_PMT,Ener_obj);
    //colvec Ce = pinv(E_prom_PMT,0.01,"std")*Ener_obj;
    //colvec Ce = LU_solve(E_prom_PMT,Ener_obj);

    mat L, U, P, Y;

    //lu(L, U, P, E_prom_PMT);

    int m = E_prom_PMT.n_rows;

    U=E_prom_PMT;
/*
    for (int i = 0 ; i<U.n_rows ; i++)
    {
        for (int k = 0 ; k<U.n_cols ; k++)
        {
            if(U(i,k) < 150)
            {
                U(i,k) = 0;
            }
        }
    }
*/



    L.eye(m,m);

    double piv;
    double mult;
    for (int i=0 ; i < m-1 ; i++)
    {
        piv = U(i,i);

              for (int k=i+1 ; k<m ; k++)
              {
                  mult = U(k,i)/piv;

                  U.row(k) = -mult*U.row(i) + U.row(k);
                  L(k,i) = mult;

              }
    }

    cout<<L<<endl;





    //Y = solve(trimatl(L),Ener_obj);
    //colvec Ce = solve(trimatu(U),Y);
    //colvec Ce = solve(trimatu(U), solve(trimatl(L), P*Ener_obj) );

    // perform forwards substitution
    int n = CANTIDADdEpMTS-1;
    mat x;
    x.set_size(CANTIDADdEpMTS,1);
    x.zeros();
    x(0)=Ener_obj(0)/L(0,0);

    for (int i=1 ; i<=n ; i++)
    {

        //cout<<Ener_obj(i)<<endl;
        //cout<<L(i,i)<<endl;
        //cout<<x.rows(0,i-1)<<endl;
        //cout<< L.row(i).cols(0,i-1)<<endl;

        //cout<<L.row(i).cols(0,i-1) *x.rows(0,i-1)<<endl;

        //x(i)=  ( Bp(i)   -   L(i,1:i-1)*x(1:i-1)   )    /    L(i,i);
        //mat aux = ( Ener_obj(i)   -  ( L.row(i).cols(0,i-1) * x.rows(0,i-1) )  )    /    L(i,i);

        double lal = 0;
        for (int r = 0 ; r<= (i-1) ; r++)
        {
            lal = lal + L(i,r) * x(r)   ;
        }

        x(i) = ( Ener_obj(i)   -  ( lal )  )    /    L(i,i);

        //cout<<aux<<endl;
        //cout<<"2a"<<endl;
        cout<<i<<endl;
        //x(i)= aux(0);
        //cout<<"1a"<<endl;

    }
    Y = x;

    cout<<"pa delante"<<endl;

    cout<<x<<endl;




    // perform backwards substitution
    x.set_size(CANTIDADdEpMTS,1);
    x.zeros();

    x(n)=Y(n)/U(n,n);
    for (int i=n-1 ; i >=0 ; i--)
    {
           //x(i)=(Y(i)-U(i,i+1:n)*x(i+1:n))/U(i,i);
        //mat aux = ( Y(i)   -   U( span(i,i), span(i+1,n) ) *x.cols(i+1,n)   )    /    U(i,i);
        mat aux = ( Y(i)   -  ( U.row(i).cols(i+1,n) * x.rows(i+1,n) )  )    /    U(i,i);
        x(i)= aux(0);
    }
    cout<<"pa tra"<<endl;

    colvec Ce = x;







    cout<<Ce<<endl;




    colvec lala = Energia_calib[cab_num_act].t() * Ce;





    // Creo el vector de centros para el histograma
    colvec centros_hist_2 = linspace<vec>(0,1024,BinsHist);
    // Calculo el histograma
    espectro_suma_crudo = hist(lala.t(), centros_hist_2);

    // Calculo el FWHM
    for (int i=0 ; i < BinsHist ; i++)
    {
        aux_espectro[i] = espectro_suma_crudo(i);
    }
    pico_sin_calib = Buscar_Pico(aux_espectro, BinsHist);
    cout<<"Sin Calibrar:"<<endl;
    cout<<"FWHM: "<<pico_sin_calib.FWHM*100<<"%"<<endl;


    // ----------------------- Ploteo
    // Paso los vectores a Qvector para plotear
    for (int i=0 ; i < BinsHist ; i++){aux_qvec_cent[i] = centros_hist_2(i);}
    for (int i=0 ; i < BinsHist ; i++){aux_qvec[i] = espectro_suma_crudo(i);}
    nombre_plot = "Espectro calibrado cabezal "+ QString::number(cab_num_act+1);
    // Parametros del ploteo
    param[0]=0;//R
    param[1]=61;//G
    param[2]=245;//B
    param[3]=5+1; //LineStyle
    param[4]=14+1;//ScatterShape
    param[5]=1/(double)RAND_MAX*2+1;//setWidthF
    plot_MCA(aux_qvec, aux_qvec_cent,&Espectro_emergente[cab_num_act], nombre_plot, param, 1);
    Espectro_emergente[cab_num_act].show();
    Espectro_emergente[cab_num_act].resize(1000,500);
    qApp->processEvents();


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

    //for(int i=0; i<1024 ;i++) { cout<<Canales[i]<<","; }
    //cout<<endl;

    // Seteo el limite
    // Copio los canales a una matriz
    mat Canales_mat(Canales,num_canales,1);
    min_count_diff = (int) (sum(sum(Canales_mat))*0.003);
    //double aux_suma = 0;
    //for(int i=0; i<num_canales ; i++){aux_suma = aux_suma + Canales[i];}
    //min_count_diff = aux_suma*0.003;
    cout<<"minimooooo "<<min_count_diff<<" ' "<<(sum(sum(Canales_mat)))<<endl;





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

    //return low_extrema-window_size+Gauss_mean_AUX;
    Pico_calculado.canal_pico = low_extrema-window_size+Gauss_mean_AUX;



    double maximo_pico = canales_peak.max();
    // Calculo el FWHM
    int i = 0;
    while (Pico_calculado.limites_FWHM[0] == 0)
    {
        if (maximo_pico*0.5 >=  Canales_mat[Pico_calculado.canal_pico - i]  )
        {
           Pico_calculado.limites_FWHM[0] =  Pico_calculado.canal_pico - i +1
                   ;
        }
        i++;
    }
    i = 0;
    while (Pico_calculado.limites_FWHM[1] == 0)
    {
        if (maximo_pico*0.5 >=  Canales_mat[Pico_calculado.canal_pico + i]  )
        {
           Pico_calculado.limites_FWHM[1] =  Pico_calculado.canal_pico + i -1;
        }
        i++;
    }
    Pico_calculado.FWHM = (Pico_calculado.limites_FWHM[1]-Pico_calculado.limites_FWHM[0])/Pico_calculado.canal_pico;

    // Calculo el FWTM
    i = 0;
    while (Pico_calculado.limites_FWTM[0] == 0)
    {
        if (maximo_pico*0.1 >=  Canales_mat[Pico_calculado.canal_pico - i]  )
        {
           Pico_calculado.limites_FWTM[0] =  Pico_calculado.canal_pico - i +1;
        }
        i++;
    }
    i = 0;
    while (Pico_calculado.limites_FWTM[1] == 0)
    {
        if (maximo_pico*0.1 >=  Canales_mat[Pico_calculado.canal_pico + i]  )
        {
           Pico_calculado.limites_FWTM[1] =  Pico_calculado.canal_pico + i -1;
        }
        i++;
    }
    Pico_calculado.FWTM = (Pico_calculado.limites_FWTM[1]-Pico_calculado.limites_FWTM[0])/Pico_calculado.canal_pico;







    return Pico_calculado;

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



void AutoCalib::plot_MCA(QVector<double> hits, QVector<double> channels_ui, QCustomPlot *graph, QString graph_legend, QVector<int> param, bool clear )
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
    cout<<nombre_adq.c_str()<<endl;

    // abro archivo
    FILE * archivo = fopen(nombre_adq.c_str(), "r+");

    if (archivo == NULL)
    {
        cout<<"ahhh exploto el archivooooo"<< endl;
        return -1;
    }

    cout<<"intento leer..."<<endl;

    // Calculo el tamaño del archivo
    fseek(archivo, 0, SEEK_END);
    long int BytesLeer = ftell(archivo);
    rewind(archivo);

    // Leo todo a memoria
    unsigned char * entrada = (unsigned char *) malloc(BytesLeer);
    cout<<"Memoria reservada"<<endl;
    int bytes_leidos = fread(entrada, sizeof(unsigned char), BytesLeer, archivo);
    cout<<"Leido"<<endl;

    // Cierro archivo
    fclose(archivo);
    cout<<"cerrado"<<endl;


    // Parseo el archivo
    int cantidadDeTramaSalida;                  /*Cantidad de bytes la trama de salida Valida */
    unsigned char *vectorSalida;                /*para la funcion */
    vectorSalida =  Trama(entrada,BytesLeer,&cantidadDeTramaSalida);
    cout<<cantidadDeTramaSalida<<endl;

    // Calculo la cantidad de eventos leidos
    int num_columnas = cantidadDeTramaSalida/CANTiNFO;

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


    /*
    cout<<"¿Todo parseado?"<<endl;

    cout << TimeStamp_calib[cab_num_act](7) << endl;

    for (int i=0 ; i < CANTIDADdEpMTS ; i++)
    {
        cout<<Energia_calib[cab_num_act](i,7)<<" ; ";
    }
    cout<< endl;
    for (int i=0 ; i < CANTIDADdEpMTS ; i++)
    {
        cout<<Tiempos_calib[cab_num_act](i,7)<<" ; ";
    }
    cout<< endl;
    */


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
 * --------------------Matematika ------------------------------------
 * -------------------------------------------------------------------
 */

arma::mat LU_solve(const arma::mat &A,const arma::mat &B)
{

    mat L, U, P, Y;

    lu(L, U, P, A);

    Y = solve(L,B);

    return solve(U,Y);



}






arma::mat qr_solve2(const arma::mat &A,const arma::mat &B)
    {
    arma::mat Q, R;
    arma::qr(Q,R,A);
    unsigned int s = R.n_rows-1;
    arma::mat R_ = R( arma::span(0,s), arma::span(0,s-1) ) ;

    R_ = arma::join_horiz(R_,R.col(s+1));
    arma::mat newB = Q.t()*B;
    arma::mat X(s+1,1);

    for (int i = s; i >= 0; i--)
        {
        X[i] = newB[i];
        for (int j = s; j != i; j--)
            X[i] = X[i] - R_(i,j) * X[j];
        X[i] = X[i]/R_(i,i);
        }

    arma::mat res = X( arma::span(0,s-1), arma::span(0,0) ) ;

    res =  arma::join_vert(res,arma::mat(1,1, arma::fill::zeros));
    res = arma::join_vert(res,X.row(s));
    return res;
    }
