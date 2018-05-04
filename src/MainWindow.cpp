#include "inc/MainWindow.h"
#include "ui_MainWindow.h"
#include "ui_SetPreferences.h"
#include "ui_SetPMTs.h"
#include <QFileInfo>


/**
 * @brief MainWindow::MainWindow
 *
 * Constructor de la clase
 *
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    is_abort_mcae(true),
    is_abort_calib(true),
    is_abort_log(true),
    debug(false),
    init(false),
    log(true),
    stdout_mode(false),
    initfile("/media/arpet/pet/calibraciones/03-info/cabezales/ConfigINI/config_cabs_linux.ini"),
    root_calib_path("/media/arpet/pet/calibraciones/campo_inundado/03-info"),
    root_config_path("/media/arpet/pet/calibraciones/03-info/cabezales"),
    root_log_path("/home/ar-pet/.qt-mca/logs"),
    preferencesdir(QDir::homePath() + "/.qt-mca"),
    preferencesfile("qt-mca.conf"),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    /** @todo: Ver la posibilidad de maximizar la ventana */
    this->setFixedSize(this->maximumSize());
    setInitialConfigurations();
    setPreferencesConfiguration();
    getPaths();
//    ui->tabWidget_general->setTabEnabled(Tab1,false); // Escondo pestaña MCA
//    ui->tabWidget_general->setTabEnabled(Tab4,false); // Escondo pestaña Autocalib
//    ui->tabWidget_general->setTabEnabled(Tab9,false); // Escondo pestaña Terminal
//    ui->comboBox_head_select_config->hide();
    ui->comboBox_head_mode_select_config->hide();
    ui->comboBox_head_select_graph->hide();
    ui->comboBox_adquire_mode->hide();
    ui->lineEdit_aqd_path_file->hide();
    ui->pushButton_aqd_file_open->hide();
    ui->cb_Calib_Cab->hide();
    ui->lineEdit_aqd_file_size->clear();

    ui->frame_multihead_graph_2->show();
    ui->tabWidget_mca->setCurrentIndex(1);
    QTimer *timerw = new QTimer(this);
    connect(timerw, SIGNAL(timeout()), this, SLOT(updateCaption()));

    CargoTemaOscuro();

    timerw->start(1000);
    //parseConfigurationFile(true, "0");

    for (int i=1;i<=6;i++){
        if (loadCalibrationTables(QString::number(i))){
            break;
        }
    }


}
/**
 * @brief MainWindow::~MainWindow
 *
 * Destructor de la clase
 *
 */
MainWindow::~MainWindow()
{
    arpet->portDisconnect();
    worker->abort();
    thread->wait();
    delete thread;
    delete worker;

    worker_fpga->abort();
    thread_fpga->wait();
    delete thread_fpga;
    delete worker_fpga;

    worker_adq->abort();
    thread_adq->wait();
    delete thread_adq;
    delete worker_adq;

    worker_copy->abort();
    thread_copy->wait();
    delete thread_copy;
    delete worker_copy;

    etime_wr->abort();
    etime_th->wait();
    delete etime_th;
    delete etime_wr;

    mcae_wr->abort();
    mcae_th->wait();
    delete mcae_th;
    delete mcae_wr;

    calib_wr->abort();
    calib_th->wait();
    delete calib_th;
    delete calib_wr;

    delete pref;
    delete pmt_select;
    delete ui;
}
/**
 * @brief MainWindow::setInitialConfigurations
 *
 * Inicialización de múltiples configuraciones relacionadas con la comunicación, los gráficos y la interfaz gráfica
 *
 */
void MainWindow::setInitialConfigurations()
{
    //ARPET
    arpet = shared_ptr<MCAE>(new MCAE());
    pref = new SetPreferences(this);
    pmt_select = new SetPMTs(this);

    pmt_select_autocalib = new SetPMTs(this);

    // Calibrador
    calibrador = shared_ptr<AutoCalib>(new AutoCalib());

    //Threads
    thread = new QThread();
    worker = new Thread(arpet, &mMutex);
    worker->moveToThread(thread);
    etime_th = new QThread();
    etime_wr = new Thread(arpet, &mMutex);
    etime_wr->moveToThread(etime_th);
    mcae_th = new QThread();
    mcae_wr = new Thread(arpet, &mMutex);
    mcae_wr->moveToThread(mcae_th);
    calib_th = new QThread();
    calib_wr = new AutoCalibThread(calibrador, &mMutex);
    calib_wr->moveToThread(calib_th);

    thread_fpga = new QThread();
    worker_fpga = new Thread(arpet, &Mutex_fpga);
    worker_fpga->moveToThread(thread_fpga);

    thread_adq = new QThread();
    worker_adq = new Thread(arpet, &Mutex_adq);
    worker_adq->moveToThread(thread_adq);

    thread_copy = new QThread();
    worker_copy = new Thread(arpet, &Mutex_copy);
    worker_copy->moveToThread(thread_copy);

    connectSlots();

    // Reconstructor
    recon_externa = shared_ptr<Reconstructor>(new Reconstructor());
    // Lleno defaults de la solapa del constructor
    ui->Box_Cant_anillos->setText(QString::number(recon_externa->getCant_anillos()));
    ui->Box_Dif_anillos->setText(QString::number(recon_externa->getDif_anillos()));
    ui->Box_Emax->setText(QString::number(recon_externa->getEmax()));
    ui->Box_Emin->setText(QString::number(recon_externa->getEmin()));
    ui->Box_Span->setText(QString::number(recon_externa->getSpan()));
    ui->Box_cant_ang->setText(QString::number(recon_externa->getcant_ang()));
    ui->Box_cant_rhos->setText(QString::number(recon_externa->getcant_rhos()));
    ui->Box_max_Rho->setText(QString::number(recon_externa->getmax_Rho()));
    ui->Box_max_Z->setText(QString::number(recon_externa->getmax_Z()));
    ui->Box_FOV_Axial->setText(QString::number(recon_externa->getFOV_Axial()));
    ui->Box_Min_dif_cab->setText(QString::number(recon_externa->getMin_dif_cab()));
    ui->Box_Radio_FOV->setText(QString::number(recon_externa->getRadio_FOV()));
    ui->Box_Radio_PET->setText(QString::number(recon_externa->getRadio_PET()));
    ui->Box_zona_muerta->setText(QString::number(recon_externa->getzona_muerta()));
    ui->Box_Iteraciones->setText(QString::number(recon_externa->getIteraciones()));
    ui->textBrowser_entrada_2->setText(recon_externa->getPathAPIRL());
    ui->textBrowser_entrada_3->setText(recon_externa->getPathINTERFILES());
    ui->textBrowser_entrada_5->setText(recon_externa->getPathPARSER());
    ui->textBrowser_entrada_4->setText(recon_externa->getPathSalida());
    ui->plainTextEdit_Recon_console->setReadOnly(true);    // Seteo el texto a modo solo lectura
    recon_externa->setConsola(ui->plainTextEdit_Recon_console); // Conecto la consola de salida
    ui->textBrowser_entrada_2->setReadOnly(false);
    //ui->frame_multihead_graph_2->hide();
    manageHeadCheckBox("config",false);
    manageHeadCheckBox("mca",false);
    setAdquireMode(ui->comboBox_adquire_mode->currentIndex());
    ui->frame_adquire_advance_mode->hide();
    ui->comboBox_head_select_calib->hide();
    ui->comboBox_head_mode_select_graph->hide();
    ui->label_calib->hide();

    // Inicializo algunos widgets de FPGA

    ui->label_FPGA_3     ->setVisible(false);
    ui->text_FPGA_1      ->setVisible(false);
    ui->pushButton_FPGA_1->setVisible(false);


    ui->lineEdit_WN->setValidator(new QIntValidator(1, 127, this));
    ui->lineEdit_WP->setValidator(new QIntValidator(1, 128, this));
    ui->lineEdit_between_logs->setValidator( new QIntValidator(1, 3600, this) );
    ui->lineEdit_pmt_terminal->setValidator( new QIntValidator(1, PMTs, this) );
    ui->lineEdit_hv_value->setValidator( new QIntValidator(0, MAX_HV_VALUE, this) );
    ui->lineEdit_pmt_hv_terminal->setValidator( new QIntValidator(0, MAX_HV_VALUE, this) );
    ui->lineEdit_alta->setValidator( new QIntValidator(MIN_HIGH_HV_VOLTAGE, MAX_HIGH_HV_VOLTAGE, this) );
    ui->lineEdit_psoc_hv_terminal->setValidator( new QIntValidator(MIN_HIGH_HV_VOLTAGE, MAX_HIGH_HV_VOLTAGE, this) );
    ui->tabWidget_general->setCurrentWidget(ui->config);

    setQListElements();
    SetQCustomPlotConfiguration(ui->specPMTs, CHANNELS_PMT);
    SetQCustomPlotConfiguration(ui->specHead, CHANNELS);
    SetQCustomPlotSlots("Cuentas por PMT", "Cuentas en el Cabezal");
    arpet->resetHitsMCA();
    setHitsInit(true);
    setAdquireMode(CABEZAL);
    Estado_Cabezales.reserve(6);



}
/**
 * @brief MainWindow::setPreferencesConfiguration
 */
void MainWindow::setPreferencesConfiguration()
{
    /*Configuración inicial de preferencias*/
    QString default_pref_file = "[Modo]\ndebug=false\nlog=true\nstdout=false\n[Paths]\nconf_set_file=" + initfile + "\ncalib_set_file=" + root_calib_path + "\n";
    writePreferencesFile(default_pref_file, preferencesfile);
    getPreferencesSettingsFile();
    writeDebugToStdOutLogFile();
}
/**
 * @brief MainWindow::SetQCustomPlotConfiguration
 * @param graph
 * @param channels
 */
void MainWindow::SetQCustomPlotConfiguration(QCustomPlot *graph, int channels)
{
    graph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                           QCP::iSelectLegend | QCP::iSelectPlottables);
    graph->axisRect()->setupFullAxesBox();
    graph->xAxis->setRange(0, channels);
    graph->yAxis->setRange(-5, 5);
    graph->xAxis->setLabel("Canales");
    graph->yAxis->setLabel("Cuentas");
    graph->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    graph->legend->setFont(legendFont);
    graph->legend->setSelectedFont(legendFont);
    graph->setNoAntialiasingOnDrag(true);
    graph->legend->setSelectableParts(QCPLegend::spItems);
    graph->plotLayout()->insertRow(0);
}
/**
 * @brief MainWindow::SetQCustomPlotSlots
 * @param title_pmt_str
 * @param title_head_str
 */
void MainWindow::SetQCustomPlotSlots(string title_pmt_str, string title_head_str)
{
    /* Slots para PMTs */

    QCPTextElement *title_pmt = new QCPTextElement(ui->specPMTs, title_pmt_str.c_str(), QFont("sans", 16, QFont::Bold));
    connect(ui->specPMTs, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChangedPMT()));
    connect(ui->specPMTs, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePressPMT()));
    connect(ui->specPMTs, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheelPMT()));
    connect(ui->specPMTs->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->specPMTs->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->specPMTs->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->specPMTs->yAxis2, SLOT(setRange(QCPRange)));
    connect(ui->specPMTs, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClickPMT(QCPAxis*,QCPAxis::SelectablePart)));
    connect(ui->specPMTs, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClickPMT(QCPLegend*,QCPAbstractLegendItem*)));
    connect(title_pmt, SIGNAL(doubleClicked(QMouseEvent*)), this, SLOT(titleDoubleClickPMT(QMouseEvent*)));
    connect(ui->specPMTs, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));
    ui->specPMTs->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->specPMTs, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequestPMT(QPoint)));
    ui->specPMTs->plotLayout()->addElement(0, 0, title_pmt);

    /* Slots para Head */
    QCPTextElement *title_head = new QCPTextElement(ui->specHead, title_head_str.c_str(), QFont("sans", 16, QFont::Bold));
    connect(ui->specHead, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChangedHead()));
    connect(ui->specHead, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePressHead()));
    connect(ui->specHead, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheelHead()));
    connect(ui->specHead->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->specHead->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->specHead->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->specHead->yAxis2, SLOT(setRange(QCPRange)));
    connect(ui->specHead, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClickHead(QCPAxis*,QCPAxis::SelectablePart)));
    connect(ui->specHead, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClickHead(QCPLegend*,QCPAbstractLegendItem*)));
    connect(title_head, SIGNAL(doubleClicked(QMouseEvent*)), this, SLOT(titleDoubleClickHead(QMouseEvent*)));
    connect(ui->specHead, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*,int)));
    ui->specHead->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->specHead, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequestHead(QPoint)));
    ui->specHead->plotLayout()->addElement(0, 0, title_head);
}
/**
 * @brief MainWindow::checkCombosStatus
 *
 * _Slots_ de intercomunicación para los diferentes _QWidget_ de la UI.
 *
 */
void MainWindow::checkCombosStatus()
{
    connect(ui->comboBox_head_mode_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(setHeadModeGraph(int)));
    connect(ui->comboBox_head_mode_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(setTabHead(int)));
    connect(ui->comboBox_head_mode_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(setHeadModeConfig(int)));
    connect(ui->comboBox_head_mode_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(setTabHead(int)));
    connect(ui->comboBox_adquire_mode ,SIGNAL(currentIndexChanged (int)),this,SLOT(setAdquireMode(int)));
    connect(ui->tabWidget_mca ,SIGNAL(currentChanged(int)),this,SLOT(setTabMode(int)));
    connect(ui->comboBox_head_mode_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadModeComboBoxToMCA(int)));
    connect(ui->comboBox_head_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadComboBoxToMCA(int)));
    connect(ui->comboBox_head_mode_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadModeComboBoxToConfig(int)));
    connect(ui->comboBox_head_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadComboBoxToConfig(int)));
    connect(ui->comboBox_adquire_mode_coin ,SIGNAL(currentIndexChanged (int)),this,SLOT(setAdvanceCoinMode(int)));
    connect(ui->checkBox_mca_1 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead1ToConfig(bool)));
    connect(ui->checkBox_mca_2 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead2ToConfig(bool)));
    connect(ui->checkBox_mca_3 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead3ToConfig(bool)));
    connect(ui->checkBox_mca_4 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead4ToConfig(bool)));
    connect(ui->checkBox_mca_5 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead5ToConfig(bool)));
    connect(ui->checkBox_mca_6 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead6ToConfig(bool)));
    connect(ui->checkBox_c_1 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead1ToMCA(bool)));
    connect(ui->checkBox_c_2 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead2ToMCA(bool)));
    connect(ui->checkBox_c_3 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead3ToMCA(bool)));
    connect(ui->checkBox_c_4 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead4ToMCA(bool)));
    connect(ui->checkBox_c_5 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead5ToMCA(bool)));
    connect(ui->checkBox_c_6 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead6ToMCA(bool)));
    connect(ui->comboBox_head_mode_select_graph_2 ,SIGNAL(currentIndexChanged (int)),this,SLOT(setTabLog(int)));

}
/**
 * @brief MainWindow::connectSlots
 */
void MainWindow::connectSlots()
{
    /* Threads signals/slots */
    connect(worker, SIGNAL(sendRatesValues(int, int, int, int)), this, SLOT(writeRatesToLog(int,  int, int, int)));

    connect(worker, SIGNAL(sendRatesValuesCoin(int, int, int,int,int,int,int,int,int)), this, SLOT(writeRatesCoinToLog(  int, int, int,int,int,int,int,int,int)));
    //connect(thread_FPGA, SIGNAL( Grabo_OK(bool)),this , SLOT(image_button(bool)) );
    connect(worker, SIGNAL(sendresetHeads()), this, SLOT(recieveresetheads()));



    connect(worker, SIGNAL(sendTempValues(int, double, double, double)), this, SLOT(writeTempToLog(int,  double, double, double)));
    connect(worker, SIGNAL(sendOffSetValues(int, int *)), this, SLOT(writeOffSetToLog(int,  int *)));
    connect(worker, SIGNAL(logRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), worker, SLOT(getLogWork()));

    connect(worker_fpga, SIGNAL(GrabarFPGArequested()), thread_fpga, SLOT(start()));
    connect(thread_fpga, SIGNAL(started()), worker_fpga, SLOT(GrabarFPGA()));

    connect(worker_adq, SIGNAL(AdquisicionRequested()), thread_adq, SLOT(start()));
    connect(thread_adq, SIGNAL(started()), worker_adq, SLOT(Adquirir_handler()));

    connect(worker_copy, SIGNAL(MoveToServerRequested()), thread_copy, SLOT(start()));
    connect(thread_copy, SIGNAL(started()), worker_copy, SLOT(MoveToServer_handler()));

    connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);
    connect(this,   SIGNAL(sendAbortCommand(bool)),worker,SLOT(setAbortBool(bool)));
    connect(worker, SIGNAL(sendLogErrorCommand()),this,SLOT(getLogErrorThread()));

    connect(worker, SIGNAL(startElapsedTime()), etime_th, SLOT(start()), Qt::DirectConnection);
    connect(worker, SIGNAL(finishedElapsedTime(bool)), etime_wr, SLOT(cancelLogging(bool)));
    connect(etime_th, SIGNAL(started()), etime_wr, SLOT(getElapsedTime()));
    connect(etime_wr, SIGNAL(finished()), etime_th, SLOT(quit()), Qt::DirectConnection);
    connect(etime_wr, SIGNAL(sendElapsedTimeString(QString)),this,SLOT(receivedElapsedTimeString(QString)));
    connect(etime_wr, SIGNAL(sendFinalElapsedTimeString(QString)),worker,SLOT(receivedFinalElapsedTimeString(QString)));

    connect(mcae_wr, SIGNAL(mcaRequested()), mcae_th, SLOT(start()));
    connect(mcae_th, SIGNAL(started()), mcae_wr, SLOT(getMCA()));
    connect(mcae_wr, SIGNAL(finished()), mcae_th, SLOT(quit()), Qt::DirectConnection);
    connect(this,   SIGNAL(sendAbortMCAECommand(bool)),mcae_wr,SLOT(setAbortBool(bool)));
    connect(mcae_wr, SIGNAL(sendMCAErrorCommand()),this,SLOT(getMCAErrorThread()));
    connect(mcae_wr, SIGNAL(sendHitsMCA(QVector<double>, int, QString, int, bool)),this,SLOT(receivedHitsMCA(QVector<double>, int, QString, int, bool)));
    connect(worker, SIGNAL(sendSaturated(int , double * )),this, SLOT( recievedSaturated(int , double *)));
    connect(worker, SIGNAL(sendPicosLog(struct Pico_espectro ,int)),this, SLOT( recievedPicosLog(struct Pico_espectro ,int)));
    connect(mcae_wr, SIGNAL(sendPicosMCA(struct Pico_espectro ,int)),this, SLOT( recievedPicosMCA(struct Pico_espectro ,int)));

    connect(mcae_wr, SIGNAL(sendValuesMCA(long long, int, int, int, bool)),this,SLOT(receivedValuesMCA(long long, int, int, int, bool)));
    connect(mcae_wr, SIGNAL(clearGraphsPMTs()),this,SLOT(clearSpecPMTsGraphs()));
    connect(mcae_wr, SIGNAL(clearGraphsHeads()),this,SLOT(clearSpecHeadsGraphs()));

    connect(calib_wr, SIGNAL(CalibRequested()), calib_th, SLOT(start()));
    connect(calib_th, SIGNAL(started()), calib_wr, SLOT(getCalib()));
    connect(calib_wr, SIGNAL(finished()), calib_th, SLOT(quit()), Qt::DirectConnection);
    connect(this, SIGNAL(sendCalibAbortCommand(bool)),calib_wr,SLOT(setAbortBool(bool)));
    connect(calib_wr, SIGNAL(sendCalibErrorCommand()),this,SLOT(getCalibErrorThread()));
    connect(calib_wr, SIGNAL(sendConnectPortArpet()),this,SLOT(connectPortArpet()));
    connect(calib_wr, SIGNAL(sendOffButtonCalib()),this,SLOT(OffButtonCalib()));
    connect(calib_wr, SIGNAL(sendHitsCalib(QVector<double>, int, QString, int, bool)),this,SLOT(receivedHitsCalib(QVector<double>, int, QString, int, bool)));
    connect(calib_wr, SIGNAL(sendValuesMCACalib(int, int, int)),this,SLOT(receivedValuesMCACalib(int, int, int)));
    connect(calib_wr, SIGNAL(clearGraphsCalib()),this,SLOT(clearSpecCalibGraphs()));

    /* THREAD FPGA*/
    connect(worker_fpga, SIGNAL(StatusFinishFPGA(bool )),this,SLOT(checkStatusFPGA(bool )));

    /* THREAD ADQ */

    connect(worker_adq, SIGNAL(StatusFinishAdq(bool )),this,SLOT(checkStatusAdq(bool)));


    connect(worker_copy, SIGNAL(StatusFinishMoveToServer(bool )),this,SLOT(checkStatusMoveToServer(bool)));


    /* Objetos */
    connect(this, SIGNAL(ToPushButtonAdquirir(bool)),ui->pushButton_adquirir,SLOT(setChecked(bool)));
    connect(this, SIGNAL(ToPushButtonLogger(bool)),ui->pushButton_logguer,SLOT(setChecked(bool)));
}
/**
 * @brief MainWindow::writeRatesToLog
 * @param index
 * @param rate_low
 * @param rate_med
 * @param rate_high
 */
void MainWindow::writeRatesToLog(int index, int rate_low, int rate_med, int rate_high)
{
    writeLogFile("[LOG-RATE],"+ QString::fromStdString(getLocalDateAndTime()) +",Cabezal,"+QString::number(index)+","+QString::number(rate_low)+","+QString::number(rate_med)+","+QString::number(rate_high)+"\n");
}

/**
 * @brief MainWindow::writeRatesCoinToLog
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
void MainWindow::writeRatesCoinToLog(int rate_uno_tres, int rate_uno_cuatro, int rate_uno_cinco,int rate_dos_cuatro,int rate_dos_cinco,int rate_dos_seis,int rate_tres_cinco,int rate_tres_seis,int rate_cuatro_seis)
{
    writeLogFile("[LOG-RATECOIN],"+ QString::fromStdString(getLocalDateAndTime()) +","+QString::number(rate_uno_tres)+","+QString::number(rate_uno_cuatro)+","+QString::number(rate_uno_cinco)+","+QString::number(rate_dos_cuatro)+","+QString::number(rate_dos_cinco)+","+QString::number(rate_dos_seis)+","+QString::number(rate_tres_cinco)+","+QString::number(rate_tres_seis)+","+QString::number(rate_cuatro_seis)+"\n");
}
/**
 * @brief MainWindow::writeTempToLog
 * @param index
 * @param min
 * @param med
 * @param max
 */
void MainWindow::writeTempToLog(int index, double min, double med, double max)
{
    writeLogFile("[LOG-TEMP],"+ QString::fromStdString(getLocalDateAndTime()) +",Cabezal,"+QString::number(index)+","+QString::number(min)+","+QString::number(med)+","+QString::number(max)+"\n");
}

/**
 * @brief MainWindow::recievedPicosLog
 * @param Pico
 * @param index
 */
void MainWindow::recievedPicosLog(struct Pico_espectro Pico,int index)
{
    writeLogFile("[LOG-PICO],"+ QString::fromStdString(getLocalDateAndTime()) +",Cabezal,"+QString::number(index)+","+QString::number(Pico.canal_pico)+","+QString::number(Pico.FWHM*100)+"\n");
    //ui->pushButton_reset->clicked(true);
}

/**
 * @brief recieveresetheads
 */
void MainWindow::recieveresetheads(){
    resetHeads();
}

/**
 * @brief MainWindow::recievedPicosMCA
 * @param Pico
 * @param index
 */
void MainWindow::recievedPicosMCA(struct Pico_espectro Pico,int index)
{
   // writeLogFile("[LOG-PICO],"+ QString::fromStdString(getLocalDateAndTime()) +",Cabezal,"+QString::number(index)+","+QString::number(Pico.canal_pico)+","+QString::number(Pico.FWHM*100)+"\n");
    //ui->pushButton_reset->clicked(true);

    ui->label_data_output->setText(/*ui->label_data_output->text()+*/" | Pico: "+QString::number(Pico.canal_pico)+" | FWHM: "+QString::number(Pico.FWHM*100)+" |");
}


/**
 * @brief MainWindow::writeOffSetToLog
 * @param index
 * @param min
 * @param med
 * @param max
 */
void MainWindow::writeOffSetToLog(int index,int *offsets)
{
    writeLogFile("[LOG-OFFSET],"+ QString::fromStdString(getLocalDateAndTime()) +",Cabezal,"+QString::number(index)+", "+QString::number(offsets[0])+", "+
        QString::number(offsets[1])+", "+QString::number(offsets[2])+", "+QString::number(offsets[3])+", "+QString::number(offsets[4])+", "+QString::number(offsets[5])+", "+
        QString::number(offsets[6])+", "+QString::number(offsets[7])+", "+QString::number(offsets[8])+", "+QString::number(offsets[9])+", "+QString::number(offsets[10])+", "+
        QString::number(offsets[11])+", "+QString::number(offsets[12])+", "+QString::number(offsets[13])+", "+QString::number(offsets[14])+", "+QString::number(offsets[15])+", "
        +QString::number(offsets[16])+", "+QString::number(offsets[17])+", "+QString::number(offsets[18])+", "+QString::number(offsets[19])+", "+QString::number(offsets[20])+", "+
        QString::number(offsets[21])+", "+QString::number(offsets[22])+", "+QString::number(offsets[23])+", "+QString::number(offsets[24])+", "+QString::number(offsets[25])+", "+
        QString::number(offsets[26])+", "+QString::number(offsets[27])+", "+QString::number(offsets[28])+", "+QString::number(offsets[29])+", "+QString::number(offsets[30])+", "+
        QString::number(offsets[31])+", "+QString::number(offsets[32])+", "+QString::number(offsets[33])+", "+QString::number(offsets[34])+", "+QString::number(offsets[35])+", "+
        QString::number(offsets[36])+", "+QString::number(offsets[37])+", "+QString::number(offsets[38])+", "+QString::number(offsets[39])+", "+QString::number(offsets[40])+", "+
        QString::number(offsets[41])+", "+QString::number(offsets[42])+", "+QString::number(offsets[43])+", "+QString::number(offsets[44])+", "+QString::number(offsets[45])+", "+
        QString::number(offsets[46])+", "+QString::number(offsets[47])+"\n");

}
/**
 * @brief MainWindow::recievedSaturated
 * @param Cabezal
 * @param Saturados
 * @param modo
 */
void MainWindow::recievedSaturated(int Cabezal, double Saturados[48]){
    writeLogFile("[LOG-SAT],"+ QString::fromStdString(getLocalDateAndTime()) +",Cabezal,"+QString::number(Cabezal)+", "+
                 QString::number(Saturados[0])+", "+
        QString::number(Saturados[1])+", "+QString::number(Saturados[2])+", "+QString::number(Saturados[3])+", "+QString::number(Saturados[4])+", "+QString::number(Saturados[5])+", "+
        QString::number(Saturados[6])+", "+QString::number(Saturados[7])+", "+QString::number(Saturados[8])+", "+QString::number(Saturados[9])+", "+QString::number(Saturados[10])+", "+
        QString::number(Saturados[11])+", "+QString::number(Saturados[12])+", "+QString::number(Saturados[13])+", "+QString::number(Saturados[14])+", "+QString::number(Saturados[15])+", "
        +QString::number(Saturados[16])+", "+QString::number(Saturados[17])+", "+QString::number(Saturados[18])+", "+QString::number(Saturados[19])+", "+QString::number(Saturados[20])+", "+
        QString::number(Saturados[21])+", "+QString::number(Saturados[22])+", "+QString::number(Saturados[23])+", "+QString::number(Saturados[24])+", "+QString::number(Saturados[25])+", "+
        QString::number(Saturados[26])+", "+QString::number(Saturados[27])+", "+QString::number(Saturados[28])+", "+QString::number(Saturados[29])+", "+QString::number(Saturados[30])+", "+
        QString::number(Saturados[31])+", "+QString::number(Saturados[32])+", "+QString::number(Saturados[33])+", "+QString::number(Saturados[34])+", "+QString::number(Saturados[35])+", "+
        QString::number(Saturados[36])+", "+QString::number(Saturados[37])+", "+QString::number(Saturados[38])+", "+QString::number(Saturados[39])+", "+QString::number(Saturados[40])+", "+
        QString::number(Saturados[41])+", "+QString::number(Saturados[42])+", "+QString::number(Saturados[43])+", "+QString::number(Saturados[44])+", "+QString::number(Saturados[45])+", "+
        QString::number(Saturados[46])+", "+QString::number(Saturados[47])+"\n");
}

/**
 * @brief MainWindow::receivedElapsedTimeString
 * @param etime_string
 */
void MainWindow::receivedElapsedTimeString(QString etime_string)
{
    ui->label_elapsed_time->setText(etime_string);
}
/**
 * @brief MainWindow::clearSpecPMTsGraphs
 */
void MainWindow::clearSpecPMTsGraphs()
{
  ui->specPMTs->clearGraphs();
}

void MainWindow::clearSpecCalibGraphs()
{
  ui->specPMTs_2->clearGraphs();
}

/**
 * @brief MainWindow::checkStatusAdq
 * @param status
 */

void MainWindow::checkStatusAdq(bool status)
{


    if (!status)
    {

        adq_running = false;
        QPixmap image;
        image.load("/home/ar-pet/Downloads/ic_cancel.png");
        cant_archivos =1;
        ui->label_gif_3->setVisible(false);
        ui->label_gif_4->setVisible(true);
        ui->label_gif_4->setPixmap(image);
        ui->label_gif_4->setScaledContents( true );
        ui->label_gif_4->show();
        return;
    }

    if(copying)
    {
            finish_adquirir=true;
            return;
    }

        if (cant_archivos==1){

               worker_adq->requestAdquirir();
               worker_adq->setCantArchivos(cant_archivos);
               cant_archivos++;

        }

        else if(ui->lineEdit_aqd_cant_archivos->text().toInt()>=cant_archivos){




            copying=true;
            worker_adq->abort();
            thread_adq->exit(0);
            usleep(500);

            worker_adq->setCantArchivos(cant_archivos);

            worker_copy->abort();
            thread_copy->exit(0);
            usleep(5000);
            worker_copy->setCantArchivos(cant_archivos-1);

            worker_copy->requestMoveToServer();
            worker_adq->requestAdquirir();
            cant_archivos++;


        }else{
            usleep(500);



            worker_copy->abort();
            thread_copy->exit(0);
            usleep(5000);
            worker_copy->setCantArchivos(cant_archivos-1);

            worker_copy->requestMoveToServer();
            copying=true;

            //worker_copy->requestMoveToServer();
            adq_running = false;
            cant_archivos =1;
        }
        return;




}



/**
 * @brief MainWindow::checkStatusFPGA
 * @param status
 */

void MainWindow::checkStatusFPGA(bool status)
{

    QPixmap image;

    if(status)
       image.load("/home/ar-pet/Downloads/ic_check_circle.png");
    else
       image.load("/home/ar-pet/Downloads/ic_cancel.png");

    ui->label_gif->setVisible(false);
    ui->label_gif_2->setVisible(true);

    ui->label_gif_2->setPixmap(image);
    ui->label_gif_2->setScaledContents( true );
    ui->label_gif_2->show();


}
/**
 * @brief MainWindow::clearSpecHeadsGraphs
 */
void MainWindow::clearSpecHeadsGraphs()
{
  ui->specHead->clearGraphs();
}
/**
 * @brief MainWindow::receivedHitsMCA
 * @param hits
 * @param channels
 * @param pmt_head
 * @param index
 * @param mode
 */
void MainWindow::receivedHitsMCA(QVector<double> hits, int channels, QString pmt_head, int index, bool mode)
{
    if(mode)
    {
        addGraph(hits, ui->specPMTs, channels, pmt_head, qcp_pmt_parameters[index]);
    }
    else
    {
        addGraph(hits, ui->specHead, channels, pmt_head, qcp_head_parameters[index]);
    }
}

void MainWindow::receivedHitsCalib(QVector<double> hits, int channels, QString pmt_head, int index, bool mode)
{
    addGraph_Calib(hits, ui->specPMTs_2, channels, pmt_head, qcp_pmt_calib_parameters[index]);
}
/**
 * @brief MainWindow::receivedValuesMCA
 * @param time
 * @param hv_pmt
 * @param offset
 * @param var
 * @param mode
 */
void MainWindow::receivedValuesMCA(long long time, int hv_pmt, int offset, int var, bool mode)
{
    QString centroid_mode = ui->checkBox_centroid->isChecked() ? "Si" : "No";
    if(mode)
    {
        if (pmt_selected_list.length()==1)
        {
            ui->label_title_output->setText("MCA Extended | PMT: " + pmt_selected_list.at(0));
            ui->label_data_output->setText("| HV: "+QString::number(hv_pmt)+" | Varianza: "+QString::number(var)+" | Offset ADC: "+QString::number(offset)+" | Tiempo (mseg):"+QString::number(time/1000) + " | Modo Centroide: " + centroid_mode + " | "+"Offset : " +QString::number(arpet->getOffSetMCA()) );
        }
        else
        {
            ui->label_title_output->setText("");
            ui->label_data_output->setText("");
        }

    }
    else
    {
        if (getCheckedHeads().length()==1)
        {
            ui->label_title_output->setText("MCA Extended | Cabezal: " + QString::number(getCheckedHeads().at(0)));
            //ui->label_data_output->setText("| Umbral: "+ QString::number(hv_pmt) + " |");
        }
        else
        {
            ui->label_title_output->setText("");
            ui->label_data_output->setText("");
        }
    }

}

void MainWindow::receivedValuesMCACalib(int umbral, int pico, int FWHM)
{
    ui->label_title_output->setText("Cabezal calibrado");
    float num = (float)(FWHM*100)/pico;
    ui->label_data_output->setText("                     | Umbral: "+QString::number(umbral)+" | Pico: "+QString::number(pico)+" | FWHM: "+QString::number(num,'g',4)+"%");
}
/**
 * @brief MainWindow::on_comboBox_head_select_config_currentIndexChangedº
 * @param arg1
 */
void MainWindow::on_comboBox_head_select_config_currentIndexChanged(const QString &arg1)
{
    //getHeadStatus(arg1.toInt());

//    arpet->portDisconnect();                  // OBSOLETO


//    switch (arg1.toInt()) {
//    case 1:{
//        port_name=Cab1;
//      //  setHeadMode(1,"config");
//        break;
//    }
//    case 2:{
//        port_name=Cab2;
//       // setHeadMode(2,"config");

//        break;
//    }
//    case 3:{
//        port_name=Cab3;
//       // setHeadMode(3,"config");

//        break;
//    }
//    case 4:{
//        port_name=Cab4;
//       // setHeadMode(4,"config");
//        break;
//    }
//    case 5:{
//        port_name=Cab5;
//       // setHeadMode(5,"config");
//        break;
//    }
//    case 6:{
//        port_name=Cab6;
//      //  setHeadMode(6,"config");
//        break;
//    }
//    default:
//        break;
//    }
//    calibrador->setPort_Name((port_name));
//    worker->setPortName((port_name));
//    arpet->portConnect(port_name.toStdString().c_str());


}
/**
 * @brief MainWindow::getLogErrorThread
 */
void MainWindow::getLogErrorThread()
{
    setButtonLoggerState(false);
    setIsAbortLogFlag(false);
    QMessageBox::critical(this,tr("Error"),tr("Imposible adquirir los valores de tasa y/o temperatura en el proceso de logueo."));
    emit ToPushButtonLogger(false);
}
void MainWindow::getCalibErrorThread()
{
    QMessageBox::critical(this,tr("Error"),tr("Imposible calibrar sararasasdasdfsdllllllllllllllllllllllllllllllllllg."));
}
void MainWindow::getMCAErrorThread()
{
    //setButtonAdquireState(false);
    setIsAbortMCAEFlag(false);
    QMessageBox::critical(this,tr("Error"),tr("Imposible adquirir los valores de MCA de los fotomultiplicadores/cabezales seleccionados."));
    emit ToPushButtonAdquirir(false);
}
/**
 * @brief MainWindow::on_comboBox_adquire_mode_coin_currentIndexChanged
 * @param index
 */
void MainWindow::on_comboBox_adquire_mode_coin_currentIndexChanged(int index)
{
    if(index==3|| index==4)
    {
        ui->comboBox_head_select_calib->show();
        ui->label_calib->show();
    }
    else
    {
        ui->comboBox_head_select_calib->hide();
        ui->label_calib->hide();
    }
}
/**
 * @brief MainWindow::on_comboBox_head_mode_select_config_currentIndexChanged
 * @param index
 */
void MainWindow::on_comboBox_head_mode_select_config_currentIndexChanged(int index)
{
    if (index == ALLHEADS)
    {
        ui->checkBox_c_1->setChecked(true);
        ui->checkBox_c_2->setChecked(true);
        ui->checkBox_c_3->setChecked(true);
        ui->checkBox_c_4->setChecked(true);
        ui->checkBox_c_5->setChecked(true);
        ui->checkBox_c_6->setChecked(true);
    }
}
/**
 * @brief MainWindow::setQListElements
 */
void MainWindow::setQListElements()
{
    pmt_label_table.push_back(ui->label_pmt_01);
    pmt_label_table.push_back(ui->label_pmt_02);
    pmt_label_table.push_back(ui->label_pmt_03);
    pmt_label_table.push_back(ui->label_pmt_04);
    pmt_label_table.push_back(ui->label_pmt_05);
    pmt_label_table.push_back(ui->label_pmt_06);
    pmt_label_table.push_back(ui->label_pmt_07);
    pmt_label_table.push_back(ui->label_pmt_08);
    pmt_label_table.push_back(ui->label_pmt_09);
    pmt_label_table.push_back(ui->label_pmt_10);
    pmt_label_table.push_back(ui->label_pmt_11);
    pmt_label_table.push_back(ui->label_pmt_12);
    pmt_label_table.push_back(ui->label_pmt_13);
    pmt_label_table.push_back(ui->label_pmt_14);
    pmt_label_table.push_back(ui->label_pmt_15);
    pmt_label_table.push_back(ui->label_pmt_16);
    pmt_label_table.push_back(ui->label_pmt_17);
    pmt_label_table.push_back(ui->label_pmt_18);
    pmt_label_table.push_back(ui->label_pmt_19);
    pmt_label_table.push_back(ui->label_pmt_20);
    pmt_label_table.push_back(ui->label_pmt_21);
    pmt_label_table.push_back(ui->label_pmt_22);
    pmt_label_table.push_back(ui->label_pmt_23);
    pmt_label_table.push_back(ui->label_pmt_24);
    pmt_label_table.push_back(ui->label_pmt_25);
    pmt_label_table.push_back(ui->label_pmt_26);
    pmt_label_table.push_back(ui->label_pmt_27);
    pmt_label_table.push_back(ui->label_pmt_28);
    pmt_label_table.push_back(ui->label_pmt_29);
    pmt_label_table.push_back(ui->label_pmt_30);
    pmt_label_table.push_back(ui->label_pmt_31);
    pmt_label_table.push_back(ui->label_pmt_32);
    pmt_label_table.push_back(ui->label_pmt_33);
    pmt_label_table.push_back(ui->label_pmt_34);
    pmt_label_table.push_back(ui->label_pmt_35);
    pmt_label_table.push_back(ui->label_pmt_36);
    pmt_label_table.push_back(ui->label_pmt_37);
    pmt_label_table.push_back(ui->label_pmt_38);
    pmt_label_table.push_back(ui->label_pmt_39);
    pmt_label_table.push_back(ui->label_pmt_40);
    pmt_label_table.push_back(ui->label_pmt_41);
    pmt_label_table.push_back(ui->label_pmt_42);
    pmt_label_table.push_back(ui->label_pmt_43);
    pmt_label_table.push_back(ui->label_pmt_44);
    pmt_label_table.push_back(ui->label_pmt_45);
    pmt_label_table.push_back(ui->label_pmt_46);
    pmt_label_table.push_back(ui->label_pmt_47);
    pmt_label_table.push_back(ui->label_pmt_48);

    pmt_status_table.push_back(ui->label_pmt_estado_1);
    pmt_status_table.push_back(ui->label_pmt_estado_2);
    pmt_status_table.push_back(ui->label_pmt_estado_3);
    pmt_status_table.push_back(ui->label_pmt_estado_4);
    pmt_status_table.push_back(ui->label_pmt_estado_5);
    pmt_status_table.push_back(ui->label_pmt_estado_6);

    hv_status_table.push_back(ui->label_hv_estado_1);
    hv_status_table.push_back(ui->label_hv_estado_2);
    hv_status_table.push_back(ui->label_hv_estado_3);
    hv_status_table.push_back(ui->label_hv_estado_4);
    hv_status_table.push_back(ui->label_hv_estado_5);
    hv_status_table.push_back(ui->label_hv_estado_6);

    head_status_table.push_back(ui->label_cabezal_estado_1);
    head_status_table.push_back(ui->label_cabezal_estado_2);
    head_status_table.push_back(ui->label_cabezal_estado_3);
    head_status_table.push_back(ui->label_cabezal_estado_4);
    head_status_table.push_back(ui->label_cabezal_estado_5);
    head_status_table.push_back(ui->label_cabezal_estado_6);

    calib_status_table.push_back(ui->label_table_1);
    calib_status_table.push_back(ui->label_table_2);
    calib_status_table.push_back(ui->label_table_3);
    calib_status_table.push_back(ui->label_table_4);
    calib_status_table.push_back(ui->label_table_5);
    calib_status_table.push_back(ui->label_table_6);

    heads_coin_table.push_back(ui->comboBox_coin_1);
    heads_coin_table.push_back(ui->comboBox_coin_2);
    heads_coin_table.push_back(ui->comboBox_coin_3);
    heads_coin_table.push_back(ui->comboBox_coin_4);
    heads_coin_table.push_back(ui->comboBox_coin_5);
    heads_coin_table.push_back(ui->comboBox_coin_6);
    heads_coin_table.push_back(ui->comboBox_coin_7);
    heads_coin_table.push_back(ui->comboBox_coin_8);
    heads_coin_table.push_back(ui->comboBox_coin_9);



}
/**
 * @brief MainWindow::getLocalDateAndTime
 * @return
 */
string MainWindow::getLocalDateAndTime()
{
    return (QDateTime::currentDateTime().toString().toStdString());
}
/**
 * @brief MainWindow::getLogFileName
 * @param main
 * @return filename
 */
QString MainWindow::getLogFileName(QString main)
{
    QString suffix = QDateTime::currentDateTime().toString("yyyyMMddhh");
    QString prefix = "LOG";
    QString extension = ".log";

    return prefix + main + suffix + extension;
}
/**
 * @brief MainWindow::writeDebugToStdOutLogFile
 * @param main
 */
void MainWindow::writeDebugToStdOutLogFile(QString main)
{
    if (stdout_mode)
    {
        QString logFile = getPreferencesDir() + "/debug/Debug_" + getLogFileName(main);
        freopen(logFile.toLocal8Bit().data(), "a+", stdout);
    }
}
/**
 * @brief MainWindow::writeLogFile
 */
void MainWindow::writeLogFile(QString log_text, QString main)
{
    if (log)
    {
        QString logFile = getPreferencesDir() + "/logs/" + getLogFileName(main);
        QFile logger( logFile );
        logger.open(QIODevice::WriteOnly | QIODevice::Append);
        //QTextStream out(&logger);
        //out << log_text;
        //for (int j; j<log_text.length();j++){
            logger.write( log_text.toUtf8());

        //  }
        logger.close();
    }
}
/**
 * @brief MainWindow::copyRecursively
 * @param srcFilePath
 * @param tgtFilePath
 * @return
 */
bool MainWindow::copyRecursively(const QString &srcFilePath, const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        QDir targetDir(tgtFilePath);
        targetDir.cdUp();
        if (!targetDir.mkdir(QFileInfo(tgtFilePath).fileName()))
              return false;
        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        foreach (const QString &fileName, fileNames) {
            const QString newSrcFilePath
                    = srcFilePath + QLatin1Char('/') + fileName;
            const QString newTgtFilePath
                    = tgtFilePath + QLatin1Char('/') + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath))
                  return false;
        }
    } else {
        if (!QFile::copy(srcFilePath, tgtFilePath))
              return false;
    }
    return true;
}
/**
 * @brief MainWindow::writeFooterAndHeaderDebug
 * @param header
 */
void MainWindow::writeFooterAndHeaderDebug(bool header)
{
    if (header)
    {
        if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================="<<endl;
    }
    else
    {
        if(debug) cout<<"[END-LOG-DBG] ======================================================"<<endl;
    }
}

/* Menu: Preferencias */
/**
 * @brief MainWindow::on_actionPreferencias_triggered
 */
void MainWindow::on_actionPreferencias_triggered()
{
    getPreferencesSettingsFile();
    pref->setCalibDir(root_calib_path);
    pref->setConfFile(initfile);
    pref->setDegugConsoleValue(debug);
    pref->setLogFileValue(log);
    pref->setDebugStdOutValue(stdout_mode);

    int ret = pref->exec();
    bool debug_console = pref->getDegugConsoleValue();
    bool log_file = pref->getLogFileValue();
    bool stdout_pref = pref->getDebugStdOutValue();
    QString file = pref->getInitFileConfigPath();
    QString calib_path = pref->getCalibDirectoryPath();

    if(ret == QDialog::Accepted)
    {
        setDebugMode(debug_console);
        setLogMode(log_file);
        setStdOutDebugMode(stdout_pref);
        QString boolDebugText = debug_console ? "true" : "false";
        QString boolLogText = log_file ? "true" : "false";
        QString boolStdOutText = stdout_pref ? "true" : "false";
        setPreferencesSettingsFile("Modo", "debug", boolDebugText );
        setPreferencesSettingsFile("Modo", "log", boolLogText );
        setPreferencesSettingsFile("Modo", "stdout", boolStdOutText );
        setPreferencesSettingsFile("Paths", "conf_set_file", file);
        setPreferencesSettingsFile("Paths", "calib_set_file", calib_path);
        getPreferencesSettingsFile();
        writeDebugToStdOutLogFile();
    }
}
/**
 * @brief MainWindow::writePreferencesFile
 * @param pref
 * @param filename
 * @param force
 * @return MCAE::FILE_NOT_FOUND o MCAE::OK
 */
int MainWindow::writePreferencesFile(QString pref, QString filename, bool force)
{
    QString path = getPreferencesDir();
    QString logs = getPreferencesDir() + "/logs";
    QString debugs = getPreferencesDir() + "/debug";
    QDir dir(path);
    QDir log(logs);
    QDir deb(debugs);
    if (!dir.exists())
    {
        dir.mkdir(path);
    }
    if (!log.exists())
    {
        log.mkdir(logs);
    }
    if (!deb.exists())
    {
        deb.mkdir(debugs);
    }

    QFile file( path + "/" + filename );
    bool exist = file.exists();

    if (!force)
    {
        if (!exist)
        {
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            if(debug) cout<<"No se encuentra el archivo de preferencias. Se crea el archivo por defecto."<<endl;
            QTextStream out(&file);
            out << pref;
            file.close();
            return MCAE::FILE_NOT_FOUND;
        }
    }
    else
    {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << pref;
        file.close();
    }

    return MCAE::OK;
}
/**
 * @brief MainWindow::getPreferencesSettingsFile
 */
void MainWindow::getPreferencesSettingsFile()
{
    QString qtmca_conf = getPreferencesDir() +"/"+ preferencesfile;
    QSettings qtmcasettins(qtmca_conf, QSettings::IniFormat);

    //qtmcasettins.status();
    debug = qtmcasettins.value("Modo/debug", "US").toBool();
    log = qtmcasettins.value("Modo/log", "US").toBool();
    stdout_mode = qtmcasettins.value("Modo/stdout", "US").toBool();
    initfile = qtmcasettins.value("Paths/conf_set_file", "US").toString();
    root_calib_path = qtmcasettins.value("Paths/calib_set_file", "US").toString();
}
/**
 * @brief MainWindow::setPreferencesSettingsFile
 * @param folder
 * @param variable
 * @param value
 */
void MainWindow::setPreferencesSettingsFile(QString folder, QString variable, QString value)
{
    QString qtmca_conf = getPreferencesDir() +"/"+ preferencesfile;
    QSettings qtmcasettins(qtmca_conf, QSettings::IniFormat);

    qtmcasettins.setValue(folder+"/"+variable, value);
}
/**
 * @brief MainWindow::showMCAEStreamDebugMode
 * @param msg
 */
void MainWindow::showMCAEStreamDebugMode(string msg)
{
    cout<< "Trama recibida: "<< msg << " | Trama enviada: "<< arpet->getTrama_MCAE() <<endl;
}

/* Pestaña: "Configuración" */
/**
 * @brief MainWindow::getARPETStatus
 *
 * Estado del ARPET
 *
 */
void MainWindow::getARPETStatus()
{

}
/**
 * @brief MainWindow::getHeadStatus
 *
 * Método que incicializa el cabezal seleccionado y determina el estado de su fuente de HV (PSOC).
 *
 */
void MainWindow::getHeadStatus(int head_index)
{
    writeFooterAndHeaderDebug(true);

    if(!arpet->isPortOpen())
    {
        QMessageBox::critical(this,tr("Error"),tr("No se puede acceder al puerto serie. Revise la conexión USB."));
        if(debug)
        {
            cout<<"No se puede acceder al puerto serie. Revise la conexión USB."<<endl;
            writeFooterAndHeaderDebug(false);
        }
        return;
    }

    if(debug) cout<<"Cabezal: "<<head_index<<endl;

    /* Inicialización del Cabezal */
    initSP3(head_index);
    initHead(head_index);

    string msg;
    QVector<QString> ans_psoc;
    setPSOCDataStream(QString::number(head_index).toStdString(), arpet->getPSOC_SIZE_RECEIVED(), arpet->getPSOC_STA());
    try
    {
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
        msg = readString();
        ans_psoc = arpet->parserPSOCStream(msg);        
        hv_status_table[head_index-1]->setText(QString::number(round(ans_psoc.at(3).toDouble()*arpet->getPSOC_ADC())));
        if (arpet->verifyMCAEStream(ans_psoc.at(1).toStdString(),"ON"))
            setLabelState(true, hv_status_table[head_index-1]);
        else
            setLabelState(false, hv_status_table[head_index-1]);
    }
    catch(Exceptions & ex)
    {
        if(debug) cout<<"Hubo un inconveniente al intentar acceder al estado de la placa PSOC del cabezal. Revise la conexión. Error: "<<ex.excdesc<<endl;
        setLabelState(false, hv_status_table[head_index-1], true);
    }
    writeFooterAndHeaderDebug(false);

}


/**
 * @brief MainWindow::on_pushButton_triple_ventana_clicked
 */
void MainWindow::on_pushButton_triple_ventana_clicked()
{
    QString fileName = openConfigurationFile();
    if (fileName!="")
      ui->textBrowser_triple_ventana->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_hv_clicked
 */
void MainWindow::on_pushButton_hv_clicked()
{
    QString fileName = openConfigurationFile();
    if (fileName!="")
      ui->textBrowser_hv->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_energia_clicked
 */
void MainWindow::on_pushButton_energia_clicked()
{
    QString fileName = openConfigurationFile();
    if (fileName!="")
      ui->textBrowser_energia->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_posicion_X_clicked
 */
void MainWindow::on_pushButton_posicion_X_clicked()
{
    QString fileName = openConfigurationFile();
    if (fileName!="")
      ui->textBrowser_posicion_X->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_posicion_Y_clicked
 */
void MainWindow::on_pushButton_posicion_Y_clicked()
{
    QString fileName = openConfigurationFile();
    if (fileName!="")
      ui->textBrowser_posicion_Y->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_tiempos_cabezal_clicked
 */
void MainWindow::on_pushButton_tiempos_cabezal_clicked()
{
    QString fileName = openConfigurationFile();
    if (fileName!="")
      ui->textBrowser_tiempos_cabezal->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_obtener_ini_clicked
 */
void MainWindow::on_pushButton_obtener_ini_clicked()
{
    writeFooterAndHeaderDebug(true);
    openConfigurationFile();
    setPreferencesSettingsFile("Paths","conf_set_file",initfile);
    if(debug)
    {
        cout<<"El nuevo archivo de configuración: "<<initfile.toStdString()<<endl;
        writeFooterAndHeaderDebug(false);
    }

}


/**
 * @brief MainWindow::on_pushButton_configure_clicked
 */
void MainWindow::on_pushButton_configure_clicked()
{
    /* Inicialización del modo Coincidencia */

    QString head;
    QString ignore;
    arpet->portDisconnect();
    writeFooterAndHeaderDebug(true);
    int index=ui->comboBox_adquire_mode_coin->currentIndex();
    try
    {
        switch (index)
        {
        case COIN_NORMAL:
            if(debug) cout<<"Modo Coincidencia: Normal"<<endl;
            initCoincidenceMode();
            usleep(5000);
            setCoincidenceModeWindowTime();
            usleep(5000);
            setCoincidenceModeDataStream(arpet->getNormal_Coin_Mode());
            usleep(5000);
            setTimeModeCoin(COIN_NORMAL);
            break;
        case COIN_AUTOCOINCIDENCE:
            if(debug) cout<<"Modo Coincidencia: Autocoincidencia"<<endl;
            initCoincidenceMode();
            usleep(5000);
            setCoincidenceModeWindowTime();
            usleep(5000);
            setCoincidenceModeDataStream(arpet->getAuto_Coin_Mode());
            usleep(5000);
            setTimeModeCoin(COIN_AUTOCOINCIDENCE);
            break;
        case COIN_AVANCED:
            if(debug) cout<<"Modo Coincidencia: Avanzado"<<endl;
            initCoincidenceMode();
            usleep(5000);
            setCoincidenceModeWindowTime();
            usleep(5000);
            setCoincidenceModeDataStream(getCoincidenceAdvanceModeDataStream());
            usleep(5000);
            setTimeModeCoin(COIN_AVANCED);
            break;
        case COIN_CALIB:
            head = ui->comboBox_head_select_calib->currentText();
            if(debug) cout<<"Modo Calibración en el cabezal: "<<head.toStdString()<<endl;
            setCalibrationMode(head);
            usleep(5000);
            setTimeModeCoin(COIN_CALIB, head);
            break;
        case COIN_VERIF:
            head = ui->comboBox_head_select_calib->currentText();
            if(debug) cout<<"Modo Calibración en el cabezal: "<<head.toStdString()<<endl;

            setCalibrationMode(head);
            setTimeModeCoin(COIN_VERIF, head);
            break;
        case COIN_INTER_CABEZAL:
            if(debug) cout<<"Modo Coincidencia: Normal"<<endl;
            initCoincidenceMode();
            usleep(5000);
            setCoincidenceModeWindowTime();
            usleep(5000);
            setCoincidenceModeDataStream(arpet->getNormal_Coin_Mode());
            usleep(5000);
            setTimeModeCoin(COIN_INTER_CABEZAL);
            break;
        default:
            break;
        }
        setLabelState(true,ui->label_coincidencia_estado);
    }
    catch(Exceptions & ex)
    {
        ui->label_data_output->setText("No se pudo realizar la Operación, verifique la comunicación");
        setLabelState(false,ui->label_coincidencia_estado);
    }
    writeFooterAndHeaderDebug(false);
}


void MainWindow::setTimeModeCoin(int modo, QString head){

    arpet->portDisconnect();
    error_code error_code;
    QString ignore;
    QString Cabezal;
    try{
        switch (modo) {
        case COIN_NORMAL:
            for ( int i = 0; i < Estado_Cabezales.length(); i++ )
            {
                port_name=Cab+QString::number(Estado_Cabezales.at(i));
                error_code= arpet->portConnect(port_name.toStdString().c_str());
                if (error_code.value()!=0){
                    arpet->portDisconnect();
                    Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
                    throw exception_Cabezal_Apagado;
                }
                for(int pmt = 0; pmt < PMTs; pmt++){
                    Cabezal=QString::number(Estado_Cabezales.at(i));
                    ignore=setTime(Cabezal.toStdString(), Matrix_coefT_values[Estado_Cabezales.at(i)-1][pmt] + coefTInter_values[Estado_Cabezales.at(i)-1], QString::number(pmt+1).toStdString());
                }
                arpet->portDisconnect();
            }
            break;
        case COIN_AUTOCOINCIDENCE:
            for ( int i = 0; i < Estado_Cabezales.length(); i++ )
            {
                port_name=Cab+QString::number(Estado_Cabezales.at(i));
                error_code= arpet->portConnect(port_name.toStdString().c_str());
                if (error_code.value()!=0){
                    arpet->portDisconnect();
                    Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
                    throw exception_Cabezal_Apagado;
                }
                for(int pmt = 0; pmt < PMTs; pmt++){ ignore=setTime(QString::number(Estado_Cabezales.at(i)).toStdString(), Matrix_coefT_values[Estado_Cabezales.at(i)-1][pmt], QString::number(pmt+1).toStdString());  }
                arpet->portDisconnect();
            }
            break;
        case COIN_AVANCED:
            for ( int i = 0; i < Estado_Cabezales.length(); i++ )
            {
                port_name=Cab+QString::number(Estado_Cabezales.at(i));
                error_code= arpet->portConnect(port_name.toStdString().c_str());
                if (error_code.value()!=0){
                    arpet->portDisconnect();
                    Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
                    throw exception_Cabezal_Apagado;
                }                for(int pmt = 0; pmt < PMTs; pmt++){ ignore=setTime(QString::number(Estado_Cabezales.at(i)).toStdString(), Matrix_coefT_values[Estado_Cabezales.at(i)-1][pmt] + coefTInter_values[Estado_Cabezales.at(i)-1], QString::number(pmt+1).toStdString());  }
                arpet->portDisconnect();
            }
            break;
        case COIN_CALIB:
            port_name=Cab+QString::number(head.toInt());
            error_code= arpet->portConnect(port_name.toStdString().c_str());
            if (error_code.value()!=0){
                arpet->portDisconnect();
                Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
                throw exception_Cabezal_Apagado;
            }            for(int pmt = 0; pmt < PMTs; pmt++){ ignore=setTime(head.toStdString(),TIEMPOS_NULOS_PMTS, QString::number(pmt+1).toStdString());}
            arpet->portDisconnect();
            break;
        case COIN_VERIF:
            port_name=Cab+QString::number(head.toInt());
            error_code= arpet->portConnect(port_name.toStdString().c_str());
            if (error_code.value()!=0){
                arpet->portDisconnect();
                Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
                throw exception_Cabezal_Apagado;
            }
            for(int pmt = 0; pmt < PMTs; pmt++){ ignore=setTime(head.toStdString(), Matrix_coefT_values[head.toInt()-1][pmt] , QString::number(pmt+1).toStdString());  }
            arpet->portDisconnect();
            break;
        case COIN_INTER_CABEZAL:
            for ( int i = 0; i < Estado_Cabezales.length(); i++ )
            {
                port_name=Cab+QString::number(Estado_Cabezales.at(i));
                error_code= arpet->portConnect(port_name.toStdString().c_str());
                if (error_code.value()!=0){
                    arpet->portDisconnect();
                    Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
                    throw exception_Cabezal_Apagado;
                }                for(int pmt = 0; pmt < PMTs; pmt++){ ignore=setTime(QString::number(Estado_Cabezales.at(i)).toStdString(), Matrix_coefT_values[Estado_Cabezales.at(i)-1][pmt], QString::number(pmt+1).toStdString());  }
                arpet->portDisconnect();
            }
            break;

        default:
            break;
        }

    }

    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr(string(ex.excdesc).c_str()));
        setLabelState(false,ui->label_coincidencia_estado);
    }
}

/**
 * @brief MainWindow::on_pushButton_initialize_clicked
 */
void MainWindow::on_pushButton_initialize_clicked()
{
    /** @todo: Verificar la inicialización y configuración de la alta tensión */
    writeFooterAndHeaderDebug(true);
    error_code error_code;
    QList<int> checkedHeads=getCheckedHeads();
    string msg;
    QString psoc_alta;
    QString psoc_alta_Tabla;

    UncheckHeads();

    arpet->portDisconnect();
    for (int i=0;i<checkedHeads.length();i++)
    {
        int head_index=checkedHeads.at(i);

        /* Inicialización del Cabezal */
        try
        {

            port_name=Cab+QString::number(head_index);
            writeFooterAndHeaderDebug(true);
            calibrador->setPort_Name((port_name));
            worker->setPortName((port_name));
            error_code= arpet->portConnect(port_name.toStdString().c_str());
            if (error_code.value()!=0){
                arpet->portDisconnect();
                Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
                throw exception_Cabezal_Apagado;
            }
            if(debug) cout<<"Puerto conectado en: "<<port_name.toStdString()<<endl;
            writeFooterAndHeaderDebug(false);
            //getARPETStatus();

            if (initHead(head_index).length()==0){
                ui->label_data_output->setText("Cabezal "+QString::number(head_index)+ " todavía no iniciado");
                return;
            }
            if(initSP3(head_index).length()==0){
                ui->label_data_output->setText("PMTs no responden");
                return;
            }
            usleep(500);

            parseConfigurationFile(true, QString::number(head_index));
            /* Configuración de la Alta Tensión*/
            //ui->lineEdit_alta->setText(QString::number(AT));
            //ui->lineEdit_limiteinferior->setText(QString::number(LowLimit));
            //LowLimit = ui->lineEdit_limiteinferior->text().toInt();

            QString q_msg = setHV(QString::number(checkedHeads.at(i)).toStdString(),QString::number(LowLimit[head_index-1]).toStdString());
            if(debug)
            {
                cout<<"Reinicio del Cabezal "<<checkedHeads.at(i)<<" en la ventana: "<<QString::number(LowLimit[head_index-1]).toStdString()<<endl;
                showMCAEStreamDebugMode(q_msg.toStdString());
            }

            psoc_alta = ui->lineEdit_alta->text();
            psoc_alta_Tabla = QString::number(AT);

            usleep(500);

            /* Encendido de HV */ /** @note: Responsabilidad de los hermanos macana: Scremin and Arbizu company */
            setPSOCDataStream(QString::number(head_index).toStdString(), arpet->getPSOC_SIZE_RECEIVED_ALL(), arpet->getPSOC_ON());
            if(debug) cout<<"Cabezal: "<<head_index<<endl;

        }
        catch(Exceptions & ex)
        {
            QMessageBox::critical(this,tr("Atención"),tr((string("El Cabezal no está respondiendo. Error: ")+string(ex.excdesc)).c_str()));

            if (debug) cout<<"No se puede prender Alta Tension Error: "<<ex.excdesc<<arpet->getTrama_MCAE()<<arpet->getEnd_PSOC() <<endl;
            setLabelState(false, hv_status_table[head_index-1], true);
        }
        try
        {
          sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
          usleep(500);
          msg = readString();
          setLabelState(arpet->verifyMCAEStream(msg,arpet->getPSOC_ANS()), hv_status_table[head_index-1]);
          if (debug){
              cout<< "------------------------"<<endl;
              cout<< "DEBUG"<<endl;

              cout<<  arpet->getTrama_MCAE()<<endl;
              cout<< "------------------------"<<endl;
          }
          usleep(500);

          if(debug) cout<< "Alta tensión encendida"<<endl;

            ui->label_psoc_estado_datos->setText(QString::fromStdString(msg)); 

            usleep(500);
         
        }
        catch(Exceptions & ex)
        {
            if (debug) cout<<"No se puede prender Alta Tension Error: "<<ex.excdesc<<arpet->getTrama_MCAE()<<arpet->getEnd_PSOC() <<endl;
            setLabelState(false, hv_status_table[head_index-1], true);
        }


        try
        {
            setPSOCDataStream(QString::number(head_index).toStdString(), arpet->getPSOC_SIZE_RECEIVED_ALL(), arpet->getPSOC_SET(),psoc_alta_Tabla);

            if(debug) cout<<"Cabezal: "<<head_index<<endl;

            sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
            msg = readString(CHAR_LF);

            usleep(500);
            hv_status_table[head_index-1]->setText(psoc_alta);
            if(debug) cout<< "Alta tensión configurada en: "<<psoc_alta.toStdString()<<endl;


        }
        catch(Exceptions & ex)
        {
            if (debug) cout<<"No se puede acceder a la placa de alta tensión. Revise la conexión al equipo. Error: "<<ex.excdesc<< arpet->getPSOC_SET()<< psoc_alta.toStdString()<<arpet->getTrama_MCAE()<<arpet->getEnd_PSOC() <<endl;
        }
        /* Configuración de las tablas de calibración */
        setCalibrationTables(head_index);
        //ui->lineEdit_alta->setText(QString::number(AT));
        //ui->lineEdit_limiteinferior->setText(QString::number(LowLimit));
        hv_status_table[head_index-1]->setText(QString::number(AT));

        arpet->portDisconnect();
    }

    //resetHeads();

    writeFooterAndHeaderDebug(false);
}



/**
 * @brief MainWindow::on_pushButton_hv_set_clicked
 */
void MainWindow::on_pushButton_hv_set_clicked()
{
    writeFooterAndHeaderDebug(true);
    //QList<int> checkedHeads = getCheckedHeads();
    error_code error_code;
    string msg;

    try
    {
        setHeadMode(ui->comboBox_head_select_config->currentText().toInt(),"config");
        arpet->portDisconnect();

        port_name=Cab+QString::number(ui->comboBox_head_select_config->currentText().toInt());
        calibrador->setPort_Name((port_name));
        worker->setPortName((port_name));
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }
    //    mMutex.lock();
        QString psoc_alta = (ui->lineEdit_alta->text());
        int head_index=setPSOCDataStream(ui->comboBox_head_select_config->currentText().toStdString(), arpet->getPSOC_SIZE_RECEIVED_ALL(), arpet->getPSOC_SET(),psoc_alta);
      //  mMutex.unlock();
        if(debug) cout<<"Cabezal: "<<head_index<<endl;

        sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
        msg = readString();
        hv_status_table[head_index-1]->setText(psoc_alta);
        //resetPMTs(false);
        //resetPMTs(true);
        if(debug) cout<< "HV configurado en: "<<psoc_alta.toStdString()<<endl;
    }
    catch(Exceptions & ex)
    {
        if (debug) cout<<"No se puede acceder a la placa de alta tensión. Revise la conexión al equipo. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede acceder a la placa de alta tensión. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    ui->label_psoc_estado_datos->setText(QString::fromStdString(msg));
    if (debug)
    {
        showMCAEStreamDebugMode(msg);
        writeFooterAndHeaderDebug(false);
    }
    arpet->portDisconnect();

}
/**
 * @brief MainWindow::on_pushButton_hv_on_clicked
 */
void MainWindow::on_pushButton_hv_on_clicked()
{
    writeFooterAndHeaderDebug(true);
    error_code error_code;
    string msg;

    //QList<int> checkedHeads = getCheckedHeads();


    setHeadMode(ui->comboBox_head_select_config->currentText().toInt(),"config");
    arpet->portDisconnect();

    port_name=Cab+QString::number(ui->comboBox_head_select_config->currentText().toInt());

    try
    {
        calibrador->setPort_Name((port_name));
        worker->setPortName((port_name));
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }
        int head_index=setPSOCDataStream(ui->comboBox_head_select_config->currentText().toStdString(), arpet->getPSOC_SIZE_RECEIVED_ALL(), arpet->getPSOC_ON());
        if(debug) cout<<"Cabezal: "<<head_index<<endl;

        sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
        msg = readString();
        setLabelState(arpet->verifyMCAEStream(msg,arpet->getPSOC_ANS()), hv_status_table[head_index-1]);
        if(debug) cout<< "HV encendido"<<endl;
    }
    catch(Exceptions & ex)
    {
        if (debug) cout<<"No se puede acceder a la placa de alta tensión. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede acceder a la placa de alta tensión. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    ui->label_psoc_estado_datos->setText(QString::fromStdString(msg));
    if (debug)
    {
        showMCAEStreamDebugMode(msg);
        writeFooterAndHeaderDebug(false);
    }
    arpet->portDisconnect();

}
/**
 * @brief MainWindow::on_pushButton_hv_off_clicked
 */
void MainWindow::on_pushButton_hv_off_clicked()
{
    writeFooterAndHeaderDebug(true);
    error_code error_code;
    //QList<int> checkedHeads = getCheckedHeads();
    setHeadMode(ui->comboBox_head_select_config->currentText().toInt(),"config");
    arpet->portDisconnect();
    string msg;

    port_name=Cab+QString::number(ui->comboBox_head_select_config->currentText().toInt());
    try
    {
        calibrador->setPort_Name((port_name));
        worker->setPortName((port_name));
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }
        int head_index=setPSOCDataStream(ui->comboBox_head_select_config->currentText().toStdString(), arpet->getPSOC_SIZE_RECEIVED_ALL(), arpet->getPSOC_OFF());
        if(debug) cout<<"Cabezal: "<<head_index<<endl;

        sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
        msg = readString();
        setLabelState(!arpet->verifyMCAEStream(msg,arpet->getPSOC_ANS()), hv_status_table[head_index-1]);
        if(debug) cout<< "HV apagado"<<endl;
    }
    catch(Exceptions & ex)
    {
        if (debug) cout<<"No se puede acceder a la placa de alta tensión. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede acceder a la placa de alta tensión. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    ui->label_psoc_estado_datos->setText(QString::fromStdString(msg));
    if (debug)
    {
        showMCAEStreamDebugMode(msg);
        writeFooterAndHeaderDebug(false);
    }
    arpet->portDisconnect();

}
/**
 * @brief MainWindow::on_pushButton_hv_estado_clicked
 */
void MainWindow::on_pushButton_hv_estado_clicked()
{
    writeFooterAndHeaderDebug(true);
    string msg;
    error_code error_code;
    setHeadMode(ui->comboBox_head_select_config->currentText().toInt(),"config");

    port_name=Cab+QString::number(ui->comboBox_head_select_config->currentText().toInt());
    try
    {
        calibrador->setPort_Name((port_name));
        worker->setPortName((port_name));
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }
        setPSOCDataStream(ui->comboBox_head_select_config->currentText().toStdString(), arpet->getPSOC_SIZE_RECEIVED(), arpet->getPSOC_STA());
        if(debug) cout<<"Cabezal: "<<getHead("config").toStdString()<<endl;

        sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
        msg = readString();
        if(debug) cout<< "Estado de la placa PSOC: "<<msg<<endl;

    }
    catch(Exceptions & ex)
    {
        if (debug) cout<<"No se puede acceder a la placa de alta tensión. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede acceder a la placa de alta tensión. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    ui->label_psoc_estado_datos->setText(QString::fromStdString(msg));
    if (debug)
    {
        showMCAEStreamDebugMode(msg);
        writeFooterAndHeaderDebug(false);
    }
    arpet->portDisconnect();

}
/**
 * @brief MainWindow::getCoincidenceAdvanceModeDataStream
 *
 * Método que configura la trama avanzada en modo coincidencia a partir de la lectura de QComboBox
 *
 * @return La trama de datos en _string_
 */
string MainWindow::getCoincidenceAdvanceModeDataStream()
{
    int index;
    string stream;
    for(int i = 0; i < COIN_BYTES_ADV; i++)
    {
        index = heads_coin_table[i]->currentIndex();
        stream = stream + to_string(index);
    }

    return stream;
}
/**
 * @brief MainWindow::setCoincidenceModeDataStream
 *
 * Configuración de la trama en modo coincidencia
 *
 * @param stream
 */
void MainWindow::setCoincidenceModeDataStream(string stream)
{
    setMCAEDataStream(arpet->getSelect_Mode_Coin(),stream,"",false);
    string msg_ans;
    error_code error_code;
    arpet->portDisconnect();

    try
    {
        error_code=arpet->portConnect("/dev/UART_Coin");
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está Coincidencia apagado");
            throw exception_Cabezal_Apagado;
        }
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg_ans = readString();
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_set_coincidence_mode_failure((string("No se puede configurar el modo coincidencia en el/los cabezal/es seleccionado/s. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str());
        if(debug)
        {
            cout<<"No se puede configurar el modo coincidencia en el/los cabezal/es seleccionado/s. Revise la conexión al equipo. Error: "<<ex.excdesc<<endl;
            showMCAEStreamDebugMode(msg_ans);
        }
        throw exception_set_coincidence_mode_failure;
    }
    if(debug)
    {
        cout<<"Selección del modo de coincidencia: "<<endl;
        showMCAEStreamDebugMode(msg_ans);
    }
}
/**
 * @brief MainWindow::initCoincidenceMode
 *
 * Método que configura la trama de inicialización en modo coincidencia
 *
 */
void MainWindow::initCoincidenceMode()
{
    arpet->portDisconnect();
    error_code error_code;
    string msg_ans;
    try
    {
    /* Inicializo nuevamente todos los cabezales */
        for ( int i = 0; i < Estado_Cabezales.length(); i++ )
        {
            port_name=Cab+QString::number(Estado_Cabezales.at(i));
            error_code= arpet->portConnect(port_name.toStdString().c_str());
            if (error_code.value()!=0){
                arpet->portDisconnect();
                Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
                throw exception_Cabezal_Apagado;
            }
            initHead(Estado_Cabezales.at(i));
            arpet->portDisconnect();

        }
        port_name="/dev/UART_Coin";
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
            throw exception_Cabezal_Apagado;
        }
        /* Inicialización de modo coincidencia */
        setMCAEDataStream(arpet->getInit_Coin(),"","",false);

        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg_ans = readString();
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_init_coincidence_failure((string("No se pueden inicializar coincidencia en el/los cabezal/es seleccionado/s. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str());
        if(debug)
        {
            cout<<"No se pueden inicializar coincidencia en el/los cabezal/es seleccionado/s. Revise la conexión al equipo. Error: "<<ex.excdesc<<endl;
            showMCAEStreamDebugMode(msg_ans);
        }
        throw exception_init_coincidence_failure;
    }
    if(debug)
    {
        cout<<"Inicialización del modo coincidencia: "<<endl;
        showMCAEStreamDebugMode(msg_ans);
    }
    //arpet->portDisconnect();
}
/**
 * @brief MainWindow::setCalibrationMode
 * @param head
 */
void MainWindow::setCalibrationMode(QString head)
{

    string msg_ans;
    error_code error_code;
    arpet->portDisconnect();
    /* Ahora le avisamos al kit de coincidencia qué cabezal está en modo coincidencia*/
    try
    {

        port_name="/dev/UART_Coin";
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
            throw exception_Cabezal_Apagado;
        }

        setMCAEDataStream(head.toStdString(), true);


        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg_ans = readString();
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_coincidence_kit_failure((string("Imposible conectarse al kit de coincidencia. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str());
        if(debug)
        {
            cout<<"Imposible conectarse al kit de coincidencia. Revise la conexión al equipo. Error: "<<ex.excdesc<<endl;
            showMCAEStreamDebugMode(msg_ans);
        }
        throw exception_coincidence_kit_failure;
    }
    if(debug)
    {
        cout<<"El kit coincidencia ha respondido correctamente. A comenzar adquirir ya!!"<<endl;
        showMCAEStreamDebugMode(msg_ans);
    }





    try
    {

        setMCAEDataStream(head.toStdString());
        cout<<"Configuro el Cabezal"<<endl;
        arpet->portDisconnect();
        port_name=Cab+head;
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
            throw exception_Cabezal_Apagado;
        }
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg_ans = readString();
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_calibration_failure((string("No se puede poner en modo calibración en el cabezal seleccionado. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str());
        if(debug)
        {
            cout<<"No se puede poner en modo calibración en el cabezal seleccionado. Revise la conexión al equipo. Error: "<<ex.excdesc<<endl;
            showMCAEStreamDebugMode(msg_ans);
        }
        throw exception_calibration_failure;
    }
    if(debug)
    {
        cout<<"Seteo modo calibración en el cabezal "<<head.toStdString()<<" fue satisfactoria."<<endl;
        showMCAEStreamDebugMode(msg_ans);
    }


}
/**
 * @brief MainWindow::setCoincidenceModeWindowTime
 *
 * Método que configura la trama de ventana de tiempo en modo coincidencia
 *
 */
void MainWindow::setCoincidenceModeWindowTime()
{
    string vn="-"+ui->lineEdit_WN->text().toStdString();
    string vp=ui->lineEdit_WP->text().toStdString();
    setMCAEDataStream(arpet->getWindow_Time_Coin(),vn,vp,true);
    string msg_ans;
    error_code error_code;
    arpet->portDisconnect();


    try
    {
        port_name="/dev/UART_Coin";
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
            throw exception_Cabezal_Apagado;
        }
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg_ans = readString();
        QString q_label_text="<span style='font-weight:600; color: blue'>"+QString::fromStdString("[" + vn + "," + vp + "]" )+"<br></span>";
        ui->label_window_interval->setText(q_label_text);
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_set_coincidence_window_time_failure((string("No se pueden configura la ventana de tiempo en el/los cabezal/es seleccionado/s. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str());
        if(debug)
        {
            cout<<"No se pueden configura la ventana de tiempo en el/los cabezal/es seleccionado/s. Revise la conexión al equipo. Error: "<<ex.excdesc<<endl;
            showMCAEStreamDebugMode(msg_ans);
        }
        throw exception_set_coincidence_window_time_failure;
    }
    if(debug)
    {
        cout<<"Configuración del intervalo de tiempo de ventana: "<<endl;
        showMCAEStreamDebugMode(msg_ans);
    }
}
/**
 * @brief MainWindow::setHeadModeConfig
 * @param index
 */
void MainWindow::setHeadModeConfig(int index)
{
    setHeadMode(index,"config");
}
/**
 * @brief MainWindow::initHead
 *
 * Inicializa el cabezal seleccionado
 *
 * @param head
 * @return Mensaje de recepción en _string_
 */
string MainWindow::initHead(int head)
{
    /* Incialización del cabezal */

    setMCAEDataStream(QString::number(head).toStdString(), arpet->getFunCHead(), arpet->getBrCst(), arpet->getInit_MCA());
    string msg_head;

    try
    {
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg_head = readString();
    }
    catch(Exceptions & ex)
    {
        /**  @note : Ver time_out interno de planar con Fede y Mati */
        usleep(WAIT_MICROSECONDS);
        if (debug) cout<<"No se puede/n inicializar el/los cabezal/es seleccionado/s. Error: "<<ex.excdesc<<endl;
    }
    if (debug)
    {
        cout<<"Inicialización del Cabezal: "<<endl;
        showMCAEStreamDebugMode(msg_head);
    }
    setLabelState(arpet->verifyMCAEStream(msg_head,arpet->getAnsHeadInit()),head_status_table[head-1]);

    return msg_head;
}
/**
 * @brief MainWindow::initSP3
 *
 * Inicialización de las SPARTANS 3 del cabezal seleccionado
 *
 * @param head
 * @return Mensaje de recepción en _string_
 */
string MainWindow::initSP3(int head)
{
    /* Inicialización de las Spartans 3*/
    setMCAEDataStream(QString::number(head).toStdString(), arpet->getFunCSP3(), arpet->getBrCst(), arpet->getInit_MCA());
    string msg_pmts;
    try
    {
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg_pmts = readString();
    }
    catch(Exceptions & ex)
    {
        /*  @note : Ver time_out interno de planar con Fede y Mati */
        usleep(WAIT_MICROSECONDS);
        if (debug) cout<<"No se pueden inicializar los PMT en el/los cabezal/es seleccionado/s. Error: "<<ex.excdesc<<endl;
    }
    if (debug)
    {
        cout<<"Inicialización de los PMTs: "<<endl;
        showMCAEStreamDebugMode(msg_pmts);
    }
    setLabelState(arpet->verifyMCAEStream(msg_pmts,arpet->getAnsMultiInit()),pmt_status_table[head-1]);

    return msg_pmts;
}
/**
 * @brief MainWindow::setCalibrationTables
 *
 * Configuración de la tabla de calibración
 *
 * @param head
 */
void MainWindow::setCalibrationTables(int head) {

    if (loadCalibrationTables(QString::number(head))){
        return;
    }

    bool x_calib = true, y_calib = true, energy_calib = true, windows_limits = true, set_hv = true, set_time = true, lowlimit = true, set_time_INTER = true;
    QString q_msg;

    mMutex.lock();

    try
    {
        q_msg = setCalibTable(QString::number(head).toStdString(), arpet->getX_Calib_Table(), Matrix_coefx_values[head-1], arpet->getAnsX_Calib_Table());
        if(debug)
        {
            cout<<"Configuración en posición X: "<<endl;
            showMCAEStreamDebugMode(q_msg.toStdString());
        }

    }
    catch( Exceptions & ex )
    {
        x_calib = false;
        if (debug) cout<<"No se pueden configurar las tablas de calibración en Posición X. Error: "<<ex.excdesc<<endl;
    }
    setTextBrowserState(x_calib, ui->textBrowser_posicion_X);

    try
    {
        q_msg = setCalibTable(QString::number(head).toStdString(), arpet->getY_Calib_Table(), Matrix_coefy_values[head-1], arpet->getAnsY_Calib_Table());
        if(debug)
        {
            cout<<"Configuración en posición Y: "<<endl;
            showMCAEStreamDebugMode(q_msg.toStdString());
        }
    }
    catch( Exceptions & ex )
    {
        y_calib = false;
        if (debug) cout<<"No se pueden configurar las tablas de calibración en Posición Y. Error: "<<ex.excdesc<<endl;
    }
    setTextBrowserState(y_calib, ui->textBrowser_posicion_Y);

    try
    {
        q_msg = setCalibTable(QString::number(head).toStdString(), arpet->getEnergy_Calib_Table(), Matrix_coefenerg_values[head-1], arpet->getAnsEnergy_Calib_Table());
        if(debug)
        {
            cout<<"Configuración en energía: "<<endl;
            showMCAEStreamDebugMode(q_msg.toStdString());
        }
    }
    catch( Exceptions & ex )
    {
        energy_calib = false;
        if (debug) cout<<"No se pueden configurar las tablas de calibración en Energía. Error: "<<ex.excdesc<<endl;
    }
    setTextBrowserState(energy_calib, ui->textBrowser_energia);

    try
    {
        q_msg = setCalibTable(QString::number(head).toStdString(), arpet->getWindow_Limits_Table(),Matrix_coefest_values[head-1], arpet->getAnsWindow_Limits_Table());
        if(debug)
        {
            cout<<"Configuración de Triple Ventana: "<<endl;
            showMCAEStreamDebugMode(q_msg.toStdString());
        }
    }
    catch( Exceptions & ex )
    {
        windows_limits = false;
        if (debug) cout<<"No se pueden configurar las tablas de calibración en Ventana de Tiempo. Error: "<<ex.excdesc<<endl;
    }
    setTextBrowserState(windows_limits, ui->textBrowser_triple_ventana);

    mMutex.unlock();

    try
    {
        LoadHVPMT(head);
    }
    catch( Exceptions & ex )
    {
        set_hv = false;
        if (debug) cout<<"No se pueden configurar las tablas de calibración en HV. Error: "<<ex.excdesc<<endl;
    }
    setTextBrowserState(set_hv, ui->textBrowser_hv);

    mMutex.lock();

    try
    {
        for(int pmt = 0; pmt < PMTs; pmt++)
        {
            q_msg = setTime(QString::number(head).toStdString(), Matrix_coefT_values[head-1][pmt]+coefTInter_values[head-1], QString::number(pmt+1).toStdString());
            if(debug)
            {
                cout<<"========================================="<<endl;
                showMCAEStreamDebugMode(q_msg.toStdString());
                cout<<"Valor de tiempo Intracabezal: "<< QString::number(Matrix_coefT_values[head-1][pmt]).toStdString() <<endl;
                cout<<"Valor de tiempo Intercabezal: "<< QString::number(coefTInter_values[head-1]).toStdString() <<endl;

                cout<<"========================================="<<endl;
            }

        }
    }
    catch( Exceptions & ex )
    {
        set_time = false;
        set_time_INTER= false;
        if (debug) cout<<"No se pueden configurar las tablas de calibración en Tiempos en el Cabezal. Error: "<<ex.excdesc<<endl;
    }

    mMutex.unlock();

    setTextBrowserState(set_time_INTER, ui->textBrowser_tiempos_Inter_cabezal);
    setTextBrowserState(set_time, ui->textBrowser_tiempos_cabezal);
    if (debug) cout<<"Final de la configuración de las tablas de calibración "<<endl;
    setLabelState(x_calib && y_calib && energy_calib && windows_limits && set_hv && set_time && set_time_INTER && lowlimit, calib_status_table[head-1]);

}
/**
 * @brief MainWindow::LoadHVPMT
 * @param head
 */
void MainWindow::LoadHVPMT(int head){
    QString q_msg;
    for(int pmt = 0; pmt < PMTs; pmt++)
    {
        QString hv=QString::number(hvtable_values[head-1][pmt]);
        //if(debug) cout<<"hv: "<<hv.toStdString()<<endl;
        q_msg = setHV(QString::number(head).toStdString() ,hv.toStdString(), QString::number(pmt+1).toStdString());
        cout<<"termina de cargar el valor de hv del pmt: "<<pmt<<endl;

        if(debug)
        {
            cout<<"========================================="<<endl;
            cout<<" "<< QString::number(pmt+1).toStdString() <<endl;
            showMCAEStreamDebugMode(q_msg.toStdString());
            cout<<"Valor de HV: "<< hv.toStdString() <<endl;
            cout<<"========================================="<<endl;

        }
    }
    return;
}




/* Pestaña: "MCA" */
/**
 * @brief MainWindow::getTemperatureCode
 *
 * Configura el rango de temperatura en función del valor de temperatura
 *
 * @param temperature
 * @return Valor de temp_code
 */
MainWindow::temp_code MainWindow::getTemperatureCode(double temperature)
{
    if (temperature<20) return ERROR;
    if (temperature>=20 && temperature<49) return NORMAL;
    if (temperature>=49 && temperature<56) return WARM;
    if (temperature>=56 && temperature<60) return HOT;
    if (temperature>=60) return TOO_HOT;
    else return NO_VALUE;
}
/**
 * @brief MainWindow::setTemperatureBoard
 *
 * Configura el valor de temperatura para el tablero
 *
 * @param temp
 * @param label_pmt
 * @param pmt
 */
void MainWindow::setTemperatureBoard(double temp, QLabel *label_pmt, int pmt)
{
    QPalette palette_temperature;
    palette_temperature.setColor(QPalette::Background,Qt::black);

    switch (getTemperatureCode(temp)) {
    case ERROR:
       // palette_temperature.setColor(QPalette::Background,Qt::lightGray);
        label_pmt->setStyleSheet("QLabel { background-color : black; }");
        break;
    case NORMAL:
       // palette_temperature.setColor(QPalette::Background,Qt::green);
        label_pmt->setStyleSheet("QLabel { background-color : green; }");
        break;
    case WARM:
        //palette_temperature.setColor(QPalette::Background,Qt::yellow);
        label_pmt->setStyleSheet("QLabel { background-color : yellow; }");
        break;
    case HOT:
        //palette_temperature.setColor(QPalette::Background,QColor::fromRgb(255,140,0)); // Naranja en RGB = 255,140,0
        label_pmt->setStyleSheet("QLabel { background-color : orange; }");
        break;
    case TOO_HOT:
        //palette_temperature.setColor(QPalette::Background,Qt::red);
        label_pmt->setStyleSheet("QLabel { background-color : red;  }");
        break;
    default:
        break;
    }
    QString label_text="<span style='font-weight:600;'>"+QString::number(pmt)+"<br></span><span style='font-size:18pt; font-weight:600;'>"+QString::number(round(temp))+"</span>";
    label_pmt->setPalette(palette_temperature);
    label_pmt->setText(label_text);
}
/**
 * @brief MainWindow::drawTemperatureBoard
 *
 * Dibuja el tablero con las diferentes temperaturas
 *
 */
void MainWindow::drawTemperatureBoard()
{
    double temp;
    QVector<double> temp_vec;
    temp_vec.fill(0);

    mMutex.lock();

    //QList<int> checkedHeads= getCheckedHeads();
    arpet->portDisconnect();
    try
    {
        //QString Cabezal = QString::number(checkedHeads.at(0));
        port_name=Cab+getHead("mca");
        arpet->portConnect(port_name.toStdString().c_str());
        if(debug) cout<<"Conecto puerto sp3"<<endl;

        arpet->InitSP3(getHead("mca").toStdString(),port_name.toStdString());
        //sleep(500);
        if(debug) cout<<"Pasa init mca sp3: "<<endl;


       // setButtonAdquireState(true);
        for(int pmt = 0; pmt < PMTs; pmt++)
        {
            {
                string msg = arpet->getTemp(getHead("mca").toStdString(), QString::number(pmt+1).toStdString(), port_name.toStdString());
                temp = arpet->getPMTTemperature(msg);
                if (temp > MIN_TEMPERATURE)temp_vec.push_back(temp);
                if(debug)
                {
                    cout<<"================================"<<endl;
                    cout<<"PMT: "<< QString::number(pmt+1).toStdString() <<endl;
                    showMCAEStreamDebugMode(msg);
                    cout<<"Valor de temperatura: "<<temp<<"°C"<<endl;
                    cout<<"================================"<<endl;
                }
                setTemperatureBoard(temp,pmt_label_table[pmt],pmt+1);
            }
        }
        arpet->portDisconnect();
    }
    catch( Exceptions & ex )
    {
        //setButtonAdquireState(false);
        if(debug) cout<<"Imposible obtener los valores de temperatura. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("Imposible obtener los valores de temperatura. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
        arpet->portDisconnect();
    }

    //setButtonAdquireState(true, true);
    double mean = std::accumulate(temp_vec.begin(), temp_vec.end(), .0) / temp_vec.size();
    double t_max = *max_element(temp_vec.begin(),temp_vec.end());
    double t_min = *min_element(temp_vec.begin(),temp_vec.end());

    if(debug)
    {
        cout<<"================================"<<endl;
        cout<<"Temperaturas"<<endl;
        cout<<"Media: "<< mean <<"°C"<<endl;
        cout<<"Máxima: "<<t_max<<"°C"<<endl;
        cout<<"Mínima: "<<t_min<<"°C"<<endl;
        cout<<"================================"<<endl;
    }

    mMutex.unlock();

    ui->label_title_output->setText("Temperatura");
    ui->label_data_output->setText("| Media: "+QString::number(mean)+"°C"+" | Máxima: "+QString::number(t_max)+"°C"+" | Mínima: "+QString::number(t_min)+"°C |");
}
/**
 * @brief MainWindow::clearTemperatureBoard
 *
 * Limpia el tablero de temperaturas
 *
 */
void MainWindow::clearTemperatureBoard()
{
    QPalette palette_temperature;
    palette_temperature.setColor(QPalette::Background,Qt::transparent);
    for(int pmt = 0; pmt < PMTs; pmt++)
    {
        pmt_label_table[pmt]->setPalette(palette_temperature);
        pmt_label_table[pmt]->setText(QString::number(pmt+1));
    }
}
/**
 * @brief MainWindow::setTabHead
 * @param index
 */
void MainWindow::setTabHead(int index)
{
    if(index==MULTIHEAD || index==ALLHEADS)
    {
        ui->tabWidget_mca->setCurrentWidget(ui->tab_esp_2);
    }
    else
    {
        ui->tabWidget_mca->setCurrentWidget(ui->tab_esp_1);
    }
}
/**
 * @brief MainWindow::setHeadModeGraph
 * @param index
 */
void MainWindow::setHeadModeGraph(int index)
{
    setHeadMode(index,"mca");
}
/**
 * @brief MainWindow::setAdquireMode
 * @param index
 */
void MainWindow::setAdquireMode(int index)
{
    adquire_mode=index;
    switch (adquire_mode) {
    case PMT:
        ui->frame_PMT->show();
        ui->frame_HV->show();
        ui->frame_MCA->show();
        ui->tabWidget_mca->setCurrentWidget(ui->tab_esp_1);
        break;
    case CABEZAL:
        ui->frame_PMT->hide();
        ui->frame_HV->hide();
        ui->frame_MCA->show();
        ui->tabWidget_mca->setCurrentWidget(ui->tab_esp_2);
        setHeadCustomPlotEnvironment();
        break;
    case TEMPERATURE:
        ui->frame_PMT->hide();
        ui->frame_HV->hide();
        ui->tabWidget_mca->setCurrentWidget(ui->tab_esp_3);
        ui->frame_MCA->hide();
    default:
        break;
    }
}
/**
 * @brief MainWindow::setAdvanceCoinMode
 * @param index
 */
void MainWindow::setAdvanceCoinMode(int index)
{
    switch (index) {
    case COIN_NORMAL:
        ui->frame_adquire_advance_mode->hide();
        ui->frame_window_coin->show();
        break;
    case COIN_AUTOCOINCIDENCE:
        ui->frame_adquire_advance_mode->hide();
        ui->frame_window_coin->show();
        break;
    case COIN_AVANCED:
        ui->frame_adquire_advance_mode->show();
        ui->frame_window_coin->show();
        break;
    case COIN_CALIB:
        ui->frame_adquire_advance_mode->hide();
        ui->frame_window_coin->hide();
        break;
    case COIN_VERIF:
        ui->frame_adquire_advance_mode->hide();
        ui->frame_window_coin->hide();
        break;
    case COIN_INTER_CABEZAL:
        ui->frame_adquire_advance_mode->hide();
        ui->frame_window_coin->show();
        break;
    default:
        break;
    }
}
/**
 * @brief MainWindow::setTabMode
 * @param index
 */
void MainWindow::setTabMode(int index)
{
    adquire_mode=index;
    ui->comboBox_adquire_mode->setCurrentIndex(adquire_mode);
}
/**
 * @brief MainWindow::getHeadMCA
 *
 * Obtiene las cuentas de MCA para un cabezal determinado
 *
 * @param head
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::getHeadMCA(QString head)
{
    QString msg;
    try
    {
        //setButtonAdquireState(true);
        msg = getMCA(head.toStdString(), arpet->getFunCHead(), true, CHANNELS);
        addGraph(arpet->getHitsMCA(),ui->specHead,CHANNELS, head, qcp_head_parameters[head.toInt()-1]);
    }
    catch(Exceptions & ex)
    {
        //setButtonAdquireState(false);
        if(debug) {
            cout<<"No se pueden obtener los valores de MCA del Cabezal. Error: "<<ex.excdesc<<endl;
           // cout<<"PMT: "<<pmt<<" "<<"Saturados "<< pmt<<": " << QString::number(arpet->getHitsMCA()[255]).toStdString()<<endl;
        }
        QMessageBox::critical(this,tr("Atención"),tr((string("No se pueden obtener los valores de MCA. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
        Exceptions exception_timeout(ex.excdesc);
        throw exception_timeout;
    }
    //setButtonAdquireState(true, true);

    return msg;
}
/**
 * @brief MainWindow::getMultiMCA
 *
 * Obtiene las cuentas de MCA correspondiente a los fotomultiplicadores en un cabezal determinado
 *
 * @param head
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::getMultiMCA(QString head)
{
    int size_pmt_selected = pmt_selected_list.length();
    double SaturatedChannel;
    QString msg;

    if (pmt_selected_list.isEmpty())
    {
        if(debug) cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
        QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
        return msg;
    }
    ui->specPMTs->clearGraphs();

    try
    {
        //setButtonAdquireState(true);
        for (int index=0;index<size_pmt_selected;index++)
        {
            string pmt = pmt_selected_list.at(index).toStdString();
            msg = getMCA(head.toStdString(), arpet->getFunCSP3(), false, CHANNELS_PMT, pmt);
            SaturatedChannel =arpet->getHitsMCA()[255];

            if(debug)
            {
                cout<<"PMT: "<<pmt<<" "<<"Saturados "<< pmt<<": " << QString::number(arpet->getHitsMCA()[255]).toStdString()<<endl;
                showMCAEStreamDebugMode(msg.toStdString());
            }

            addGraph(arpet->getHitsMCA(),ui->specPMTs,CHANNELS_PMT, QString::fromStdString(pmt), qcp_pmt_parameters[index]);
        }
        if(debug) cout<<"Se obtuvieron las cuentas MCA de los PMTs seleccionados de forma satisfactoria."<<endl;
    }
    catch(Exceptions & ex)
    {
        //setButtonAdquireState(false);
        if(debug) cout<<"No se pueden obtener los valores de MCA de los PMTs seleccionados. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se pueden obtener los valores de MCA. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }
    //setButtonAdquireState(true, true);

    return msg;
}
/**
 * @brief MainWindow::getMCA
 *
 * Obtiene las cuentas de MCA
 *
 * @param head
 * @param function
 * @param multimode
 * @param channels
 * @param pmt
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::getMCA(string head, string function, bool multimode, int channels, string pmt)
{
    string msg;

    mMutex.lock();

    try
    {
        msg= arpet->getMCA(pmt, function, head, channels, port_name.toStdString());
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_mca(ex.excdesc);
        mMutex.unlock();
        throw exception_mca;
    }

    long long time_mca;
    int frame, HV_pmt, offset, var;

    frame=arpet->getFrameMCA();
    time_mca=arpet->getTimeMCA();
    HV_pmt=arpet->getHVMCA();
    offset=arpet->getOffSetMCA();
    var=arpet->getVarMCA();

    if (multimode)
    {
        QString centroid_mode = ui->checkBox_centroid->isChecked() ? "Si" : "No";
        vector<int> rates = arpet->getRate(head, port_name.toStdString());
        if (debug) cout<<"Tasas: "<<rates.at(0)<<","<<rates.at(1)<<","<<rates.at(2)<<" | "<<arpet->getTrama_MCAE()<<endl;
        ui->label_title_output->setText("MCA Extended | PMT: " + QString::fromStdString(pmt));
        ui->label_data_output->setText("| Tasas: " + QString::number(rates.at(0)) + "," + QString::number(rates.at(1)) + "," + QString::number(rates.at(2)) + "| HV: "+QString::number(HV_pmt)+" | Varianza: "+QString::number(var)+" | Offset ADC: "+QString::number(offset)+" | Tiempo (mseg):"+QString::number(time_mca/1000) + " | Modo Centroide: " + centroid_mode + " |" );
    }
    else
    {
        ui->label_title_output->setText("MCA Extended");
        ui->label_data_output->setText("");
    }

    if(debug && !multimode)
    {
        cout<<"================================"<<endl;
        cout<<"Datos extraídos por MCA en el PMT: "<< pmt <<endl;
        cout<<"Frame: "<< frame <<endl;
        cout<<"Varianza (unidades cuadráticas de ADC): "<< var <<endl;
        cout<<"Offset (unidades de ADC): "<< offset <<endl;
        cout<<"Tiempo de adquisición (medido en milisegundos): "<< time_mca/1000 <<endl;
        cout<<"Valor de HV: "<<HV_pmt<<endl;
        showMCAEStreamDebugMode(msg);
        cout<<"================================"<<endl;
    }

    mMutex.unlock();

    return QString::fromStdString(msg);
}
/**
 * @brief MainWindow::setCalibTable
 *
 * Configura la tabla de calibración especificada
 *
 * @param head
 * @param function
 * @param table
 * @param msg_compare
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::setCalibTable(string head, string function, QVector<double> table, string msg_compare)
{
    string msg;
    try
    {
        msg = arpet->setCalibTable( head, function, table, port_name.toStdString());
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_calib(ex.excdesc);
        throw exception_calib;
    }

    if(!arpet->verifyMCAEStream(msg, msg_compare))
    {
        Exceptions exception_calib(string("Función: "+function+" Respuesta: "+msg).c_str());
        throw exception_calib;
    }

    return QString::fromStdString(msg);
}
/**
 * @brief MainWindow::setTime
 *
 * Configura el tiempo relativo para cada PMT (obtenido en la calibración)
 *
 * @param head
 * @param time_value
 * @param pmt
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::setTime(string head, double time_value, string pmt)
{
    string msg;
    //mMutex.lock();

    try
    {
        msg = arpet->setTime(head, time_value, pmt, port_name.toStdString());
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_time(ex.excdesc);
       //mMutex.unlock();

        throw exception_time;
    }
    //mMutex.unlock();

    return QString::fromStdString(msg);
}
/**
 * @brief MainWindow::setHV
 *
 * Configuración de HV para un PMT determinado
 *
 * @param head
 * @param hv_value
 * @param pmt
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::setHV(string head, string hv_value, string pmt)
{
    string msg;

    mMutex.lock();

    try
    {
       // if (debug) cout<<"Envio: "<<endl<<"Cabezal: "<<head<<" - "<<"PMT: "<<pmt<<endl<<"HV: "<<hv_value<<endl<<port_name.toStdString();
        msg = arpet->setHV(head, pmt, hv_value, port_name.toStdString());
    }
    catch(Exceptions & ex)
    {
        cout<<"No se pudo hacer el SetHV"<<endl;
        Exceptions exception_hv(ex.excdesc);
        mMutex.unlock();
        throw exception_hv;
    }
//    arpet->resetHitsMCA();
//    setHitsInit(true);

    mMutex.unlock();

    return QString::fromStdString(msg);
}
/**
 * @brief MainWindow::setHV
 *
 * Configuración del umbral para un cabezal determinado
 *
 * @param head
 * @param hv_value
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::setHV(string head, string hv_value)
{
    string msg;

    mMutex.lock();

    try
    {
        msg = arpet->setHV(head, hv_value, port_name.toStdString());
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_hv(ex.excdesc);
        mMutex.unlock();
        throw exception_hv;
    }
    arpet->resetHitsMCA();
    setHitsInit(true);

    mMutex.unlock();

    return QString::fromStdString(msg);
}
/**
 * @brief MainWindow::getPMT
 * @param line_edit
 * @return Valor de PMT
 */
int MainWindow::getPMT(QLineEdit *line_edit)
{
    QString pmt=line_edit->text();
    switch (adquire_mode) {
    case PMT:
        if(pmt.isEmpty() || pmt.toInt()==0)
        {
            pmt=QString::number(1);
            line_edit->setText(pmt);
        }
        break;
    case CABEZAL:
        line_edit->setText(0);
        break;
    default:
        break;
    }

    return line_edit->text().toInt();
}
/**
 * @brief MainWindow::getPSOCAlta
 * @param line_edit
 * @return Valor de Tensión en _QString_
 */
QString MainWindow::getPSOCAlta(QLineEdit *line_edit)
{
    QString psoc_value=line_edit->text();
    if(psoc_value.isEmpty() || psoc_value.toInt()<MIN_HIGH_HV_VOLTAGE)
    {
        psoc_value=QString::number(MIN_HIGH_HV_VOLTAGE);
        line_edit->setText(psoc_value);
    }

    return line_edit->text();
}
/**
 * @brief MainWindow::getHVValue
 * @param line_edit
 * @param value
 * @return
 */
string MainWindow::getHVValue(QLineEdit *line_edit, int value)
{
    int hv_value_int;
    if (line_edit->text().isEmpty()) hv_value_int=0;
    if (value==0) hv_value_int=line_edit->text().toInt();
    else  hv_value_int=line_edit->text().toInt() + value;
    if (hv_value_int<0) hv_value_int=0;
    line_edit->setText(QString::number(hv_value_int));

    return QString::number(hv_value_int).toStdString();
}
/**
 * @brief MainWindow::setMCAEDataStream
 *
 * Configuración de la trama MCAE
 *
 * Se configura la trama general de MCAE para el envío de MCA. Este método recibe como parámetros el 'head'
 * siendo el cabezal, la 'function' de MCAE (si es para planar o SP3), el valor de 'pmt', la función MCA ('mca_function'),
 * el tamaño de la trama de recepción 'bytes_mca' (opcional) y en el caso que se realice la confifuración de HV se debe
 * incorporar el valor de HV, caso contrario dejar este campo en blanco.
 *
 * @param head
 * @param function
 * @param pmt
 * @param mca_function
 * @param bytes_mca (opcional)
 * @param hv_value (opcional)
 */
void MainWindow::setMCAEDataStream(string head, string function, string pmt, string mca_function, int bytes_mca, string hv_value)
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + head + function);
    arpet->setMCAEStream(pmt, bytes_mca, mca_function, hv_value);
}
/**
 * @brief MainWindow::setMCAEDataStream
 * @overload
 *
 * Configuración de la trama MCAE
 *
 * Se configura la trama general de MCAE para la configuración de calibración de Tiempos en el Cabezal. Este método
 * recibe como parámetros el 'tab' del entorno gráfico, la 'function' de MCAE (si es para planar o SP3), el valor de 'pmt',
 * la función MCA ('mca_function') y el tiempo en double. Este método se utiliza para la configuración de las tablas
 * de Tiempos en el Cabezal.
 *
 * @param tab
 * @param function
 * @param pmt
 * @param mca_function
 * @param time
 */
void MainWindow::setMCAEDataStream(string tab, string function, string pmt, string mca_function, double time)
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead(tab).toStdString() + function);
    arpet->setMCAEStream(pmt, mca_function, time);
}
/**
 * @brief MainWindow::setMCAEDataStream
 * @overload
 *
 * Configuración de la trama MCAE
 *
 * Se configura la trama general de MCAE para la configuración de las tablas de calibración. Este método recibe como
 * parámetros el 'tab' del entorno gráfico, la 'calib_function' correspondiente a la función de calibración y 'table'
 * que corresponde a la tabla con los valores de calibración correspondiente.
 *
 * @param tab
 * @param calib_function
 * @param table
 */
void MainWindow::setMCAEDataStream(string tab, string calib_function, QVector<double> table )
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead(tab).toStdString() + arpet->getFunCHead());
    arpet->setMCAEStream(calib_function, table);
}
/**
 * @brief MainWindow::setMCAEDataStream
 * @overload
 *
 * Configuración de la trama MCAE
 *
 * Se configura la trama general de MCAE para la configuración de los modos de autocoincidencia. Este método recibe
 * como parámetros la función de coincidencia ('coin_function'), las tramas 'data_one' y 'data_two', y el valor
 * booleano 'time'. Cuando se define la ventana temporal (_subclocks_) se utilizan las tramas 'data_one' y 'data_two'
 * (como el valor de ventana inferior y superior respectivamente), y la variable booleana 'time' se configura en _true_.
 * Para los otros modos solo se configura la trama 'data_one', la trama 'data_two' queda en blanco y la variable booleana
 * 'time' se configura en _false_.
 *
 * @param coin_function
 * @param data_one
 * @param data_two
 * @param time
 */
void MainWindow::setMCAEDataStream(string coin_function, string data_one, string data_two, bool time)
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + arpet->getHead_Coin() + arpet->getFunCHead());
    arpet->setMCAEStream(coin_function, data_one, data_two, time);
}
/**
 * @brief MainWindow::setMCAEDataStream
 *
 * Se configura la trama general de MCAE para la configuración del modo de calibración. Solo es necesario
 * pasarle el cabezal a calibrar en _string_. Además recibe una variable _booleana_ indicando si se le habla
 * al kit de coincidencias o al cabezal _head_.
 *
 * @param head
 * @param coin
 */
void MainWindow::setMCAEDataStream(string head, bool coin)
{
    error_code error_code;
    arpet->portDisconnect();
    try{
        if (!coin)
        {
            port_name=Cab+QString::fromStdString(head);
            cout<<port_name.toStdString()<<endl;

            error_code= arpet->portConnect(port_name.toStdString().c_str());
            if (error_code.value()!=0){
                arpet->portDisconnect();
                Exceptions exception_Cabezal_Apagado("Está el cabezal apagado! Revise las conexiones");
                throw exception_Cabezal_Apagado;
            }
            //cout<<port_name.toStdString()<<endl;
            arpet->setHeader_MCAE(arpet->getHead_MCAE() + head + arpet->getFunCHead());
            arpet->setMCAEStream("0",0,arpet->getCalib_Mode());
        }
        else
        {
            port_name="/dev/UART_Coin";
            error_code= arpet->portConnect(port_name.toStdString().c_str());
            if (error_code.value()!=0){
                arpet->portDisconnect();
                Exceptions exception_Cabezal_Apagado("Está coincidencia apagado! Revise las conexiones");
                throw exception_Cabezal_Apagado;
            }
            arpet->setHeader_MCAE(arpet->getHead_MCAE() + arpet->getHead_Coin() + arpet->getFunCHead());
            arpet->setMCAEStream("0",0,arpet->getInit_Calib_MCAE()+head+"1");

        }
    }
    catch (Exceptions ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr((string("Imposible reiniciar el/los cabezal/es. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
        arpet->portDisconnect();

    }
}
/**
 * @brief MainWindow::setPSOCDataStream
 *
 * Configuración de la trama MCAE para la placa PSOC
 *
 * @param head
 * @param function
 * @param psoc_value
 * @return
 */
int MainWindow::setPSOCDataStream(string head, string size_received, string function, QString psoc_value)
{
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + head + arpet->getFunCPSOC());
    arpet->setPSOCEStream(function, size_received, psoc_value.toStdString());

    return QString::fromStdString(head).toInt();
}
/**
 * @brief MainWindow::resetHitsValues
 */
void MainWindow::resetHitsValues(QString head)
{
    QString Cabezal = Cab +head;
    arpet->resetHitsMCA();
    setHitsInit(true);
    //asdf
    if (debug) cout<<"Reinicio los graficos"<<endl;
    arpet->portDisconnect();
    arpet->portConnect(Cabezal.toStdString().c_str());
    LoadHVPMT(head.toInt());
    arpet->portDisconnect();

}
/**
 * @brief MainWindow::resetHeads
 */
bool MainWindow::resetHeads()
{
    QList<int> checkedHeads = getCheckedHeads();
    bool status = true;

    for (int i=0;i < Estado_Cabezales.length();i++)
    {
        arpet->portDisconnect();
        port_name=Cab+QString::number(Estado_Cabezales.at(i));
        arpet->portConnect(port_name.toStdString().c_str());
        parseConfigurationFile(true, QString::number(Estado_Cabezales.at(i)));

        try
        {
            QString q_msg = setHV(QString::number(Estado_Cabezales.at(i)).toStdString(),QString::number(LowLimit[Estado_Cabezales.at(i)-1]).toStdString());
            if(debug)
            {
                cout<<"Reinicio del Cabezal "<<Estado_Cabezales.at(i)<<" en la ventana: "<<QString::number(LowLimit[Estado_Cabezales.at(i)-1]).toStdString()<<endl;
                showMCAEStreamDebugMode(q_msg.toStdString());
            }
        }
        catch (Exceptions ex)
        {
            if(debug) cout<<"No se puede reiniciar el cabezal "<<Estado_Cabezales.at(i)<<". Error: "<<ex.excdesc<<endl;
            status = false;
            QMessageBox::critical(this,tr("Atención"),tr((string("Imposible reiniciar el/los cabezal/es. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
            arpet->portDisconnect();

        }
        arpet->portDisconnect();

    }
    return status;
}


bool MainWindow::resetHead(QString Cabezal)
{
    bool status = true;

    //for (int i=0;i < checkedHeads.length();i++)
    {
        arpet->portDisconnect();
        port_name=Cab+Cabezal;
        arpet->portConnect(port_name.toStdString().c_str());
        parseConfigurationFile(true, Cabezal);

        try
        {
            QString q_msg = setHV(Cabezal.toStdString(),QString::number(LowLimit[Cabezal.toInt()-1]).toStdString());
            if(debug)
            {
                cout<<"Reinicio del Cabezal "<<Cabezal.toStdString()<<" en la ventana: "<<QString::number(LowLimit[Cabezal.toInt()-1]).toStdString()<<endl;
                showMCAEStreamDebugMode(q_msg.toStdString());
            }
        }
        catch (Exceptions ex)
        {
            if(debug) cout<<"No se puede reiniciar el cabezal "<<Cabezal.toStdString()<<". Error: "<<ex.excdesc<<endl;
            status = false;
            QMessageBox::critical(this,tr("Atención"),tr((string("Imposible reiniciar el/los cabezal/es. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
            arpet->portDisconnect();

        }
        arpet->portDisconnect();

    }
    return status;
}
/**
 * @brief MainWindow::resetPMTs
 * @return
 */
bool MainWindow::resetPMTs(bool centroide)
{
    QList<int> checkedHeads = getCheckedHeads();
    bool status = true;


        parseConfigurationFile(true, QString::number(checkedHeads.at(0)));

        try
        {
            /*if (centroide)*/ setHV(QString::number(checkedHeads.at(0)).toStdString(), ui->lineEdit_limiteinferior->text().toStdString());
        }
        catch( Exceptions & ex )
        {
            status = false;
            if (debug) cout<<"No se pueden configurar las tablas de calibración en HV. Error: "<<ex.excdesc<<endl;
        }



    return status;
}

/**
 * @brief MainWindow::on_pushButton_reset_clicked
 */
void MainWindow::on_pushButton_reset_clicked()
{
    /** @todo Verificar el reinicio de datos en los vectores de cuentas de MCA. Reiniciar con la función '67' */
    writeFooterAndHeaderDebug(true);
    if(debug) cout<<"Cabezal: "<<getHead("mca").toStdString()<<endl;

    switch (adquire_mode) {
    case PMT:
        if(debug) cout<<"Se reiniciaron los valores de los PMTs"<<endl;
        resetHead(getHead("mca"));
        resetHitsValues(getHead("mca"));
        setPMTCustomPlotEnvironment(pmt_selected_list);
        resetPMTs(ui->checkBox_centroid->isChecked());
        removeAllGraphsPMT();
        break;
    case CABEZAL:
        if(debug) cout<<"Se reiniciaron los valores del cabezal"<<endl;
        resetHeads();
        arpet->resetHitsMCA();
        setHitsInit(true);
        setHeadCustomPlotEnvironment();
        removeAllGraphsHead();
        break;
    case TEMPERATURE:
        if(debug) cout<<"Se reiniciaron los valores de temperatura"<<endl;
        clearTemperatureBoard();
        break;
    default:
        break;
    }
    writeFooterAndHeaderDebug(false);
}
/**
 * @brief MainWindow::on_pushButton_select_pmt_clicked
 */
void MainWindow::on_pushButton_select_pmt_clicked()
{

    writeFooterAndHeaderDebug(true);
    arpet->resetHitsMCA();
    setHitsInit(true);
    removeAllGraphsPMT();

    int ret = pmt_select->exec();

    QList<QString> qlist = pmt_select->GetPMTSelectedList();

    if(ret == QDialog::Accepted)
    {
        setPMTSelectedList(qlist);
    }

    if(debug)
    {
        cout<<"La lista seleccionada tiene "<< qlist.size()<<" elementos"<<endl;
        QList<QString>::const_iterator stlIter;
        for( stlIter = qlist.begin(); stlIter != qlist.end(); ++stlIter )
            cout<<(*stlIter).toStdString()<<endl;
    }

    setPMTCustomPlotEnvironment(qlist);

    qSort(qlist);
    ui->listWidget->clear();
    ui->listWidget->addItems(qlist);
    writeFooterAndHeaderDebug(false);
}
/**
 * @brief MainWindow::on_pushButton_hv_configure_clicked
 */
void MainWindow::on_pushButton_hv_configure_clicked()
{
    error_code error_code;
    arpet->portDisconnect();

    writeFooterAndHeaderDebug(true);
    port_name=Cab+ ui->comboBox_head_select_graph->currentText();

    if (pmt_selected_list.isEmpty())
    {
        QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
        if(debug)
        {
            cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
            writeFooterAndHeaderDebug(false);
        }
        return;
    }

    QString q_msg;
    try
    {

        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }

        if (ui->checkBox_centroid->isChecked()) setHV(getHead("mca").toStdString(), ui->lineEdit_limiteinferior->text().toStdString());
        q_msg =setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value),pmt_selected_list.at(0).toStdString());
        if(debug) cout<<ui->lineEdit_hv_value->text().toStdString()<<endl;
        ui->label_data_output->setText("| Canal configurado: " + QString::fromStdString(getHVValue(ui->lineEdit_hv_value))+" | Configuración OK.");
    }
    catch (Exceptions ex)
    {
        if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
        ui->label_data_output->setText("Error en la configuración de la tensión de dinodo.");
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }
    ui->label_title_output->setText("HV de Dinodo | PMT: " + pmt_selected_list.at(0));
    if (debug)
    {
        showMCAEStreamDebugMode(q_msg.toStdString());
        writeFooterAndHeaderDebug(false);
    }
}
/**
 * @brief MainWindow::on_pushButton_l_5_clicked
 */
void MainWindow::on_pushButton_l_5_clicked()
{
    error_code error_code;
    port_name = Cab+ui->comboBox_head_select_graph->currentText();
    arpet->portDisconnect();

    writeFooterAndHeaderDebug(true);

    if (pmt_selected_list.isEmpty())
    {
        QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
        if(debug)
        {
            cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
            writeFooterAndHeaderDebug(false);
        }
        return;
    }

    QString q_msg;
    try
    {
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }
        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        ui->lineEdit_hv_value->setText(QString::number(arpet->getHVMCA()));
        if (ui->checkBox_centroid->isChecked()) setHV(getHead("mca").toStdString(), ui->lineEdit_limiteinferior->text().toStdString());
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,-5),pmt_selected_list.at(0).toStdString());
        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        if(debug) cout<<"El nuevo canal configurado en el PMT "<<pmt_selected_list.at(0).toStdString()<<" es: "<<ui->lineEdit_hv_value->text().toStdString()<<endl;
    }
    catch (Exceptions ex)
    {
        if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
        showMCAEStreamDebugMode(q_msg.toStdString());
        writeFooterAndHeaderDebug(false);
    }
}
/**
 * @brief MainWindow::on_pushButton_l_10_clicked
 */
void MainWindow::on_pushButton_l_10_clicked()
{
    error_code error_code;
    port_name = Cab+ui->comboBox_head_select_graph->currentText();
    arpet->portDisconnect();

    writeFooterAndHeaderDebug(true);

    if (pmt_selected_list.isEmpty())
    {
        QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
        if(debug)
        {
            cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
            writeFooterAndHeaderDebug(false);
        }
        return;
    }

    QString q_msg;
    try
    {
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }

        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        ui->lineEdit_hv_value->setText(QString::number(arpet->getHVMCA()));
        if (ui->checkBox_centroid->isChecked()) setHV(getHead("mca").toStdString(), ui->lineEdit_limiteinferior->text().toStdString());
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,-10),pmt_selected_list.at(0).toStdString());
        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        if(debug) cout<<"El nuevo canal configurado en el PMT "<<pmt_selected_list.at(0).toStdString()<<" es: "<<ui->lineEdit_hv_value->text().toStdString()<<endl;
    }
    catch (Exceptions ex)
    {
        if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
        showMCAEStreamDebugMode(q_msg.toStdString());
        writeFooterAndHeaderDebug(false);
    }
}
/**
 * @brief MainWindow::on_pushButton_l_50_clicked
 */
void MainWindow::on_pushButton_l_50_clicked()
{
    error_code error_code;
    arpet->portDisconnect();

    port_name = Cab+ui->comboBox_head_select_graph->currentText();
    writeFooterAndHeaderDebug(true);

    if (pmt_selected_list.isEmpty())
    {
        QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
        if(debug)
        {
            cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
            writeFooterAndHeaderDebug(false);
        }
        return;
    }

    QString q_msg;
    try
    {

        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }
        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        ui->lineEdit_hv_value->setText(QString::number(arpet->getHVMCA()));
        if (ui->checkBox_centroid->isChecked()) setHV(getHead("mca").toStdString(), ui->lineEdit_limiteinferior->text().toStdString());
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,-50),pmt_selected_list.at(0).toStdString());
        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        if(debug) cout<<"El nuevo canal configurado en el PMT "<<pmt_selected_list.at(0).toStdString()<<" es: "<<ui->lineEdit_hv_value->text().toStdString()<<endl;
    }
    catch (Exceptions ex)
    {
        if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
        showMCAEStreamDebugMode(q_msg.toStdString());
        writeFooterAndHeaderDebug(false);
    }
}
/**
 * @brief MainWindow::on_pushButton_p_5_clicked
 */
void MainWindow::on_pushButton_p_5_clicked()
{
    error_code error_code;
    arpet->portDisconnect();

    port_name = Cab+ui->comboBox_head_select_graph->currentText();
    writeFooterAndHeaderDebug(true);

    if (pmt_selected_list.isEmpty())
    {
        QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
        if(debug)
        {
            cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
            writeFooterAndHeaderDebug(false);
        }
        return;
    }

    QString q_msg;
    try
    {

        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }

        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        ui->lineEdit_hv_value->setText(QString::number(arpet->getHVMCA()));
        if (ui->checkBox_centroid->isChecked()) setHV(getHead("mca").toStdString(), ui->lineEdit_limiteinferior->text().toStdString());
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,5),pmt_selected_list.at(0).toStdString());
        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        if(debug) cout<<"El nuevo canal configurado en el PMT "<<pmt_selected_list.at(0).toStdString()<<" es: "<<ui->lineEdit_hv_value->text().toStdString()<<endl;
    }
    catch (Exceptions ex)
    {
        if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
        showMCAEStreamDebugMode(q_msg.toStdString());
        writeFooterAndHeaderDebug(false);
    }
}
/**
 * @brief MainWindow::on_pushButton_p_10_clicked
 */
void MainWindow::on_pushButton_p_10_clicked()
{
    error_code error_code;
    arpet->portDisconnect();

    port_name = Cab+ui->comboBox_head_select_graph->currentText();
    writeFooterAndHeaderDebug(true);

    if (pmt_selected_list.isEmpty())
    {
        QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
        if(debug)
        {
            cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
            writeFooterAndHeaderDebug(false);
        }
        return;
    }

    QString q_msg;
    try
    {
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }

        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        ui->lineEdit_hv_value->setText(QString::number(arpet->getHVMCA()));
        if (ui->checkBox_centroid->isChecked()) setHV(getHead("mca").toStdString(), ui->lineEdit_limiteinferior->text().toStdString());
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,10),pmt_selected_list.at(0).toStdString());
        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        if(debug) cout<<"El nuevo canal configurado en el PMT "<<pmt_selected_list.at(0).toStdString()<<" es: "<<ui->lineEdit_hv_value->text().toStdString()<<endl;
    }
    catch (Exceptions ex)
    {
        if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
        showMCAEStreamDebugMode(q_msg.toStdString());
        writeFooterAndHeaderDebug(false);
    }
}
/**
 * @brief MainWindow::on_pushButton_p_50_clicked
 */
void MainWindow::on_pushButton_p_50_clicked()
{
    error_code error_code;
    arpet->portDisconnect();

    port_name = Cab+ui->comboBox_head_select_graph->currentText();
    writeFooterAndHeaderDebug(true);

    if (pmt_selected_list.isEmpty())
    {
        QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
        if(debug)
        {
            cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
            writeFooterAndHeaderDebug(false);
        }
        return;
    }

    QString q_msg;
    try
    {
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }

        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        ui->lineEdit_hv_value->setText(QString::number(arpet->getHVMCA()));
        if (ui->checkBox_centroid->isChecked()) setHV(getHead("mca").toStdString(), ui->lineEdit_limiteinferior->text().toStdString());
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,50),pmt_selected_list.at(0).toStdString());
        getMCA(getHead("mca").toStdString(),arpet->getFunCSP3(),true,CHANNELS_PMT,pmt_selected_list.at(0).toStdString());
        if(debug) cout<<"El nuevo canal configurado en el PMT "<<pmt_selected_list.at(0).toStdString()<<" es: "<<ui->lineEdit_hv_value->text().toStdString()<<endl;
    }
    catch (Exceptions ex)
    {
        if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
        showMCAEStreamDebugMode(q_msg.toStdString());
        writeFooterAndHeaderDebug(false);
    }
}
/**
 * @brief MainWindow::on_pushButton_logguer_toggled
 * @param checked
 */
void MainWindow::on_pushButton_logguer_toggled(bool checked)
{

    if(checked)
    {

        connect(timer, SIGNAL(timeout()), this, SLOT(TimerUpdate()));
        timer->setInterval(ui->lineEdit_between_logs->text().toInt()*1000);
        timer->start();

        QList<int> checkedHeads=getCheckedHeads();
        worker->setCheckedHeads(checkedHeads);
        if (ui->checkBox_temp->isChecked()) worker->setTempBool(true); //Logueo temperatura
        if (ui->checkBox_tasa->isChecked()) worker->setRateBool(true); //Logueo tasa
        //TODO: aca poner el check del log de offset
        worker->setDebugMode(debug);
        //worker->setTimeBetweenLogs(ui->lineEdit_between_logs->text().toInt());

        worker->abort();
        thread->wait();
        worker->requestLog();
    }
    else
    {
        //setButtonLoggerState(true,true);
        worker->abort();
        if (is_abort_log)
        {
            if (debug) cout<<"Atención!! Se emitió una señal de aborto al thread: "<<thread->currentThreadId()<<endl;

        }
    }
    setButtonLoggerState(checked);
    setIsAbortLogFlag(~checked);
    emit sendAbortCommand(~checked);
}

/* Métodos generales del entorno gráfico */
/**
 * @brief MainWindow::getCheckedHeads
 * @return
 */
QList<int> MainWindow::getCheckedHeads()
{
    QList<int> checkedHeads;
    if (ui->comboBox_head_mode_select_config->currentIndex()==MULTIHEAD || ui->comboBox_head_mode_select_config->currentIndex()==ALLHEADS)
    {
        for(int i = 0; i < ui->frame_multihead_config->children().size(); i++)
        {
            QCheckBox *q = qobject_cast<QCheckBox*>(ui->frame_multihead_config->children().at(i));
            if(q->checkState() == Qt::Checked)
            {
                checkedHeads.append(i+1);
            }
        }
    }
    else
    {
        checkedHeads.append(getHead("config").toInt());
    }

    if(checkedHeads.length() == 0)
    {
        QMessageBox::critical(this,tr("Atención"),tr("No se ha seleccionado ningún cabezal"));
        return checkedHeads;
    }

    return checkedHeads;
}

/**
 * @brief MainWindow::getValuesFromFiles
 *
 * Analizador de archivo de texto
 *
 * @param filename
 * @param hv
 * @return Vector con los valores obtenidos del archivo
 */
QVector<double> MainWindow::getValuesFromFiles(QString filename, bool hv)
{
    QVector<double> values;
    QRegExp rx("(\\t)");
    QFile inputFile(filename);

    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        if(hv)
        {
            while (!in.atEnd())
            for(int i=0;i<PMTs;i++)
            {
                QString line = in.readLine();
                QStringList fields = line.split(rx);
                QString q_hv=fields.at(1);
                values.push_back(q_hv.toDouble());
            }
        }
        else
        {
            while (!in.atEnd())
            {
                QString line = in.readLine();
                values.push_back(line.toDouble());

            }
        }
        inputFile.close();
    }

    return values;
}


/**
 * @brief MainWindow::parseConfigurationFile
 *
 * Analizador del archivo de configuración
 *
 * @param filename
 * @return
 */
int MainWindow::parseConfigurationFile(bool mode, QString head)
{
    getPreferencesSettingsFile();
    QSettings settings(initfile, QSettings::IniFormat);

    /* Paths to the configuration files */

    if (!mode) // True: multimode | False: Monomode
    {
        head = getHead("config");
    }

    QString root = settings.value("Paths/root", "US").toString();
    /* Parameters */
    AT = settings.value("Cabezal"+head+"/AT", "US").toInt();
    LowLimit[head.toInt()-1] = settings.value("Cabezal"+head+"/LowLimit", "US").toInt();
    Target = settings.value("Cabezal"+head+"/Target", "US").toInt();
    coefenerg = root+settings.value("Cabezal"+head+"/coefenerg", "US").toString();
    hvtable = root+settings.value("Cabezal"+head+"/hvtable", "US").toString();
    coefx = root+settings.value("Cabezal"+head+"/coefx", "US").toString();
    coefy = root+settings.value("Cabezal"+head+"/coefy", "US").toString();
    coefest = root+settings.value("Cabezal"+head+"/coefest", "US").toString();
    coefT = root+settings.value("Cabezal"+head+"/coefT", "US").toString();
    coefTInter = root+settings.value("Cabezal"+head+"/coefTInter", "US").toString();
    path_Planar_bit  = settings.value("Paths/Planar_bit", "US").toString();
    path_SP3_bit     = settings.value("Paths/SP3_bit", "US").toString();
    path_adq_Calib  = settings.value("Paths/Adq_Calib", "US").toString();
    path_adq_Coin  = settings.value("Paths/Adq_Coin", "US").toString();
    path_Coin_bit    = settings.value("Paths/Coin_bit", "US").toString();
    device_planar     = settings.value("model/Planar", "US").toString();
    device_SP3        = settings.value("model/SP3", "US").toString();
    device_SP3_MEM    = settings.value("model/SP3_MEM", "US").toString();
    device_coin       = settings.value("model/Coin", "US").toString();
    model_planar     = settings.value("model/Planar_model", "US").toString();
    model_SP3        = settings.value("model/SP3_model", "US").toString();
    model_SP3_MEM    = settings.value("model/SP3_MEM_model", "US").toString();
    model_coin       = settings.value("model/Coin_model", "US").toString();
    name_Planar_bit  = settings.value("model/Bit_name_Planar", "US").toString();
    name_Coin_bit    = settings.value("model/Bit_name_Coin", "US").toString();
    name_SP3_bit     = settings.value("model/Bit_name_SP3", "US").toString();

    return MCAE::OK;
}

/**
 * @brief MainWindow::openConfigurationFile
 * @return
 */
QString MainWindow::openConfigurationFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de configuración"),
                                                    root_config_path,
                                                    tr("Configuración (*.ini);;Texto (*.txt)"));
    initfile = filename;
    return filename;
}
/**
 * @brief MainWindow::openLogFile
 * @return
 */
QString MainWindow::openLogFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de log"),
                                                    root_log_path,
                                                    tr("Log (*.log)"));
    initfile = filename;
    return filename;
}
/**
 * @brief MainWindow::openDirectory
 * @return
 */
QString MainWindow::openDirectory()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Seleccionar el directorio"),
                                                         QDir::homePath());
    return directory;
}
/**
 * @brief MainWindow::getPaths
 */
void MainWindow::getPaths()
{
    if (ui->comboBox_head_mode_select_config->currentIndex()==MONOHEAD)
    {
        parseConfigurationFile(false);
        ui->textBrowser_triple_ventana->setText(coefest);
        ui->textBrowser_hv->setText(hvtable);
        ui->textBrowser_energia->setText(coefenerg);
        ui->textBrowser_posicion_X->setText(coefx);
        ui->textBrowser_posicion_Y->setText(coefy);
        ui->textBrowser_tiempos_cabezal->setText(coefT);
        ui->textBrowser_tiempos_Inter_cabezal->setText(coefTInter);
        ui->lineEdit_alta->setText(QString::number(AT));
        //ui->lineEdit_limiteinferior->setText(QString::number(LowLimit));
    }
    else
    {
        ui->textBrowser_triple_ventana->setText("");
        ui->textBrowser_hv->setText("");
        ui->textBrowser_energia->setText("");
        ui->textBrowser_posicion_X->setText("");
        ui->textBrowser_posicion_Y->setText("");
        ui->textBrowser_tiempos_cabezal->setText("");
        ui->textBrowser_tiempos_Inter_cabezal->setText("");
        ui->lineEdit_alta->setText("");
        ui->lineEdit_limiteinferior->setText("");
    }
}
/**
 * @brief MainWindow::setLabelState
 * @param state
 * @param label
 */
void MainWindow::setLabelState(bool state, QLabel *label, bool error)
{
    QPalette palette;

    if (state)
    {
        //palette.setColor(QPalette::Background,Qt::green);
        label->setStyleSheet("QLabel { background-color : #3daee9; }");

        //label->setPalette(palette);
    }
    else
    {
        //palette.setColor(QPalette::Background,Qt::red);
       // QLabel* pLabel = new QLabel;
        label->setStyleSheet("QLabel { background-color : red;  }");
        //label->setPalette(palette);

    }

    if(!state && error)
    {


        label->setStyleSheet("QLabel { background-color : grey;  }");

    }
}
/**
 * @brief MainWindow::setTextBrowserState
 * @param state
 * @param tbro
 */
void MainWindow::setTextBrowserState(bool state, QTextBrowser *tbro)
{
    if (state)
    {
        tbro->setStyleSheet("background-color: #3daee9;");;
    }
    else
    {
        tbro->setStyleSheet("background-color: red;");
    }
}
/**
 * @brief MainWindow::setButtonState
 * @param state
 * @param button
 * @param disable
 */
void MainWindow::setButtonState(bool state, QPushButton * button, bool disable)
{
    QString color;

    if (state && !disable)
    {
        color="background-color: #2196F3";
    }
    else if (!state && !disable)
    {
        color="background-color: #EF5350";
    }
    else
    {
        color=" ";
    }
    button->setStyleSheet(color);
    button->update();
    button->setChecked(!disable);
}
/**
 * @brief MainWindow::setButtonAdquireState
 * @param state
 * @param disable
 */
//void MainWindow::setButtonAdquireState(bool state, bool disable)
//{
//    QString qt_text;

//    if (state && !disable)
//    {
//        qt_text="Adquiriendo";
//        setButtonState(state,ui->pushButton1_adquirir,disable);
//    }
//    else if (!state && !disable)
//    {
//        qt_text="Error";
//        setButtonState(state,ui->pushButton_adqui1rir,disable);
//    }
//    else
//    {
//        qt_text="Adquirir";
//        setButtonState(state,ui->pushButt1on_adquirir,disable);
//    }
//    ui->pushButt1on_adquirir->setText(qt_text);
//    ui->pushButt1on_adquirir->update();
//}

void MainWindow::setButtonCalibState(bool state, bool disable)
{
    QString qt_text;

    if (state && !disable)
    {
        qt_text="Adquiriendo";
        setButtonState(state,ui->pb_Autocalib,disable);///qwerty
    }
    else if (!state && !disable)
    {
        qt_text="Error";
        setButtonState(state,ui->pb_Autocalib,disable);
    }
    else
    {
        qt_text="Adquirir";
        setButtonState(state,ui->pb_Autocalib,disable);
    }
    ui->pb_Autocalib->setText(qt_text);
    ui->pb_Autocalib->update();
}
/**
 * @brief MainWindow::setButtonConnectState
 * @param state
 * @param disable
 */
void MainWindow::setButtonConnectState(bool state, bool disable)
{
//    QString qt_text;

//    if (state && !disable)
//    {
//        qt_text="Conectar";
//        setButtonState(state,ui->pushButton_init_configure,disable);
//    }
//    else if (!state && !disable)
//    {
//        qt_text="Desconectar";
//        setButtonState(state,ui->pushButton_init_configure,disable);
//    }
//    else
//    {
//        qt_text="Conectar";
//        setButtonState(state,ui->pushButton_init_configure,disable);
//    }
//    ui->pushButton_init_configure->setText(qt_text);
//    ui->pushButton_init_configure->update();
}
void MainWindow::setButtonLoggerState(bool state, bool disable)
{
    QString qt_text;

    if (state && !disable)
    {
        qt_text="Logueando";
        //setButtonState(state,ui->pushButton_logguer,disable);
    }
    else if (!state && !disable)
    {
        qt_text="Cancelado";
        //setButtonState(state,ui->pushButton_logguer,disable);
    }
    else
    {
        qt_text="Iniciar";
        //setButtonState(state,ui->pushButton_logguer,disable);
    }
    ui->pushButton_logguer->setText(qt_text);
    ui->pushButton_logguer->update();
}
/**
 * @brief MainWindow::availablePortsName
 *
 * Método que localiza los puertos conectados a la PC y la muestra en una lista
 *
 * @return Lista de puertos disponibles
 */
QStringList MainWindow::availablePortsName()
{

    QStringList portsName;
    int portavailable;
    QDir dir("/dev/");
    QStringList filters;
    try {
        filters << "UART*";
        dir.setNameFilters(filters);
        dir.setFilter(QDir::Files | QDir::System);
        QFileInfoList list = dir.entryInfoList();

        for (int i=0; i< list.size(); i++)
        {
            portsName.append(list.at(i).absoluteFilePath());

        }

        if (list.empty()) {
            ui->tabWidget_general->setTabEnabled(Tab0,false); // Escondo pestaña Configuracion
            ui->tabWidget_general->setTabEnabled(Tab1,false); // Escondo pestaña MCA
            ui->tabWidget_general->setTabEnabled(Tab4,false); // Escondo pestaña Autocalib
            ui->tabWidget_general->setTabEnabled(Tab9,false); // Escondo pestaña Terminal
            Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
            throw exception_Cabezal_Apagado;
        }
        for (int i=0;i<6;i++){
            if (list.at(0).absoluteFilePath().contains(QString::number(1+i))) {
                portavailable=i; break;
            }
        }

        bool oldState =ui->comboBox_head_select_config->blockSignals(true);
        ui->comboBox_head_select_config->setCurrentIndex(portavailable);
        ui->comboBox_head_select_config->blockSignals(oldState);
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
        arpet->portDisconnect();
    }

    return portsName;
}
/**
 * @brief MainWindow::getHead
 * @param tab
 * @return head
 */
QString MainWindow::getHead(string tab)
{
    QString head;
    if (tab=="mca")
    {
        //if (ui->comboBox_head_mode_select_graph->currentIndex()==MONOHEAD)
        {
            head=ui->comboBox_head_select_graph->currentText();
        }
    }
    else if (tab=="config")
    {
        if (ui->comboBox_head_mode_select_config->currentIndex()==MONOHEAD)
        {
            head=ui->comboBox_head_select_config->currentText();
        }
    }
    else if (tab=="terminal")
    {
        head=ui->comboBox_head_select_terminal->currentText();
    }
    else head="";

    return head;
}
/**
 * @brief MainWindow::readString
 *
 * Método general de lectura de datos por protocolo serie
 *
 * @param delimeter
 * @return Mensaje recibido en _string_
 */
string MainWindow::readString(char delimeter)
{
    mMutex.lock();

    string msg;
    try{
        msg = arpet->readString(delimeter, port_name.toStdString());
    }
    catch( Exceptions & ex ){
        Exceptions exception_stop(ex.excdesc);
        mMutex.unlock();
        throw exception_stop;
    }

    mMutex.unlock();

    return msg;
}
/**
 * @brief MainWindow::readBufferString
 *
 * Método general de lectura de _buffer_ de datos por protocolo serie
 *
 * @param buffer_size
 * @return Mensaje recibido en _string_
 */
string MainWindow::readBufferString(int buffer_size)
{
    mMutex.lock();

    string msg;
    try{
        msg = arpet->readBufferString(buffer_size,port_name.toStdString());
    }
    catch( Exceptions & ex ){
        Exceptions exception_stop(ex.excdesc);
        mMutex.unlock();
        throw exception_stop;
    }

    mMutex.unlock();

    return msg;
}
/**
 * @brief MainWindow::sendString
 *
 * Método general de escritura de datos por protocolo serie
 *
 * @param msg
 * @param end
 * @return Mensaje recibido en _string_
 */
size_t MainWindow::sendString(string msg, string end)
{
    mMutex.lock();

    arpet->portFlush();
    size_t bytes_transfered = 0;

    try{
        bytes_transfered = arpet->sendString(msg, end, port_name.toStdString());
    }
    catch(boost::system::system_error e){
        Exceptions exception_serial_port((string("No se puede acceder al puerto serie. Error: ")+string(e.what())).c_str());
        mMutex.unlock();
        throw exception_serial_port;
    }

    mMutex.unlock();

    return bytes_transfered;
}
/**
 * @brief MainWindow::manageHeadCheckBox
 * @param tab
 * @param show
 */
void MainWindow::manageHeadCheckBox(string tab, bool show)
{
    if (tab=="config")
    {
//        if (show) ui->frame_multihead_config->show();
//        else ui->frame_multihead_config->hide();
    }
    else if(tab=="mca")
    {
        if (show) ui->frame_multihead_graph->show();
        //else ui->frame_multihead_graph->hide();
    }
    else return;
}
/**
 * @brief MainWindow::manageHeadComboBox
 * @param tab
 * @param show
 */
void MainWindow::manageHeadComboBox(string tab, bool show)
{
/*    if (tab=="config"){
        if (show) ui->comboBox_head_select_config->show();
        else ui->comboBox_head_select_config->hide();
    }
    else*/ if(tab=="mca")
    {
        //if (show) ui->comboBox_head_select_graph->show();
       /* else*/ ui->comboBox_head_select_graph->hide();
    }
    else return;
}
/**
 * @brief MainWindow::setHeadMode
 * @param index
 * @param tab
 */
void MainWindow::setHeadMode(int index, string tab)
{
    switch (index) {
    case MONOHEAD:
        manageHeadComboBox(tab, true);
        manageHeadCheckBox(tab, false);
        break;
    case MULTIHEAD:
        manageHeadComboBox(tab, false);
        manageHeadCheckBox(tab, true);
        break;
    case ALLHEADS:
        manageHeadComboBox(tab, false);
        manageHeadCheckBox(tab, true);
    default:
        break;
    }
}
/**
 * @brief MainWindow::syncHeadModeComboBoxToConfig
 * @param index
 */
void MainWindow::syncHeadModeComboBoxToConfig(int index)
{
    getPaths();
    ui->comboBox_head_mode_select_config->setCurrentIndex(index);
}
/**
 * @brief MainWindow::syncHeadComboBoxToConfig
 * @param index
 */
void MainWindow::syncHeadComboBoxToConfig(int index)
{
    getPaths();
    setHeadCustomPlotEnvironment();
    ui->comboBox_head_select_config->setCurrentIndex(index);
}
/**
 * @brief MainWindow::syncHeadModeComboBoxToMCA
 * @param index
 */
void MainWindow::syncHeadModeComboBoxToMCA(int index)
{
    getPaths();
    ui->comboBox_head_mode_select_graph->setCurrentIndex(index);
}
/**
 * @brief MainWindow::syncHeadComboBoxToMCA
 * @param index
 */
void MainWindow::syncHeadComboBoxToMCA(int index)
{
    getPaths();
    setHeadCustomPlotEnvironment();
    ui->comboBox_head_select_graph->setCurrentIndex(index);
}
/**
 * @brief MainWindow::syncCheckBoxHead1ToConfig
 * @param check
 */
void MainWindow::syncCheckBoxHead1ToConfig(bool check)
{
    ui->checkBox_c_1->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead2ToConfig
 * @param check
 */
void MainWindow::syncCheckBoxHead2ToConfig(bool check)
{
    ui->checkBox_c_2->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead3ToConfig
 * @param check
 */
void MainWindow::syncCheckBoxHead3ToConfig(bool check)
{
    ui->checkBox_c_3->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead4ToConfig
 * @param check
 */
void MainWindow::syncCheckBoxHead4ToConfig(bool check)
{
    ui->checkBox_c_4->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead5ToConfig
 * @param check
 */
void MainWindow::syncCheckBoxHead5ToConfig(bool check)
{
    ui->checkBox_c_5->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead6ToConfig
 * @param check
 */
void MainWindow::syncCheckBoxHead6ToConfig(bool check)
{
    ui->checkBox_c_6->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead1ToMCA
 * @param check
 */
void MainWindow::syncCheckBoxHead1ToMCA(bool check)
{
    ui->checkBox_mca_1->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead2ToMCA
 * @param check
 */
void MainWindow::syncCheckBoxHead2ToMCA(bool check)
{
    ui->checkBox_mca_2->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead3ToMCA
 * @param check
 */
void MainWindow::syncCheckBoxHead3ToMCA(bool check)
{
    ui->checkBox_mca_3->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead4ToMCA
 * @param check
 */
void MainWindow::syncCheckBoxHead4ToMCA(bool check)
{
    ui->checkBox_mca_4->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead5ToMCA
 * @param check
 */
void MainWindow::syncCheckBoxHead5ToMCA(bool check)
{
    ui->checkBox_mca_5->setChecked(check);
}
/**
 * @brief MainWindow::syncCheckBoxHead6ToMCA
 * @param check
 */
void MainWindow::syncCheckBoxHead6ToMCA(bool check)
{
    ui->checkBox_mca_6->setChecked(check);
}

/* Terminal */
/**
 * @brief MainWindow::on_pushButton_send_terminal_clicked
 */
void MainWindow::on_pushButton_send_terminal_clicked()
{
    QString sended = ui->lineEdit_terminal->text();
    size_t bytes=0;
    string msg;
    string end_stream=arpet->getEnd_MCA();
    QString Cabezal=ui->comboBox_head_select_terminal->currentText();
    arpet->portDisconnect();

    try
    {

        if (Cabezal == "Coin")
        {
            port_name="/dev/UART_Coin";
        }else{

            if (Estado_Cabezales.contains(Cabezal.toInt())){
                port_name=Cab+Cabezal;

            }
            else{
                Exceptions exception_Cabezal_Apagado("El cabezal está apagado, seleccione otro");
                throw exception_Cabezal_Apagado;
            }
        }
        arpet->portConnect(port_name.toStdString().c_str());

        if(ui->checkBox_end_terminal->isChecked()) end_stream=arpet->getEnd_PSOC();

        bytes = sendString(sended.toStdString(),end_stream);
        msg = readString();

        QString q_msg=QString::fromStdString(msg);
        QString q_bytes=QString::number(bytes);

        ui->label_size_terminal->setText(q_bytes);
        ui->label_received_terminal->setText(q_msg);
        arpet->portDisconnect();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
        arpet->portDisconnect();
    }
}
/**
 * @brief MainWindow::on_pushButton_flush_terminal_clicked
 */
void MainWindow::on_pushButton_flush_terminal_clicked()
{
    arpet->portFlush();
}
/**
 * @brief MainWindow::on_pushButton_clear_terminal_clicked
 */
void MainWindow::on_pushButton_clear_terminal_clicked()
{
    ui->lineEdit_terminal->clear();
}
/**
 * @brief MainWindow::on_pushButton_stream_configure_mca_terminal_clicked
 */
void MainWindow::on_pushButton_stream_configure_mca_terminal_clicked()
{
    string mca_function, function, hv_value;
    int bytes_mca=0;
    switch (ui->comboBox_mca_function_terminal->currentIndex())
    {
    case 0:
        mca_function=arpet->getInit_MCA();
        hv_value="";
        break;
    case 1:
        mca_function=arpet->getData_MCA();
        hv_value="";
        break;
    case 2:
        mca_function=arpet->getSetHV_MCA();
        hv_value=getHVValue(ui->lineEdit_pmt_hv_terminal);
        break;
    case 3:
        mca_function=arpet->getTemp_MCA();
        hv_value="";
        break;
    default:
        break;
    }

    switch (ui->comboBox_func_terminal->currentIndex())
    {
    case 0:
        function=arpet->getFunCHead();
        if (ui->comboBox_mca_function_terminal->currentIndex()==1) bytes_mca = CHANNELS*6+16;
        break;
    case 1:
        function=arpet->getFunCSP3();
        if (ui->comboBox_mca_function_terminal->currentIndex()==1) bytes_mca = CHANNELS_PMT*6+16;
        break;
    default:
        break;
    }

    int pmt=getPMT(ui->lineEdit_pmt_terminal);
    setMCAEDataStream(getHead("terminal").toStdString(), function, QString::number(pmt).toStdString(), mca_function, bytes_mca, hv_value);
    ui->lineEdit_terminal->setText(QString::fromStdString(arpet->getTrama_MCAE()));

}
/**
 * @brief MainWindow::on_pushButton_stream_configure_psoc_terminal_clicked
 */
void MainWindow::on_pushButton_stream_configure_psoc_terminal_clicked()
{
    QString psoc_alta;
    switch (ui->comboBox_psoc_function_terminal->currentIndex())
    {
    case 0:
        setPSOCDataStream(getHead("terminal").toStdString(), arpet->getPSOC_SIZE_RECEIVED_ALL(), arpet->getPSOC_ON());
        break;
    case 1:
        setPSOCDataStream(getHead("terminal").toStdString(), arpet->getPSOC_SIZE_RECEIVED_ALL(), arpet->getPSOC_OFF());
        break;
    case 2:
        psoc_alta = getPSOCAlta(ui->lineEdit_psoc_hv_terminal);
        setPSOCDataStream(getHead("terminal").toStdString(), arpet->getPSOC_SIZE_RECEIVED_ALL(), arpet->getPSOC_SET(),psoc_alta);
        break;
    case 3:
        setPSOCDataStream(getHead("terminal").toStdString(), arpet->getPSOC_SIZE_RECEIVED(), arpet->getPSOC_STA());
        break;
    default:
        break;
    }
    ui->lineEdit_terminal->setText(QString::fromStdString(arpet->getTrama_MCAE()));
}

/* Métodos para QCustomPlot */
/**
 * @brief MainWindow::getCustomPlotParameters
 * @return
 */
QVector<int> MainWindow::getCustomPlotParameters()
{
    QVector<int> param(6);
    param[0]=rand()%245+10;//R
    param[1]=rand()%245+10;//G
    param[2]=rand()%245+10;//B
    param[3]=rand()%5+1; //LineStyle
    param[4]=rand()%14+1;//ScatterShape
    param[5]=rand()/(double)RAND_MAX*2+1;//setWidthF

    return param;
}
/**
 * @brief MainWindow::setPMTCustomPlotEnvironment
 * @param qlist
 */
void MainWindow::setPMTCustomPlotEnvironment(QList<QString> qlist)
{
    for (unsigned int index=0; index < qlist.length(); index++)
    {
        qcp_pmt_parameters.insert(index, 1, getCustomPlotParameters());
    }
}

void MainWindow::setPMTCalibCustomPlotEnvironment(QList<int> qlist)
{
    for (unsigned int index=0; index < qlist.length(); index++)
    {
        qcp_pmt_calib_parameters.insert(index, 1, getCustomPlotParameters());
    }
}

/**
 * @brief MainWindow::setHeadCustomPlotEnvironment
 */
void MainWindow::setHeadCustomPlotEnvironment()
{
    for (unsigned int index=0; index < HEADS; index++)
    {
        qcp_head_parameters.insert(index, 1, getCustomPlotParameters());
    }
}
/**
 * @brief MainWindow::addGraph
 * @param hits
 * @param graph
 * @param channels
 * @param graph_legend
 * @param param
 */
void MainWindow::addGraph(QVector<double> hits,  QCustomPlot *graph, int channels, QString graph_legend, QVector<int> param)
{
    channels_ui.resize(channels);
    channels_ui = arpet->getChannels();

    graph->addGraph();
    graph->graph()->setAdaptiveSampling(true);
    graph->graph()->setName(graph_legend);
    graph->graph()->addData(channels_ui,hits);
    graph->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(param[4])));
    QPen graphPen;
    graphPen.setColor(QColor(param[0], param[1], param[2]));
    graphPen.setWidthF(param[5]);
    graph->graph()->setPen(graphPen);
    graph->legend->setVisible(true);
    graph->legend->setWrap(8);
    graph->legend->setRowSpacing(1);
    graph->legend->setColumnSpacing(2);
    graph->rescaleAxes();
    graph->replot();
}

void MainWindow::addGraph_Calib(QVector<double> hits,  QCustomPlot *graph, int channels, QString graph_legend, QVector<int> param)
{
    channels_ui.resize(channels);
    channels_ui = calibrador->getChannels();

//     *data = new graph->graph();
//    size_t len = channels;
//    auto xp = std::begin(channels);
//    auto yp = std::begin(hits);
//    while (len--){
//        data->insertMulti(data->constEnd(), *xp, QCPData(*xp++, *yp++));
//    }
//graph->graph()
    graph->addGraph();
    graph->graph()->setName(graph_legend);
//    graph->addData(data);
    graph->graph()->setAdaptiveSampling(true);
    graph->graph()->addData(channels_ui,hits);
    graph->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(param[4])));
    QPen graphPen;
    graphPen.setColor(QColor(param[0], param[1], param[2]));
    graphPen.setWidthF(param[5]);
    graph->graph()->setPen(graphPen);
    graph->legend->setVisible(true);
    graph->legend->setWrap(8);
    graph->legend->setRowSpacing(1);
    graph->legend->setColumnSpacing(2);
    graph->rescaleAxes();
    graph->replot();
}
/**
 * @brief MainWindow::axisLabelDoubleClickPMT
 * @param axis
 * @param part
 */
void MainWindow::axisLabelDoubleClickPMT(QCPAxis *axis, QCPAxis::SelectablePart part)
{
    // Renombra la etiqueta del eje a partir de un doble-clic
    if (part == QCPAxis::spAxisLabel)
    {
        bool ok;
        QString newLabel = QInputDialog::getText(this, "QCustomPlot", "Nueva etiqueta: ", QLineEdit::Normal, axis->label(), &ok);
        if (ok)
        {
            axis->setLabel(newLabel);
            ui->specPMTs->replot();
        }
    }
}
/**
 * @brief MainWindow::axisLabelDoubleClickHead
 * @param axis
 * @param part
 */
void MainWindow::axisLabelDoubleClickHead(QCPAxis *axis, QCPAxis::SelectablePart part)
{
    // Renombra la etiqueta del eje a partir de un doble-clic
    if (part == QCPAxis::spAxisLabel)
    {
        bool ok;
        QString newLabel = QInputDialog::getText(this, "QCustomPlot", "Nueva etiqueta: ", QLineEdit::Normal, axis->label(), &ok);
        if (ok)
        {
            axis->setLabel(newLabel);
            ui->specHead->replot();
        }
    }
}
/**
 * @brief MainWindow::titleDoubleClickPMT
 * @param event
 */
void MainWindow::titleDoubleClickPMT(QMouseEvent* event)
{
    Q_UNUSED(event)
    if (QCPTextElement *title = qobject_cast<QCPTextElement*>(sender()))
    {
        // Renombra el título del gráfico a partir de un doble-clic
        bool ok;
        QString newTitle = QInputDialog::getText(this, "QCustomPlot", "Nuevo título: ", QLineEdit::Normal, title->text(), &ok);
        if (ok)
        {
            title->setText(newTitle);
            ui->specPMTs->replot();
        }
    }
}
/**
 * @brief MainWindow::titleDoubleClickHead
 * @param event
 */
void MainWindow::titleDoubleClickHead(QMouseEvent* event)
{
    Q_UNUSED(event)
    if (QCPTextElement *title = qobject_cast<QCPTextElement*>(sender()))
    {
        // Renombra el título del gráfico a partir de un doble-clic
        bool ok;
        QString newTitle = QInputDialog::getText(this, "QCustomPlot", "Nuevo título: ", QLineEdit::Normal, title->text(), &ok);
        if (ok)
        {
            title->setText(newTitle);
            ui->specHead->replot();
        }
    }
}
/**
 * @brief MainWindow::legendDoubleClickPMT
 * @param legend
 * @param item
 */
void MainWindow::legendDoubleClickPMT(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    // Renombra el gráfico a partir de un doble-clic
    Q_UNUSED(legend)
    if (item)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        bool ok;
        QString newName = QInputDialog::getText(this, "QCustomPlot", "Nuevo nombre de gráfico: ", QLineEdit::Normal, plItem->plottable()->name(), &ok);
        if (ok)
        {
            plItem->plottable()->setName(newName);
            ui->specPMTs->replot();
        }
    }
}
/**
 * @brief MainWindow::legendDoubleClickHead
 * @param legend
 * @param item
 */
void MainWindow::legendDoubleClickHead(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    // Renombra el gráfico a partir de un doble-clic
    Q_UNUSED(legend)
    if (item)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        bool ok;
        QString newName = QInputDialog::getText(this, "QCustomPlot", "Nuevo nombre de gráfico: ", QLineEdit::Normal, plItem->plottable()->name(), &ok);
        if (ok)
        {
            plItem->plottable()->setName(newName);
            ui->specHead->replot();
        }
    }
}
/**
 * @brief MainWindow::contextMenuRequestPMT
 * @param pos
 */
void MainWindow::contextMenuRequestPMT(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->specPMTs->legend->selectTest(pos, false) >= 0)
    {
        menu->addAction("Mover hacia arriba a la izquierda", this, SLOT(moveLegendPMT()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
        menu->addAction("Mover hacia arriba al centro", this, SLOT(moveLegendPMT()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
        menu->addAction("Mover hacia arriba a la derecha", this, SLOT(moveLegendPMT()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
        menu->addAction("Mover hacia abajo a la derecha", this, SLOT(moveLegendPMT()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
        menu->addAction("Mover hacia abajo a la izquierda", this, SLOT(moveLegendPMT()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
    } else
    {
        menu->addAction("Restaurar el tamaño del gráfico", this, SLOT(resetGraphZoomPMT()));
        if (ui->specPMTs->selectedGraphs().size() > 0)
            menu->addAction("Eliminar el gráfico seleccionado", this, SLOT(removeSelectedGraphPMT()));
        if (ui->specPMTs->graphCount() > 0)
            menu->addAction("Eliminar todos los gráficos", this, SLOT(removeAllGraphsPMT()));
    }

    menu->popup(ui->specPMTs->mapToGlobal(pos));
}
/**
 * @brief MainWindow::contextMenuRequestHead
 * @param pos
 */
void MainWindow::contextMenuRequestHead(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->specHead->legend->selectTest(pos, false) >= 0)
    {
        menu->addAction("Mover hacia arriba a la izquierda", this, SLOT(moveLegendHead()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
        menu->addAction("Mover hacia arriba al centro", this, SLOT(moveLegendHead()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
        menu->addAction("Mover hacia arriba a la derecha", this, SLOT(moveLegendHead()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
        menu->addAction("Mover hacia abajo a la derecha", this, SLOT(moveLegendHead()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
        menu->addAction("Mover hacia abajo a la izquierda", this, SLOT(moveLegendHead()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
    } else
    {
        menu->addAction("Restaurar el tamaño del gráfico", this, SLOT(resetGraphZoomHead()));
        if (ui->specHead->selectedGraphs().size() > 0)
            menu->addAction("Eliminar el gráfico seleccionado", this, SLOT(removeSelectedGraphHead()));
        if (ui->specHead->graphCount() > 0)
            menu->addAction("Eliminar todos los gráficos", this, SLOT(removeAllGraphsHead()));

    }

    menu->popup(ui->specHead->mapToGlobal(pos));
}
/**
 * @brief MainWindow::moveLegendPMT
 */
void MainWindow::moveLegendPMT()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender()))
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);
        if (ok)
        {
            ui->specPMTs->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
            ui->specPMTs->replot();
        }
    }
}
/**
 * @brief MainWindow::moveLegendHead
 */
void MainWindow::moveLegendHead()
{
    if (QAction* contextAction = qobject_cast<QAction*>(sender()))
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);
        if (ok)
        {
            ui->specHead->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
            ui->specHead->replot();
        }
    }
}
/**
 * @brief MainWindow::mousePressPMT
 */
void MainWindow::mousePressPMT()
{
    if (ui->specPMTs->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->specPMTs->axisRect()->setRangeDrag(ui->specPMTs->xAxis->orientation());
    else if (ui->specPMTs->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->specPMTs->axisRect()->setRangeDrag(ui->specPMTs->yAxis->orientation());
    else
        ui->specPMTs->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}
/**
 * @brief MainWindow::mousePressHead
 */
void MainWindow::mousePressHead()
{
    if (ui->specHead->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->specHead->axisRect()->setRangeDrag(ui->specHead->xAxis->orientation());
    else if (ui->specHead->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->specHead->axisRect()->setRangeDrag(ui->specHead->yAxis->orientation());
    else
        ui->specHead->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}
/**
 * @brief MainWindow::mouseWheelPMT
 */
void MainWindow::mouseWheelPMT()
{
    if (ui->specPMTs->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->specPMTs->axisRect()->setRangeZoom(ui->specPMTs->xAxis->orientation());
    else if (ui->specPMTs->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->specPMTs->axisRect()->setRangeZoom(ui->specPMTs->yAxis->orientation());
    else
        ui->specPMTs->axisRect()->setRangeZoom(/*Qt::Horizontal|*/Qt::Vertical);
}
/**
 * @brief MainWindow::mouseWheelHead
 */
void MainWindow::mouseWheelHead()
{
    if (ui->specHead->xAxis->selectedParts().testFlag(QCPAxis::spAxis));
        //ui->specHead->axisRect()->setRangeZoom(ui->specHead->xAxis->orientation());
    else if (ui->specHead->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->specHead->axisRect()->setRangeZoom(ui->specHead->yAxis->orientation());
    else
        ui->specHead->axisRect()->setRangeZoom(/*Qt::Horizontal|*/Qt::Vertical);
}
/**
 * @brief MainWindow::removeSelectedGraphPMT
 */
void MainWindow::removeSelectedGraphPMT()
{
    if (ui->specPMTs->selectedGraphs().size() > 0)
    {
        ui->specPMTs->removeGraph(ui->specPMTs->selectedGraphs().first());
        ui->specPMTs->replot();
    }
}
/**
 * @brief MainWindow::removeSelectedGraphHead
 */
void MainWindow::removeSelectedGraphHead()
{
    if (ui->specHead->selectedGraphs().size() > 0)
    {
        ui->specHead->removeGraph(ui->specHead->selectedGraphs().first());
        ui->specHead->replot();
    }
}
/**
 * @brief MainWindow::removeAllGraphsPMT
 */
void MainWindow::removeAllGraphsPMT()
{
    ui->specPMTs->clearGraphs();
    ui->specPMTs->replot();
}
/**
 * @brief MainWindow::removeAllGraphsHead
 */
void MainWindow::removeAllGraphsHead()
{
    ui->specHead->clearGraphs();
    ui->specHead->replot();
}
/**
 * @brief MainWindow::selectionChangedPMT
 */
void MainWindow::selectionChangedPMT()
{
    if (ui->specPMTs->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->specPMTs->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            ui->specPMTs->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->specPMTs->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->specPMTs->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->specPMTs->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    if (ui->specPMTs->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->specPMTs->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            ui->specPMTs->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->specPMTs->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->specPMTs->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->specPMTs->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }
    for (int i=0; i<ui->specPMTs->graphCount(); ++i)
    {
        QCPGraph *graph = ui->specPMTs->graph(i);
        QCPPlottableLegendItem *item = ui->specPMTs->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected())
        {
            item->setSelected(true);
            graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
        }
    }
}
/**
 * @brief MainWindow::selectionChangedHead
 */
void MainWindow::selectionChangedHead()
{
    if (ui->specHead->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->specHead->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            ui->specHead->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->specHead->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->specHead->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->specHead->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    if (ui->specHead->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->specHead->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
            ui->specHead->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->specHead->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
    {
        ui->specHead->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
        ui->specHead->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }


    for (int i=0; i<ui->specHead->graphCount(); ++i)
    {
        QCPGraph *graph = ui->specHead->graph(i);
        QCPPlottableLegendItem *item = ui->specHead->legend->itemWithPlottable(graph);
        if (item->selected() || graph->selected())
        {
            item->setSelected(true);
            graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
        }
    }
}
/**
 * @brief MainWindow::resetGraphZoomHead
 */
void MainWindow::resetGraphZoomHead()
{
    ui->specHead->rescaleAxes();
    ui->specHead->replot();
}
/**
 * @brief MainWindow::resetGraphZoomPMT
 */
void MainWindow::resetGraphZoomPMT()
{
    ui->specPMTs->rescaleAxes();
    ui->specPMTs->replot();
}
/**
 * @brief MainWindow::graphClicked
 * @param plottable
 * @param dataIndex
 */
void MainWindow::graphClicked(QCPAbstractPlottable *plottable, int dataIndex)
{
    /** @todo : Ver el funcionamiento y requerimiento de este método.*/
    double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
    QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
    ui->statusBar->showMessage(message, 2500);
}

/* Autocalib */
void MainWindow::on_pb_Autocalib_toggled(bool checked)
{
    QList<QString> qlist = pmt_select_autocalib->GetPMTSelectedList();
    int head_index;
    error_code error_code;
    arpet->portDisconnect();

    head_index=ui->comboBox_head_select_graph_2->currentText().toInt();

    port_name=Cab+QString::number(head_index);

    if(debug) cout<<"Cabezal: "<< QString::number(head_index).toStdString()<<endl;
    try{
    error_code= arpet->portConnect(port_name.toStdString().c_str());
    if (error_code.value()!=0){
        arpet->portDisconnect();
        Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
        throw exception_Cabezal_Apagado;
    }

    calibrador->setPort_Name(port_name);
    worker->setPortName(port_name);

    if (checked)
    {
        //setLabelState(true, ui->pushButton);

        QList<int> checked_PMTs, checked_Cab;
        QMessageBox messageBox;

        cout<<"Autocalibrando"<<endl;

        if (ui->checkBox_Cab_Completo->isChecked())
        {
            for(int i = 0;  i< PMTs ; i++ )
            {
                checked_PMTs.append(i+1);
                cout<<checked_PMTs[i]<<endl;
            }
            calibrador->setPMT_List(checked_PMTs);
        }
        else
        {
            // Recupero los PMT checkeados
            for(int i = 0;  i< qlist.length() ; i++ )
            {
                checked_PMTs.append(qlist[i].toInt());
                cout<<checked_PMTs[i]<<endl;
            }
            if(qlist.length() == 0)
            {
                messageBox.critical(0,"Error","No se ha seleccionado ningún PMT.");
                messageBox.setFixedSize(500,200);
                setButtonCalibState(false);
                setIsAbortCalibFlag(false);
                setButtonCalibState(true,true);
                emit ToPushButtonCalib(false);
                return;
            }
            calibrador->setPMT_List(checked_PMTs);
        }

        setPMTCalibCustomPlotEnvironment(calibrador->PMTs_List);// checked_PMTs);

        // Recupero el tiempo de adquisicion
        QString Tiempo_adq = ui->Tiempo_adq_box->text();
        if(Tiempo_adq.toInt() < 0 || Tiempo_adq.toInt() > 360)
        {
            messageBox.critical(0,"Error","Tiempo adquisicion fuera de los limites fijados.");
            messageBox.setFixedSize(500,200);
            setButtonCalibState(false);
            setIsAbortCalibFlag(false);
            setButtonCalibState(true,true);
            emit ToPushButtonCalib(false);
            return;
        }
        calibrador->setTiempo_adq(Tiempo_adq.toInt());
        checked_Cab.append(ui->comboBox_head_select_graph_2->currentIndex()+1);
        calibrador->setCab_List(checked_Cab);

        arpet->portDisconnect();

        calib_wr->abort();
        calib_th->wait();
        calib_wr->requestCalib();

    }
    else
    {
        setButtonCalibState(true,true);
        if (is_abort_calib)
        {
            if (debug) cout<<"Atención!! Se emitió una señal de aborto al AutoCalibThread: "<<mcae_th->currentThreadId()<<endl;
            emit sendCalibAbortCommand(true);
        }
        setIsAbortCalibFlag(true);
    }

    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr((string("El Cabezal no está respondiendo. Error: ")+string(ex.excdesc)).c_str()));

        if (debug) cout<<"No se puede Configurar: "<<ex.excdesc<<arpet->getTrama_MCAE()<<arpet->getEnd_PSOC() <<endl;
        setLabelState(false, hv_status_table[head_index-1], true);
    }
}

void MainWindow::connectPortArpet()
{
    // Devuelvo serial a arpet
    //cout<<"Devolviendo puerto serie de arpet..."<<endl;
    arpet->portConnect(port_name.toStdString().c_str());
}

void MainWindow::OffButtonCalib()
{
    setButtonCalibState(true,true);
}


/* Calibracion Fina */
/**
 * @brief MainWindow::on_pushButton_2_clicked
 */
void MainWindow::on_pushButton_2_clicked()
{
    QList<int> checked_Cab;
    QMessageBox messageBox;




    // Recupero los cabezales
    for(int i = 0; i < ui->frame_multihead_graph_3->children().size(); i++)
    {
        QCheckBox *q = qobject_cast<QCheckBox*>(ui->frame_multihead_graph_3->children().at(i));

        if(q->checkState() == Qt::Checked)
        {
            checked_Cab.append(i+1);
        }
    }
    if(checked_Cab.length() == 0)
    {
        messageBox.critical(0,"Error","No se ha seleccionado ni coincidencias ni ningún cabezal.");
        messageBox.setFixedSize(500,200);
        return;
    }
    calibrador->setCab_List(checked_Cab);

    if (ui->checkBox_count_skimm->checkState() == Qt::Checked) calibrador->setCount_skimming();
    if (ui->checkBox_count_skimm_2->checkState() == Qt::Checked) calibrador->setCount_skimming_total();

    // Invoco al calibracor
    calibrador->calibrar_fina();
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_2_clicked
 */
void MainWindow::on_pushButton_triple_ventana_2_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_1(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_1->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_3_clicked
 */
void MainWindow::on_pushButton_triple_ventana_3_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_2(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_2->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_4_clicked
 */
void MainWindow::on_pushButton_triple_ventana_4_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_3(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_3->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_6_clicked
 */
void MainWindow::on_pushButton_triple_ventana_6_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_4(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_4->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_7_clicked
 */
void MainWindow::on_pushButton_triple_ventana_7_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_5(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_5->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_5_clicked
 */
void MainWindow::on_pushButton_triple_ventana_5_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_6(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_6->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_8_clicked
 */
void MainWindow::on_pushButton_triple_ventana_8_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición cruda (*.raw)"));
    calibrador->setAdq_Coin(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_coin->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_9_clicked
 */
void MainWindow::on_pushButton_triple_ventana_9_clicked()
{
    QString root="Salidas/";

    QFileDialog dialog;
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir directorio de salida"),
                                                   root);

    calibrador->setPathSalida(filename+"/");
    cout<<"Salida: "<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_salida->setText(filename);
}

void MainWindow::on_parser_coincidencia_clicked()
{
    QString root="../../../Parser/";

    QFileDialog dialog;
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir carpeta del parser"),
                                                   root);
    calibrador->setPathPARSER(filename+"/");
    cout<<"Directorio parser: "<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_parser_coin->setText(filename);
}


/* Analizar Planar */
/**
 * @brief MainWindow::on_pushButton_3_clicked
 */
void MainWindow::on_pushButton_3_clicked()
{
    QList<int> checked_Cab, checked_met;
    QMessageBox messageBox;




    // Recupero los cabezales
    for(int i = 0; i < ui->frame_multihead_graph_4->children().size(); i++)
    {
        QCheckBox *q = qobject_cast<QCheckBox*>(ui->frame_multihead_graph_4->children().at(i));

        if(q->checkState() == Qt::Checked)
        {
            checked_Cab.append(i+1);
        }
    }
    if(checked_Cab.length() == 0)
    {
        messageBox.critical(0,"Error","No se ha seleccionado ningún cabezal.");
        messageBox.setFixedSize(500,200);
        return;
    }
    calibrador->setCab_List(checked_Cab);

    // Recupero los metodos
    for(int i = 0; i < ui->frame_multihead_graph_5->children().size(); i++)
    {
        QCheckBox *q = qobject_cast<QCheckBox*>(ui->frame_multihead_graph_5->children().at(i));

        if(q->checkState() == Qt::Checked)
        {
            checked_met.append(i);
        }
    }
    if(checked_met.length() == 0)
    {
        messageBox.critical(0,"Error","No se ha seleccionado ninguna visualizacion.");
        messageBox.setFixedSize(500,200);
        return;
    }
    calibrador->setVis_List(checked_met);

    // Invoco al visualizador
    calibrador->visualizar_planar();
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_13_clicked
 */
void MainWindow::on_pushButton_triple_ventana_13_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_1(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_8->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_10_clicked
 */
void MainWindow::on_pushButton_triple_ventana_10_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_2(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_10->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_11_clicked
 */
void MainWindow::on_pushButton_triple_ventana_11_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_3(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_7->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_16_clicked
 */
void MainWindow::on_pushButton_triple_ventana_16_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_4(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_9->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_15_clicked
 */
void MainWindow::on_pushButton_triple_ventana_15_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_5(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_12->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_12_clicked
 */
void MainWindow::on_pushButton_triple_ventana_12_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_6(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_adq_11->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_14_clicked
 */
void MainWindow::on_pushButton_triple_ventana_14_clicked()
{
    QString root="Salidas/";

    QFileDialog dialog;

    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir directorio de entrada"),
                                                   root);

    calibrador->setPathEntrada(filename+"/");
    cout<<"Salida: "<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_entrada->setText(filename);
}

/* FPGA */


/* RECONSTRUCCION */
/**
 * @brief MainWindow::on_pushButton_5_clicked
 */
void MainWindow::on_pushButton_5_clicked()
{
    QMessageBox messageBox;

    // Apretaron el boton, si estaba muerto lo vuelvo a la vida
    recon_externa->resetMuerto();



    // Checkeo que es lo que voy a hacer
    if (ui->checkBox_Mostrar->checkState() == Qt::Checked)
    {
        recon_externa->setMostrar();
    }
    else
    {
        recon_externa->resetMostrar();
    }
    if (ui->checkBox_parsear->checkState() == Qt::Checked)
    {
        recon_externa->setParsear();
    }
    else
    {
        recon_externa->resetParsear();
    }
    if (ui->checkBox_Server->checkState() == Qt::Checked)
    {
        recon_externa->setReconServer();
    }
    else
    {
        recon_externa->resetReconServer();
    }
    if (ui->checkBox_MLEM->checkState() == Qt::Checked)
    {
        recon_externa->setMLEM();
    }
    else
    {
        recon_externa->resetBackprojection();
    }
    if (ui->checkBox_Backprojection->checkState() == Qt::Checked)
    {
        recon_externa->setBackprojection();
    }
    else
    {
        recon_externa->resetBackprojection();
    }

    // Armo flago de reconstruccion
    if (recon_externa->getMLEM()  | recon_externa->getBackprojection()  )
    {
        recon_externa->setReconstruir();
    }
    else
    {
        recon_externa->resetReconstruir();
    }

    if (ui->checkBox_Reconstruir->checkState() == Qt::Checked)
    {
        recon_externa->setReconstruir();
    }
    else
    {
        recon_externa->resetReconstruir();
    }

    // Si parseo sin reconstruir no se como conectar el RAW al amide, asi que te mando a lrpmqtp
    if (recon_externa->getReconstruir() == 0 && recon_externa->getParsear() && recon_externa->getMostrar())
    {
        messageBox.critical(0,"Error","Metodos seleccionados incompatibles.");
        messageBox.setFixedSize(500,200);
        return;
    }




    if ((recon_externa->getReconstruir() == 0 )& (recon_externa->getParsear() == 0) & (recon_externa->getMostrar() == 0 )| (recon_externa->getReconstruir() == 0 & recon_externa->getReconServer()))
    {
        messageBox.critical(0,"Error","Seleccionar metodo.");
        messageBox.setFixedSize(500,200);
        return;
    }

    // Checkeo que los path existan
    if (!QDir(recon_externa->getPathAPIRL()).exists() & recon_externa->getReconstruir() & recon_externa->getReconServer() == 0)
    {
        messageBox.critical(0,"Error","Paths de APIRL invalido.");
        messageBox.setFixedSize(500,200);
        return;
    }
    // Si elegí reconstruir el server checkeo que el path sea un IP
    if (recon_externa->getReconServer())
    {
        QString dir_IP = ui->textBrowser_entrada_2->toPlainText();
        cout<<dir_IP.toStdString()<<endl;
        boost::system::error_code ec;
        boost::asio::ip::address::from_string( dir_IP.toStdString(), ec );
        if ( ec )
        {
            std::cerr << ec.message( ) << std::endl;
            messageBox.critical(0,"Error","Direccion de IP del server no valida.");
            messageBox.setFixedSize(500,200);
            return;
        }

        recon_externa->setServerIP(dir_IP);

    }
    if (!QDir(recon_externa->getPathINTERFILES()).exists() & recon_externa->getParsear() )
    {
        messageBox.critical(0,"Error","Paths de interfiles invalido.");
        messageBox.setFixedSize(500,200);
        return;
    }
    if (!QDir(recon_externa->getPathPARSER()).exists() & recon_externa->getParsear())
    {
        messageBox.critical(0,"Error","Paths de parser invalido.");
        messageBox.setFixedSize(500,200);
        return;
    }
    // Checkeo que los path existan
    if (!QDir(recon_externa->getPathSalida()).exists())
    {
        messageBox.critical(0,"Error","Paths de salida invalido.");
        messageBox.setFixedSize(500,200);
        return;
    }

    // Checkeo que se hallan seleccionado archivos
    if (recon_externa->getArchRecon() == "-")
    {
        messageBox.critical(0,"Error","Archivo a reconstruir, parsear o mostrar no seleccionado.");
        messageBox.setFixedSize(500,200);
        return;
    }
    else
    {
        recon_externa->setNombre_archivo( recon_externa->getArchRecon().split('.').first().split('/').last() );
    }

    if ((recon_externa->getArchInicial() == "-") &  recon_externa->getReconstruir() )
    {
        messageBox.critical(0,"Error","Archivo de imagen inicial no seleccionado.");
        messageBox.setFixedSize(500,200);
        return;
    }
    if ((recon_externa->getArchSensib() == "-" ) & recon_externa->getReconstruir() )
    {
        recon_externa->resetPreSensibilidad();
        messageBox.warning(0,"Warning","Se recalculara la sensibilidad.");
        messageBox.setFixedSize(500,200);
    }
    else
    {
        recon_externa->setPreSensibilidad();
    }
    if ((recon_externa->getArchCountSkimm() == "-" ) & recon_externa->getReconstruir() )
    {
        recon_externa->resetAplicarCountSkimming();
        messageBox.warning(0,"Warning","No se aplicara count skimming.");
        messageBox.setFixedSize(500,200);
    }
    else
    {
        recon_externa->setAplicarCountSkimming();
    }




    // Cargo los valores de los campos
    QString aux_str;
    aux_str = ui->Box_Emin->text();
    if(aux_str.toInt() < 0)
    {
        messageBox.critical(0,"Error","La enería minima debe ser superior a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setEmin(aux_str.toDouble());
    aux_str = ui->Box_Emax->text();
    if(aux_str.toInt() < recon_externa->getEmin())
    {
        messageBox.critical(0,"Error","La enería máxima debe ser mayor a la mínima.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setEmax(aux_str.toDouble());
    aux_str = ui->Box_Cant_anillos->text();
    if(aux_str.toInt() < 0)
    {
        messageBox.critical(0,"Error","La cantidad de anillos debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setCant_anillos(aux_str.toDouble());
    aux_str = ui->Box_Dif_anillos->text();
    if(aux_str.toInt() < 0)
    {
        messageBox.critical(0,"Error","La diferencia entre anillos debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setDif_anillos(aux_str.toDouble());
    aux_str = ui->Box_Span->text();
    if(aux_str.toInt() < 0)
    {
        messageBox.critical(0,"Error","El span debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setSpan(aux_str.toDouble());
    aux_str = ui->Box_cant_ang->text();
    if(aux_str.toInt() <= 0)
    {
        messageBox.critical(0,"Error","La cantidad de angulos debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setcant_ang(aux_str.toDouble());
    aux_str = ui->Box_cant_rhos->text();
    if(aux_str.toInt() <= 0)
    {
        messageBox.critical(0,"Error","La cantidad de rhos debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setcant_rhos(aux_str.toDouble());
    aux_str = ui->Box_max_Rho->text();
    if(aux_str.toInt() <= 0)
    {
        messageBox.critical(0,"Error","El Rho máximo debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setmax_Rho(aux_str.toDouble());
    aux_str = ui->Box_max_Z->text();
    if(aux_str.toInt() <= 0)
    {
        messageBox.critical(0,"Error","El Z máximo debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setmax_Z(aux_str.toDouble());
    aux_str = ui->Box_FOV_Axial->text();
    if(aux_str.toDouble() <= 0)
    {
        messageBox.critical(0,"Error","El FOV axial debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setFOV_Axial(aux_str.toDouble());
    aux_str = ui->Box_Min_dif_cab->text();
    if((aux_str.toInt() < 0) | (aux_str.toInt() > 2))
    {
        messageBox.critical(0,"Error","La minima diferencia entre detectores debe estar entre 0 y 2.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setMin_dif_cab(aux_str.toDouble());
    aux_str = ui->Box_Radio_PET->text();
    if(aux_str.toInt() <= 0)
    {
        messageBox.critical(0,"Error","El Radio del PET debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setRadio_PET(aux_str.toDouble());
    aux_str = ui->Box_Radio_FOV->text();
    if((aux_str.toDouble() <= 0) | (aux_str.toDouble() > recon_externa->getRadio_PET()))
    {
        messageBox.critical(0,"Error","El Radio del FOV debe ser mayor a 0 y menor al FOV del PET.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setRadio_FOV(aux_str.toDouble());
    aux_str = ui->Box_zona_muerta->text();
    if(aux_str.toInt() < 0)
    {
        messageBox.critical(0,"Error","La zona muerta debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setzona_muerta(aux_str.toDouble());
    aux_str = ui->Box_Iteraciones->text();
    if(aux_str.toInt() < 0)
    {
        messageBox.critical(0,"Error","La cantidad de iteraciones debe ser mayor a 0.");
        messageBox.setFixedSize(500,200);
        return;
    }
    recon_externa->setIteraciones(aux_str.toDouble());


    // Reservo memoria para los procesos


    if (recon_externa->getParsear() == 1)
    {
        recon_externa->SetearListasProcesos();

        ui->plainTextEdit_Recon_console->appendPlainText("Parseando archivo:");
        ui->plainTextEdit_Recon_console->appendPlainText(recon_externa->getArchRecon());

        recon_externa->Parsear();

        //ui->plainTextEdit_Recon_console->appendPlainText("Parseado finalizado.");
    }

    if (recon_externa->getReconstruir())
    {
        // Si voy a parsear, bloqueo el proceso y lo ejecuto cuando termine el parser
        if (recon_externa->getParsear())
        {
            // Y me pongo a esperar...
            ui->plainTextEdit_Recon_console->appendPlainText("Esperando el fin del Parseado");
            recon_externa->loop_parser.exec();
            // Si me mataron el proceso durante la espera...
            if (recon_externa->getMuerto())
                return;
            ui->plainTextEdit_Recon_console->appendPlainText("Parseado finalizado.");
            recon_externa->resetParsear();
            // Apago el loop
            recon_externa->loop_parser.exit();
        }

        ui->plainTextEdit_Recon_console->appendPlainText("Reconstruyendo archivo:");
        ui->plainTextEdit_Recon_console->appendPlainText(recon_externa->getArchRecon());

        recon_externa->SetearListasProcesos();

        recon_externa->Reconstruir();

        //ui->plainTextEdit_Recon_console->appendPlainText("Reconstruccion finalizada.");

    }


    if (recon_externa->getMostrar())
    {
        // Si voy a mostrar algo que reconstrui, ¡espero!
        if (recon_externa->getReconstruir())
        {
            // Y me pongo a esperar...
            ui->plainTextEdit_Recon_console->appendPlainText("Esperando el fin del la reconstruccion");
            recon_externa->loop_reconstruccion.exec();
            // Si me mataron el proceso durante la espera...
            if (recon_externa->getMuerto())
                return;
            recon_externa->resetReconstruir();
            ui->plainTextEdit_Recon_console->appendPlainText("Reconstruccion finalizada, ¡a mostrar!");
            // Apago el loop
            recon_externa->loop_reconstruccion.exit();
        }

        ui->plainTextEdit_Recon_console->appendPlainText("Mostrando archivo:");
        ui->plainTextEdit_Recon_console->appendPlainText(recon_externa->getArchRecon());

        recon_externa->SetearListasProcesos();

        recon_externa->Mostrar();

        //ui->plainTextEdit_Recon_console->appendPlainText("Mostrandocion finalizada.");

    }
    //m_logFile.write(text); // Logs to file

}
/**
 * @brief MainWindow::on_pushButton_APIRL_PATH_clicked
 */
void MainWindow::on_pushButton_APIRL_PATH_clicked()
{
    QString root=recon_externa->getPathAPIRL();

    QFileDialog dialog;
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir carpeta del build de APIRL"),
                                                   root);

    recon_externa->setPathAPIRL(filename+"/");


    cout<<"Directorio APIRL: "<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_entrada_2->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_INTERFILES_clicked
 */
void MainWindow::on_pushButton_INTERFILES_clicked()
{
    QString root=recon_externa->getPathINTERFILES();

    QFileDialog dialog;
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir carpeta de scripts de interfiles"),
                                                   root);

    recon_externa->setPathINTERFILES(filename+"/");


    cout<<"Directorio INTERFILES: "<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_entrada_3->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_arch_recon_clicked
 */
void MainWindow::on_pushButton_arch_recon_clicked()
{

    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw);; Interfiles (*.h33)"));
    recon_externa->setArchRecon(filename);
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_archivo_recon->setText(filename);

}
/**
 * @brief MainWindow::on_pushButton_Est_ini_clicked
 */
void MainWindow::on_pushButton_Est_ini_clicked()
{

    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de estimacion inicial"),
                                                    root_calib_path,
                                                    tr("Interfiles (*.h33)"));
    recon_externa->setArchInicial(filename);
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_estimacion_ini->setText(filename);

}
/**
 * @brief MainWindow::on_pushButton_Arch_sens_clicked
 */
void MainWindow::on_pushButton_Arch_sens_clicked()
{

    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de sensibilidad"),
                                                    root_calib_path,
                                                    tr("Interfiles (*.h33)"));
    recon_externa->setArchSensib(filename);
    cout<<filename.toStdString()<<endl;

    if (filename!="")
      ui->textBrowser_Imagensensib->setText(filename);


}
/**
 * @brief MainWindow::on_pushButton_Arch_count_skimming_clicked
 */
void MainWindow::on_pushButton_Arch_count_skimming_clicked()
{

    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de Count Skimming"),
                                                    root_calib_path,
                                                    tr("Conut Skimming (*.csv)"));
    recon_externa->setArchCountSkimm(filename);
    cout<<filename.toStdString()<<endl;
    if (filename!="")
      ui->textBrowser_Conunt_skimming->setText(filename);

}
/**
 * @brief MainWindow::on_pushButton_INTERFILES_2_clicked
 */
void MainWindow::on_pushButton_INTERFILES_2_clicked()
{
    QString root=recon_externa->getPathSalida();

    QFileDialog dialog;
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir carpeta de salida"),
                                                   root);

    recon_externa->setPathSalida(filename+"/");


    cout<<"Directorio Salida: "<<filename.toStdString()<<endl;
    if (filename!="")
      ui->textBrowser_entrada_4->setText(filename);
}
/**
 * @brief MainWindow::on_pushButton_INTERFILES_3_clicked
 */
void MainWindow::on_pushButton_INTERFILES_3_clicked()
{
    QString root=recon_externa->getPathPARSER();

    QFileDialog dialog;
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir carpeta del parser"),
                                                   root);

    recon_externa->setPathPARSER(filename+"/");


    cout<<"Directorio parser: "<<filename.toStdString()<<endl;
    if (filename!="")
      ui->textBrowser_entrada_5->setText(filename);
}
/**
 * @brief MainWindow::on_checkBox_MLEM_clicked
 * @param checked
 */
void MainWindow::on_checkBox_MLEM_clicked(bool checked)
{
    ui->checkBox_Backprojection->setCheckState( Qt::Unchecked);
}
/**
 * @brief MainWindow::on_checkBox_Backprojection_clicked
 * @param checked
 */
void MainWindow::on_checkBox_Backprojection_clicked(bool checked)
{
    ui->checkBox_MLEM->setCheckState( Qt::Unchecked);
}
/**
 * @brief MainWindow::on_pushButton_6_clicked
 */
void MainWindow::on_pushButton_6_clicked()
{
    recon_externa->matar_procesos();
}

/* CUIPET */
/**
 * @brief MainWindow::on_pushButton_aqd_file_open_clicked
 */
void MainWindow::on_pushButton_aqd_file_open_clicked()
{
    QString directory = openDirectory();
    ui->lineEdit_aqd_path_file->setText(directory);
}

/**
 * @brief MainWindow::on__2_clicked
 */
void MainWindow::on_pushButton_select_pmt_2_clicked()
{
    writeFooterAndHeaderDebug(true);
    arpet->resetHitsMCA();
    setHitsInit(true);
    removeAllGraphsPMT();

    int ret = pmt_select_autocalib->exec();
    QList<QString> qlist = pmt_select_autocalib->GetPMTSelectedList();

    if(ret == QDialog::Accepted)
    {
        //setPMTSelectedList(qlist);
        setPMTSelectedListAutocalib(qlist);
    }

    if(debug)
    {
        cout<<"La lista seleccionada tiene "<< qlist.size()<<" elementos"<<endl;
        QList<QString>::const_iterator stlIter;
        for( stlIter = qlist.begin(); stlIter != qlist.end(); ++stlIter )
            cout<<(*stlIter).toStdString()<<endl;
    }

    setPMTCustomPlotEnvironment(qlist);

    qSort(qlist);
    ui->listWidget_2->clear();
    ui->listWidget_2->addItems(qlist);

    writeFooterAndHeaderDebug(false);
}
/**
 * @brief MainWindow::on_pushButton_tiempos_cabezal_2_clicked
 */
void MainWindow::on_pushButton_tiempos_cabezal_2_clicked()
{
    QString fileName = openConfigurationFile();
    if (fileName!="")
      ui->textBrowser_tiempos_Inter_cabezal->setText(fileName);
}



/**
 * @brief MainWindow::on_comboBox_head_mode_select_graph_2_currentIndexChanged
 * @param index
 */
void MainWindow::on_comboBox_head_mode_select_graph_2_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        ui->comboBox_head_select_graph_3->show();
       // ui->frame_multihead_graph_2->hide();
        break;
    case 1:
        ui->comboBox_head_select_graph_3->hide();
        ui->frame_multihead_graph_2->show();
        ui->checkBox_mca_12->setChecked(false);
        ui->checkBox_mca_11->setChecked(false);
        ui->checkBox_mca_10->setChecked(false);
        ui->checkBox_mca_9->setChecked(false);
        ui->checkBox_mca_7->setChecked(false);
        ui->checkBox_mca_8->setChecked(false);
        break;
    case 2:
        ui->comboBox_head_select_graph_3->hide();
        ui->frame_multihead_graph_2->show();

        ui->checkBox_mca_12->setChecked(true);
        ui->checkBox_mca_11->setChecked(true);
        ui->checkBox_mca_10->setChecked(true);
        ui->checkBox_mca_9->setChecked(true);
        ui->checkBox_mca_7->setChecked(true);
        ui->checkBox_mca_8->setChecked(true);
        break;
    default:
        break;
    }
}

/**
 * @brief MainWindow::setTabLog
 * @param index
 */
void MainWindow::setTabLog(int index) {
    if(index==0) {
        ui->comboBox_head_select_graph_3->show();
       // ui->frame_multihead_graph_2->hide();
    }
}

/**
 * @brief MainWindow::on_checkBox_temp_log_toggled
 * @param checked
 */
void MainWindow::on_checkBox_temp_log_toggled(bool checked) {
    if (checked) {
       // ui->on_calendarWidget_selectionChanged();
        on_calendarWidget_selectionChanged();
    }
}

/**
 * @brief MainWindow::on_ChechBox_Pico_toggled
 * @param checked
 */
void MainWindow::on_ChechBox_Pico_toggled(bool checked)
{
  if (checked){
      ui->checkBox_Rate_Coin->setChecked(false);
      ui->checkBox_rate_log->setChecked(false);
      on_calendarWidget_selectionChanged();
  }
}

/**
 * @brief MainWindow::on_checkBox_Rate_Coin_toggled
 * @param checked
 */
void MainWindow::on_checkBox_Rate_Coin_toggled(bool checked)
{
  if (checked){
      ui->checkBox_rate_log->setChecked(false);
      ui->ChechBox_Pico->setChecked(false);
      on_calendarWidget_selectionChanged();
  }
}

/**
 * @brief MainWindow::on_checkBox_rate_log_toggled
 * @param checked
 */
void MainWindow::on_checkBox_rate_log_toggled(bool checked)
{
    if (checked){
        ui->checkBox_Rate_Coin->setChecked(false);
        ui->ChechBox_Pico->setChecked(false);
        on_calendarWidget_selectionChanged();
    }
}

/**
 * @brief MainWindow::getLogFromFiles
 *
 * Analizador de archivo de texto
 *
 * @param filename
 * @param hv
 * @return Vector con los valores obtenidos del archivo
 */
QStringList MainWindow::getLogFromFiles(QString filename,QRegExp rx, string parser)
{
    QString values;
    QFile inputFile(filename);

    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);

            while (!in.atEnd())
            {
                QString line = in.readLine();
                while ((-1)==line.toStdString().find(parser)&&!in.atEnd()){
                   line = in.readLine();
                }
                if (in.atEnd()) break;
                values+=line+",";
            }

        inputFile.close();
    }

    return values.split(rx, QString::SkipEmptyParts);
}

void MainWindow::on_pushButton_p_51_clicked()
{
    error_code error_code;
    port_name=Cab+ ui->comboBox_head_select_graph->currentText();
    writeFooterAndHeaderDebug(true);

    if (pmt_selected_list.isEmpty())
    {
        QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
        if(debug)
        {
            cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
            writeFooterAndHeaderDebug(false);
        }
        return;
    }

    QString q_msg;
    try
    {
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
            throw exception_Cabezal_Apagado;
        }
        for(int i=0;i < pmt_selected_list.length();i++)
        {
            q_msg =setHV(getHead("mca").toStdString(),QString::number(3500).toStdString(),pmt_selected_list.at(i).toStdString());
        }
        setHV(getHead("mca").toStdString(), ui->lineEdit_limiteinferior->text().toStdString());
        ui->label_data_output->setText("| Canal configurado en todos los PMT: 350 |");
    }
    catch (Exceptions ex)
    {
        if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
        ui->label_data_output->setText("Error en la configuración de la tensión de dinodo.");
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }
    if (debug)
    {
        showMCAEStreamDebugMode(q_msg.toStdString());
        writeFooterAndHeaderDebug(false);
    }
}

void MainWindow::on_RATECAB1_clicked()
{
  vector<int> rates(3);

    try{
        arpet->portDisconnect();
        port_name=Cab1;
        arpet->portConnect(port_name.toStdString().c_str());
        rates = arpet->getRate(QString::number(1).toStdString(), port_name.toStdString());
        ui->label_CAB1->setText(QString::number(rates.at(0)) + " "+QString::number(rates.at(1))+" "+QString::number(rates.at(2)));
        arpet->portDisconnect();
  }
  catch(Exceptions &ex){

  }
}

void MainWindow::on_RATECAB2_clicked()
{
    vector<int> rates(3);
    try{
        arpet->portDisconnect();
        port_name=Cab2;
        arpet->portConnect(port_name.toStdString().c_str());
        rates = arpet->getRate(QString::number(2).toStdString(), port_name.toStdString());
        ui->label_CAB2->setText(QString::number(rates.at(0)) + " "+QString::number(rates.at(1))+" "+QString::number(rates.at(2)));
        arpet->portDisconnect();
    }
    catch(Exceptions &ex){

    }
}


void MainWindow::on_RATECAB3_clicked()
{
  vector<int> rates(3);
  try{
      arpet->portDisconnect();
      port_name=Cab3;
      arpet->portConnect(port_name.toStdString().c_str());
      rates = arpet->getRate(QString::number(3).toStdString(), port_name.toStdString());
      ui->label_CAB3->setText(QString::number(rates.at(0)) + " "+QString::number(rates.at(1))+" "+QString::number(rates.at(2)));
      arpet->portDisconnect();
  }
  catch(Exceptions &ex){

  }
}

void MainWindow::on_RATECAB4_clicked()
{
  vector<int> rates(3);

  try{
      arpet->portDisconnect();
      port_name=Cab4;
      arpet->portConnect(port_name.toStdString().c_str());
      rates = arpet->getRate(QString::number(4).toStdString(), port_name.toStdString());
      ui->label_CAB4->setText(QString::number(rates.at(0)) + " "+QString::number(rates.at(1))+" "+QString::number(rates.at(2)));
      arpet->portDisconnect();
  }
  catch(Exceptions &ex){

  }
}

void MainWindow::on_RATECAB5_clicked()
{
  vector<int> rates(3);
  try{
      arpet->portDisconnect();
      port_name=Cab5;
      arpet->portConnect(port_name.toStdString().c_str());
      rates = arpet->getRate(QString::number(5).toStdString(), port_name.toStdString());
      ui->label_CAB5->setText(QString::number(rates.at(0)) + " "+QString::number(rates.at(1))+" "+QString::number(rates.at(2)));
      arpet->portDisconnect();
    }
    catch(Exceptions &ex){

    }
}

void MainWindow::on_RATECAB6_clicked()
{
  vector<int> rates(3);
  try{
      arpet->portDisconnect();
      port_name=Cab6;
      arpet->portConnect(port_name.toStdString().c_str());
      rates = arpet->getRate(QString::number(6).toStdString(), port_name.toStdString());
      ui->label_CAB6->setText(QString::number(rates.at(0)) + " "+QString::number(rates.at(1))+" "+QString::number(rates.at(2)));
      arpet->portDisconnect();
    }
    catch(Exceptions &ex){

    }
}


void MainWindow::on_pushButton_adquirir_clicked()
{
  bool centroide = ui->checkBox_centroid->isChecked();
  bool espectro_calib = ui->checkBox_espectro_calibrado->isChecked();

  QList<int> checkedHeads;

    {

     bMutex.tryLock();
      {

          switch (adquire_mode)
          {
          case PMT:
              if (pmt_selected_list.isEmpty())
              {


                  writeFooterAndHeaderDebug(true);
                  //setButtonAdquireState(false);
                  setIsAbortMCAEFlag(false);
                  if(debug) cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
                  QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
                  emit ToPushButtonAdquirir(false);
                  mcae_wr->abort();
                  //setButtonAdquireState(true, true);
                  writeFooterAndHeaderDebug(false);
                  //bMutex.unlock();
                  break;
              }

              checkedHeads.clear();
              checkedHeads.append(ui->comboBox_head_select_graph->currentText().toInt());
              if(debug) cout<<"Cabezal: "<<checkedHeads.at(0)<<endl;
              if(debug) cout<<"iteracion: "<<checkedHeads.length()<<endl;
              mcae_wr->setCheckedHeads(checkedHeads);
              ui->specPMTs->clearGraphs();
              mcae_wr->setPMTSelectedList(pmt_selected_list);
              mcae_wr->setDebugMode(debug);
              mcae_wr->setModeBool(true);
              mcae_wr->setCentroidMode(centroide);
              //mcae_wr->abort();
              mcae_th->wait();
              mcae_wr->requestMCA();

              break;
          case CABEZAL:

              checkedHeads= getCheckedHeads();

              mcae_wr->setCheckedHeads(checkedHeads);

              ui->specHead->clearGraphs();
              //setButtonAdquireState(true);
              mcae_wr->setDebugMode(debug);
              mcae_wr->setModeBool(false);
              //double lala = calibrador->Buscar_Pico(, 256);
              mcae_wr->setModeCabCalib(espectro_calib);
              mcae_wr->abort();
              mcae_th->wait();
              mcae_wr->requestMCA();
              setIsAbortMCAEFlag(true);

              break;
          case TEMPERATURE:

              drawTemperatureBoard();
              emit ToPushButtonAdquirir(false);
              break;
          default:
              break;
          }

      }
      switch (adquire_mode)
      {
      case PMT...CABEZAL:
          //setButtonAdquireState(true,true);
          if (is_abort_mcae)
          {
              if (debug) cout<<"Atención!! Se emitió una señal de aborto al thread: "<<mcae_th->currentThreadId()<<endl;
              emit sendAbortMCAECommand(true);
          }
          setIsAbortMCAEFlag(true);
          break;
      case TEMPERATURE:
          break;
      default:
          break;
      }

      emit ToPushButtonAdquirir(false);
      bMutex.unlock();
      cout<<"fin del handler del boton"<<endl;
 }
}


void MainWindow::on_calendarWidget_selectionChanged()
{

  ui->specPMTs_3->setInteraction(QCP::iRangeDrag, true);
  QDate nombreunivoco=ui->calendarWidget->selectedDate();
  QTime hora(0, 0, 0);
  initfile =nombreunivoco.toString("yyyyMMdd");
  initfile= root_log_path+"/"+"LOG"+initfile;

  try{

    QRegExp rx("[,]");
    int graph=0;
    QStringList lista,listaRate;
    ui->specPMTs_3->clearGraphs();

    if (ui->checkBox_temp_log->isChecked()) {

      for(int i=0;i<24;i++){
         hora.setHMS(i,0,0);
         initfile=root_log_path+"/"+"LOG"+nombreunivoco.toString("yyyyMMdd")+hora.toString("hh")+".log";
         if(fileExists(initfile)){
           cout<< QString(initfile).toStdString()<< endl;
           lista.append(getLogFromFiles(initfile,rx, "[LOG-TEMP]"));
         }
      }

      if (lista.length()!=0){
        /////////////////////////////////////////////////////////////////////////////////////////////////
        // Configuracion de la trama:
        ///   ENCABEZADO-FECHA--------------------CABEZAL---NUMCABEZAL--VALOR_MIN--VALOR_MED--VALOR_MAX//
        //    [LOG-TEMP],Tue May 30 06:55:25 2017,Cabezal,  4,          27.375,    35.1393,   44.5625
        //              "ddd MMM dd HH:mm:ss yyyy"
        /////////////////////////////////////////////////////////////////////////////////////////////////

        QDateTime FechaMedicionInterna;
        FechaMedicionInterna= QDateTime::fromString( lista.at(1),Qt::RFC2822Date);

        // generate some data:
        QVector<double> x;//(lista.length()/7);
        QVector<double> ymed;//(lista.length()/7); // initialize with entries 0..100
        QVector<double> ymin;//(lista.length()/7); // initialize with entries 0..100
        QVector<double> ymax;//(lista.length()/7); // initialize with entries 0..100

        QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
        dateTicker->setDateTimeFormat("ddd MMM d HH:mm:ss yyyy");
        ui->specPMTs_3->xAxis->setTicker(dateTicker);

        for (int i=0; i<(lista.length()/7); i++){
            if (QString(lista.at(3+i*7)).toInt()==ui->comboBox_head_select_graph_3->currentText().toInt()){
                QDateTime tmp = QDateTime::fromString((lista.at(1+i*7)),"ddd MMM d HH:mm:ss yyyy");
                cout<<QString(lista.at(1+i*7)).toStdString()<<endl;
                x.append(QCPAxisTickerDateTime::dateTimeToKey(tmp));
                ymed.append( QString(lista.at(5+i*7)).toDouble()); // let's plot a quadratic function
                ymin.append(QString(lista.at(4+i*7)).toDouble()); // let's plot a quadratic function
                ymax.append( QString(lista.at(6+i*7)).toDouble()); // let's plot a quadratic function
              }
          }

        for (int i=0; i<x.length();i++){
            //cout<<QString::number(x[i]).toStdString() <<"  "<< QString::number(i).toStdString()<<endl;
            ymin[i]=ymed[i]-ymin[i];
            ymax[i]=ymax[i]-ymed[i];
          }

        // create graph and assign data to it:
        ui->specPMTs_3->addGraph();

        ui->specPMTs_3->graph(graph)->setData(x,ymed);
        ui->specPMTs_3->graph(graph)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
        QCPErrorBars *errorBars = new QCPErrorBars(ui->specPMTs_3->xAxis, ui->specPMTs_3->yAxis);
        errorBars->removeFromLegend();
        ////void QCPErrorBars::setData ( const QVector< double > &  errorMinus, const QVector< double > &  errorPlus  )
        errorBars->setData(ymin,ymax);
        errorBars->setDataPlottable(ui->specPMTs_3->graph(graph));

        ui->specPMTs_3->xAxis->setLabel("Tiempo");
        ui->specPMTs_3->yAxis->setLabel("ºC");
        // set axes ranges, so we see all data:
        ui->specPMTs_3->rescaleAxes();
        ui->specPMTs_3->replot();

        x.clear();
        ymed.clear();
        ymax.clear();
        ymin.clear();
        lista.clear();
        graph++;
      }
        else {
            QMessageBox::critical(this,tr("Atención"),tr(string("No se encuentra el archivo o está vacio:" +QString(initfile).toStdString()).c_str()));
        }
      }

    if (ui->checkBox_rate_log->isChecked()) {

        for(int i=0;i<24;i++){
           hora.setHMS(i,0,0);
           initfile=root_log_path+"/"+"LOG"+nombreunivoco.toString("yyyyMMdd")+hora.toString("hh")+".log";
           if(fileExists(initfile)){
             cout<< QString(initfile).toStdString()<< endl;
             lista.append(getLogFromFiles(initfile,rx, "[LOG-RATE]"));
           }
        }

        if (lista.length()!=0){
          /////////////////////////////////////////////////////////////////////////////////////////////////
          // Configuracion de la trama:
          ///   ENCABEZADO-FECHA--------------------CABEZAL---NUMCABEZAL--VALOR_MIN--VALOR_MED--VALOR_MAX--OFFSET
          //    [LOG-RATE],Tue May 30 06:55:25 2017,Cabezal,  4,          0,         168,       0,         0
          /////////////////////////////////////////////////////////////////////////////////////////////////
          cout<<"---------------------------------";
          cout<<"fecha: "+lista.at(1).toStdString();
          cout<<"---------------------------------";

          QVector<double>x ,y;
          QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
          dateTicker->setDateTimeFormat("ddd MMM d HH:mm:ss yyyy");
          ui->specPMTs_3->xAxis->setTicker(dateTicker);

          for (int i=0; i<(lista.length()/7); i++){
              if (QString(lista.at(3+i*7)).toInt()==ui->comboBox_head_select_graph_3->currentText().toInt()){
                QDateTime tmp = QDateTime::fromString((lista.at(1+i*7)),"ddd MMM d HH:mm:ss yyyy");
                x.append(QCPAxisTickerDateTime::dateTimeToKey(tmp));
                if (ui->checkBox_temp_log->isChecked()) {
                  y.append( QString(lista.at(5+i*7)).toDouble()/1000); // let's plot a quadratic function
                  ui->specPMTs_3->xAxis2->setLabel("Tempura");
                }
                else y.append( QString(lista.at(5+i*7)).toDouble()); // let's plot a quadratic function
                }
          }

          ui->specPMTs_3->addGraph();

          ui->specPMTs_3->graph(graph)->setData(x, y);

          ui->specPMTs_3->yAxis->setTickLength(3, 3);
          ui->specPMTs_3->yAxis->setSubTickLength(1, 1);
          // give the axes some labels:
          ui->specPMTs_3->xAxis->setLabel("Tiempo");
          ui->specPMTs_3->yAxis->setLabel("Tasa");
          // set axes ranges, so we see all data:
          ui->specPMTs_3->rescaleAxes();
          ui->specPMTs_3->replot();
          x.clear();
          lista.clear();
        }
        else {
             QMessageBox::critical(this,tr("Atención"),tr(string("No se encuentra el archivo o está vacio:" +QString(initfile).toStdString()).c_str()));
        }
      }

    if (ui->checkBox_Rate_Coin->isChecked()){

        for(int i=0;i<24;i++){
           hora.setHMS(i,0,0);
           initfile=root_log_path+"/"+"LOG"+nombreunivoco.toString("yyyyMMdd")+hora.toString("hh")+".log";
           if(fileExists(initfile)){
             cout<< QString(initfile).toStdString()<< endl;
             lista.append(getLogFromFiles(initfile,rx, "[LOG-RATECOIN]"));
           }
        }

        if (lista.length()!=0){

          /////////////////////////////////////////////////////////////////////////////////////////////////
          // Configuracion de la trama:
          ///   ENCABEZADO-----FECHA--------------------MAQUINAS
          //    [LOG-RATECOIN],Thu Oct 26 12:59:45 2017,0,0,0,0,0,0,0,0,0
          /////////////////////////////////////////////////////////////////////////////////////////////////
          cout<<"---------------------------------";
          cout<<"fecha: "+lista.at(1).toStdString();
          cout<<"---------------------------------";

          QVector<double>x ,y,fwhm;
          QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
          dateTicker->setDateTimeFormat("ddd MMM d HH:mm:ss yyyy");
          ui->specPMTs_3->xAxis->setTicker(dateTicker);

          for (int i=0; i<(lista.length()/7); i++){
              if (QString(lista.at(3+i*7)).toInt()==ui->comboBox_head_select_graph_3->currentText().toInt()){
                QDateTime tmp = QDateTime::fromString((lista.at(1+i*7)),"ddd MMM d HH:mm:ss yyyy");
                x.append(QCPAxisTickerDateTime::dateTimeToKey(tmp));
                y.append( QString(lista.at(4+i*7)).toDouble());
                fwhm.append(QString(lista.at(5+i*7)).toDouble());
                }
          }
  //        // create graph and assign data to it:
          ui->specPMTs_3->clearGraphs();
          ui->specPMTs_3->addGraph();

          ui->specPMTs_3->graph(0)->setData(x, y);
          ui->specPMTs_3->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
          QCPErrorBars *errorBars = new QCPErrorBars(ui->specPMTs_3->xAxis, ui->specPMTs_3->yAxis);
          errorBars->removeFromLegend();
          ////void QCPErrorBars::setData ( const QVector< double > &  errorMinus, const QVector< double > &  errorPlus  )
          errorBars->setData(fwhm,fwhm);
          errorBars->setDataPlottable(ui->specPMTs_3->graph(0));
          // give the axes some labels:
          ui->specPMTs_3->xAxis->setLabel("Tiempo");
          ui->specPMTs_3->yAxis->setLabel("Tasa");
          // set axes ranges, so we see all data:
          ui->specPMTs_3->rescaleAxes();
          ui->specPMTs_3->replot();
          lista.clear();

        }
        else {
             QMessageBox::critical(this,tr("Atención"),tr(string("No se encuentra el archivo o está vacio:" +QString(initfile).toStdString()).c_str()));
        }
      }

    if (ui->ChechBox_Pico->isChecked()){

        for(int i=0;i<24;i++){
           hora.setHMS(i,0,0);
           initfile=root_log_path+"/"+"LOG"+nombreunivoco.toString("yyyyMMdd")+hora.toString("hh")+".log";
           if(fileExists(initfile)){
             lista.append(getLogFromFiles(initfile,rx, "[LOG-PICO]"));
           }
        }

        if (lista.length()!=0){
          /////////////////////////////////////////////////////////////////////////////////////////////////
          // Configuracion de la trama:
          ///   ENCABEZADO-FECHA--------------------CABEZAL---NUMCABEZAL--Pico------FWHM
          //    [LOG-PICO],Fri Oct 27 16:12:18 2017,Cabezal,  4,          124.978,  13.6024
          /////////////////////////////////////////////////////////////////////////////////////////////////

          QVector<double>x ,y,fwhm;
          QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
          dateTicker->setDateTimeFormat("ddd MMM d HH:mm:ss yyyy");
          ui->specPMTs_3->xAxis->setTicker(dateTicker);

          for (int i=0; i<(lista.length()/7); i++){
              if (QString(lista.at(3+i*7)).toInt()==ui->comboBox_head_select_graph_3->currentText().toInt()){
                QDateTime tmp = QDateTime::fromString((lista.at(1+i*7)),"ddd MMM d HH:mm:ss yyyy");
                x.append(QCPAxisTickerDateTime::dateTimeToKey(tmp));
                y.append( QString(lista.at(4+i*7)).toDouble());
                fwhm.append(QString(lista.at(5+i*7)).toDouble());
                }
          }

  //        // create graph and assign data to it:
          ui->specPMTs_3->addGraph();

          ui->specPMTs_3->graph(graph)->setData(x, y);
          ui->specPMTs_3->graph(graph)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
          QCPErrorBars *errorBars = new QCPErrorBars(ui->specPMTs_3->xAxis, ui->specPMTs_3->yAxis);
          errorBars->removeFromLegend();
          ////void QCPErrorBars::setData ( const QVector< double > &  errorMinus, const QVector< double > &  errorPlus  )
          errorBars->setData(fwhm,fwhm);
          errorBars->setDataPlottable(ui->specPMTs_3->graph(graph));
          // give the axes some labels:
          ui->specPMTs_3->yAxis2->setLabel("Tasa");
          // set axes ranges, so we see all data:
          ui->specPMTs_3->rescaleAxes();
          ui->specPMTs_3->replot();
          lista.clear();
          x.clear();
          graph++;
        }
        else {
             QMessageBox::critical(this,tr("Atención"),tr(string("No se encuentra el archivo o está vacio:" +QString(initfile).toStdString()).c_str()));
        }
      }
  }

  catch(Exceptions ex){
    cout<< QString(initfile).toStdString()<< endl;
    QMessageBox::critical(this,tr("Atención"),tr((string("No se puede abrir el archivo:" +QString(initfile).toStdString()+"Error: ")+string(ex.excdesc)).c_str()));
  }

}

bool MainWindow::fileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile()) {
        return true;
    } else {
          return false;
    }
}



/**
 * @brief MainWindow::updateCaption Este es un timer que actualiza la lista de cabezales disponible y habilita o deshabilita los checkbox de cabezales
 *
 */
void MainWindow::updateCaption(){

    QDir dir("/dev/");
    QString numerocabezal;
    QStringList filters;
    static int inicio=1;
    static QVector<int> Estado_Aux_Cabezales;
    QRegExp  RegExp("(-?\\d+(?:[\\.,]\\d+(?:e\\d+)?)?)");
    filters << "UART*";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::System);
    QFileInfoList list = dir.entryInfoList();
    int size = 0;
    QString mensaje;

    if(adq_running){

        QString input, output,tasa,error;

        QProcess ver_tasa;
        ver_tasa.waitForStarted();
        ver_tasa.start("cat ./log_ETH.info");
        ver_tasa.waitForFinished(-1);

        output=(ver_tasa.readAllStandardOutput());
        tasa=output.left(output.indexOf("Tramas"));
        if(tasa.toInt() > 0 )
        ui->label_adq_tasa_2->setText(tasa + "Tramas/seg");
        error=output.mid(output.indexOf("con") + 3);
        error = error.left(error.indexOf("\n"));
        if(!error.isEmpty())
        ui->label_adq_error_2->setText(error);
        ver_tasa.close();


        QProcess size_of_adq;



        size_of_adq.waitForStarted();
        input= "du -h -m "+nombre_archivo_adq+".raw";

        size_of_adq.start(input);
        size_of_adq.waitForFinished(10000);
        output=(size_of_adq.readAllStandardOutput());
        output=output.left(output.indexOf("\t"));
        size_of_adq.close();
        if(output.toInt() > 0 )
        ui->progressBar->setValue(output.toInt());

    }



    Estado_Aux_Cabezales.clear();
    for (int i=0;i<list.length();i++){
        RegExp.indexIn(list.at(i).absoluteFilePath());
        numerocabezal=RegExp.capturedTexts().at(0);
        if( numerocabezal!="" ){
            Estado_Aux_Cabezales.push_back(numerocabezal.toInt());
        }
    }
    Estado_Cabezales=Estado_Aux_Cabezales;

    if (Estado_Cabezales.contains(1)){
        ui->checkBox_c_1->setEnabled(true);
        ui->checkBox_mca_1->setEnabled(true);
        ui->pushButton_Encendido_1->setChecked(true);


    }else{
        ui->checkBox_c_1->setEnabled(false);
        ui->checkBox_mca_1->setEnabled(false);
        ui->checkBox_c_1->setChecked(false);
        ui->pushButton_Encendido_1->setChecked(false);

       // setButtonState(false,ui->pushButton_Encendido_1,false);
    }
    if (Estado_Cabezales.contains(2)){
        ui->checkBox_c_2->setEnabled(true);
        ui->checkBox_mca_2->setEnabled(true);
        ui->pushButton_Encendido_2->setChecked(true);

        //setButtonState(true,ui->pushButton_Encendido_2,false);
    }else{
        ui->checkBox_c_2->setEnabled(false);
        ui->checkBox_mca_2->setEnabled(false);
        ui->checkBox_c_2->setChecked(false);
        ui->pushButton_Encendido_2->setChecked(false);

        //setButtonState(false,ui->pushButton_Encendido_2,false);
    }
    if (Estado_Cabezales.contains(3)){
        ui->checkBox_c_3->setEnabled(true);
        ui->checkBox_mca_3->setEnabled(true);
        ui->pushButton_Encendido_3->setChecked(true);

        //setButtonState(true,ui->pushButton_Encendido_3,false);
    }else{
        ui->checkBox_c_3->setEnabled(false);
        ui->checkBox_mca_3->setEnabled(false);
        ui->checkBox_c_3->setChecked(false);
       // setButtonState(false,ui->pushButton_Encendido_3,false);
        ui->pushButton_Encendido_3->setChecked(false);

    }
    if (Estado_Cabezales.contains(4)){
        ui->checkBox_c_4->setEnabled(true);
        ui->checkBox_mca_4->setEnabled(true);
        ui->pushButton_Encendido_4->setChecked(true);

        //setButtonState(true,ui->pushButton_Encendido_4,false);
    }else{
        ui->checkBox_c_4->setEnabled(false);
        ui->checkBox_mca_4->setEnabled(false);
        ui->checkBox_c_4->setChecked(false);
        ui->pushButton_Encendido_4->setChecked(false);

       // setButtonState(false,ui->pushButton_Encendido_4,false);
    }
    if (Estado_Cabezales.contains(5)){
        ui->checkBox_c_5->setEnabled(true);
        ui->checkBox_mca_5->setEnabled(true);
        //setButtonState(true,ui->pushButton_Encendido_5,false);
        ui->pushButton_Encendido_5->setChecked(true);
    }else{
        ui->checkBox_c_5->setEnabled(false);
        ui->checkBox_mca_5->setEnabled(false);
        ui->checkBox_c_5->setChecked(false);
        //setButtonState(false,ui->pushButton_Encendido_5,false);
        ui->pushButton_Encendido_5->setChecked(false);

    }
    if (Estado_Cabezales.contains(6)){
        ui->checkBox_c_6->setEnabled(true);
        ui->checkBox_mca_6->setEnabled(true);
        //setButtonState(true,ui->pushButton_Encendido_6,false);
        ui->pushButton_Encendido_6->setChecked(true);

    }else{
        ui->checkBox_c_6->setEnabled(false);
        ui->checkBox_mca_6->setEnabled(false);
        ui->checkBox_c_6->setChecked(false);
        ui->pushButton_Encendido_6->setChecked(false);

        //setButtonState(false,ui->pushButton_Encendido_6,false);
    }
}

void MainWindow::on_checkBox_c_3_toggled(bool checked)
{
    if (checked){
       // setHeadMode(3,"config");
    }
}

void MainWindow::on_checkBox_c_6_toggled(bool checked)
{
    if (checked){
       // setHeadMode(6,"config");
    }
}

void MainWindow::on_checkBox_c_5_toggled(bool checked)
{
    if (checked){
       // setHeadMode(5,"config");
    }
}

void MainWindow::on_checkBox_c_4_toggled(bool checked)
{
    if (checked){
      //  setHeadMode(4,"config");
    }
}

void MainWindow::on_checkBox_c_2_toggled(bool checked)
{
    if (checked){
       // setHeadMode(2,"config");
    }
}

void MainWindow::on_checkBox_c_1_toggled(bool checked)
{
    if (checked){
       // setHeadMode(1,"config");
    }
}

void MainWindow::on_comboBox_head_select_graph_currentIndexChanged(int index)
{
    
}

void MainWindow::on_checkBox_mca_1_toggled(bool checked)
{

}

void MainWindow::on_checkBox_mca_2_toggled(bool checked)
{

}

void MainWindow::on_checkBox_mca_3_toggled(bool checked)
{

}

void MainWindow::on_checkBox_mca_4_toggled(bool checked)
{

}

void MainWindow::on_checkBox_mca_5_toggled(bool checked)
{

}

void MainWindow::on_checkBox_mca_6_toggled(bool checked)
{

}

void MainWindow::on_tabWidget_mca_currentChanged(int index)
{
    if (index==1){
        ui->comboBox_head_select_graph->hide();
        ui->frame_multihead_graph->show();
        ui->checkBox_espectro_calibrado->show();
    }else if (index==0){
        setAdquireMode(PMT);
        ui->checkBox_espectro_calibrado->hide();
        ui->comboBox_head_select_graph->show();
        ui->frame_multihead_graph->hide();
    }else{
        ui->comboBox_head_select_graph->show();
        ui->checkBox_espectro_calibrado->hide();
        ui->frame_multihead_graph->hide();
    }
}

void MainWindow::on_pushButton_Encendido_1_toggled(bool checked)
{
    Cabezal_On_Off(1,checked);
}

void MainWindow::on_pushButton_Encendido_2_toggled(bool checked)
{
    Cabezal_On_Off(2,checked);
}

void MainWindow::on_pushButton_Encendido_3_toggled(bool checked)
{
    Cabezal_On_Off(3,checked);
}

void MainWindow::on_pushButton_Encendido_4_toggled(bool checked)
{
    Cabezal_On_Off(4,checked);
}

void MainWindow::on_pushButton_Encendido_5_toggled(bool checked)
{
    Cabezal_On_Off(5,checked);
}

void MainWindow::on_pushButton_Encendido_6_toggled(bool checked)
{
    Cabezal_On_Off(6,checked);
}


void MainWindow::Cabezal_On_Off(int Cabezal, bool estado){
    error_code error_code;
    QVector<string> Cabezales;
    char checksum;
    for (int i=0;i<6;i++){Cabezales.push_back("0");}
    try{
        arpet->portDisconnect();
        port_name="/dev/UART_Coin";
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
            throw exception_Cabezal_Apagado;
        }
        if (estado){
            checksum=1;
            //Estado_Cabezales.replace(0,1);
            for (int i=1;i<=6;i++){
                if (Estado_Cabezales.contains(i)){
                    Cabezales.replace(i-1,"1");
                    checksum++;
                }
            }
            Cabezales.replace(Cabezal-1,"1");
            checksum+=48+6;
            sendString(arpet->getInit_on_off()
                       +Cabezales.at(0)+Cabezales.at(1)+Cabezales.at(2)
                       +Cabezales.at(3)+Cabezales.at(4)+Cabezales.at(5)+"0"+checksum,arpet->getEnd_MCA());
        }
        else{
            checksum=0;
            for (int i=1;i<=6;i++){
                if (Estado_Cabezales.contains(i)){
                    Cabezales.replace(i-1,"1");
                    checksum++;
                }
            }
            Cabezales.replace(Cabezal-1,"0");
            checksum+=48+6-1;
            sendString(arpet->getInit_on_off()
                       +Cabezales.at(0)+Cabezales.at(1)+Cabezales.at(2)
                       +Cabezales.at(3)+Cabezales.at(4)+Cabezales.at(5)+"0"+checksum,arpet->getEnd_MCA());
        }
        arpet->portDisconnect();
    }

    catch(Exceptions &ex){
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
        arpet->portDisconnect();
    }


}

void MainWindow::Cabezales_On_Off(bool estado){
    error_code error_code;
    try{
        arpet->portDisconnect();
        port_name="/dev/UART_Coin";
        error_code= arpet->portConnect(port_name.toStdString().c_str());
        if (error_code.value()!=0){
            arpet->portDisconnect();
            Exceptions exception_Cabezal_Apagado("Está coincidencia y los cabezales apagados! Revise las conexiones");
            throw exception_Cabezal_Apagado;
        }
        if (estado){
//            sendString(arpet->getInit_on_off()+"11100009",arpet->getEnd_MCA());
//            usleep(50000);
//            sendString(arpet->getInit_on_off()+"1111110<",arpet->getEnd_MCA());
        if (Estado_Cabezales.length()>2)    {
            sendString(arpet->getInit_on_off()+"1111110<",arpet->getEnd_MCA());
        }else{
            sendString(arpet->getInit_on_off()+"111"+getEstadoCabezal(4)+getEstadoCabezal(5)+getEstadoCabezal(6)+"0"+(char)(0x39+Estado_Cabezales.length()-(int)Estado_Cabezales.contains(1)-(int)Estado_Cabezales.contains(2)-(int)Estado_Cabezales.contains(3)),arpet->getEnd_MCA());
            usleep(50000);
            sendString(arpet->getInit_on_off()+"1111110<",arpet->getEnd_MCA());
        }
        }else{
            sendString(arpet->getInit_on_off()+"00000006",arpet->getEnd_MCA());
        }
        arpet->portDisconnect();
    }
    catch(Exceptions &ex){
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
        arpet->portDisconnect();
    }
}

string MainWindow::getEstadoCabezal(int head){
    return Estado_Cabezales.contains(head) ? "1" : "0";
}

void MainWindow::on_pushButton_On_Off_Cabs_toggled(bool checked)
{
    Cabezales_On_Off(checked);
}


int MainWindow::loadCalibrationTables(QString head){

    getPreferencesSettingsFile();
    QSettings settings(initfile, QSettings::IniFormat);

    /* Paths to the configuration files */
    if (QFile(initfile).exists()){
        QString root = settings.value("Paths/root", "US").toString();

        AT = settings.value("Cabezal"+head+"/AT", "US").toInt();
        LowLimit[head.toInt()-1] = settings.value("Cabezal"+head+"/LowLimit", "US").toInt();
        Target = settings.value("Cabezal"+head+"/Target", "US").toInt();
        coefenerg = root+settings.value("Cabezal"+head+"/coefenerg", "US").toString();
        hvtable = root+settings.value("Cabezal"+head+"/hvtable", "US").toString();
        coefx = root+settings.value("Cabezal"+head+"/coefx", "US").toString();
        coefy = root+settings.value("Cabezal"+head+"/coefy", "US").toString();
        coefest = root+settings.value("Cabezal"+head+"/coefest", "US").toString();
        coefT = root+settings.value("Cabezal"+head+"/coefT", "US").toString();
        coefTInter = root+settings.value("Cabezal"+head+"/coefTInter", "US").toString();
        path_Planar_bit  = settings.value("Paths/Planar_bit", "US").toString();
        path_adq_Calib  = settings.value("Paths/Adq_Calib", "US").toString();
        path_adq_Coin  = settings.value("Paths/Adq_Coin", "US").toString();
        path_SP3_bit     = settings.value("Paths/SP3_bit", "US").toString();
        path_Coin_bit    = settings.value("Paths/Coin_bit", "US").toString();
        device_planar     = settings.value("model/Planar", "US").toString();
        device_SP3        = settings.value("model/SP3", "US").toString();
        device_SP3_MEM    = settings.value("model/SP3_MEM", "US").toString();
        device_coin       = settings.value("model/Coin", "US").toString();
        model_planar     = settings.value("model/Planar_model", "US").toString();
        model_SP3        = settings.value("model/SP3_model", "US").toString();
        model_SP3_MEM    = settings.value("model/SP3_MEM_model", "US").toString();
        model_coin       = settings.value("model/Coin_model", "US").toString();
        name_Planar_bit  = settings.value("model/Bit_name_Planar", "US").toString();
        name_Coin_bit    = settings.value("model/Bit_name_Coin", "US").toString();
        name_SP3_bit     = settings.value("model/Bit_name_SP3", "US").toString();

        coefTInter_values=getValuesFromFiles(coefTInter);

        QVector <double> Cabezal;


        Cabezal.append(getValuesFromFiles(hvtable,true));
        if (hvtable_values.empty()){
            for (int i=0;i<6;i++){
                hvtable_values.push_back(Cabezal);
            }
        }
        hvtable_values.replace(head.toInt()-1,Cabezal);
        Cabezal.clear();

        Cabezal.append(getValuesFromFiles(coefenerg));
        if (Matrix_coefenerg_values.empty()){
            for (int i=0;i<6;i++){
                Matrix_coefenerg_values.push_back(Cabezal);
            }
        }
        Matrix_coefenerg_values.replace(head.toInt()-1,Cabezal);
        Cabezal.clear();


        Cabezal.append(getValuesFromFiles(coefx));
        if (Matrix_coefx_values.empty()){
            for (int i=0;i<6;i++){
                Matrix_coefx_values.push_back(Cabezal);
            }
        }
        Matrix_coefx_values.replace(head.toInt()-1,Cabezal);
        Cabezal.clear();

        Cabezal.append(getValuesFromFiles(coefy));
        if (Matrix_coefy_values.empty()){
            for (int i=0;i<6;i++){
                Matrix_coefy_values.push_back(Cabezal);
            }
        }
        Matrix_coefy_values.replace(head.toInt()-1,Cabezal);
        Cabezal.clear();

        Cabezal.append(getValuesFromFiles(coefT));
        if (Matrix_coefT_values.empty()){
            for (int i=0;i<6;i++){
                Matrix_coefT_values.push_back(Cabezal);
            }
        }
        Matrix_coefT_values.replace(head.toInt()-1,Cabezal);
        Cabezal.clear();

        Cabezal.append(getValuesFromFiles(coefest));
        if (Matrix_coefest_values.empty()){
            for (int i=0;i<6;i++){
                Matrix_coefest_values.push_back(Cabezal);
            }
        }
        Matrix_coefest_values.replace(head.toInt()-1,Cabezal);
        Cabezal.clear();
        return 0;
    }
    else{
        QMessageBox::critical(this,tr("Atención"),"No se puede acceder al archivo de configuracion! Verifique la conexion al servidor.");
        return -1;
    }


}

void MainWindow::CargoTemaOscuro(){

    QFile f(":qdarkstyle/style.qss");
    if (!f.exists())
    {
        printf("Unable to set stylesheet, file not found\n");
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
}

void MainWindow::on_pbAdquirir_toggled(bool checked)
{
    error_code error_code;
    string msg;
    QString psoc_alta;
    QString psoc_alta_Tabla;
    QStringList commands;
    QString NombredeArchivo;
    QString time =QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm");
    arpet->portDisconnect();
    QVector<int> Cabezales;


  //  Cabezales_On_Off(checked);

    if (checked){

        cant_archivos=1;
        cant_archivos_copiados=0;
        /////////////////////////Verificacion previa antes de configurar y adquirir//////////////////////////
         int archivos = ui->lineEdit_aqd_cant_archivos->text().toInt() - cant_archivos + 1;
        QString mensaje= "Restan: " + QString::number(archivos) + " archivos";
        ui->label_cant_archivos->setText(mensaje);

        ui->label_gif_4->setVisible(false);
        ui->label_gif_3  ->setVisible(true);


        //QMovie movie("/home/ar-pet/Downloads/ajax-loader.gif");
        ui->label_gif_3->setMovie(movie_cargando);
        movie_cargando->start();
        ui->label_gif_3->setScaledContents( false );
        ui->label_gif_3->show();


        if (ui->lineEdit_Titulo_Medicion->text().isEmpty()){
            QMessageBox::critical(this,tr("Error"),tr("La medición debe contener un título."));
            return;
        }else{

        }

        if (ui->lineEdit_aqd_file_size->text().isEmpty()){
            QMessageBox::critical(this,tr("Error"),tr("Se debe definir un tamaño de archivo de adquisición."));
            return;
        }else{
            commands.append(ui->lineEdit_aqd_file_size->text());
            size_archivo_adq = ui->lineEdit_aqd_file_size->text();

        }
        if(ui->cb_Path_alternativo_adq->isChecked()){
            if(ui->lineEdit_aqd_path_file->text().isEmpty()){
                QMessageBox::critical(this,tr("Error"),tr("Se debe definir una ruta específica si se definió ."));
                return;
            }else{
                commands.append(ui->lineEdit_aqd_path_file->text());
                //nombre_archivo_adq = ui->lineEdit_aqd_path_file->text();
                if(ui->comboBox_aqd_mode->currentIndex()==1){
                    NombredeArchivo="acquire_calib_"+ui->cb_Calib_Cab->currentText();
                    Cabezales.append(ui->cb_Calib_Cab->currentText().toInt());

                }else{
                    NombredeArchivo="acquire_coin";
                    Cabezales=Estado_Cabezales;
                }
            }
        }else{
            if(ui->comboBox_aqd_mode->currentIndex()==1){
                commands.append(path_adq_Calib);
                //nombre_archivo_adq = path_adq_Calib;
                NombredeArchivo="acquire_calib_"+ui->cb_Calib_Cab->currentText();
                Cabezales.append(ui->cb_Calib_Cab->currentText().toInt());


            }else{
                commands.append(path_adq_Coin);
                //nombre_archivo_adq = path_adq_Coin;
                NombredeArchivo="acquire_coin";
                Cabezales=Estado_Cabezales;

            }

        }



        ui->progressBar->setRange(0,size_archivo_adq.toInt());

        commands.append(NombredeArchivo);
        nombre_archivo_adq =  NombredeArchivo ;
        worker_adq->abort();
        thread_adq->exit(0);
        usleep(500);

        ///////////////////////////////////////////////////////////////////////////////////////
        // commands Parametros:     1er valor Cantidad de MB de medicion
        //                          2do valor Path Coincidencia/Calibracion
        //                          3er valor Nombre de Archivo
        ///////////////////////////////////////////////////////////////////////////////////////




        QString logFileAdq = "./LOG_Adquisicion_"+time+".txt";
        QFile logger( logFileAdq );
        QString log;
        logger.open(QIODevice::WriteOnly | QIODevice::Append);

        log.append(ui->lineEdit_Titulo_Medicion->text()+"\n");
        log.append(ui->Comentarios_Adquisicion->toPlainText()+"\n");







        //return;

        /////////////////////////Fin de verificacion previa antes de configurar y adquirir//////////////////////////


        /////////////// CONFIGURACION Y CARGA DE TABLAS
        for (int i=0;i<Cabezales.length();i++)
        {

            int head_index=Cabezales.at(i);
            /* Inicialización del Cabezal */
            try
            {


                log.append("[CAB-"+QString::number(head_index)+"]\t");
                log.append("[HV]\t");
                log.append("[EN]\t");
                log.append("[XPOS]\t");
                log.append("[YPOS]\t");
                log.append("[TIME]\t");
                log.append("\n");

                for (int j=0;j<PMTs;j++){
                    log.append(QString::number(j)+"\t");
                    log.append(QString::number(hvtable_values[head_index-1][j])+"\t");
                    log.append(QString::number(Matrix_coefenerg_values[head_index-1][j])+"\t");
                    log.append(QString::number(Matrix_coefx_values[head_index-1][j])+"\t");
                    log.append(QString::number(Matrix_coefy_values[head_index-1][j])+"\t");
                    log.append(QString::number(Matrix_coefT_values[head_index-1][j])+"\t");
                    log.append("\n");

                }

                log.append("[LOW-LIMIT], "+ QString::number(LowLimit[head_index-1])+"\n");

                log.append("[VENTANAS-ENERGIA]\n");
                //log.append(QString::number(Matrix_coefest_values[0][i])+", ");
                log.append("Vent. Inf.: "+QString::number(Matrix_coefest_values[head_index-1][0])+"-"
                        +QString::number(Matrix_coefest_values[head_index-1][1])+"\n");

                log.append("Vent. Med.: "+QString::number(Matrix_coefest_values[head_index-1][2])+"-"
                        +QString::number(Matrix_coefest_values[head_index-1][3])+"\n");
                log.append("Vent. Sup.: "+QString::number(Matrix_coefest_values[head_index-1][4])+"-"
                        +QString::number(Matrix_coefest_values[head_index-1][5])+"\n");





                port_name=Cab+QString::number(head_index);
                calibrador->setPort_Name((port_name));
                worker->setPortName((port_name));
                error_code= arpet->portConnect(port_name.toStdString().c_str());
                if (error_code.value()!=0){
                    arpet->portDisconnect();
                    Exceptions exception_Cabezal_Apagado("Está el cabezal apagado");
                    throw exception_Cabezal_Apagado;
                }

                if (initHead(head_index).length()==0){
                    ui->label_data_output->setText("Cabezal "+QString::number(head_index)+ " todavía no iniciado");
                    return;
                }
                if(initSP3(head_index).length()==0){
                    ui->label_data_output->setText("PMTs no responden");
                    return;
                }
                parseConfigurationFile(true, QString::number(head_index));

                usleep(500);
                QString q_msg = setHV(QString::number(head_index).toStdString(),QString::number(LowLimit[head_index-1]).toStdString());

                psoc_alta_Tabla = QString::number(AT);

                usleep(500);

                setPSOCDataStream(QString::number(head_index).toStdString(), arpet->getPSOC_SIZE_RECEIVED_ALL(), arpet->getPSOC_ON());

                sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
                usleep(500);
                msg = readString();
                //setLabelState(arpet->verifyMCAEStream(msg,arpet->getPSOC_ANS()), hv_status_table[head_index-1]);
                usleep(500);

                setPSOCDataStream(QString::number(head_index).toStdString(), arpet->getPSOC_SIZE_RECEIVED_ALL(), arpet->getPSOC_SET(),psoc_alta_Tabla);

                if(debug) cout<<"Cabezal: "<<head_index<<endl;

                sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
                msg = readString(CHAR_LF);

                usleep(500);
                //hv_status_table[head_index-1]->setText(psoc_alta);

                setCalibrationTables(head_index);

                arpet->portDisconnect();

            }
            catch(Exceptions & ex)
            {
                QMessageBox::critical(this,tr("Atención"),tr((string("El Cabezal no está respondiendo. Error: ")+string(ex.excdesc)).c_str()));

                if (debug) cout<<"No se puede Configurar: "<<ex.excdesc<<arpet->getTrama_MCAE()<<arpet->getEnd_PSOC() <<endl;
                setLabelState(false, hv_status_table[head_index-1], true);
            }
        }

        log.append("[TIEMPOS-INTER-CAB]\n");
        log.append("CAB1: "+QString::number(coefTInter_values[0])+"\n");
        log.append("CAB2: "+QString::number(coefTInter_values[1])+"\n");
        log.append("CAB3: "+QString::number(coefTInter_values[2])+"\n");
        log.append("CAB4: "+QString::number(coefTInter_values[3])+"\n");
        log.append("CAB5: "+QString::number(coefTInter_values[4])+"\n");
        log.append("CAB6: "+QString::number(coefTInter_values[5])+"\n");

        //head = ui->comboBox_head_select_calib->currentText();


        if(ui->comboBox_aqd_mode->currentIndex()==1){
            if(debug) cout<<"Modo Calibración en el cabezal: "<<ui->cb_Calib_Cab->currentText().toStdString()<<endl;
            setCalibrationMode(ui->cb_Calib_Cab->currentText());
            usleep(5000);
            setTimeModeCoin(COIN_CALIB, ui->cb_Calib_Cab->currentText());
        }else{
            if(debug) cout<<"Modo Coincidencia: Normal"<<endl;
            initCoincidenceMode();
            usleep(5000);
            setCoincidenceModeWindowTime();
            usleep(5000);
            setCoincidenceModeDataStream(arpet->getNormal_Coin_Mode());
            usleep(5000);
            setTimeModeCoin(COIN_NORMAL);
        }

        logger.write( log.toUtf8());
        logger.close();
        commands.append(logFileAdq);
        commands.append(QDate::currentDate().toString("yyyy-MM-dd"));
        commands.append(QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm"));
        commands.append(ui->lineEdit_aqd_cant_archivos->text());
        worker_adq->setCommands(commands);
        worker_copy->setCommands(commands);

        checkStatusAdq(true);
        adq_running = true;
        ///////// FIN DE CONFIGURACION Y CARGA DE TABLAS
    }
    else{

        QProcess killall;
        killall.waitForStarted();
        killall.execute("pkill recvRawEth");
        killall.waitForFinished(1000);
        cout<<killall.readAll().toStdString()<<endl;
        adq_running = false;


        worker_copy->abort();
        thread_copy->exit(0);
        usleep(5000);



    }
}


void MainWindow::UncheckHeads(){

    QList<int> checkedHeads=getCheckedHeads();


    for (int i=0;i<checkedHeads.length();i++)
    {
        int head_index=checkedHeads.at(i);

        setLabelState(false, hv_status_table[head_index-1], true);
        setLabelState(false, pmt_status_table[head_index-1], true);
        setLabelState(false, head_status_table[head_index-1], true);
        setLabelState(false, calib_status_table[head_index-1], true);
    }
}

void MainWindow::on_pushButton_FPGA_4_clicked()

{
    QStringList device_list;
    bool error;
    QString output, command;
    QString JTAG[7] ={ "jtag_cab1","jtag_cab2","jtag_cab3","jtag_cab4","jtag_cab5","jtag_cab6","jtag_coin"}; /// Nombres de los dipositivos JTAG
    QString filtro  = "JTAG";                                                                                 /// Filtro para encontrar los dispositivos dentro de lo que retorna el XC3SPROG
    QPixmap image;

    int Cantidad_elementos;

    if (ui->comboBox_FPGA_Cab->currentIndex() == 6)
        Cantidad_elementos = CANTIDAD_ELEMENTOS_COINCIDENCIA;
    else
        Cantidad_elementos = CANTIDAD_ELEMENTOS_PLANAR;

       QProcess read_device;

        command = "xc3sprog -c " + JTAG[ui->comboBox_FPGA_Cab->currentIndex()] ;

        read_device.waitForStarted();
        read_device.start(command);
        read_device.waitForFinished(-1);


        output=(read_device.readAllStandardError());
        output =  output.mid(output.indexOf(filtro));
        device_list = output.split("\n");
        cout << "Dispositivos encontrados: " << endl;

        for( int i=0 ; i < device_list.length() -1 ; i++ )
        {
            device_list[i] = device_list[i].mid(device_list[i].indexOf("X"));
            device_list[i] = device_list[i].left(device_list[i].indexOf(" "));
            if (device_list[i] == device_coin || device_list[i] == device_planar || device_list[i] == device_SP3 || device_list[i] == device_SP3_MEM )
              cout << QString::number(i).toStdString() << "= "<< device_list[i].toStdString()  << endl;
            else
            {
              cout << "Error: No coincide el device "<< QString::number(i).toStdString() << " con ningun dispositivo valido" << endl;

              error =  true;
              break;
            }
        }

    read_device.close();



    if(device_list.length() == Cantidad_elementos + 1 && !error )
       image.load("/home/ar-pet/Downloads/ic_check_circle.png");
    else
       image.load("/home/ar-pet/Downloads/ic_cancel.png");

    ui->label_gif->setVisible(false);
    ui->label_gif_2->setVisible(true);

    ui->label_gif_2->setPixmap(image);
    ui->label_gif_2->setScaledContents( true );
    ui->label_gif_2->show();
    return;

}
void MainWindow::on_pushButton_FPGA_1_clicked()

{

    QString filename = QFileDialog::getOpenFileName(this,
        tr("Abrir Archivo.bit"), "/home/ar-pet", tr("Image Files (*.bit)"));

    // = QFileDialog::getOpenFileName();

    QFile file(filename);
     if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
     {
         cout << "Error al abrir el archivo .bit : " << filename.toStdString() << endl ;
         return;
     }


    //QString content = file.readAll();



    QTextStream in (&file);
    QString line;
    QString filtro;
    bool correcto =false;

    if(ui->comboBox_FPGA_Cab->currentIndex() == 6)
        filtro = model_coin;
    else if(ui->comboBox_FPGA_DISP->currentIndex() == 0)
        filtro = model_planar;
    else
        filtro = model_SP3;

    do {
        line = in.readLine();
        if (line.contains(filtro, Qt::CaseSensitive))
            correcto = true;
       }while (!line.isNull());



    file.close();


    if(correcto)
    {
        cout << "Archivo correcto" << endl;
        ui->text_FPGA_1->setText(filename);
    }
    else
    {
        cout << "No coincide el archivo con el dispositivo seleccionado" << endl;
        ui->text_FPGA_1->setText("");
        return;
    }



}
/**
 * @brief MainWindow::on_pushButton_FPGA_3_clicked
 */
void MainWindow::on_pushButton_FPGA_3_clicked()
{
    QPixmap image;
    int modo ;

    ui->label_gif_2->setVisible(false);
    ui->label_gif  ->setVisible(true);

    //QMovie movie("/home/ar-pet/Downloads/ajax-loader.gif");
    ui->label_gif->setMovie(movie_cargando);
    movie_cargando->start();
    ui->label_gif->setScaledContents( false );
    ui->label_gif->show();

    qApp->processEvents();

    if(ui->checkBox_FPGA_1->isChecked())
        modo = 1;
    else
        modo = 0;

    QStringList commands = Mensaje_Grabar_FPGA(modo);

    if(!commands.isEmpty()) {

        worker_fpga->abort();
        thread_fpga->exit(0);
        usleep(500);
        worker_fpga->setCommands(commands);
        worker_fpga->requestGrabarFPGA();
    }
    else
    {
        image.load("/home/ar-pet/Downloads/ic_cancel.png");

        ui->label_gif->setVisible(false);
        ui->label_gif_2->setVisible(true);

        ui->label_gif_2->setPixmap(image);
        ui->label_gif_2->setScaledContents( true );
        ui->label_gif_2->show();
    }
    return;




}
/**
 * @brief MainWindow::Mensaje_Grabar_FPGA
 * int modo
                0 = MODO PROGRAMACION
                1 = MODO PROGRAMACION MEMORIA
                2 = MODO BORRAR MEMORIA
    Returns QStringList = comandos a ser ejecutados por el XC3sprog
 */

QStringList MainWindow::Mensaje_Grabar_FPGA(int modo)
{
    QStringList command;                                                                                     /// Vector de comandos a despachar por consola
    QString path= "";                                                                                        /// Ruta al archivo que se va grabar
    QString device ="";                                                                                      /// Nombre del dispositivo que ve el JTAG
    QString model ="";                                                                                       /// Modelo del dispositivo que aparece dentro del archivo .bit
    QString bit_name ="";                                                                                       /// Nombre de la Entiti principal que aparece dentro del archivo .bit
    QStringList device_list;                                                                                 /// lista de los dispositivos conectados al jtag
    QString JTAG[7] ={ "jtag_cab1","jtag_cab2","jtag_cab3","jtag_cab4","jtag_cab5","jtag_cab6","jtag_coin"}; /// Nombres de los dipositivos JTAG
    QString Dispositivo[7] ={ "Cab1","Cab2","Cab3","Cab4","Cab5","Cab6","Coincidencias"};                    /// Nombres de los dipositivos JTAG
    QString filtro = "JTAG";                                                                                 /// Filtro para encontrar los dispositivos dentro de lo que retorna el XC3SPROG
    QString command_check = "";                                                                              /// Comando a enviar por consola
    QString output  = "";                                                                                    /// Salida que retorna el XC3SPROG
    QString Options = "";                                                                                    /// Agrega opciones a los comandos



        // Diferancio el Target y guardo los parametros a utilizar
            if(ui->comboBox_FPGA_Cab->currentIndex() != 6)
                    {
                        if(ui->comboBox_FPGA_DISP->currentIndex() == 0)
                           {
                              path= path_Planar_bit;
                              device = device_planar;
                              model = model_planar;
                              bit_name = name_Planar_bit;
                           }
                        else
                           {
                             path= path_SP3_bit;
                             model = model_SP3;
                             bit_name = name_SP3_bit;
                             if (modo == 1)
                             {
                                device = device_SP3_MEM;
                                offset_MEM = 1;
                             }
                             else
                                device = device_SP3;
                           }

                    }
            else
            {
                    path= path_Coin_bit;
                    device = device_coin;
                    model = model_coin;
                    bit_name = name_Coin_bit;
            }

            if (ui->checkBox_FPGA_2->isChecked())
                if ( ui->text_FPGA_1->toPlainText() != "")
                    path = ui->text_FPGA_1->toPlainText();


            switch(modo)
            {
                case 0 : // MODO PROGRAMACION
                    Options = " -v -p ";
                break;

                case 1 : // MODO PROGRAMACION MEMORIA
                    if (model == model_SP3 || model == model_SP3_MEM)
                        Options = " -v -p ";
                    else
                        Options = " -v -I -p ";
                break;

                case 2 : // MODO BORRAR MEMORIA
                    Options = " -v -e -p ";
                    path = "";
                break;

                default:
                    Options = " ";
                break;
            }

        // verifico que el archivo seleccionado sea .bit y contenga el deviceo de dispositivo
       if(modo!=2)
       {
            QFile file(path);
             if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
             {
                 cout << "Error al abrir el archivo .bit : " << path.toStdString() << endl ;
                   return command;
             }

            QTextStream in (&file);
            QString line;
            bool correcto =false;

            do {
                line = in.readLine();
                if (line.contains(model, Qt::CaseSensitive) && line.contains(bit_name, Qt::CaseSensitive) )
                    correcto = true;
               }while (!line.isNull());


            if(!correcto)
            {
                cout << "No coinciden los parametros del archivo .bit con el dispositivo seleccionado" << endl;

                return command;
            }


            file.close();
       }




        // Identifico los dispositivos conectados y los listo

            QProcess read_device;

                command_check = "xc3sprog -c " + JTAG[ui->comboBox_FPGA_Cab->currentIndex()] ;

                read_device.waitForStarted();
                read_device.start(command_check);
                read_device.waitForFinished(-1);


                output=(read_device.readAllStandardError());
                output =  output.mid(output.indexOf(filtro));
                device_list = output.split("\n");


                for( int i=0 ; i < device_list.length() -1 ; i++ )
                {
                    device_list[i] = device_list[i].mid(device_list[i].indexOf("X"));
                    device_list[i] = device_list[i].left(device_list[i].indexOf(" "));
                    if (!(device_list[i] == device_coin || device_list[i] == device_planar || device_list[i] == device_SP3 || device_list[i] == device_SP3_MEM ))
                    {
                      cout << "Error: No coincide el device "<< QString::number(i).toStdString() << " con ningun dispositivo valido" << endl;
                      return command;
                      break;
                    }
                }

            read_device.close();



        // Grabado de dispositivos


                if (device == device_SP3 || device == device_SP3_MEM)
                {
                    for( int i=0 ; i < array_PMT.length() ; i++ )
                        if ( device_list[PMT_posJTAG[array_PMT[i].toInt() - 1].toInt() + offset_MEM] == device)
                        {
                            command.append( Dispositivo[ui->comboBox_FPGA_Cab->currentIndex()] + " PMT= " + array_PMT[i] + "#" +"xc3sprog -c " + JTAG[ui->comboBox_FPGA_Cab->currentIndex()] + Options + QString::number(PMT_posJTAG[array_PMT[i].toInt() - 1].toInt() + offset_MEM)  + " " + path);

                            //cout << command.toStdString() << endl;
                            //QProcess prog_fpga;
                            //prog_fpga.waitForStarted();
                            //prog_fpga.start(command);
                            //prog_fpga.waitForFinished(-1);
                            //output=(prog_fpga.readAllStandardError());
                            //prog_fpga.close();

                            //if (output.contains("done"))
                            //{
                            //    output =  output.mid(output.indexOf("done"));
                            //    output =  output.left(output.indexOf("\n"));
                            //    cout << "PMT= "  << array_PMT[i].toStdString() << " " << output.toStdString() << endl;
                            //}
                            //else
                            //{
                            //   cout << output.toStdString() << endl;
                            //     return null;
                            // }

                        }
                        else
                        {
                            cout << "No coincide el dispositivo: " << PMT_posJTAG[array_PMT[i].toInt() - offset_MEM].toStdString() << endl;
                              return command;
                        }

                }
                else
                    if (device_list[0] == device)
                            command.append(Dispositivo[ui->comboBox_FPGA_Cab->currentIndex()] +  "#xc3sprog -c " + JTAG[ui->comboBox_FPGA_Cab->currentIndex()] + Options+  "0 " + path);
                    else
                    {
                        cout << "No coincide el dispositivo: 0 con el device de FPGA" << device.toStdString() << endl;
                        return command;
                    }

               return command;
}


void MainWindow::on_comboBox_FPGA_DISP_currentIndexChanged(int index)
{

}

void MainWindow::on_checkBox_FPGA_2_clicked(bool checked)
{

    ui->label_FPGA_3     ->setVisible(checked);
    ui->text_FPGA_1      ->setVisible(checked);
    ui->pushButton_FPGA_1->setVisible(checked);
 }

void MainWindow::on_comboBox_FPGA_Cab_currentIndexChanged(int index)
{

       ui->text_FPGA_1->setText("");
       if(ui->comboBox_FPGA_Cab->currentIndex() == 6)

           ui->comboBox_FPGA_DISP->setEnabled(false);
       else

           ui->comboBox_FPGA_DISP->setEnabled(true);

}
void MainWindow::TimerUpdate(){
    worker->TimerUpdate();
}

void MainWindow::on_cb_Path_alternativo_adq_toggled(bool checked)
{
    if (checked){
        ui->lineEdit_aqd_path_file->show();
        ui->pushButton_aqd_file_open->show();
    }else{
        ui->lineEdit_aqd_path_file->hide();
        ui->pushButton_aqd_file_open->hide();
    }
}

void MainWindow::on_comboBox_aqd_mode_currentIndexChanged(const QString &arg1)
{
    if (arg1.contains("Coincidencia")){
        ui->cb_Calib_Cab->hide();
    }else if(arg1.contains("Calibración")){
        ui->cb_Calib_Cab->show();
    }

}

void MainWindow::on_pushButton_FPGA_2_clicked()
{
    QPixmap image;

    QMessageBox msgBox;//(QMessageBox.Warning);
    msgBox.setIcon(QMessageBox::Warning);
    //msgBox.setWindowIcon(QMessageBox.icon(QMessa));
    msgBox.setWindowTitle("Borrar dispositivo");
    msgBox.setText("Desea borrar la/s memorias ?");
    QPushButton *connectButton = msgBox.addButton(tr("Aceptar"), QMessageBox::YesRole);
    QPushButton *abortButton = msgBox.addButton(tr("Cancelar"),  QMessageBox::NoRole);
    msgBox.setDefaultButton(abortButton);
    msgBox.setEscapeButton(abortButton);

    msgBox.exec();

    if (msgBox.clickedButton() == abortButton)
        return;
    else if ( msgBox.clickedButton() == connectButton)
    {

        ui->label_gif_2->setVisible(false);
        ui->label_gif  ->setVisible(true);

        //QMovie movie("/home/ar-pet/Downloads/ajax-loader.gif");
        ui->label_gif->setMovie(movie_cargando);
        movie_cargando->start();
        ui->label_gif->setScaledContents( false );
        ui->label_gif->show();

        qApp->processEvents();

        QStringList commands = Mensaje_Grabar_FPGA(2);

        if(!commands.isEmpty()) {

            worker_fpga->abort();
            thread_fpga->exit(0);
            usleep(500);
            worker_fpga->setCommands(commands);
            worker_fpga->requestGrabarFPGA();
        }
        else
        {
            image.load("/home/ar-pet/Downloads/ic_cancel.png");

            ui->label_gif->setVisible(false);
            ui->label_gif_2->setVisible(true);

            ui->label_gif_2->setPixmap(image);
            ui->label_gif_2->setScaledContents( true );
            ui->label_gif_2->show();
        }
     }

}

void MainWindow::on_comboBox_FPGA_DISP_activated(int index)
{
    ui->text_FPGA_1->setText("");
    if(ui->comboBox_FPGA_DISP->currentIndex() == 1)
    {

        int ret = pmt_select->exec();

        array_PMT = pmt_select->GetPMTSelectedList();

        if(ret == QDialog::Accepted)
        {
             cout << "PMTs Seleccionados"<< endl;
             for( int i=0 ; i < array_PMT.length() ; i++ )
             cout << array_PMT[i].toStdString() << " Pos_JTAG: " << QString::number(PMT_posJTAG[array_PMT[i].toInt() - 1].toInt() + offset_MEM).toStdString() << endl;
        }
    }


}
void MainWindow::checkStatusMoveToServer(bool status){
    QPixmap image;
    //adq_running = false;

    cant_archivos_copiados = cant_archivos_copiados + 1;
    int archivos = ui->lineEdit_aqd_cant_archivos->text().toInt() - cant_archivos_copiados;
    QString mensaje= "Restan: " + QString::number(archivos) + " archivos";
    ui->label_cant_archivos->setText(mensaje);
    qApp->processEvents();

    copying=false;



    if (!status){
        worker_adq->abort();
        thread_adq->exit(0);
        usleep(500);


        worker_copy->abort();
        thread_copy->exit(0);
        usleep(5000);
        image.load("/home/ar-pet/Downloads/ic_cancel.png");

         ui->label_gif_3->setVisible(false);
         ui->label_gif_4->setVisible(true);

         ui->label_gif_4->setPixmap(image);
         ui->label_gif_4->setScaledContents( true );
         ui->label_gif_4->show();
         adq_running = false;
         return;
    }

    if (ui->lineEdit_aqd_cant_archivos->text().toInt()==cant_archivos_copiados)
    {

        image.load("/home/ar-pet/Downloads/ic_check_circle.png");
        ui->progressBar->setValue(size_archivo_adq.toInt());
        ui->pbAdquirir->blockSignals(true);
        ui->pbAdquirir->setChecked(false);
        ui->pbAdquirir->blockSignals(false);
        ui->label_gif_3->setVisible(false);
        ui->label_gif_4->setVisible(true);

        ui->label_gif_4->setPixmap(image);
        ui->label_gif_4->setScaledContents( true );
        ui->label_gif_4->show();
    }
    else
    {
         if(finish_adquirir){
            checkStatusAdq(true);
            finish_adquirir=false;
         }

    }
}

