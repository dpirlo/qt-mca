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
#include "apAutoCalib.hpp"
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
      void requestMCA();
      void abort();
      string getLocalDateAndTime();



  private:
      shared_ptr<MCAE> arpet;
      QList<int> checkedHeads;
      QList<QString> pmt_selected_list;
      bool _abort;
      bool _logging;
      bool _mode;
      bool _mca;
      bool _centroid;
      bool _CabCalib;
      QMutex* mutex;
      QString port_name;
      bool temp;
      bool rate;
      bool debug;
      int time_sec;
      bool log_finished;
      bool Timer_concluded=true;
      QString etime;
      QStringList commands;
      int cantidad_archivos;
      QString time;
      QString date;

      bool running;

  private:    //funciones privadas
      bool Grabar_FPGA();
      bool Adquirir();
      bool MoveToServer();


  signals:
      /**
       * @brief GrabarFPGArequested
       */
      void GrabarFPGArequested();

      /**
       * @brief AdquisicionRequested
       */
      void AdquisicionRequested();


      /**
       * @brief logRequested
       */
      void logRequested();
      /**
       * @brief mcaRequest
       */
      void mcaRequested();

      void MoveToServerRequested();

      /**
       * @brief sendRatesValuesCoin
       * @param index
       * @param rate_uno_tres
       * @param rate_uno_cuatro
       * @param rate_uno_cinco
       * @param rate_dos_cuatro
       * @param rate_dos_cinco
       * @param rate_dos_seis
       * @param rate_tres_cinco
       * @param rate_tres_seis
       * @param rate_cuatro_seis
       */
      void sendRatesValuesCoin( int rate_uno_tres, int rate_uno_cuatro, int rate_uno_cinco,int rate_dos_cuatro,int rate_dos_cinco,int rate_dos_seis,int rate_tres_cinco,int rate_tres_seis,int rate_cuatro_seis);

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
       * @brief sendOffSetValues
       * @param index
       * @param offsets
       */
      void sendOffSetValues(int index,int *offsets);
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
       * @brief sendLogErrorCommand
       */
      void sendLogErrorCommand();
      /**
       * @brief sendMCAErrorCommand
       */
      void sendMCAErrorCommand();
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
      /**
       * @brief sendHitsMCA
       * @param hits
       * @param channels
       * @param pmt_head
       * @param mode
       */
      void sendHitsMCA(QVector<double> hits, int channels, QString pmt_head, int index, bool mode);

      /**
       * @brief sendSaturated
       * @param head
       * @param Saturados
       */
      void sendSaturated(int head, double *Saturados );

      /**
       * @brief sendPicosLog
       * @param Pico
       * @param index
       */
      void sendPicosLog(struct Pico_espectro Pico,int index);

      /**
       * @brief sendPicosMCA
       * @param Pico
       * @param index
       */
      void sendPicosMCA(struct Pico_espectro Pico,int index);

      /**
       * @brief sendValuesMCA
       * @param time
       * @param hv_pmt
       * @param offset
       * @param var
       * @param mode
       */
      void sendValuesMCA(long long time, int hv_pmt, int offset, int var, bool mode);
      /**
       * @brief clearGraphsPMTs
       */
      void clearGraphsPMTs();

      /**
       * @brief clearGraphsCalib
       */
      void clearGraphsCalib();

      /**
       * @brief clearGraphsHeads
       */
      void clearGraphsHeads();

      /**
       * @brief StatusFinishFPGA
       * @param status
       */
      void StatusFinishFPGA(bool status);

      /**
       * @brief StatusFinishAdq
       */
      void StatusFinishAdq(bool status);
      void StatusFinishMoveToServer(bool status);

      void sendresetHeads();



  public slots:
      void getLogWork();
      void setAbortBool(bool abort);
      void setModeBool(bool mode) { _mode = mode; }
      void setModeCabCalib(bool cabcalib) {_CabCalib=cabcalib;}
      void setCentroidMode(bool mode) { _centroid = mode; }
      void cancelLogging(bool var) { log_finished = var; }
      void receivedFinalElapsedTimeString(QString eatime_string) { etime = eatime_string; }

      void setCommands(QStringList Command){commands=(Command);}
      void setCantArchivos(int cant){cantidad_archivos=cant;}
      void getElapsedTime();
      void getMCA();
      void TimerUpdate();
      void GrabarFPGA( );
      void requestGrabarFPGA();
      void requestAdquirir();
      bool Adquirir_handler();
      void requestMoveToServer();
      bool MoveToServer_handler();
      void prtstdoutput();
      void prtstderror();





  public:
      /**
       * @brief restoreLoggingVariable
       */
      void restoreLoggingVariable() { log_finished = false; }
      /**
       * @brief cancelLogging
       */
      void setCheckedHeads(QList<int> list) {checkedHeads = list; }
      /**
       * @brief setPMTSelectedList
       * @param list
       */
      void setPMTSelectedList(QList<QString> list) {pmt_selected_list = list;}
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

      bool getrunning(){return running;}

  };

}

#endif // APTHREAD_H
