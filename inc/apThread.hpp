/**
 * @class ap::Thread
 *
 * @brief Clase Thread para la aplicación *qt-mca*
 *
 * Esta clase provee métodos y propiedades para el manejo de hilos de procesamiento en
 * segundo plano durante la ejecución de la aplicación *qt-mca*. Principalmente se utiliza
 * para el _logging_ de tasa y temperatura de cada cámara gamma.
 *
 * @note Clase heredada de QObject
 *
 * @author Ariel Hernández
 *
 * @version $Version
 *
 * Contacto: ahernandez@cae.cnea.gov.ar
 *           ariel.h.estevenz@ieee.org
 *
 */
#ifndef APTHREAD_H
#define APTHREAD_H
#include "apMCAE.hpp"
#include <QTimer>
#include <QMutex>
#include <QEventLoop>
#include <QThread>
#include <QDebug>
#include <QObject>
#include "qdatetime.h"

namespace ap {

  class Thread : public QObject
  {
      Q_OBJECT

  public:
      explicit Thread(shared_ptr<MCAE> _arpet, QMutex* _mutex ,QObject *parent = 0);
      void requestLog();
      void abort();
      string getLocalDateAndTime();

  private:
      shared_ptr<MCAE> arpet;
      QList<int> checkedHeads;
      bool _abort;
      bool _logging;
      QMutex* mutex;
      QString port_name;
      bool temp;
      bool rate;
      bool debug;
      int time_sec;
      bool log_finished;
      QString etime;


  signals:
      /**
       * @brief logRequested
       */
      void logRequested();            
      /**
       * @brief sendRatesValues
       * @param index
       * @param rate_low
       * @param rate_med
       * @param rate_high
       */
      void sendRatesValues(int index, int rate_low, int rate_med, int rate_high);
      /**
       * @brief sendTempValues
       * @param index
       * @param min
       * @param medium
       * @param max
       */
      void sendTempValues(int index, double min, double medium, double max);
      /**
       * @brief finished
       */
      void finished();
      /**
       * @brief finishedElapsedTime
       * @param var
       */
      void finishedElapsedTime(bool var);
      /**
       * @brief sendErrorCommand
       */
      void sendErrorCommand();
      /**
       * @brief startElapsedTime
       */
      void startElapsedTime();
      /**
       * @brief sendElapsedTimeString
       * @param eatime_string
       */
      void sendElapsedTimeString(QString eatime_string);
      /**
       * @brief sendFinalElapsedTimeString
       * @param eatime_string
       */
      void sendFinalElapsedTimeString(QString eatime_string);

  public slots:      
      void getLogWork();
      void setAbortBool(bool abort) {_abort = abort;}
      void cancelLogging(bool var) { log_finished = var; }
      void receivedFinalElapsedTimeString(QString eatime_string) { etime = eatime_string; }
      void getElapsedTime();

  public:
      /**
       * @brief restoreLoggingVariable
       */
      void restoreLoggingVariable() { log_finished = false; }
      /**
       * @brief cancelLogging
       */
      void setCheckedHeads(QList<int> list) {checkedHeads = list;}
      /**
       * @brief setPortName
       * @param port
       */
      void setPortName(QString port) {port_name = port;}
      /**
       * @brief setRateBool
       * @param _rate
       */
      void setRateBool(bool _rate) {rate = _rate;}
      /**
       * @brief setTempBool
       * @param _temp
       */
      void setTempBool(bool _temp) {temp = _temp;}
      /**
       * @brief setDebugMode
       * @param _debug
       */
      void setDebugMode(bool _debug) {debug = _debug;}
      /**
       * @brief setTimeBetweenLogs
       * @param _sec
       */
      void setTimeBetweenLogs(int _sec) {time_sec = _sec;}
  };

}

#endif // APTHREAD_H
