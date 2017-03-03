#include "inc/apAutoCalib.hpp"

using namespace ap;


AutoCalib::AutoCalib()
{

}



bool AutoCalib::calibrar_simple()
{

    cout << "Enviando a cabezal "<<Cab_List[0]<<" PMT "<<PMTs_List[0]<<endl;
    pedir_MCA_PMT(Cab_List[0] , PMTs_List[0], 256);

/*
    for(int i=0; i<PMTs_List.length();i++) { cout<<PMTs_List[i]<<endl; }
    for(int i=0; i<Cab_List.length();i++) { cout<<Cab_List[i]<<endl; }
    cout<<"Canal Objetivo:"<<Canal_Obj<<endl;
*/

    return 1;
}



void AutoCalib::pedir_MCA_PMT(int Cabezal, int PMT, int canales)
{
    portConnect(port_name.toStdString().c_str());

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
        bytes_transfered = portWrite(&sended);
    }
    catch(boost::system::system_error e)
    {
      cout << "No se puede acceder al puerto serie."<<endl;
        Exceptions exception_serial_port((string("No se puede acceder al puerto serie. Error: ")+string(e.what())).c_str());
    }

    cout << "Leyendo"<<endl;
    try
    {
         portReadString(&msg,'\r');                  //     msg = readString();
    }
    catch( Exceptions & ex )
    {
      cout << "No se puede leer."<<endl;
         Exceptions exception_stop(ex.excdesc);

    }

    cout<<msg<<endl;

    cout << "Leyendo el buffer"<<endl;
    try{
             portReadBufferString(&msg_data,canales*6+16);    //   msg_data = readBufferString(channels*6+16);
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

    portDisconnect();

}


