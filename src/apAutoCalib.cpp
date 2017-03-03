#include "inc/apAutoCalib.hpp"

using namespace ap;


AutoCalib::AutoCalib(shared_ptr<MCAE> copy):MCAE(copy, 1000)
{

}



bool AutoCalib::calibrar_simple()
{

    cout << "Enviando a cabezal "<<Cab_List[0]<<" PMT "<<PMTs_List[0]<<endl;
    //pedir_MCA_PMT(Cab_List[0] , PMTs_List[0], 256);


    for(int i=0; i<PMTs_List.length();i++) { cout<<PMTs_List[i]<<endl; }
    for(int i=0; i<Cab_List.length();i++) { cout<<Cab_List[i]<<endl; }
    cout<<"Canal Objetivo:"<<Canal_Obj<<endl;


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

    cout << "Enviando a cabezal "<<Cabezal_str.toStdString()<<" PMT "<<PMT_str.toStdString()<<endl;


    try
    {
        bytes_transfered = portWrite(&sended);
    }
    catch(boost::system::system_error e)
    {
        Exceptions exception_serial_port((string("No se puede acceder al puerto serie. Error: ")+string(e.what())).c_str());
        throw exception_serial_port;
    }


    try
    {
         portReadString(&msg,'\r');                  //     msg = readString();
    }
    catch( Exceptions & ex )
    {
         Exceptions exception_stop(ex.excdesc);
         throw exception_stop;
    }



    portReadBufferString(&msg,canales*6+16);    //   msg_data = readBufferString(channels*6+16);

    getMCASplitData(msg_data, canales);


    canales_pmt = getChannels();
    hits_pmt = getHitsMCA();


    cout<<"Canales:"<<endl;
    for(int i=0; i<canales_pmt.length();i++) { cout<<canales_pmt[i]<<endl; }
    cout<<"Hits:"<<endl;
    for(int i=0; i<hits_pmt.length();i++) { cout<<hits_pmt[i]<<endl; }


}


