#include "inc/apAutoCalibThread.hpp"

using namespace ap;

AutoCalibThread::AutoCalibThread(shared_ptr<AutoCalib> _calibrador, QMutex *_mutex, QObject *parent) :
    QObject(parent),
    mutex(_mutex),
    _abort(false),
    _calibing(false),
    calibrador(_calibrador),
    debug(false)
{

}

void AutoCalibThread::abort()
{
    mutex->lock();
    if (_calibing)
    {
        _abort = true;
        if (debug) cout<<"Se aborta la operación en el AutoCalibThread: "<<thread()->currentThreadId()<<endl;
    }
    mutex->unlock();
}

void AutoCalibThread::setAbortBool(bool abort)
{
    _abort = abort;
//    if (debug)
    cout<<"Se aborta la operación en el AutoCalibThread: "<<thread()->currentThreadId()<<endl;
}

void AutoCalibThread::requestCalib()
{
    mutex->lock();
    _calibing = true;
    _abort = false;
    mutex->unlock();

    emit CalibRequested();
}

void AutoCalibThread::getCalib()
{
    // Calibro
    //cout<<"Calibrador..."<<endl;
    calibrador->initCalib();
    while(!_abort)
    {
        QEventLoop loop;
        QTimer::singleShot(calibrador->getTiempo_adq()*1000, &loop, SLOT(quit()));
        loop.exec();
        QVector<double> aux_hits;
        emit clearGraphsCalib();
        for (int i=0 ; i<calibrador->PMTs_List.length() ; i++)
        {
            // Pido estado de SP3
            calibrador->getMCA(QString::number(calibrador->PMTs_List[i]).toStdString(), calibrador->getFunCSP3(), QString::number(calibrador->Cab_actual).toStdString(), CHANNELS, calibrador->port_name.toStdString());
            calibrador->Dinodos_PMT[calibrador->PMTs_List[i]-1] = calibrador->getHVMCA();

            // Pido MCA de calibracion
            calibrador->getMCA(QString::number(calibrador->PMTs_List[i]).toStdString(), calibrador->getFunCHead(), QString::number(calibrador->Cab_actual).toStdString(), CHANNELS, calibrador->port_name.toStdString());

            // Leo los hits y los paso a double
            aux_hits = calibrador->getHitsMCA();
            for (int j = 0 ; j < CHANNELS ; j++)
            {
              calibrador->Hist_Double[calibrador->PMTs_List[i]-1][j] = aux_hits[j];
            }
            emit sendHitsCalib(aux_hits, CHANNELS, QString::number(calibrador->PMTs_List[i]), i, false);
        }
        calibrador->calibrar_simple();
    }
    calibrador->portDisconnect();
    emit sendConnectPortArpet();
    cout<<"ME JUUUIIIII"<<endl;
    emit finished();
}



