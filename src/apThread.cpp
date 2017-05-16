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
 * @param parent
 */
Thread::Thread(shared_ptr<MCAE> _arpet, QMutex *_mutex, QObject *parent) :
    QObject(parent),
    arpet(_arpet),
    temp(false),
    rate(false),
    debug(false),
    log_finished(false),
    mutex(_mutex),
    time_sec(1)
{
    _logging =false;
    _abort = false;
}

string Thread::getLocalDateAndTime()
{
  return (QDateTime::currentDateTime().toString().toStdString());
}

void Thread::requestLog()
{
    mutex->lock();
    _logging = true;
    _abort = false;
    mutex->unlock();

    emit logRequested();
}

void Thread::abort()
{
    mutex->lock();
    if (_logging)
    {
        _abort = true;
        if (debug) cout<<"Se aborta la operación de logueo bajo el thread: "<<thread()->currentThreadId()<<endl;
    }
    mutex->unlock();
}

void Thread::getLogWork()
{
    if(debug)
    {
        cout<<"[LOG-DBG-THR] "<<getLocalDateAndTime()<<" ============================="<<endl;
        cout<<"Comienza el log cada "<<time_sec<<" segundos"<<endl;
    }

    while(!_abort)
    {
        vector<int> rates(3);
        double tempe;

        mutex->lock();

        for (int i=0;i<checkedHeads.length();i++)
        {
         try
         {
                int head_index=checkedHeads.at(i);
                if (debug) cout<<"Cabezal: "<<head_index<<endl;

                if(rate)
                {
                    rates = arpet->getRate(QString::number(head_index).toStdString(), port_name.toStdString());
                    emit sendRatesValues(head_index, rates.at(0), rates.at(1), rates.at(2));
                    if (debug) cout<<"Tasas: "<<rates.at(0)<<","<<rates.at(1)<<","<<rates.at(2)<<" | "<<arpet->getTrama_MCAE()<<endl;
                }
                emit startElapsedTime();

                QVector<double> temp_vec;
                temp_vec.fill(0);
                if(temp)
                {
                    for(int pmt = 0; pmt < PMTs; pmt++)
                    {
                     {
                        string msg = arpet->getTemp(QString::number(head_index).toStdString(), QString::number(pmt+1).toStdString(), port_name.toStdString());
                        if (debug) cout<<"PMT: "<<QString::number(pmt+1).toStdString()<<" | "<<msg<<" | "<<arpet->getTrama_MCAE()<<endl;
                        tempe = arpet->getPMTTemperature(msg);
                        if (tempe > MIN_TEMPERATURE) temp_vec.push_back(tempe);
                    }
              }
              double mean = std::accumulate(temp_vec.begin(), temp_vec.end(), .0) / temp_vec.size();
              double t_max = *max_element(temp_vec.begin(),temp_vec.end());
              double t_min = *min_element(temp_vec.begin(),temp_vec.end());
              emit sendTempValues(head_index, t_min, mean, t_max);
              if (debug) cout<<"Temperaturas | Mínima: "<<QString::number(t_min).toStdString()<<" | Media: "<<QString::number(mean).toStdString()<<" | Máxima: "<<QString::number(t_max).toStdString()<<endl;
            }
          }
          catch (Exceptions &ex)
          {
              if (debug) cout<<"Imposible adquirir los valores de tasa y/o temperatura en el proceso de logueo. Error: "<<ex.excdesc<<endl;
              emit sendErrorCommand();
              setAbortBool(true);
              //mutex->unlock();
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
