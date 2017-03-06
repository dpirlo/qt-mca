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






    int PMT_index = 0;
    int Cab_index = 0;

    int PMT_actual = PMTs_List[PMT_index];
    int Cab_actual = Cab_List[Cab_index];

    QString nombre_plot;
    nombre_plot = "PMT "+ QString::number(PMT_actual);

    // Borro memoria
    for (int i=1 ; i < PMTs ; i++)
    {
        Acum_PMT[PMT_actual-1][i] = 0;
    };

    // Reseteo la memoria de SP6
    reset_Mem_Cab(Cab_actual);

    // Espero el tiempo indicado
    sleep(tiempo_adq);

    // Acumulo estadistica
    for (int i=1 ; i<PMTs ; i++)
    {

        // Pido MCA de calibracion del PMT actual
        pedir_MCA_PMT(Cab_actual , i+1, CHANNELS, 1);

        // Leo los hits y los paso a double
        QVector<double> aux_hits;
        aux_hits = getHitsMCA();
        double aux_double_hits[CHANNELS];
        for (int i = 1 ; i < CHANNELS ; i++)
        {
          aux_double_hits[i] = aux_hits[i];
        }

        // Acumulo en mi memoria
        for (int j=1 ; j < PMTs ; j++)
        {
            Acum_PMT[i][j] = Acum_PMT[i][j] +  aux_double_hits[j];
        };

        // Paso a Qvector y ploteo
        std::vector<double> auxVector;
        auxVector.assign(Acum_PMT[i], Acum_PMT[i] + CHANNELS);
        aux_hits.fromStdVector(auxVector);
        plot_MCA(aux_hits, plot_hand , nombre_plot, param);

    }







    //cout << "Enviando a cabezal "<<Cab_actual<<" PMT "<<PMT_actual<<endl;

    // Leo el HV actual
    pedir_MCA_PMT(Cab_actual , PMT_actual, 256, 0);
    cout<<getHVMCA()<<endl;
    int dinodo_act = getHVMCA();








    // Seteo HV de dinodo
    modificar_HV_PMT(Cab_actual , PMT_actual, 1100+10);






    // Ploteo
    //plot_MCA(getHitsMCA(), plot_hand , nombre_plot, param);


    portDisconnect();

    return 1;
}



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
    val_dinodo_str = "000";
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
