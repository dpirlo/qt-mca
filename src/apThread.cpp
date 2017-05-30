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
    temp(false),
    rate(false),
    debug(false),
    log_finished(false),
    mutex(_mutex),
    time_sec(1)
{

}
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
void Thread::getLogWork()
{
    if(debug)
    {
        cout<<"[LOG-DBG-THR] "<<getLocalDateAndTime()<<" ============================="<<endl;
        cout<<"Comienza el log cada "<<time_sec<<" segundos"<<endl;
    }

    int try_error_count = 0;

    while(!_abort)
    {
        vector<int> rates(3);
        double tempe;
        int head_index;

        mutex->lock();


        for (int i=0;i<checkedHeads.length();i++)
        {
            try
            {
                head_index=checkedHeads.at(i);
                if (debug) cout<<"Cabezal: "<<head_index<<endl;

                emit startElapsedTime();

                QVector<double> temp_vec;
                temp_vec.fill(0);
                if(temp)
                {
                    for(int pmt = 0; pmt < PMTs; pmt++)
                    {
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
                }

                if(rate)
                {
                    rates = arpet->getRate(QString::number(head_index).toStdString(), port_name.toStdString());
                    emit sendRatesValues(head_index, rates.at(0), rates.at(1), rates.at(2));
                    if (debug) cout<<"Tasas: "<<rates.at(0)<<","<<rates.at(1)<<","<<rates.at(2)<<" | "<<arpet->getTrama_MCAE()<<endl;
                }
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
        }

        mutex->unlock();

        QEventLoop loop;
        QTimer::singleShot(time_sec*1000, &loop, SLOT(quit()));
        loop.exec();
    }

    mutex->lock();
    _logging = false;
    mutex->unlock();

    if(debug)
    {
        cout<<"El proceso del log ha sido finalizado. Tiempo transcurrido: "<<etime.toStdString()<<endl;
        cout<<"[END-LOG-DBG-THR] =================================================="<<endl;
    }

    emit finishedElapsedTime(true);
    emit finished();
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

    string pmt_function;

    if (_centroid)
    {
        pmt_function = arpet->getFunCHead();
    }
    else
    {
        pmt_function = arpet->getFunCSP3();
    }

    while(!_abort)
    {

        mutex->lock();

        try
        {
            if (_mode)
            {
                int size_pmt_selected = pmt_selected_list.length();
                for (int index=0;index<size_pmt_selected;index++)
                {
                    string pmt = pmt_selected_list.at(index).toStdString();
                    string msg = arpet->getMCA(pmt, pmt_function, QString::number(checkedHeads.at(0)).toStdString(),CHANNELS_PMT, port_name.toStdString());
                    if(debug)
                    {
                        cout<<"Cabezal: "<<checkedHeads.at(0)<<endl;
                        cout<<"PMT: "<<pmt<<" "<<endl;
                        cout<< "Trama recibida: "<< msg << " | Trama enviada: "<< arpet->getTrama_MCAE() <<endl;
                    }
                    emit sendHitsMCA(arpet->getHitsMCA(), CHANNELS_PMT, QString::fromStdString(pmt), index, _mode);
                    emit sendValuesMCA(arpet->getTimeMCA(), arpet->getHVMCA(), arpet->getOffSetMCA(), arpet->getVarMCA(), _mode);
                }
            }
            else
            {
                for (int index=0;index<checkedHeads.length();index++)
                {
                    string msg = arpet->getMCA("0", arpet->getFunCHead() , QString::number(checkedHeads.at(index)).toStdString(),CHANNELS, port_name.toStdString());
                    if(debug)
                    {
                        cout<<"Cabezal: "<<checkedHeads.at(index)<<endl;
                        cout<< "Trama recibida: "<< msg << " | Trama enviada: "<< arpet->getTrama_MCAE() <<endl;
                    }
                    emit sendHitsMCA(arpet->getHitsMCA(), CHANNELS, QString::number(checkedHeads.at(index)),index, _mode);
                    emit sendValuesMCA(arpet->getTimeMCA(), arpet->getHVMCA(), arpet->getOffSetMCA(), arpet->getVarMCA(), _mode);
                }
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
        QTimer::singleShot(100, &loop, SLOT(quit()));
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

    emit finished();
}
