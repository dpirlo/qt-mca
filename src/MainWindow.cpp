#include "inc/MainWindow.h"
#include "ui_MainWindow.h"


/**
 * @brief MainWindow::MainWindow
 *
 * Constructor de la clase
 *
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    debug(false),
    init(false),
    initfile("/media/arpet/pet/calibraciones/03-info/cabezales/ConfigINI/config_cabs_linux.ini"),
    root_calib_path("/media/arpet/pet/calibraciones/campo_inundado/03-info"),
    root_config_path("/media/arpet/pet/calibraciones/03-info/cabezales"),
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
    if (debug) fclose (stdout);
    delete ui;
    delete pref;
    delete pmt_select;
}
/**
 * @brief MainWindow::setInitialConfigurations
 *
 * Inicialización de múltiples configuraciones relacionadas con la comunicación, los gráficos y la interfaz gráfica
 *
 */
void MainWindow::setInitialConfigurations()
{
    arpet = shared_ptr<MCAE>(new MCAE());
    pref = new SetPreferences(this);
    pmt_select = new SetPMTs(this);

    // Calibrador
    calibrador = shared_ptr<AutoCalib>(new AutoCalib());
    // Reconstructor
    recon_externa = shared_ptr<Reconstructor>(new Reconstructor());
    // Lleno defaults del contructor
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
    ui->textBrowser_entrada_2->setText(recon_externa->getPathAPIRL());
    ui->textBrowser_entrada_3->setText(recon_externa->getPathINTERFILES());








    manageHeadCheckBox("config",false);
    manageHeadCheckBox("mca",false);
    setAdquireMode(ui->comboBox_adquire_mode->currentIndex());
    ui->frame_adquire_advance_mode->hide();

    ui->lineEdit_WN->setValidator(new QIntValidator(1, 127, this));
    ui->lineEdit_WP->setValidator(new QIntValidator(1, 128, this));
    ui->lineEdit_pmt->setValidator( new QIntValidator(1, PMTs, this) );
    ui->lineEdit_pmt_terminal->setValidator( new QIntValidator(1, PMTs, this) );
    ui->lineEdit_hv_value->setValidator( new QIntValidator(0, MAX_HV_VALUE, this) );
    ui->lineEdit_pmt_hv_terminal->setValidator( new QIntValidator(0, MAX_HV_VALUE, this) );
    ui->lineEdit_alta->setValidator( new QIntValidator(MIN_HIGH_HV_VOLTAGE, MAX_HIGH_HV_VOLTAGE, this) );
    ui->lineEdit_psoc_hv_terminal->setValidator( new QIntValidator(MIN_HIGH_HV_VOLTAGE, MAX_HIGH_HV_VOLTAGE, this) );
    ui->tabWidget_general->setCurrentWidget(ui->config);
    ui->comboBox_port->addItems(availablePortsName());
    setQListElements();
    SetQCustomPlotConfiguration(ui->specPMTs, CHANNELS_PMT);
    SetQCustomPlotConfiguration(ui->specHead, CHANNELS);
    SetQCustomPlotSlots("Cuentas por PMT", "Cuentas en el Cabezal");
    resetHitsValues();    
}
/**
 * @brief MainWindow::setPreferencesConfiguration
 */
void MainWindow::setPreferencesConfiguration()
{
    /*Configuración inicial de preferencias*/
    QString default_pref_file = "\n[Debug]\ndebug=false\n[Paths]\nconf_set_file=" + initfile + "\ncalib_set_file=" + root_calib_path + "\n";
    writePreferencesFile(default_pref_file, preferencesfile);
    getPreferencesSettingsFile();
    writeLogFile();
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
     QObject::connect(ui->comboBox_head_mode_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(setHeadModeGraph(int)));
     QObject::connect(ui->comboBox_head_mode_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(setTabHead(int)));
     QObject::connect(ui->comboBox_head_mode_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(setHeadModeConfig(int)));
     QObject::connect(ui->comboBox_head_mode_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(setTabHead(int)));
     QObject::connect(ui->comboBox_adquire_mode ,SIGNAL(currentIndexChanged (int)),this,SLOT(setAdquireMode(int)));
     QObject::connect(ui->tabWidget_mca ,SIGNAL(currentChanged(int)),this,SLOT(setTabMode(int)));
     QObject::connect(ui->comboBox_head_mode_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadModeComboBoxToMCA(int)));
     QObject::connect(ui->comboBox_head_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadComboBoxToMCA(int)));
     QObject::connect(ui->comboBox_head_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(getHeadStatus()));
     QObject::connect(ui->comboBox_head_mode_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadModeComboBoxToConfig(int)));
     QObject::connect(ui->comboBox_head_select_graph ,SIGNAL(currentIndexChanged (int)),this,SLOT(syncHeadComboBoxToConfig(int)));
     QObject::connect(ui->comboBox_adquire_mode_coin ,SIGNAL(currentIndexChanged (int)),this,SLOT(setAdvanceCoinMode(int)));
     QObject::connect(ui->checkBox_mca_1 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead1ToConfig(bool)));
     QObject::connect(ui->checkBox_mca_2 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead2ToConfig(bool)));
     QObject::connect(ui->checkBox_mca_3 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead3ToConfig(bool)));
     QObject::connect(ui->checkBox_mca_4 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead4ToConfig(bool)));
     QObject::connect(ui->checkBox_mca_5 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead5ToConfig(bool)));
     QObject::connect(ui->checkBox_mca_6 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead6ToConfig(bool)));
     QObject::connect(ui->checkBox_c_1 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead1ToMCA(bool)));
     QObject::connect(ui->checkBox_c_2 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead2ToMCA(bool)));
     QObject::connect(ui->checkBox_c_3 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead3ToMCA(bool)));
     QObject::connect(ui->checkBox_c_4 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead4ToMCA(bool)));
     QObject::connect(ui->checkBox_c_5 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead5ToMCA(bool)));
     QObject::connect(ui->checkBox_c_6 ,SIGNAL(toggled(bool)),this,SLOT(syncCheckBoxHead6ToMCA(bool)));
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


    pmt_button_table.push_back(ui->pushButton_01);
    pmt_button_table.push_back(ui->pushButton_02);
    pmt_button_table.push_back(ui->pushButton_03);
    pmt_button_table.push_back(ui->pushButton_04);
    pmt_button_table.push_back(ui->pushButton_05);
    pmt_button_table.push_back(ui->pushButton_06);
    pmt_button_table.push_back(ui->pushButton_07);
    pmt_button_table.push_back(ui->pushButton_08);
    pmt_button_table.push_back(ui->pushButton_09);
    pmt_button_table.push_back(ui->pushButton_10);
    pmt_button_table.push_back(ui->pushButton_11);
    pmt_button_table.push_back(ui->pushButton_12);
    pmt_button_table.push_back(ui->pushButton_13);
    pmt_button_table.push_back(ui->pushButton_14);
    pmt_button_table.push_back(ui->pushButton_15);
    pmt_button_table.push_back(ui->pushButton_16);
    pmt_button_table.push_back(ui->pushButton_17);
    pmt_button_table.push_back(ui->pushButton_18);
    pmt_button_table.push_back(ui->pushButton_19);
    pmt_button_table.push_back(ui->pushButton_20);
    pmt_button_table.push_back(ui->pushButton_21);
    pmt_button_table.push_back(ui->pushButton_22);
    pmt_button_table.push_back(ui->pushButton_23);
    pmt_button_table.push_back(ui->pushButton_24);
    pmt_button_table.push_back(ui->pushButton_25);
    pmt_button_table.push_back(ui->pushButton_26);
    pmt_button_table.push_back(ui->pushButton_27);
    pmt_button_table.push_back(ui->pushButton_28);
    pmt_button_table.push_back(ui->pushButton_29);
    pmt_button_table.push_back(ui->pushButton_30);
    pmt_button_table.push_back(ui->pushButton_31);
    pmt_button_table.push_back(ui->pushButton_32);
    pmt_button_table.push_back(ui->pushButton_33);
    pmt_button_table.push_back(ui->pushButton_34);
    pmt_button_table.push_back(ui->pushButton_35);
    pmt_button_table.push_back(ui->pushButton_36);
    pmt_button_table.push_back(ui->pushButton_37);
    pmt_button_table.push_back(ui->pushButton_38);
    pmt_button_table.push_back(ui->pushButton_39);
    pmt_button_table.push_back(ui->pushButton_40);
    pmt_button_table.push_back(ui->pushButton_41);
    pmt_button_table.push_back(ui->pushButton_42);
    pmt_button_table.push_back(ui->pushButton_43);
    pmt_button_table.push_back(ui->pushButton_44);
    pmt_button_table.push_back(ui->pushButton_45);
    pmt_button_table.push_back(ui->pushButton_46);
    pmt_button_table.push_back(ui->pushButton_47);
    pmt_button_table.push_back(ui->pushButton_48);
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
    QString suffix = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    QString prefix = "LOG";
    QString extension = ".log";

    return prefix + main + suffix + extension;
}
/**
 * @brief MainWindow::writeLogFile
 */
void MainWindow::writeLogFile(QString main)
{
    QString logFile = getPreferencesDir() + "/logs/" + getLogFileName(main);
    freopen(logFile.toLocal8Bit().data(), "a+", stdout);
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

    int ret = pref->exec();
    bool debug_console = pref->getDegugConsoleValue();
    QString file = pref->getInitFileConfigPath();
    QString calib_path = pref->getCalibDirectoryPath();

    if(ret == QDialog::Accepted)
    {
        setDebugMode(debug_console);
        QString boolText = debug_console ? "true" : "false";        
        setPreferencesSettingsFile("Debug", "debug", boolText );        
        setPreferencesSettingsFile("Paths", "conf_set_file", file);
        setPreferencesSettingsFile("Paths", "calib_set_file", calib_path);
        getPreferencesSettingsFile();        
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
    QDir dir(path);
    QDir log(logs);
    if (!dir.exists())
    {
      dir.mkdir(path);      
    }
    if (!log.exists())
    {
        log.mkdir(logs);
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

    debug = qtmcasettins.value("Debug/debug", "US").toBool();
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
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    string msg;
    try
    {
        sendString(arpet->getAP_STATUS(),arpet->getEnd_MCA());
        msg = readString();
        if(arpet->verifyMCAEStream(msg,arpet->getAnsAP_ON()))
            {
                setButtonState(true,ui->pushButton_arpet_on);
                setButtonState(true,ui->pushButton_arpet_off, true);
            } else if(arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()))
            {
                setButtonState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->pushButton_arpet_off);
                setButtonState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->pushButton_arpet_on, true);
            }
    }
    catch(Exceptions & ex)
    {
        if(debug)
            {
                cout<<"Hubo un inconveniente al intentar acceder al estado del equipo. Revise la conexión. Error: "<<ex.excdesc<<endl;
                cout<<"[END-LOG-DBG] ====================================================="<<endl;
            }
        QMessageBox::critical(this,tr("Atención"),tr((string("Hubo un inconveniente al intentar acceder al estado del equipo. Revise la conexión. Error: ")+string(ex.excdesc)).c_str()));
    }
}
/**
 * @brief MainWindow::getHeadStatus
 *
 * Slot que incicializa el cabezal seleccionado y determina el estado de su fuente de HV (PSOC).
 *
 */
void MainWindow::getHeadStatus()
{
  if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;

  if(!arpet->isPortOpen())
  {
    QMessageBox::critical(this,tr("Error"),tr("No se puede acceder al puerto serie. Revise la conexión USB."));
    if(debug)
    {
      cout<<"No se puede acceder al puerto serie. Revise la conexión USB."<<endl;
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
    return;
  }

  int head_index = getHead("config").toInt();
  if(debug) cout<<"Cabezal: "<<head_index<<endl;

  /* Inicialización del Cabezal */
  initHead(head_index);
  initSP3(head_index);

    string msg;
    QVector<QString> ans_psoc;
    setPSOCDataStream("config",arpet->getPSOC_STA());
    try
    {
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
        msg = readString();
        ans_psoc = arpet->parserPSOCStream(msg);
        hv_status_table[head_index-1]->setText(QString::number(round(ans_psoc.at(2).toDouble()*arpet->getPSOC_ADC())));
        if (arpet->verifyMCAEStream(ans_psoc.at(1).toStdString(),"ON"))
            setLabelState(true, hv_status_table[head_index-1]);
        else
            setLabelState(false, hv_status_table[head_index-1]);
    }
    catch(Exceptions & ex)
    {
        if(debug)
            {
                cout<<"Hubo un inconveniente al intentar acceder al estado de la placa PSOC del cabezal. Revise la conexión. Error: "<<ex.excdesc<<endl;
                cout<<"[END-LOG-DBG] ====================================================="<<endl;
            }
        QMessageBox::critical(this,tr("Atención"),tr((string("Hubo un inconveniente al intentar acceder al estado de la placa PSOC del cabezal. Revise la conexión. Error: ")+string(ex.excdesc)).c_str()));
    }

}
/**
 * @brief MainWindow::on_pushButton_arpet_on_clicked
 *
 * Encendido del ARPET
 *
 */
void MainWindow::on_pushButton_arpet_on_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;

    string msg;
    try
    {
        sendString(arpet->getAP_ON(),arpet->getEnd_MCA());
        sleep(1);
        sendString(arpet->getAP_ON(),arpet->getEnd_MCA());
        sleep(1);
        sendString(arpet->getAP_ON(),arpet->getEnd_MCA());
        msg = readString();
        setButtonState(arpet->verifyMCAEStream(msg,arpet->getAnsAP_ON()),ui->pushButton_arpet_on);
        setButtonState(arpet->verifyMCAEStream(msg,arpet->getAnsAP_ON()),ui->pushButton_arpet_off, true);
        if(debug) cout<<"AR-PET ENCENDIDO"<<endl;

    }
    catch(Exceptions & ex)
    {
        if(debug) cout<<"Hubo un inconveniente al intentar encender el equipo. Revise la conexión. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("Hubo un inconveniente al intentar encender el equipo. Revise la conexión. Error: ")+string(ex.excdesc)).c_str()));
        setButtonState(arpet->verifyMCAEStream(msg,arpet->getAnsAP_ON()),ui->pushButton_arpet_on, true);
    }
    if(debug) cout<<"[END-LOG-DBG] ====================================================="<<endl;
}
/**
 * @brief MainWindow::on_pushButton_arpet_off_clicked
 *
 * Apagado del ARPET
 *
 */
void MainWindow::on_pushButton_arpet_off_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    string msg;
    try
    {
        sendString(arpet->getAP_OFF(),arpet->getEnd_MCA());
        msg = readString();
        setButtonState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->pushButton_arpet_off);
        setButtonState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->pushButton_arpet_on, true);
        if(debug) cout<<"AR-PET APAGADO"<<endl;
    }
    catch(Exceptions & ex)
    {
        if(debug) cout<<"Hubo un inconveniente al intentar apagar el equipo. Revise la conexión. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("Hubo un inconveniente al intentar apagar el equipo. Revise la conexión. Error: ")+string(ex.excdesc)).c_str()));
        setButtonState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->pushButton_arpet_off, true);
    }
    if(debug) cout<<"[END-LOG-DBG] ====================================================="<<endl;
}
/**
 * @brief MainWindow::on_pushButton_triple_ventana_clicked
 */
void MainWindow::on_pushButton_triple_ventana_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_triple_ventana->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_hv_clicked
 */
void MainWindow::on_pushButton_hv_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_hv->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_energia_clicked
 */
void MainWindow::on_pushButton_energia_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_energia->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_posicion_X_clicked
 */
void MainWindow::on_pushButton_posicion_X_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_posicion_X->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_posicion_Y_clicked
 */
void MainWindow::on_pushButton_posicion_Y_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_posicion_Y->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_tiempos_cabezal_clicked
 */
void MainWindow::on_pushButton_tiempos_cabezal_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_tiempos_cabezal->setText(fileName);
}
/**
 * @brief MainWindow::on_pushButton_obtener_ini_clicked
 */
void MainWindow::on_pushButton_obtener_ini_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    openConfigurationFile();
    setPreferencesSettingsFile("Paths","conf_set_file",initfile);
    if(debug)
    {
      cout<<"El nuevo archivo de configuración: "<<initfile.toStdString()<<endl;
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }

}
/**
 * @brief MainWindow::on_pushButton_init_configure_clicked
 */
void MainWindow::on_pushButton_init_configure_clicked()
{
    /** @todo: Ver de agregar un boton indicando el estado de conexión del puerto */
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    if(arpet->isPortOpen())
    {
        arpet->portDisconnect();
    }
    else
    {
        try{
            port_name=ui->comboBox_port->currentText();
            calibrador->setPort_Name(port_name);
            arpet->portConnect(port_name.toStdString().c_str());
            QMessageBox::information(this,tr("Información"),tr("Conectado al puerto: ") + port_name);            
            if(debug) cout<<"Puerto conectado en: "<<port_name.toStdString()<<endl;
            getARPETStatus();
            getHeadStatus();
        }
        catch(boost::system::system_error e)
        {
          if(debug) cout<<"No se puede acceder al puerto serie. Revise la conexión USB. Error: "<<e.what()<<endl;
          QMessageBox::critical(this,tr("Error"),tr("No se puede acceder al puerto serie. Revise la conexión USB. Error: ")+tr(e.what()));
        }
    }
    if(debug) cout<<"[END-LOG-DBG] ====================================================="<<endl;
}
/**
 * @brief MainWindow::on_pushButton_configure_clicked
 * @todo : Agregar el modo calibración
 */
void MainWindow::on_pushButton_configure_clicked()
{
   /* Inicialización del modo Coincidencia */

   if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
   int index=ui->comboBox_adquire_mode_coin->currentIndex();
   try{
       switch (index) {
   case COIN_NORMAL:
       if(debug) cout<<"Modo Coincidencia: Normal"<<endl;
       initCoincidenceMode();
       usleep(5000);
       setCoincidenceModeWindowTime();
       usleep(5000);
       setCoincidenceModeDataStream(arpet->getNormal_Coin_Mode());
       break;
   case COIN_AUTOCOINCIDENCE:
       if(debug) cout<<"Modo Coincidencia: Autocoincidencia"<<endl;
       initCoincidenceMode();
       usleep(5000);
       setCoincidenceModeWindowTime();
       usleep(5000);
       setCoincidenceModeDataStream(arpet->getAuto_Coin_Mode());
       break;
   case COIN_AVANCED:
       if(debug)
       {
           cout<<"Modo Coincidencia: Avanzado"<<endl;
           cout<<"Trama utilizada para la configuración avanzada: "<<getCoincidenceAdvanceModeDataStream()<<endl;
       }
       initCoincidenceMode();
       usleep(5000);
       setCoincidenceModeWindowTime();
       usleep(5000);
       setCoincidenceModeDataStream(getCoincidenceAdvanceModeDataStream());
       break;
   case COIN_CALIB:
       cout<<"No implementado aún"<<endl;
       break;
       return;
   default:
       break;
   }
       setLabelState(true,ui->label_coincidencia_estado);
   }
   catch(Exceptions & ex)
   {
       QMessageBox::critical(this,tr("Atención"),tr(string(ex.excdesc).c_str()));
       setLabelState(false,ui->label_coincidencia_estado);
   }
   if(debug) cout<<"[END-LOG-DBG] ====================================================="<<endl;
}
/**
 * @brief MainWindow::on_pushButton_initialize_clicked
 */
void MainWindow::on_pushButton_initialize_clicked()
{
   if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
   if(!arpet->isPortOpen())
   {
     QMessageBox::critical(this,tr("Error"),tr("No se puede acceder al puerto serie. Revise la conexión USB."));
     if(debug)
     {
       cout<<"No se puede acceder al puerto serie. Revise la conexión USB."<<endl;
       cout<<"[END-LOG-DBG] ====================================================="<<endl;
     }
     return;
   }

   QList<int> checkedHeads;
   if (ui->comboBox_head_mode_select_config->currentIndex()==MULTIHEAD)
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
       return;
   }

    for (int i=0;i<checkedHeads.length();i++)
    {
        int head_index=checkedHeads.at(i);
        parseConfigurationFile(true, QString::number(head_index));

        /* Configuración de las tablas de calibración */
        setCalibrationTables(head_index);

        /* Configuración del HV*/
        ui->lineEdit_alta->setText(QString::number(AT));
        ui->lineEdit_limiteinferior->setText(QString::number(LowLimit));
        string msg;
        QString psoc_alta = getPSOCAlta(ui->lineEdit_alta);
        setPSOCDataStream("config",arpet->getPSOC_SET(),psoc_alta);
        if(debug) cout<<"Cabezal: "<<head_index<<endl;
        try
        {
            sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
            msg = readString();
            hv_status_table[head_index-1]->setText(psoc_alta);
            if(debug) cout<< "HV configurado en: "<<psoc_alta.toStdString()<<endl;
        }
        catch(Exceptions & ex)
        {
            if (debug) cout<<"No se puede acceder a la placa de alta tensión. Revise la conexión al equipo. Error: "<<ex.excdesc<<endl;
            QMessageBox::critical(this,tr("Atención"),tr((string("No se puede acceder a la placa de alta tensión. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
        }        
    }

    if(debug) cout<<"[END-LOG-DBG] ====================================================="<<endl;
}
/**
 * @brief MainWindow::on_pushButton_hv_set_clicked
 */
void MainWindow::on_pushButton_hv_set_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    string msg;
    QString psoc_alta = getPSOCAlta(ui->lineEdit_alta);
    int head_index=setPSOCDataStream("config",arpet->getPSOC_SET(),psoc_alta);
    if(debug) cout<<"Cabezal: "<<head_index<<endl;
    try
    {
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_PSOC());
        msg = readString();
        hv_status_table[head_index-1]->setText(psoc_alta);
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
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
}
/**
 * @brief MainWindow::on_pushButton_hv_on_clicked
 */
void MainWindow::on_pushButton_hv_on_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    string msg;
    int head_index=setPSOCDataStream("config",arpet->getPSOC_ON());
    if(debug) cout<<"Cabezal: "<<head_index<<endl;
    try
    {
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
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
}
/**
 * @brief MainWindow::on_pushButton_hv_off_clicked
 */
void MainWindow::on_pushButton_hv_off_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    string msg;
    int head_index=setPSOCDataStream("config",arpet->getPSOC_OFF());
    if(debug) cout<<"Cabezal: "<<head_index<<endl;
    try
    {
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
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
}
/**
 * @brief MainWindow::on_pushButton_hv_estado_clicked
 */
void MainWindow::on_pushButton_hv_estado_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    string msg;
    setPSOCDataStream("config",arpet->getPSOC_STA());
    if(debug) cout<<"Cabezal: "<<getHead("config").toStdString()<<endl;
    try
    {
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
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
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
    try
    {
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
    /* Inicialización de modo coincidencia */
    setMCAEDataStream(arpet->getInit_Coin(),"","",false);
    string msg_ans;
    try
    {
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

    try
    {
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
    setMCAEDataStream("config", arpet->getFunCHead(), arpet->getBrCst(), arpet->getInit_MCA());
    string msg_head;

    try
    {
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg_head = readString();
    }
    catch(Exceptions & ex)
    {
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
   setMCAEDataStream("config", arpet->getFunCSP3(), arpet->getBrCst(), arpet->getInit_MCA());
   string msg_pmts;
   try
   {
       sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
       msg_pmts = readString();
   }
   catch(Exceptions & ex)
   {
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
void MainWindow::setCalibrationTables(int head)
{
    coefenerg_values=getValuesFromFiles(coefenerg);
    hvtable_values=getValuesFromFiles(hvtable,true);
    coefx_values=getValuesFromFiles(coefx);
    coefy_values=getValuesFromFiles(coefy);
    coefT_values=getValuesFromFiles(coefT);
    coefest_values=getValuesFromFiles(coefest);
    bool x_calib = true, y_calib = true, energy_calib = true, windows_limits = true, set_hv = true, set_time = true;
    QString q_msg;

    try
    {
        q_msg = setCalibTable(QString::number(head).toStdString(), arpet->getX_Calib_Table(), coefx_values, arpet->getAnsX_Calib_Table());
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
        q_msg = setCalibTable(QString::number(head).toStdString(), arpet->getY_Calib_Table(), coefy_values, arpet->getAnsY_Calib_Table());
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
        q_msg = setCalibTable(QString::number(head).toStdString(), arpet->getEnergy_Calib_Table(), coefenerg_values, arpet->getAnsEnergy_Calib_Table());
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
        q_msg = setCalibTable(QString::number(head).toStdString(), arpet->getWindow_Limits_Table(),coefest_values, arpet->getAnsWindow_Limits_Table());
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

    try
    {
        for(int pmt = 0; pmt < PMTs; pmt++)
        {
            QString hv=QString::number(hvtable_values[pmt]);
            q_msg = setHV(QString::number(head).toStdString() ,hv.toStdString(), QString::number(pmt+1).toStdString());
            if(debug)
            {
                cout<<"========================================="<<endl;
                cout<<"PMT: "<< QString::number(pmt+1).toStdString() <<endl;
                showMCAEStreamDebugMode(q_msg.toStdString());
                cout<<"Valor de HV: "<< hv.toStdString() <<endl;
                cout<<"========================================="<<endl;

            }
        }
    }
    catch( Exceptions & ex )
    {
        set_hv = false;
        if (debug) cout<<"No se pueden configurar las tablas de calibración en HV. Error: "<<ex.excdesc<<endl;
    }
    setTextBrowserState(set_hv, ui->textBrowser_hv);

    try
    {
        for(int pmt = 0; pmt < PMTs; pmt++)
        {
            q_msg = setTime(QString::number(head).toStdString(), coefT_values[pmt], QString::number(pmt+1).toStdString());
            if(debug)
            {
                cout<<"========================================="<<endl;
                showMCAEStreamDebugMode(q_msg.toStdString());
                cout<<"Valor de tiempo: "<< QString::number(coefT_values[pmt]).toStdString() <<endl;
                cout<<"========================================="<<endl;
            }

        }
    }
    catch( Exceptions & ex )
    {
        set_time = false;
        if (debug) cout<<"No se pueden configurar las tablas de calibración en Tiempos en el Cabezal. Error: "<<ex.excdesc<<endl;
    }
    setTextBrowserState(set_time, ui->textBrowser_tiempos_cabezal);
    if (debug) cout<<"Final de la configuración de las tablas de calibración "<<endl;
    setLabelState(x_calib && y_calib && energy_calib && windows_limits && set_hv && set_time, calib_status_table[head-1]);
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
        palette_temperature.setColor(QPalette::Background,Qt::lightGray);
        break;
    case NORMAL:
        palette_temperature.setColor(QPalette::Background,Qt::green);
        break;
    case WARM:
        palette_temperature.setColor(QPalette::Background,Qt::yellow);
        break;
    case HOT:
        palette_temperature.setColor(QPalette::Background,QColor::fromRgb(255,140,0)); // Naranja en RGB = 255,140,0
        break;
    case TOO_HOT:
        palette_temperature.setColor(QPalette::Background,Qt::red);
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

    try
    {
        setButtonAdquireState(true);
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
    }
    catch( Exceptions & ex )
    {
        setButtonAdquireState(false);
        if(debug) cout<<"Imposible obtener los valores de temperatura. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("Imposible obtener los valores de temperatura. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    setButtonAdquireState(true, true);
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

    ui->label_title_output->setText("Temperatura");
    ui->label_data_output->setText("Media: "+QString::number(mean)+"°C"+"\nMáxima: "+QString::number(t_max)+"°C"+"\nMínima: "+QString::number(t_min)+"°C");
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
    if(index==MULTIHEAD)
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
    ui->specHead->clearGraphs();
    try
    {
        setButtonAdquireState(true);
        msg = getMCA(head.toStdString(), arpet->getFunCHead(), true, CHANNELS);
        if(debug) showMCAEStreamDebugMode(msg.toStdString());
        addGraph(arpet->getHitsMCA(),ui->specHead,CHANNELS, head, qcp_head_parameters[0]);        
    }
    catch(Exceptions & ex)
    {
        setButtonAdquireState(false);
        if(debug) cout<<"No se pueden obtener los valores de MCA del Cabezal. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se pueden obtener los valores de MCA. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
        Exceptions exception_timeout(ex.excdesc);
        throw exception_timeout;
    }
    setButtonAdquireState(true, true);

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
        setButtonAdquireState(true);
        for (int index=0;index<size_pmt_selected;index++)
            {
                string pmt = pmt_selected_list.at(index).toStdString();
                msg = getMCA(head.toStdString(), arpet->getFunCSP3(), false, CHANNELS_PMT, pmt);
                if(debug)
                    {
                        cout<<"PMT: "<<pmt<<" "<<endl;
                        showMCAEStreamDebugMode(msg.toStdString());
                    }
                addGraph(arpet->getHitsMCA(),ui->specPMTs,CHANNELS_PMT, QString::fromStdString(pmt), qcp_pmt_parameters[index]);                
            }
        if(debug) cout<<"Se obtuvieron las cuentas MCA de los PMTs seleccionados de forma satisfactoria."<<endl;
    }
    catch(Exceptions & ex)
    {
        setButtonAdquireState(false);
        if(debug) cout<<"No se pueden obtener los valores de MCA de los PMTs seleccionados. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se pueden obtener los valores de MCA. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }
    setButtonAdquireState(true, true);

    if (debug)
    {
        cout<<getLocalDateAndTime()<<" Tasa de conteo en el Cabezal "<<head.toStdString()<<": "<<QString::number(arpet->getRate(head.toStdString(), port_name.toStdString())).toStdString()<<endl;
    }

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
    try
    {
        msg= arpet->getMCA(pmt, function, head, channels, port_name.toStdString());
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_mca(ex.excdesc);
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
        ui->label_title_output->setText("MCA Extended");
        ui->label_data_output->setText("Tasa: "+QString::number(arpet->getRate(head, port_name.toStdString()))+" Varianza: "+QString::number(var)+" Offset ADC: "+QString::number(offset)+" Tiempo (mseg): "+QString::number(time_mca/1000));
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

    if (debug && multimode)
    {
        cout<<getLocalDateAndTime()<<" Tasa de conteo en el Cabezal "<<head<<": "<<QString::number(arpet->getRate(head, port_name.toStdString())).toStdString()<<endl;
    }

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
    try
    {
        msg = arpet->setTime(head, time_value, pmt, port_name.toStdString());
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_time(ex.excdesc);
        throw exception_time;
    }

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
    try
    {
        msg = arpet->setHV(head, pmt, hv_value, port_name.toStdString());
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_hv(ex.excdesc);
        throw exception_hv;
    }
    resetHitsValues();

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
 * @brief MainWindow::setPMT
 * @param value
 */
void MainWindow::setPMT(int value)
{
     ui->lineEdit_pmt->setText(QString::number(value));
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
 * Se configura la trama general de MCAE para el envío de MCA. Este método recibe como parámetros el 'tab'
 * del entorno gráfico, la 'function' de MCAE (si es para planar o SP3), el valor de 'pmt', la función MCA ('mca_function'),
 * el tamaño de la trama de recepción 'bytes_mca' (opcional) y en el caso que se realice la confifuración de HV se debe
 * incorporar el valor de HV, caso contrario dejar este campo en blanco.
 *
 * @param tab
 * @param function
 * @param pmt
 * @param mca_function
 * @param bytes_mca (opcional)
 * @param hv_value (opcional)
 */
void MainWindow::setMCAEDataStream(string tab, string function, string pmt, string mca_function, int bytes_mca, string hv_value)
{
  arpet->setHeader_MCAE(arpet->getHead_MCAE() + getHead(tab).toStdString() + function);
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
 * @brief MainWindow::setPSOCDataStream
 *
 * Configuración de la trama MCAE para la placa PSOC
 *
 * @param tab
 * @param function
 * @param psoc_value
 * @return
 */
int MainWindow::setPSOCDataStream(string tab, string function, QString psoc_value)
{
    QString head=getHead(tab);
    int head_index=head.toInt();
    arpet->setHeader_MCAE(arpet->getHead_MCAE() + head.toStdString() + arpet->getFunCPSOC());
    arpet->setPSOCEStream(function, psoc_value.toStdString());

    return head_index;
}
/**
 * @brief MainWindow::resetHitsValues
 */
void MainWindow::resetHitsValues()
{
  arpet->resetHitsMCA();
  setHitsInit(true);
}
/**
 * @brief MainWindow::on_pushButton_adquirir_clicked
 */
void MainWindow::on_pushButton_adquirir_clicked()
{
    /** @todo: Agregar el modo continuo*/
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    if(debug) cout<<"Cabezal: "<<getHead("mca").toStdString()<<endl;

    QList<int> checkedHeads;
    if (ui->comboBox_head_mode_select_config->currentIndex()==MULTIHEAD)
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
        return;
    }

    QString q_msg;

    switch (adquire_mode) {
    case PMT:
        if (ui->comboBox_head_mode_select_config->currentIndex()==MONOHEAD) {q_msg = getMultiMCA(QString::number(checkedHeads.at(0)));}
        else
        {
            QMessageBox::critical(this,tr("Atención"),tr("Esta función se encuentra habilitada solo para un cabezal seleccionado"));
            return;
        }
        break;
    case CABEZAL:
        for (int i=0;i<checkedHeads.length();i++)
        {
            try
            {
                q_msg = getHeadMCA(QString::number(checkedHeads.at(i)));
            }
            catch(Exceptions & ex)
            {
                if(debug) cout<<"No se puede continuar con el proceso de adquisición. Error: "<<ex.excdesc<<endl;
                setButtonAdquireState(true, true);
                return;
            }
        }
        break;
    case TEMPERATURE:
        if (ui->comboBox_head_mode_select_config->currentIndex()==MONOHEAD) {drawTemperatureBoard();}
        else
        {
            QMessageBox::critical(this,tr("Atención"),tr("Esta función se encuentra habilitada solo para un cabezal seleccionado"));
            return;
        }
        break;
    default:
        break;
    }
    if(debug) cout<<"[END-LOG-DBG] ====================================================="<<endl;
}
/**
 * @brief MainWindow::on_pushButton_reset_clicked
 */
void MainWindow::on_pushButton_reset_clicked()
{
    /** @todo Verificar el reinicio de datos en los vectores de cuentas de MCA. Reiniciar con la función '67' */
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    if(debug) cout<<"Cabezal: "<<getHead("mca").toStdString()<<endl;

    switch (adquire_mode) {
        case PMT:
            if(debug) cout<<"Se reiniciaron los valores de los PMTs"<<endl;
            resetHitsValues();
            setPMTCustomPlotEnvironment(pmt_selected_list);
            removeAllGraphsPMT();
            break;
        case CABEZAL:
            if(debug) cout<<"Se reiniciaron los valores del cabezal"<<endl;
            resetHitsValues();
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
    if(debug) cout<<"[END-LOG-DBG] ====================================================="<<endl;
}
/**
 * @brief MainWindow::on_pushButton_select_pmt_clicked
 */
void MainWindow::on_pushButton_select_pmt_clicked()
{

    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    resetHitsValues();
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
    if(debug) cout<<"[END-LOG-DBG] ====================================================="<<endl;
}
/**
 * @brief MainWindow::on_pushButton_hv_configure_clicked
 */
void MainWindow::on_pushButton_hv_configure_clicked()
{    
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    QString q_msg;
    try
    {
        q_msg =setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
        if(debug) cout<<getHVValue(ui->lineEdit_hv_value)<<endl;
        ui->label_data_output->setText("PMT: "+QString::number(getPMT(ui->lineEdit_pmt))+"\nCanal configurado: " + QString::fromStdString(getHVValue(ui->lineEdit_hv_value))+"\nConfiguración OK.");
    }
    catch (Exceptions ex)
    {
        if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }
    ui->label_title_output->setText("Tensión de Dinodo");
    if (debug)
    {
      showMCAEStreamDebugMode(q_msg.toStdString());
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
}
/**
 * @brief MainWindow::on_pushButton_l_5_clicked
 */
void MainWindow::on_pushButton_l_5_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    QString q_msg;
    try
    {
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,-5),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
        if(debug) cout<<getHVValue(ui->lineEdit_hv_value,-5)<<endl;
    }
    catch (Exceptions ex)
    {
      if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
      QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
      showMCAEStreamDebugMode(q_msg.toStdString());
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
}
/**
 * @brief MainWindow::on_pushButton_l_10_clicked
 */
void MainWindow::on_pushButton_l_10_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    QString q_msg;
    try
    {
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,-10),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
        if(debug) cout<<getHVValue(ui->lineEdit_hv_value,-10)<<endl;
    }
    catch (Exceptions ex)
    {
      if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
      QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
      showMCAEStreamDebugMode(q_msg.toStdString());
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
}
/**
 * @brief MainWindow::on_pushButton_l_50_clicked
 */
void MainWindow::on_pushButton_l_50_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    QString q_msg;
    try
    {
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,-50),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
        if(debug) cout<<getHVValue(ui->lineEdit_hv_value,-50)<<endl;
    }
    catch (Exceptions ex)
    {
      if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
      QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
      showMCAEStreamDebugMode(q_msg.toStdString());
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
}
/**
 * @brief MainWindow::on_pushButton_p_5_clicked
 */
void MainWindow::on_pushButton_p_5_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    QString q_msg;
    try
    {
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,5),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
        if(debug) cout<<getHVValue(ui->lineEdit_hv_value,5)<<endl;
    }
    catch (Exceptions ex)
    {
        if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
        QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
      showMCAEStreamDebugMode(q_msg.toStdString());
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
}
/**
 * @brief MainWindow::on_pushButton_p_10_clicked
 */
void MainWindow::on_pushButton_p_10_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    QString q_msg;
    try
    {
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,10),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
        if(debug) cout<<getHVValue(ui->lineEdit_hv_value,10)<<endl;
    }
    catch (Exceptions ex)
    {
      if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
      QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
      showMCAEStreamDebugMode(q_msg.toStdString());
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
}
/**
 * @brief MainWindow::on_pushButton_p_50_clicked
 */
void MainWindow::on_pushButton_p_50_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    QString q_msg;
    try
    {
        q_msg = setHV(getHead("mca").toStdString(),getHVValue(ui->lineEdit_hv_value,50),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
        if(debug) cout<<getHVValue(ui->lineEdit_hv_value,50)<<endl;
    }
    catch (Exceptions ex)
    {
      if(debug) cout<<"No se puede configurar el valor de HV. Error: "<<ex.excdesc<<endl;
      QMessageBox::critical(this,tr("Atención"),tr((string("No se puede configurar el valor de HV. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

    if (debug)
    {
      showMCAEStreamDebugMode(q_msg.toStdString());
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }
}


/* Métodos generales del entorno gráfico */
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
    LowLimit = settings.value("Cabezal"+head+"/LowLimit", "US").toInt();
    Target = settings.value("Cabezal"+head+"/Target", "US").toInt();
    coefenerg = root+settings.value("Cabezal"+head+"/coefenerg", "US").toString();
    hvtable = root+settings.value("Cabezal"+head+"/hvtable", "US").toString();
    coefx = root+settings.value("Cabezal"+head+"/coefx", "US").toString();
    coefy = root+settings.value("Cabezal"+head+"/coefy", "US").toString();
    coefest = root+settings.value("Cabezal"+head+"/coefest", "US").toString();
    coefT = root+settings.value("Cabezal"+head+"/coefT", "US").toString();

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
        ui->lineEdit_alta->setText(QString::number(AT));
        ui->lineEdit_limiteinferior->setText(QString::number(LowLimit));
    }
    else
    {
        ui->textBrowser_triple_ventana->setText("");
        ui->textBrowser_hv->setText("");
        ui->textBrowser_energia->setText("");
        ui->textBrowser_posicion_X->setText("");
        ui->textBrowser_posicion_Y->setText("");
        ui->textBrowser_tiempos_cabezal->setText("");
        ui->lineEdit_alta->setText("");
        ui->lineEdit_limiteinferior->setText("");
    }

}

/**
 * @brief MainWindow::setLabelState
 * @param state
 * @param label
 */
void MainWindow::setLabelState(bool state, QLabel *label)
{
    QPalette palette;

    if (state)
    {
        palette.setColor(QPalette::Background,Qt::green);
        label->setPalette(palette);
    }
    else
    {
        palette.setColor(QPalette::Background,Qt::red);
        label->setPalette(palette);
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
        tbro->setStyleSheet("background-color: green;");;
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
        color="background-color: green";
    }
    else if (!state && !disable)
    {
        color="background-color: red";
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
void MainWindow::setButtonAdquireState(bool state, bool disable)
{
    QString qt_text;

    if (state && !disable)
        {
           qt_text="Adquiriendo";
           setButtonState(state,ui->pushButton_adquirir,disable);
        }
    else if (!state && !disable)
        {
           qt_text="Error";
           setButtonState(state,ui->pushButton_adquirir,disable);
        }
    else
        {
           qt_text="Adquirir";
           setButtonState(state,ui->pushButton_adquirir,disable);
        }
    ui->pushButton_adquirir->setText(qt_text);
    ui->pushButton_adquirir->update();
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

    QDir dir("/dev/");
    QStringList filters;

    filters << "ttyUSB*";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::System);
    QFileInfoList list = dir.entryInfoList();

    for (int i=0; i< list.size(); i++)
    {
        portsName.append(list.at(i).canonicalFilePath ());
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
        if (ui->comboBox_head_mode_select_graph->currentIndex()==MONOHEAD)
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
    string msg;
    try{
        msg = arpet->readString(delimeter, port_name.toStdString());
    }
    catch( Exceptions & ex ){
        Exceptions exception_stop(ex.excdesc);
        throw exception_stop;
    }
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
    string msg;
    try{
        msg = arpet->readBufferString(buffer_size,port_name.toStdString());
    }
    catch( Exceptions & ex ){
        Exceptions exception_stop(ex.excdesc);
        throw exception_stop;
    }
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
    arpet->portFlush();
    size_t bytes_transfered = 0;

    try{
        bytes_transfered = arpet->sendString(msg, end, port_name.toStdString());
    }
    catch(boost::system::system_error e){
        Exceptions exception_serial_port((string("No se puede acceder al puerto serie. Error: ")+string(e.what())).c_str());
        throw exception_serial_port;
    }

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
        if (show) ui->frame_multihead_config->show();
        else ui->frame_multihead_config->hide();
    }
    else if(tab=="mca")
    {
        if (show) ui->frame_multihead_graph->show();
        else ui->frame_multihead_graph->hide();
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
    if (tab=="config"){
        if (show) ui->comboBox_head_select_config->show();
        else ui->comboBox_head_select_config->hide();
    }
    else if(tab=="mca")
    {
        if (show) ui->comboBox_head_select_graph->show();
        else ui->comboBox_head_select_graph->hide();
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

    if(ui->checkBox_end_terminal->isChecked()) end_stream=arpet->getEnd_PSOC();

    try
    {
        bytes = sendString(sended.toStdString(),end_stream);
        msg = readString();
    }
    catch(Exceptions & ex)
    {
        QMessageBox::critical(this,tr("Atención"),tr(ex.excdesc));
    }

    QString q_msg=QString::fromStdString(msg);
    QString q_bytes=QString::number(bytes);
    ui->label_size_terminal->setText(q_bytes);
    ui->label_received_terminal->setText(q_msg);
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
    setMCAEDataStream("terminal", function, QString::number(pmt).toStdString(), mca_function, bytes_mca, hv_value);
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
            setPSOCDataStream("terminal",arpet->getPSOC_ON());
            break;
        case 1:
            setPSOCDataStream("terminal",arpet->getPSOC_OFF());
            break;
        case 2:
            psoc_alta = getPSOCAlta(ui->lineEdit_psoc_hv_terminal);
            setPSOCDataStream("terminal",arpet->getPSOC_SET(),psoc_alta);
            break;
        case 3:
            setPSOCDataStream("terminal",arpet->getPSOC_STA());
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
    graph->graph()->setName(graph_legend);
    graph->graph()->setData(channels_ui,hits);
    graph->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(param[4])));
    QPen graphPen;
    graphPen.setColor(QColor(param[0], param[1], param[2]));
    graphPen.setWidthF(param[5]);
    graph->graph()->setPen(graphPen);
    graph->legend->setVisible(true);
    graph->legend->setWrap(4);
    graph->legend->setRowSpacing(1);
    graph->legend->setColumnSpacing(2);
    graph->replot();
    graph->rescaleAxes();
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
    ui->specPMTs->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}
/**
 * @brief MainWindow::mouseWheelHead
 */
void MainWindow::mouseWheelHead()
{
  if (ui->specHead->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->specHead->axisRect()->setRangeZoom(ui->specHead->xAxis->orientation());
  else if (ui->specHead->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->specHead->axisRect()->setRangeZoom(ui->specHead->yAxis->orientation());
  else
    ui->specHead->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
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

void MainWindow::on_pushButton_clicked()
{
    QList<int> checked_PMTs, checked_Cab;
    QMessageBox messageBox;


    if (ui->checkBox_Cab_Completo->isChecked())
    {
        for(int i = 0;  i< PMTs ; i++ )
        {
            checked_PMTs.append(i+1);
        }
    }
    else
    {
        // Recupero los PMT checkeados
        for(int i = 0;  i< PMTs ; i++ )
        {
            if (pmt_button_table[i]->isChecked())
            {
                checked_PMTs.append(i+1);
            }
        }
        if(checked_PMTs.length() == 0)
        {
            messageBox.critical(0,"Error","No se ha seleccionado ningún PMT.");
            messageBox.setFixedSize(500,200);
            return;
        }
        calibrador->setPMT_List(checked_PMTs);
    }



    // Recupero el canal objetivo
    QString Canal_obj = ui->Canal_objetivo->text();
    if(Canal_obj.toFloat() < 50 || Canal_obj.toFloat() > 250)
    {
        messageBox.critical(0,"Error","Canal fuera de los limites fijados.");
        messageBox.setFixedSize(500,200);
        return;
    }
    calibrador->setCanal_Obj(Canal_obj.toInt());

    // Recupero el tiempo de adquisicion
    QString Tiempo_adq = ui->Tiempo_adq_box->text();
    if(Tiempo_adq.toInt() < 0 || Tiempo_adq.toInt() > 360)
    {
        messageBox.critical(0,"Error","Tiempo adquisicion fuera de los limites fijados.");
        messageBox.setFixedSize(500,200);
        return;
    }
    calibrador->setTiempo_adq(Tiempo_adq.toInt());



    // Recupero los cabezales
    for(int i = 0; i < ui->frame_multihead_graph_2->children().size(); i++)
    {
        QCheckBox *q = qobject_cast<QCheckBox*>(ui->frame_multihead_graph_2->children().at(i));

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

    // Debug
    //for(int i=0; i<checked_PMTs.length();i++) { cout<<checked_PMTs[i]<<endl; }
    //cout<<"Canal Objetivo:"<<Canal_obj.toStdString()<<endl;

/*
    double canales_locos[1024] = {0,0,0,0,35,21,7,9,20,24,20,18,18,20,29,38,478,445,426,393,434,555,683,667,713,690,744,738,736,690,731,656,698,656,656,613,640,656,643,649,653,701,747,711,791,824,803,923,830,900,881,880,868,973,926,976,1037,962,1077,997,980,1041,973,1053,992,976,973,879,974,927,974,971,940,949,972,918,977,944,939,906,921,902,935,906,831,836,765,741,632,548,578,468,447,399,362,350,311,228,231,203,186,190,140,138,143,108,119,116,114,108,116,119,106,122,83,115,88,104,96,108,91,93,88,94,96,107,96,99,106,96,83,89,104,93,81,85,87,91,95,97,101,75,105,84,101,98,91,104,87,105,105,102,90,98,112,97,110,100,114,125,123,123,115,128,115,117,131,129,127,122,134,125,153,121,137,141,139,142,127,155,136,143,150,153,138,137,146,152,113,131,135,157,131,135,120,129,123,143,118,113,116,115,103,122,106,123,104,98,112,115,139,102,128,98,95,102,113,95,103,106,103,83,91,90,92,122,88,109,109,109,99,124,131,104,117,107,98,119,127,129,101,114,117,119,129,116,100,103,81,108,103,90,91,94,93,68,78,75,73,67,63,59,65,55,51,36,44,57,40,27,39,33,41,28,24,22,16,19,18,20,20,15,21,12,17,10,12,7,17,12,9,20,14,14,8,12,14,5,7,15,13,5,6,6,6,6,1,9,6,7,4,9,5,8,7,6,4,4,4,3,4,2,6,1,5,8,5,6,6,3,6,6,1,3,7,5,3,3,5,4,2,0,4,4,4,1,6,3,2,1,0,4,4,4,1,5,1,6,3,1,3,1,1,2,4,2,1,2,1,1,3,0,2,2,4,0,0,3,1,2,1,1,0,3,1,2,0,1,1,3,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,2,1,0,0,0,0,0,0,0,1,0,0,3,2,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1,0,3,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    double lala = calibrador->Buscar_Pico(canales_locos, 1024);
    cout<<lala<<endl;
*/


    // Cierro el serie de arpet
    //cout<<"Soltando puerto serie de arpet..."<<endl;
    arpet->portDisconnect();

    // Calibro
    //cout<<"Calibrador..."<<endl;
    calibrador->calibrar_simple(ui->specPMTs_2);

    // Devuelvo serial a arpet
    //cout<<"Devolviendo puerto serie de arpet..."<<endl;
    arpet->portConnect(port_name.toStdString().c_str());

}


/* Calibracion Fina */

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



void MainWindow::on_pushButton_triple_ventana_2_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_1(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_1->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_3_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_2(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_2->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_4_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_3(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_3->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_6_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_4(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_4->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_7_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_5(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_5->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_5_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_6(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_6->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_8_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición parseada (*.dat)"));
    calibrador->setAdq_Coin(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_coin->setText(filename);
}


void MainWindow::on_pushButton_triple_ventana_9_clicked()
{
    QString root="Salidas/";

    QFileDialog dialog;
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir directorio de salida"),
                           root);

    calibrador->setPathSalida(filename+"/");
    cout<<"Salida: "<<filename.toStdString()<<endl;

    ui->textBrowser_salida->setText(filename);
}



/* Analizar Planar */


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


void MainWindow::on_pushButton_triple_ventana_13_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_1(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_8->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_10_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_2(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_10->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_11_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_3(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_7->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_16_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_4(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_9->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_15_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_5(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_12->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_12_clicked()
{
    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw)"));
    calibrador->setAdq_Cab_6(filename.toStdString());
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_adq_11->setText(filename);
}

void MainWindow::on_pushButton_triple_ventana_14_clicked()
{
    QString root="Salidas/";

    QFileDialog dialog;
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir directorio de entrada"),
                           root);

    calibrador->setPathEntrada(filename+"/");
    cout<<"Salida: "<<filename.toStdString()<<endl;

    ui->textBrowser_entrada->setText(filename);
}

/* FPGA */


/* RECONSTRUCCION */

void MainWindow::on_pushButton_5_clicked()
{

    // Seteo el texto a modo solo lectura
    ui->plainTextEdit_Recon_console->setReadOnly(true);

    ui->plainTextEdit_Recon_console->appendPlainText("T"); // Adds the message to the widget

    //m_logFile.write(text); // Logs to file



}


void MainWindow::on_pushButton_APIRL_PATH_clicked()
{
    QString root=recon_externa->getPathAPIRL();

    QFileDialog dialog;
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir carpeta del build de APIRL"),
                           root);

    recon_externa->setPathAPIRL(filename+"/");


    cout<<"Directorio APIRL: "<<filename.toStdString()<<endl;

    ui->textBrowser_entrada_2->setText(filename);
}

void MainWindow::on_pushButton_INTERFILES_clicked()
{
    QString root=recon_externa->getPathINTERFILES();

    QFileDialog dialog;
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QString filename = dialog.getExistingDirectory(this, tr("Abrir carpeta de scripts de interfiles"),
                           root);

    recon_externa->setPathINTERFILES(filename+"/");


    cout<<"Directorio INTERFILES: "<<filename.toStdString()<<endl;

    ui->textBrowser_entrada_3->setText(filename);
}


void MainWindow::on_pushButton_arch_recon_clicked()
{

    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de adquisición"),
                                                    root_calib_path,
                                                    tr("Adquisición (*.raw);; Interfiles (*.h33)"));
    recon_externa->setArchRecon(filename);
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_archivo_recon->setText(filename);

}

void MainWindow::on_pushButton_Est_ini_clicked()
{

    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de estimacion inicial"),
                                                    root_calib_path,
                                                    tr("Interfiles (*.h33)"));
    recon_externa->setArchInicial(filename);
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_estimacion_ini->setText(filename);

}

void MainWindow::on_pushButton_Arch_sens_clicked()
{

    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de sensibilidad"),
                                                    root_calib_path,
                                                    tr("Interfiles (*.h33)"));
    recon_externa->setArchSensib(filename);
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_Imagensensib->setText(filename);


}

void MainWindow::on_pushButton_Arch_count_skimming_clicked()
{

    getPreferencesSettingsFile();
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de Count Skimming"),
                                                    root_calib_path,
                                                    tr("Conut Skimming (*.csv)"));
    recon_externa->setArchCountSkimm(filename);
    cout<<filename.toStdString()<<endl;

    ui->textBrowser_Conunt_skimming->setText(filename);


}

