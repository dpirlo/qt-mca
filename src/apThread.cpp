#include "inc/apThread.hpp"

using namespace ap;

Thread::Thread(shared_ptr<MCAE> _arpet, QObject *parent) :
    QObject(parent),
    arpet(_arpet),
    temp(false),
    rate(false),
    debug(false)
{
    _working =false;
    _abort = false;
}

string Thread::getLocalDateAndTime()
{
  return (QDateTime::currentDateTime().toString().toStdString());
}

void Thread::requestWork()
{
    mutex.lock();
    _working = true;
    _abort = false;
    mutex.unlock();

    emit workRequested();
}

void Thread::abort()
{
    mutex.lock();
    if (_working) {
        _abort = true;
    }
    mutex.unlock();
}

void Thread::doWork()
{
    if(debug) cout<<"[LOG-DBG-THR] "<<getLocalDateAndTime()<<" ============================="<<endl;

    while(!_abort)
    {
        vector<int> rates(3);
        double tempe;
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
              mutex.lock();
              setAbortBool(true);
              mutex.unlock();
         }
       }

       QEventLoop loop;
       QTimer::singleShot(1000, &loop, SLOT(quit()));
       loop.exec();

    }

    mutex.lock();
    _working = false;
    mutex.unlock();

    if(debug) cout<<"[END-LOG-DBG-THR] =================================================="<<endl;

    emit finished();
}
