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
 try{
        vector<int> rates(3);
        double tempe;
        for (int i=0;i<checkedHeads.length();i++)
        {
            mutex.lock();
            bool abort = _abort;
            mutex.unlock();

            if(abort) cout<<"Abortando el proceso de log..."<<endl;

            int head_index=checkedHeads.at(i);
            cout<<"Cabezal: "<<head_index<<endl;

            if(rate)
            {
              rates = arpet->getRate(QString::number(head_index).toStdString(), port_name.toStdString());
              emit sendRatesValues(head_index, rates.at(0), rates.at(1), rates.at(2));
              if (debug) cout<<"Tasas: "<<rates.at(0)<<rates.at(1)<<rates.at(2)<<" | "<<arpet->getTrama_MCAE()<<endl;
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
            }
          }
        }
        catch (Exceptions &ex)
        {
            cout<<"nada"<<endl;
        }

        mutex.lock();
        _working = false;
        mutex.unlock();

//    for (int i = 0; i < 3; i ++) {

//        // Checks if the process should be aborted
//        mutex.lock();
//        bool abort = _abort;
//        mutex.unlock();

//        if (abort) {
//            qDebug()<<"Aborting worker process in Thread "<<thread()->currentThreadId();
//            break;
//        }

//        // This will stupidly wait 1 sec doing nothing...
//        QEventLoop loop;
//        QTimer::singleShot(1000, &loop, SLOT(quit()));
//        loop.exec();

//        // Once we're done waiting, value is updated
//        emit valueChanged(QString::number(i));
//    }

    // Set _working to false, meaning the process can't be aborted anymore.
    mutex.lock();
    _working = false;
    mutex.unlock();

    qDebug()<<"Worker process finished in Thread "<<thread()->currentThreadId();

    //Once 60 sec passed, the finished signal is sent
    emit finished();
}
