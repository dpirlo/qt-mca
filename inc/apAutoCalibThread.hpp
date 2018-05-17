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
      void requestCalib();
      void requestCalibFina();
      void requestCalibFinaProgress();
      void abort();

  private:
      shared_ptr<AutoCalib> calibrador;
      QList<QString> pmt_selected_list;
      QMutex* mutex;
      bool _abort;
      bool debug;
      bool _calibing;

  signals:
      void CalibRequested();
      void CalibFinaRequested();
      void CalibFinaProgressRequested();
      void finished();
      void sendConnectPortArpet();
      void sendOffButtonCalib();
      void sendCalibErrorCommand();
      void sendHitsCalib(QVector<double> hits, int channels, QString pmt_head, int index, bool mode);
      void sendHitsTiempos(QVector<double> hits, int channels, QString pmt_head, int index, bool mode);
      void sendValuesMCACalib(int umbral,int pico, int FWHM);
      void sendAutocalibReady(bool state,int pmt_roto=0);
      void clearGraphsCalib();
      void sendCalibFinaReady(bool state);
      void sendSetCalibFinaProgress(double progress);
      void sendTerminandoCalibFina();

  public slots:
      void setAbortBool(bool abort);
      void getCalib();
      void getCalibFina();
      void getCalibFinaProgress();

  public:
      void setPMTSelectedList(QList<QString> list) {pmt_selected_list = list;}
      void setDebugMode(bool _debug) {debug = _debug;}

  };

}

#endif // APAUTOCALIBTHREAD_H
