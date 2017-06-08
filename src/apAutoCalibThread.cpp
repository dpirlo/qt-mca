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
        cout<<"Tiempo_Adq :"<<calibrador->getTiempo_adq()<<"  Abort = "<<_abort<<endl;
        calibrador->calibrar_simple();
        emit clearGraphsCalib();
        emit sendHitsCalib(calibrador->getHitsMCA(), CHANNELS, QString::number(calibrador->PMTs_List[0]), 0, false);
    }
    calibrador->portDisconnect();
    emit sendConnectPortArpet();
    cout<<"ME JUUUIIIII"<<endl;
    emit finished();
}


