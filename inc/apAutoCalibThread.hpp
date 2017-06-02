#ifndef APAUTOCALIBTHREAD_H
#define APAUTOCALIBTHREAD_H
#include "apAutoCalib.hpp"
#include <QTimer>
#include <QMutex>
#include <QEventLoop>
#include <QThread>
#include <QDebug>
#include <QObject>
#include "qdatetime.h"

namespace ap {

  class AutoCalibThread : public QObject
  {
      Q_OBJECT

  public:
      explicit AutoCalibThread(shared_ptr<AutoCalib> _calibrador, QMutex *_mutex, QObject *parent = 0);


  private:
      shared_ptr<AutoCalib> calibrador;
      QMutex* mutex;


  signals:


  public slots:


  public:

  };

}

#endif // APAUTOCALIBTHREAD_H
