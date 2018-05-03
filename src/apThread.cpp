#include "inc/apThread.hpp"

using namespace ap;

/**
 * @brief Thread::Thread
 *
 * Constructor de la clase
 *
 * Se inicializan todos los _flags_ utilizados durante la ejecución del hilo. Esta clase contiene todos los métodos y propiedades para acceder, administrar,
 * configurar los threads ejecutados en la aplicación *qt-mca*.
 *
 * Recibe como parametro un objeto de la clase MCAE para acceder a todos los dispositivos del tómografo ARPET (fotomultiplicadores, cabezales, placas de
 * alta tensión, etc) y un objeto de la clase QMutex para compartir el recurso del puerto serie.
 *
 * @note Se documentan las propiedades más importantes.
 *
 * @param _arpet
 * @param _mutex
 * @param parent
 */
Thread::Thread(shared_ptr<MCAE> _arpet, QMutex *_mutex, QObject *parent) :
    QObject(parent),
    arpet(_arpet),
    _logging(false),
    _mca(false),
    _abort(false),
    _centroid(false),
    _CabCalib(false),
    temp(false),
    rate(false),
    debug(false),
    log_finished(false),
    mutex(_mutex),
    time_sec(1)
{
}
/**
 * @brief Thread::setAbortBool
 *
 * Slot de aborto del proceso
 *
 * @param abort
 */
void Thread::setAbortBool(bool abort)
{
    _abort = abort;
    if (debug) cout<<"Se aborta la operación en el thread: "<<thread()->currentThreadId()<<endl;
}
/**
 * @brief Thread::getLocalDateAndTime
 * @return date en _string_
 */
string Thread::getLocalDateAndTime()
{
    return (QDateTime::currentDateTime().toString().toStdString());
}
/**
 * @brief Thread::requestLog
 */
void Thread::requestLog()
{
    mutex->lock();
    _logging = true;
    _abort = false;
    mutex->unlock();

    emit logRequested();
}


void Thread::requestGrabarFPGA()
{


    emit GrabarFPGArequested();
}
/**
 * @brief Thread::requestMCA
 */
void Thread::requestMCA()
{
    mutex->lock();
    _mca= false;
    _abort = false;
    mutex->unlock();

    emit mcaRequested();
}
/**
 * @brief Thread::abort
 */
void Thread::abort()
{
    mutex->lock();
    if (_logging || _mca)
    {
        _abort = true;
        if (debug) cout<<"Se aborta la operación en el thread: "<<thread()->currentThreadId()<<endl;
    }
    mutex->unlock();
}
/**
 * @brief Thread::getLogWork
 *
 * Método que realiza el logueo de datos en el ARPET.
 * Obtiene tasa y temperatura de los cabezales seleccionados y los envía a través
 * de una señal a MainWindow. Se finaliza con la variable _booleana_ _abort_.
 *
 */
void Thread::getLogWork() {

    double CuentasTotales=0;
    double Saturados[48];
    string pmt_function;
    double Hist_Double[CHANNELS];
    QVector<double> aux_hits;
    AutoCalib InitBuscaPico;
    struct Pico_espectro aux;


    if(debug)
    {
        cout<<"[LOG-DBG-THR] "<<getLocalDateAndTime()<<" ============================="<<endl;
        cout<<"Comienza el log cada "<<time_sec<<" segundos"<<endl;
    }

    if (_centroid)
    {
        pmt_function = arpet->getFunCHead();
    }
    else
    {
        pmt_function = arpet->getFunCSP3();
    }


    int try_error_count = 0;

    while(!_abort)
    {
        vector<int> rates(9);
        double tempe;
        int head_index;
        int offsets[48];
        string msg_MCA;

        if (Timer_concluded){
            mutex->lock();

            Timer_concluded=false;
            try
                {
                  arpet->portDisconnect();

                  for (int i=0;i<checkedHeads.length();i++)
                  {
                    emit startElapsedTime();
                    head_index=checkedHeads.at(i);
                    if (debug) cout<<"Cabezal: "<<head_index<<endl;
                    QVector<double> temp_vec;
                    port_name="/dev/UART_Cab"+QString::number(checkedHeads.at(i));
                    temp_vec.fill(0);

                    arpet->portConnect(port_name.toStdString().c_str());


                    if(temp)
                    {

                        for (int pmt=0;pmt<48;pmt++){
                            string msg = arpet->getTemp(QString::number(head_index).toStdString(), QString::number(pmt+1).toStdString(), port_name.toStdString());
                            if (debug) cout<<"PMT: "<<QString::number(pmt+1).toStdString()<<" | "<<msg<<" | "<<arpet->getTrama_MCAE()<<endl;
                            tempe = arpet->getPMTTemperature(msg);
                            if (tempe > MIN_TEMPERATURE) temp_vec.push_back(tempe);
                        }

                        double mean = std::accumulate(temp_vec.begin(), temp_vec.end(), .0) / temp_vec.size();
                        double t_max = *max_element(temp_vec.begin(),temp_vec.end());
                        double t_min = *min_element(temp_vec.begin(),temp_vec.end());
                        emit sendTempValues(head_index, t_min, mean, t_max);
                        if (debug) cout<<"Temperaturas | Mínima: "<<QString::number(t_min).toStdString()<<" | Media: "<<QString::number(mean).toStdString()<<" | Máxima: "<<QString::number(t_max).toStdString()<<endl;
    //                    emit sendOffSetValues(head_index, offsets);
    //                    if (debug) cout<<"termino el volcado de offset"<<endl;
                    }
                    if(rate)
                    {
                        rates = arpet->getRate(QString::number(head_index).toStdString(), port_name.toStdString());
                        emit sendRatesValues(head_index, rates.at(0), rates.at(1), rates.at(2));
                        if (debug) cout<<"Tasas: "<<rates.at(0)<<","<<rates.at(1)<<","<<rates.at(2)<<" | "<<arpet->getTrama_MCAE()<<endl;
                    }
                    ///// PICO //////////////////

                    // Pido MCA a cabezal
                    arpet->getMCA("0", arpet->getFunCHead(), QString::number(head_index).toStdString(), CHANNELS, port_name.toStdString());

                    aux_hits = arpet->getHitsMCA();

                    for (int j = 0 ; j < CHANNELS ; j++) {

                        Hist_Double[j] = aux_hits[j];
                        //cout<<QString::number(aux_hits[j]).toStdString()<<endl;
                    }

                    aux = InitBuscaPico.Buscar_Pico(Hist_Double, CHANNELS);

                    emit sendPicosLog(aux, head_index);

                    // FIN PICO //////////////////
                    //////////////////////////////
                    // TASA///////////////////////
    //                for (int j=0;j<48;j++){
    //                    if (debug) cout<<"Cabezal: "<<checkedHeads.at(0)<<endl;

    //                    for(int k=0;k<255;k++){
    //                        CuentasTotales+=arpet->getHitsMCA()[k];
    //                    }
    //                    Saturados[j]=(arpet->getHitsMCA()[255]/CuentasTotales)*100;
    //                    if (debug) cout<<"PMT: "<<QString::number( j+1).toStdString()<<" "<<"Saturados "<<": " << QString::number(Saturados[j]).toStdString()<<endl;

    //                }
    //                emit sendSaturated(QString::number(head_index).toInt(), Saturados);
                    //// FIN TASA
                    /////////////////////////

                    arpet->portDisconnect();

                  }
                  port_name="/dev/UART_Coin";
                  arpet->portConnect(port_name.toStdString().c_str());
                  if (debug) cout<<"Aca llega"<<endl;

                  rates = arpet->getRateCoin(QString::number(7).toStdString(), port_name.toStdString());
                  emit sendRatesValuesCoin( rates.at(0), rates.at(1), rates.at(2),rates.at(3),rates.at(4),rates.at(5),rates.at(6),rates.at(7),rates.at(8));
                  if (debug) cout<<"Tasas COIN: "<<rates.at(0)<<","<<rates.at(1)<<","<<rates.at(2)<<","<<rates.at(3)<<","<<rates.at(4)<<","<<rates.at(5)<<","<<rates.at(6)<<","<<rates.at(7)<<","<<rates.at(8)<<" | "<<arpet->getTrama_MCAE()<<endl;

            }
            catch (Exceptions &ex)
            {
                try_error_count++;
                if (debug)
                {
                    cout<<"Imposible adquirir los valores de tasa y/o temperatura en el cabezal "<<head_index<<". Error: "<<ex.excdesc<<endl;
                    cout<<"Se continua con el proceso de logueo."<<endl;
                }

                if(try_error_count>4) //Si supero los 4 reintentos de acceso envío una señal de error para abortar.
                {
                    if (debug)
                    {
                        cout<<"Se supera los "<<try_error_count<<" reintentos de adquisición. Se aborta el proceso de logueo."<<endl;
                    }
                    emit sendLogErrorCommand();
                    setAbortBool(true);
                }
            }


            emit sendresetHeads();

            mutex->unlock();
        }


    }//termina el log por señal de aborto

    mutex->lock();
    _logging = false;
    mutex->unlock();


    if(debug)
    {
        cout<<"El proceso del log ha sido finalizado. Tiempo transcurrido: "<<etime.toStdString()<<endl;
        cout<<"[END-LOG-DBG-THR] =================================================="<<endl;
    }

    Timer_concluded=true;

    emit finishedElapsedTime(true);
    emit finished();
}



void Thread::TimerUpdate(){
    Timer_concluded=true;
}

/**
 * @brief Thread::getElapsedTime
 *
 * Obtiene el tiempo transcurrido de un proceso. Se finaliza con la variable
 * _booleana_ *log_finished*
 *
 */
void Thread::getElapsedTime()
{
    QTime t;
    int secs, mins, hours;
    int elapsed_time=0;
    while (!log_finished)
    {
        t.start();
        QEventLoop loop;
        QTimer::singleShot(1000, &loop, SLOT(quit()));
        loop.exec();
        elapsed_time=elapsed_time + t.elapsed();
        secs = elapsed_time / 1000;
        mins = (secs / 60) % 60;
        hours = (secs / 3600);
        secs = secs % 60;
        etime = QString("%1:%2:%3")
                .arg(hours, 2, 10, QLatin1Char('0'))
                .arg(mins, 2, 10, QLatin1Char('0'))
                .arg(secs, 2, 10, QLatin1Char('0'));
        emit sendElapsedTimeString(etime);
        emit sendFinalElapsedTimeString(etime);
    }
    restoreLoggingVariable();

    emit finished();
}
/**
 * @brief Thread::getMCA
 *
 * Realiza la adquisición de cuentas MCA
 *
 */
void Thread::getMCA()
{


    if(debug)
    {
        cout<<"[LOG-DBG-THR] "<<getLocalDateAndTime()<<" ============================="<<endl;
        cout<<"Comienza la adquisición MCA."<<endl;
    }

    string pmt;
    string pmt_function;
    int timer_wait_milisec;
    double CuentasTotales=0;
    double Hist_Double[CHANNELS];
    QVector<double> aux_hits;
    AutoCalib InitBuscaPico;
    struct Pico_espectro aux;

    if (_centroid)
    {

           pmt_function = arpet->getFunCHead();
    }
    else
    {
        pmt_function = arpet->getFunCSP3();
    }

   // while(!_abort)
    {

        mutex->lock();

        try
        {
            if (_mode)
            {
                emit clearGraphsPMTs();
                int size_pmt_selected = pmt_selected_list.length();
                for (int index=0;index<size_pmt_selected;index++)
                {
                    arpet->portDisconnect();
                    QString Cabezal_conectado="/dev/UART_Cab"+ QString::number( checkedHeads.at(0));
                    arpet->portConnect(Cabezal_conectado.toStdString().c_str());
                    pmt = pmt_selected_list.at(index).toStdString();
                    string msg = arpet->getMCA(pmt, pmt_function, QString::number(checkedHeads.at(0)).toStdString(),CHANNELS_PMT, port_name.toStdString());
                    if(debug)
                    {
                        cout<<"Cabezal: "<<checkedHeads.at(0)<<endl;
//                        cout<<"PMT: "<<pmt<<" "<<endl;

                        for(int i=0;i<255;i++){
                            CuentasTotales+=arpet->getHitsMCA()[i];
                        }
                        cout<<"PMT: "<<pmt<<" "<<"Saturados "<< pmt<<": " << QString::number((arpet->getHitsMCA()[255]/CuentasTotales)*100).toStdString()<<endl;
                        cout<< "Trama recibida: "<< msg << " | Trama enviada: "<< arpet->getTrama_MCAE() <<endl;
                    }

                    emit sendHitsMCA(arpet->getHitsMCA(), CHANNELS_PMT, QString::fromStdString(pmt), index, _mode);
                    emit sendValuesMCA(arpet->getTimeMCA(), arpet->getHVMCA(), arpet->getOffSetMCA(), arpet->getVarMCA(), _mode);
                }
                timer_wait_milisec = (20 + size_pmt_selected*5);
            }
            else
            {
                emit clearGraphsHeads();
                for (int index=0;index<checkedHeads.length();index++)
                {
                    arpet->portDisconnect();
                    QString Cabezal_conectado="/dev/UART_Cab"+ QString::number( checkedHeads.at(index));
                    error_code error_code=arpet->portConnect(Cabezal_conectado.toStdString().c_str());
                    if(debug) cout<<"error code serie: "<<error_code<<endl;

                    pmt = _CabCalib==true ? "49" : "0";

                    string msg = arpet->getMCA(pmt, arpet->getFunCHead() , QString::number(checkedHeads.at(index)).toStdString(),CHANNELS, port_name.toStdString());
                    if(debug)
                    {

                        cout<<"Cabezal: "<<checkedHeads.at(index)<<endl;
                        cout<< "Trama recibida: "<< msg << " | Trama enviada: "<< arpet->getTrama_MCAE() <<endl;
                    }

                    aux_hits = arpet->getHitsMCA();
                    if (checkedHeads.length()==1){

                        for (int j = 0 ; j < CHANNELS ; j++) {

                            Hist_Double[j] = aux_hits[j];
                            //cout<<QString::number(aux_hits[j]).toStdString()<<endl;
                        }

                        aux=InitBuscaPico.Buscar_Pico(Hist_Double, CHANNELS);

                        emit sendPicosMCA(aux, checkedHeads.at(0));
                    }
                    emit sendHitsMCA(arpet->getHitsMCA(), CHANNELS, QString::number(checkedHeads.at(index)),index, _mode);
                    emit sendValuesMCA(arpet->getTimeMCA(), arpet->getHVMCA(), arpet->getOffSetMCA(), arpet->getVarMCA(), _mode);
                }

                timer_wait_milisec = (20 + checkedHeads.length()*20);
            }
        }
        catch(Exceptions & ex)
        {
            if (debug)
            {
                if (_mode) cout<<"No se pueden obtener los valores de MCA de los PMTs seleccionados en el cabezal "<<checkedHeads.at(0)<<". Error: "<<ex.excdesc<<endl;
                else cout<<"No se pueden obtener los valores de MCA en el cabezal "<<checkedHeads.at(0)<<". Error: "<<ex.excdesc<<endl;
            }
            emit sendMCAErrorCommand();
            setAbortBool(true);
        }

        mutex->unlock();

        QEventLoop loop;
        QTimer::singleShot(timer_wait_milisec, &loop, SLOT(quit()));
        loop.exec();
    }

    mutex->lock();
    _mca = false;
    mutex->unlock();

    if(debug)
    {
        cout<<"La adquisición MCA ha sido finalizada."<<endl;
        cout<<"[END-LOG-DBG-THR] =================================================="<<endl;
    }
    arpet->portDisconnect();
    emit finished();
}

//void Thread::get()

void Thread::GrabarFPGA(){

    emit StatusFinishFPGA(Grabar_FPGA());

}

bool Thread::Adquirir_handler()
{
//    connect(&Adquisidor,SIGNAL(Adquisidor.readyReadStandardOutput()),this,SLOT(prtstdoutput()));
//    connect(&Adquisidor,SIGNAL(Adquisidor.readyReadStandardError()),this,SLOT(prtstderror()));
    emit StatusFinishAdq(Adquirir());

}


bool Thread::Grabar_FPGA()
{
        QString output,PMT;
        int retries = 10;
        bool exit;


        for(int i=0; i< commands.length() ; i++)
        {
            PMT =  commands[i].left(commands[i].indexOf("#"));
            commands[i] =  commands[i].mid(commands[i].indexOf("#") + 1);
            exit = false;
            retries = 10;
            cout << commands[i].toStdString() << endl;
            while (!exit)
            {
                QProcess prog_fpga;
                prog_fpga.waitForStarted();
                prog_fpga.start(commands[i]);
                prog_fpga.waitForFinished(-1);
                output=(prog_fpga.readAllStandardError());
                prog_fpga.close();

                if (output.contains("done"))
                {

                    output =  output.mid(output.indexOf("done"));
                    output =  output.left(output.indexOf("\n"));
                    cout << PMT.toStdString() << " " << output.toStdString() << endl;
                    exit = true;
                }

                else if(output.contains("Success!"))
                {
                    output =  output.mid(output.indexOf("Success!"));
                    //output =  output.left(output.indexOf("\n"));
                    cout << PMT.toStdString() << " " << output.toStdString() << endl;
                    exit = true;
                }


               else
                {
                    if(retries == 0)
                        return false;
                    else
                    {
                        retries --;
                        cout << output.toStdString() << endl;
                    } //return false;
                }

            }
        }
        return true;
}


bool Thread::Adquirir()
{
        QString input,output;
        //cout << commands[i].toStdString() << endl;
        QProcess *Adquisidor=new QProcess(this);
        QString date =QDate::currentDate().toString("yyyy-MM-dd");
        QString time =QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm");

        Adquisidor->waitForStarted();
        input=  "recvRawEth -q -s " + commands.at(0); /// "echo Megas: "+commands.at(0)+" path: "+commands.at(1)+" Nombre Archivo: "+commands.at(2);

        Adquisidor->start(input );

        Adquisidor->waitForFinished(-1);
        output=Adquisidor->readAllStandardError();


        //Adquisidor.close();
        if(output.contains("timeout"))
            return false;
        else if(output.contains("FIN")) {

            QDir mDir;
            QString mpath=commands.at(1)+"/"+date;
            if (!mDir.exists(mpath))
            {
                mDir.mkpath(mpath);
                qDebug() <<"Created";
            }
            else if (mDir.exists(mpath))
            {
                qDebug() <<"Already existed";
            }
            else
            {
                return false;
                qDebug()<<"Directory could not be created";
            }

            Adquisidor->waitForStarted();
            QProcess Adquisidor_copia;

            input= "mv ./" + commands.at(2)+".raw" + " " + "./"+commands.at(2)+ "_"+time+".raw";
            cout << input.toStdString() << endl;
            Adquisidor->start(input);
            Adquisidor->waitForFinished(10000);
            output=(Adquisidor->readAllStandardError());
            if(!output.isEmpty()) return false;
            input= "mv ./" +commands.at(2)+ "_"+time+".raw" + " " + mpath;
            cout << input.toStdString() << endl;
            Adquisidor_copia.start(input);
            Adquisidor_copia.waitForFinished(-1);
            output=(Adquisidor_copia.readAllStandardError());
            if(!output.isEmpty()) return false;
            input= "mv " +commands.at(3) + " " + mpath;
            cout << input.toStdString() << endl;
            Adquisidor_copia.start(input);
            Adquisidor_copia.waitForFinished(-1);
            output=(Adquisidor_copia.readAllStandardError());
            if(!output.isEmpty()) return false;
            Adquisidor->close();
            Adquisidor_copia.close();

        }
        else return false;
    return true;
}

void Thread::requestAdquirir()
{
    emit AdquisicionRequested();
}

void Thread::prtstdoutput(){
   // cout<<Adquisidor.readAllStandardOutput().toStdString()<<endl;
}


void Thread::prtstderror(){
   // cout<<Adquisidor.readAllStandardError().toStdString()<<endl;
}


/*
void Thread::MoveToServer(){
    QProcess Adquisidor_copia;
    QString input,output;
  //  if(){
        input= "mv ./" +commands.at(2)+ "_"+time+".raw" + " " + mpath;
        cout << input.toStdString() << endl;
        Adquisidor_copia->start(input);
        Adquisidor_copia->waitForFinished(-1);
        output=(Adquisidor_copia->readAllStandardError());
        if(!output.isEmpty()) return false;
        input= "mv " +commands.at(3) + " " + mpath;
        cout << input.toStdString() << endl;
        Adquisidor_copia->start(input);
        Adquisidor_copia->waitForFinished(-1);
        output=(Adquisidor_copia->readAllStandardError());
        if(!output.isEmpty()) return false;
//}
    Adquisidor_copia->close();

}*/
