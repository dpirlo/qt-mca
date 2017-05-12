#ifndef APTHREAD_H
#define APTHREAD_H
#include "apMCAE.hpp"
#include <QTimer>
#include <QMutex>
#include <QEventLoop>
#include <QThread>
#include <QDebug>
#include <QObject>

namespace ap {

  class Thread : public QObject
  {
      Q_OBJECT

  public:
      explicit Thread(shared_ptr<MCAE> _arpet, QObject *parent = 0);
      /**
       * @brief Requests the process to start
       *
       * It is thread safe as it uses #mutex to protect access to #_working variable.
       */
      void requestWork();
      /**
       * @brief Requests the process to abort
       *
       * It is thread safe as it uses #mutex to protect access to #_abort variable.
       */
      void abort();      

  private:
      shared_ptr<MCAE> arpet;
      QList<int> checkedHeads;
      /**
       * @brief Process is aborted when @em true
       */
      bool _abort;
      /**
       * @brief @em true when Worker is doing work
       */
      bool _working;
      /**
       * @brief Protects access to #_abort
       */
      QMutex mutex;
      /**
       * @brief port_name
       */
      QString port_name;


  signals:
      /**
       * @brief This signal is emitted when the Worker request to Work
       * @sa requestWork()
       */
      void workRequested();
      /**
       * @brief This signal is emitted when counted value is changed (every sec)
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
       * @brief This signal is emitted when process is finished (either by counting 60 sec or being aborted)
       */
      void finished();

  public slots:
      /**
       * @brief Does something
       *
       * Counts 60 sec in this example.
       * Counting is interrupted if #_aborted is set to true.
       */
      void doWork();

  public:
      void setCheckedHeads(QList<int> list) {checkedHeads = list;}
      void setPortName(QString port) {port_name = port;}
  };

}

#endif // APTHREAD_H
