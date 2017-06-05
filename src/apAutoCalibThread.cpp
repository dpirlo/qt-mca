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
//        calibrador->calibrar_simple(ui->specPMTs_2);
        QEventLoop loop;
        QTimer::singleShot(calibrador->getTiempo_adq()*1000, &loop, SLOT(quit()));
        loop.exec();
        cout<<"Tiempo_Adq :"<<calibrador->getTiempo_adq()<<"  Abort = "<<_abort<<endl;
//        sleep(calibrador->getTiempo_adq());
//        QVector<double> aux_hits;
//        calibrador->calibrar_simple();
//        QVector<double> aux_hits = calibrador->calibrar_simple();
        calibrador->calibrar_simple();
//        cout<<aux_hits<<endl;
        //emit sendHitsCalib(, CHANNELS_PMT, QString::number(1), 1, true);
//        emit sendHitsCalib(calibrador->getHitsMCA(), CHANNELS_PMT, QString::number(PMTs_List[5]), Cab_actual-1, true);
    }
    calibrador->portDisconnect();
    emit sendConnectPortArpet();
    cout<<"ME JUUUIIIII"<<endl;
    emit finished();

}



