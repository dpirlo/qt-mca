#include "inc/apAutoCalib.hpp"

using namespace ap;


AutoCalib::AutoCalib()
{

}



bool AutoCalib::calibrar_simple(QCustomPlot* plot_hand )
{


    cout << "Enviando a cabezal "<<Cab_List[0]<<" PMT "<<PMTs_List[0]<<endl;
    pedir_MCA_PMT(Cab_List[0] , PMTs_List[0], 256);

/*
    for(int i=0; i<PMTs_List.length();i++) { cout<<PMTs_List[i]<<endl; }
    for(int i=0; i<Cab_List.length();i++) { cout<<Cab_List[i]<<endl; }
    cout<<"Canal Objetivo:"<<Canal_Obj<<endl;
*/

    QVector<int> param(6);
    param[0]=rand()%245+10;//R
    param[1]=rand()%245+10;//G
    param[2]=rand()%245+10;//B
    param[3]=rand()%5+1; //LineStyle
    param[4]=rand()%14+1;//ScatterShape
    param[5]=rand()/(double)RAND_MAX*2+1;//setWidthF

    QString nombre_plot;
    nombre_plot = "PMT "+ QString::number(PMTs_List[0]);


    plot_MCA(getHitsMCA(), plot_hand , nombre_plot, param);

/*
    double lala[] = {1 , 0 , 0, 0 , 0 ,0 ,0 ,2, 10, 1, 0 , 5};
    std::vector<double> v;
    v.assign(lala, lala + sizeof(lala) / sizeof(lala[0]));
    QVector<double> lalo = QVector<double>::fromStdVector(v);
    plot_MCA(lalo, plot_hand , nombre_plot, param);
*/



    return 1;
}



void AutoCalib::pedir_MCA_PMT(int Cabezal, int PMT, int canales)
{


    string msg, msg_data;
    size_t bytes_transfered = 0;
    string sended;
    QVector<double> canales_pmt, hits_pmt;

    QString Cabezal_str, PMT_str;
    Cabezal_str = QString::number(Cabezal);
    PMT_str = QString::number(PMT);

    setHeader_MCAE(getHead_MCAE() + Cabezal_str.toStdString() + getFunCSP3());
    setMCAEStream(PMT_str.toStdString(), canales*6+16, getData_MCA(), "");

    sended = getTrama_MCAE() + getEnd_MCA();

    portFlush();

    cout << "Enviando a cabezal "<<Cabezal_str.toStdString()<<" PMT "<<PMT_str.toStdString()<<endl;
    cout<<sended<<endl;


    try
    {
        bytes_transfered = portWrite(&sended, port_name.toStdString().c_str());
    }
    catch(boost::system::system_error e)
    {
      cout << "No se puede acceder al puerto serie."<<endl;
        Exceptions exception_serial_port((string("No se puede acceder al puerto serie. Error: ")+string(e.what())).c_str());
    }

    cout << "Leyendo"<<endl;
    try
    {
         portReadString(&msg,'\r', port_name.toStdString().c_str());                  //     msg = readString();
    }
    catch( Exceptions & ex )
    {
      cout << "No se puede leer."<<endl;
         Exceptions exception_stop(ex.excdesc);

    }

    cout<<msg<<endl;

    cout << "Leyendo el buffer"<<endl;
    try{
             portReadBufferString(&msg_data,canales*6+16, port_name.toStdString().c_str());    //   msg_data = readBufferString(channels*6+16);
        }
        catch( Exceptions & ex ){
          cout << "No se leer... aparentemente..."<<endl;
             Exceptions exception_stop(ex.excdesc);
        }

    cout << "Leyendo los datos"<<endl;
        getMCASplitData(msg_data, canales);

    cout << "Obteniendo channels"<<endl;
    canales_pmt = getChannels();
    cout << "Obteniendo hits"<<endl;
    hits_pmt = getHitsMCA();


    cout<<"Canales:"<<endl;
    for(int i=0; i<canales_pmt.length();i++) { cout<<canales_pmt[i]<<","; }
    cout<<endl<<"Hits:"<<endl;
    for(int i=0; i<hits_pmt.length();i++) { cout<<hits_pmt[i]<<","; }
    cout<<endl;

}



void AutoCalib::plot_MCA(QVector<double> hits, QCustomPlot *graph, QString graph_legend, QVector<int> param)
{

    QVector<double> channels_ui = getChannels();

/*
    double lala[] = {0, 1 , 2 , 3, 4 , 5 ,6 , 7 ,8, 9, 10, 11};
    std::vector<double> v;
    v.assign(lala, lala + sizeof(lala) / sizeof(lala[0]));
    QVector<double> lalo = QVector<double>::fromStdVector(v);
    QVector<double> channels_ui;
    channels_ui.resize(12);
    channels_ui = lalo;
*/

    for(int i=0; i<channels_ui.length();i++) { cout<<channels_ui[i]<<","; }
    cout<<endl;
    for(int i=0; i<hits.length();i++) { cout<<hits[i]<<","; }
    cout<<endl;

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

