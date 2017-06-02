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
    if (debug) cout<<"Se aborta la operación en el AutoCalibThread: "<<thread()->currentThreadId()<<endl;
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

}
