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
    cout<<"Calibrador..."<<endl;

    if(calibrador->Mode_AutoCalibTiempos)
        calibrador->initCalibTiempos();
    else
        calibrador->initCalib(); // Modo autocalib normal

    while(!_abort)
    {
     try
     {
        QEventLoop loop;
        // Espera tiempo de acumulación
        for (int i=0 ; i<1000 ; i++)
        {
            QTimer::singleShot(calibrador->getTiempo_adq(), &loop, SLOT(quit()));
            if(!_abort) loop.exec();
        }
        QVector<double> aux_hits;
        emit clearGraphsCalib();
        if(!_abort){

            if(calibrador->Mode_AutoCalibTiempos) // Calibrador de tiempos
            {
                // Recordar que los PMTs son las máquinas de Coincidencias
                for (int i=0 ; i<calibrador->PMTs_List.length() ; i++)
                {
                    // Pido MCA de calibracion
                    calibrador->getMCA(QString::number(calibrador->PMTs_List[i]).toStdString(), calibrador->getFunCHead(), "07", CHANNELS, calibrador->port_name.toStdString()); // el "07" es para Coin

                    // Leo los hits y los paso a double
                    aux_hits = calibrador->getHitsMCA();
                    for (int j = 0 ; j < CHANNELS ; j++)
                    {
                      calibrador->Hist_Double[calibrador->PMTs_List[i]-1][j] = aux_hits[j];
                    }
                    emit sendHitsCalib(aux_hits, CHANNELS, QString::number(calibrador->PMTs_List[i]), i, false);
                }

                calibrador->calibrar_tiempos();

                emit sendOffButtonCalib();

            }
            else // Modo autocalib normal
            {
                calibrador->InitSP3(QString::number(calibrador->Cab_actual).toStdString(), calibrador->port_name.toStdString());
                for (int i=0 ; i<calibrador->PMTs_List.length() ; i++)
                {
                    // Pido estado de SP3
//                    calibrador->getMCA(QString::number(calibrador->PMTs_List[i]).toStdString(), calibrador->getFunCSP3(), QString::number(calibrador->Cab_actual).toStdString(), CHANNELS, calibrador->port_name.toStdString());
//                    calibrador->Dinodos_PMT[calibrador->PMTs_List[i]-1] = calibrador->getHVMCA();

                    // Pido MCA de calibracion
                    calibrador->getMCA(QString::number(calibrador->PMTs_List[i]).toStdString(), calibrador->getFunCHead(), QString::number(calibrador->Cab_actual).toStdString(), CHANNELS, calibrador->port_name.toStdString());

                    // Leo los hits y los paso a double
                    aux_hits = calibrador->getHitsMCA();
                    for (int j = 0 ; j < CHANNELS ; j++)
                        calibrador->Hist_Double[calibrador->PMTs_List[i]-1][j] = aux_hits[j];

                    if(!calibrador->AutoCalibBackground) emit sendHitsCalib(aux_hits, CHANNELS, QString::number(calibrador->PMTs_List[i]), i, false);
                }


                int error = calibrador->calibrar_simple();
                if( error < 0)
                    emit sendAutocalibReady(false,error);


                cout<<"PMT calibrados: "<<calibrador->PMTsEnPico<<endl;

                if(calibrador->PMTsEnPico == calibrador->PMTs_List.length())
                {
                    cout<<"Lista calibrada para copiar a archivo:"<<endl;
                    for (int i=0 ; i<calibrador->PMTs_List.length() ; i++)
                    {
                        cout<<calibrador->PMTs_List[i]<<'\t'<<calibrador->Dinodos_PMT[calibrador->PMTs_List[i]-1]<<endl;
                    }

                    if(calibrador->PMTs_List.length() == PMTs)
                    {
                        // Pido MCA a cabezal
                        calibrador->getMCA(QString::number(0).toStdString(), calibrador->getFunCHead(), QString::number(calibrador->Cab_actual).toStdString(), CHANNELS, calibrador->port_name.toStdString());

                        if(!calibrador->AutoCalibBackground)
                        {
                            // Leo los hits y los paso a double
                            aux_hits = calibrador->getHitsMCA();
                            for (int j = 0 ; j < CHANNELS ; j++)
                            {
                              calibrador->Hist_Double[calibrador->PMTs_List[0]-1][j] = aux_hits[j];
                            }
                            // Dibujo el espectro resultante del cabezal después de la calibración
                            emit sendHitsCalib(aux_hits, CHANNELS, QString::number(calibrador->PMTs_List[0]), 0, false);
                            struct Pico_espectro aux;
                            aux = calibrador->Buscar_Pico(calibrador->Hist_Double[calibrador->PMTs_List[0]-1], CHANNELS);
                            emit sendValuesMCACalib(calibrador->getHVMCA(), aux.canal_pico, (aux.limites_FWTM[1] - aux.limites_FWTM[0]));
                        }

                        calibrador->Pico_MAX = 0;
                        calibrador->Pico_MIN = 255;
                        calibrador->Dinodo_MAX = 0;
                        calibrador->Dinodo_MIN = 3000;

                        for (int j = 0 ; j < PMTs ; j++)
                        {
                          if( calibrador->Pico_MAX < calibrador->Picos_PMT[j] )
                              calibrador->Pico_MAX = calibrador->Picos_PMT[j];
                          if( calibrador->Pico_MIN > calibrador->Picos_PMT[j] )
                              calibrador->Pico_MIN = calibrador->Picos_PMT[j];
                          if( calibrador->Dinodo_MAX < calibrador->Dinodos_PMT[j] )
                              calibrador->Dinodo_MAX = calibrador->Dinodos_PMT[j];
                          if( calibrador->Dinodo_MIN > calibrador->Dinodos_PMT[j] )
                              calibrador->Dinodo_MIN = calibrador->Dinodos_PMT[j];
                        }
                    }

                    if(calibrador->AutoCalibBackground)
                        emit sendAutocalibReady(true);
                    else
                        emit sendOffButtonCalib();
                }

            }

        }
     }
     catch(Exceptions ex)
     {
        cout<<"Se va el Thread por esta Exception: "<<ex.excdesc<<endl;
     }
    }
    calibrador->portDisconnect();
    emit sendConnectPortArpet();
    emit finished();
}



