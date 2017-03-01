#include "inc/MainWindow.h"
#include "ui_MainWindow.h"
#include "QMessageBox"

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
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->maximumSize());
    setInitialConfigurations();
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
    SetQCustomPlotSlots("Cuentas por PMT", "Cuentas en el Cabezal" );
    resetHitsValues();
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
     QObject::connect(ui->comboBox_head_mode_select_config ,SIGNAL(currentIndexChanged (int)),this,SLOT(setHeadModeConfig(int)));
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

/* Menu: Preferencias */
/**
 * @brief MainWindow::on_actionPreferencias_triggered
 */
void MainWindow::on_actionPreferencias_triggered()
{
    int ret = pref->exec();
    bool debug_console=pref->GetDegugConsoleValue();

    if(ret == QDialog::Accepted)
    {
        setDebugMode(debug_console);
    }
}
/**
 * @brief MainWindow::showMCAEStreamDebugMode
 * @param msg
 */
void MainWindow::showMCAEStreamDebugMode(string msg)
{
    cout<< "Trama recibida: "<< msg << " | Trama enviada: "<< arpet->getTrama_MCAE()<<endl;
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
        SetButtonState(true,ui->pushButton_arpet_on);
        SetButtonState(true,ui->pushButton_arpet_off, true);
      } else if(arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()))
      {
        SetButtonState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->pushButton_arpet_off);
        SetButtonState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->pushButton_arpet_on, true);
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
        SetButtonState(arpet->verifyMCAEStream(msg,arpet->getAnsAP_ON()),ui->pushButton_arpet_on);
        SetButtonState(arpet->verifyMCAEStream(msg,arpet->getAnsAP_ON()),ui->pushButton_arpet_off, true);
        if(debug) cout<<"AR-PET ENCENDIDO"<<endl;

    }
    catch(Exceptions & ex)
    {
      if(debug) cout<<"Hubo un inconveniente al intentar encender el equipo. Revise la conexión. Error: "<<ex.excdesc<<endl;
      QMessageBox::critical(this,tr("Atención"),tr((string("Hubo un inconveniente al intentar encender el equipo. Revise la conexión. Error: ")+string(ex.excdesc)).c_str()));
      SetButtonState(arpet->verifyMCAEStream(msg,arpet->getAnsAP_ON()),ui->pushButton_arpet_on, true);
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
        SetButtonState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->pushButton_arpet_off);
        SetButtonState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->pushButton_arpet_on, true);
        if(debug) cout<<"AR-PET APAGADO"<<endl;
    }
    catch(Exceptions & ex)
    {
      if(debug) cout<<"Hubo un inconveniente al intentar apagar el equipo. Revise la conexión. Error: "<<ex.excdesc<<endl;
      QMessageBox::critical(this,tr("Atención"),tr((string("Hubo un inconveniente al intentar apagar el equipo. Revise la conexión. Error: ")+string(ex.excdesc)).c_str()));
      SetButtonState(!arpet->verifyMCAEStream(msg,arpet->getAnsAP_OFF()),ui->pushButton_arpet_off, true);
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
    if(debug)
    {
      cout<<"El nuevo archivo de configuración: "<<initfile<<endl;
      cout<<"[END-LOG-DBG] ====================================================="<<endl;
    }

}
/**
 * @brief MainWindow::on_pushButton_conectar_clicked
 */
void MainWindow::on_pushButton_conectar_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    if(arpet->isPortOpen())
    {
        ui->pushButton_conectar->setText("Conectar");
        arpet->portDisconnect();
    }
    else
    {
        try{
            QString port_name=ui->comboBox_port->currentText();
            arpet->portConnect(port_name.toStdString().c_str());
            QMessageBox::information(this,tr("Información"),tr("Conectado al puerto: ") + port_name);
            ui->pushButton_conectar->setText("Desconectar");
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

   int head_index=getHead("config").toInt();
   if(debug) cout<<"Cabezal: "<<head_index<<endl;

   /* Configuración de las tablas de calibración */
   setCalibrationTables(head_index);
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
        if(debug) cout<< "HV configurado"<<endl;
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
        q_msg = setCalibTable(arpet->getX_Calib_Table(), coefx_values, arpet->getAnsX_Calib_Table());
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
        q_msg = setCalibTable(arpet->getY_Calib_Table(), coefy_values, arpet->getAnsY_Calib_Table());
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
        q_msg = setCalibTable(arpet->getEnergy_Calib_Table(), coefenerg_values, arpet->getAnsEnergy_Calib_Table());
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
        q_msg = setCalibTable(arpet->getWindow_Limits_Table(),coefest_values, arpet->getAnsWindow_Limits_Table());
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
            q_msg = setHV("config",hv.toStdString(), QString::number(pmt+1).toStdString());
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
            q_msg = setTime("config", coefT_values[pmt], QString::number(pmt+1).toStdString());
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
        for(int pmt = 0; pmt < PMTs; pmt++)
        {
            setMCAEDataStream("mca", arpet->getFunCSP3(), QString::number(pmt+1).toStdString(), arpet->getTemp_MCA());
            sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
            string msg = readString();
            temp=arpet->getPMTTemperature(msg);
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
    catch( Exceptions & ex )
    {
      if(debug) cout<<"Imposible obtener los valores de temperatura. Error: "<<ex.excdesc<<endl;
      QMessageBox::critical(this,tr("Atención"),tr((string("Imposible obtener los valores de temperatura. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
    }

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
 * @param tab
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::getHeadMCA(string tab)
{
   QString msg;
   QVector<QVector<double> >  hits(HEADS,QVector<double>(CHANNELS));

   try
   {
     msg = getMCA(tab, arpet->getFunCHead(), true, CHANNELS);
     if(debug) showMCAEStreamDebugMode(msg.toStdString());
     hits.insert(HEAD, 1, arpet->getHitsMCA());
   }
   catch(Exceptions & ex)
   {
     if(debug) cout<<"No se pueden obtener los valores de MCA del Cabezal. Error: "<<ex.excdesc<<endl;
     QMessageBox::critical(this,tr("Atención"),tr((string("No se pueden obtener los valores de MCA. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
   }
   setHeadVectorHits(hits);

   return msg;
}
/**
 * @brief MainWindow::getMultiMCA
 *
 * Obtiene las cuentas de MCA correspondiente a los fotomultiplicadores en un cabezal determinado
 *
 * @param tab
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::getMultiMCA(string tab)
{

   int size_pmt_selected = pmt_selected_list.length();
   QVector<QVector<double> >  hits(size_pmt_selected,QVector<double>(CHANNELS_PMT));
   QString msg;

   if (pmt_selected_list.isEmpty())
   {
     if(debug) cout<<"La lista de PMTs seleccionados se encuentra vacía."<<endl;
     QMessageBox::information(this,tr("Información"),tr("No se encuentran PMTs seleccionados para la adquisición. Seleccione al menos un PMT."));
     return msg;
   }

   try
   {
     for (int index=0;index<size_pmt_selected;index++)
     {
        string pmt = pmt_selected_list.at(index).toStdString();
        msg = getMCA(tab, arpet->getFunCSP3(), false, CHANNELS_PMT, pmt);
        if(debug)
        {
          cout<<"PMT: "<<pmt<<" "<<endl;
          showMCAEStreamDebugMode(msg.toStdString());
        }
        hits.insert(index, 1, arpet->getHitsMCA());
     }
     if(debug) cout<<"Se obtuvieron las cuentas MCA de los PMTs seleccionados de forma satisfactoria."<<endl;
   }
   catch(Exceptions & ex)
   {
     if(debug) cout<<"No se pueden obtener los valores de MCA de los PMTs seleccionados. Error: "<<ex.excdesc<<endl;
     QMessageBox::critical(this,tr("Atención"),tr((string("No se pueden obtener los valores de MCA. Revise la conexión al equipo. Error: ")+string(ex.excdesc)).c_str()));
   }

   setPMTVectorHits(hits);
   return msg;
}
/**
 * @brief MainWindow::getMCA
 *
 * Obtiene las cuentas de MCA
 *
 * @param tab
 * @param function
 * @param multimode
 * @param channels
 * @param pmt
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::getMCA(string tab, string function, bool multimode, int channels, string pmt)
{
    setMCAEDataStream(tab, function, pmt, arpet->getData_MCA(), channels*6+16);
    string msg, msg_data;

    sendString(arpet->getTrama_MCAE(), arpet->getEnd_MCA());
    msg = readString();
    msg_data = readBufferString(channels*6+16);

    arpet->getMCASplitData(msg_data, channels);

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
        ui->label_data_output->setText("Frame: "+QString::number(frame)+"\nVarianza: "+QString::number(var)+"\nOffset ADC: "+QString::number(offset)+"\nTiempo (mseg):\n"+QString::number(time_mca/1000));
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

    return QString::fromStdString(msg);
}
/**
 * @brief MainWindow::setCalibTable
 *
 * Configura la tabla de calibración especificada
 *
 * @param function
 * @param table
 * @param msg_compare
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::setCalibTable(string function, QVector<double> table, string msg_compare)
{
    setMCAEDataStream("config", function, table);
    string msg;
    try
    {
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg = readString();
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_time_out(ex.excdesc);
        throw exception_time_out;
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
 * @param tab
 * @param time_value
 * @param pmt
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::setTime(string tab, double time_value, string pmt)
{
    setMCAEDataStream(tab, arpet->getFunCSP3(), pmt, arpet->getSet_Time_MCA(), time_value);
    string msg;
    try
    {
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg = readString();
    }
    catch(Exceptions & ex)
    {
        Exceptions exception_hv(ex.excdesc);
        throw exception_hv;
    }

    return QString::fromStdString(msg);
}
/**
 * @brief MainWindow::setHV
 *
 * Configuración de HV para un PMT determinado
 *
 * @param tab
 * @param hv_value
 * @param pmt
 * @return Devuelve el mensaje de respuesta en _QString_
 */
QString MainWindow::setHV(string tab, string hv_value, string pmt)
{
    setMCAEDataStream(tab, arpet->getFunCSP3(), pmt, arpet->getSetHV_MCA(),0, hv_value);
    string msg;
    try
    {
        sendString(arpet->getTrama_MCAE(),arpet->getEnd_MCA());
        msg = readString();
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
 * @param tab
 * @param function
 * @param pmt
 * @param mca_function
 * @param bytes_mca
 * @param hv_value
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
    hits_head_ui.clear();
    hits_pmt_ui.clear();
    setHitsInit(true);
}
/**
 * @brief MainWindow::on_pushButton_adquirir_clicked
 */
void MainWindow::on_pushButton_adquirir_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    if(debug) cout<<"Cabezal: "<<getHead("mca").toStdString()<<endl;

    QString q_msg;

    switch (adquire_mode) {
    case PMT:
        q_msg = getMultiMCA("mca");
        getMultiplePlot(ui->specPMTs);
        break;
    case CABEZAL:
        q_msg = getHeadMCA("mca");
        getHeadPlot(ui->specHead);
        break;
    case TEMPERATURE:
        drawTemperatureBoard();
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
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    if(debug) cout<<"Cabezal: "<<getHead("mca").toStdString()<<endl;
    /** @todo Verificar el reinicio de datos en los vectores de cuentas de MCA */
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
        qDebug() << "La lista seleccionada tiene "<< qlist.size() << " elementos";
        QList<QString>::const_iterator stlIter;
        for( stlIter = qlist.begin(); stlIter != qlist.end(); ++stlIter )
            qDebug() << (*stlIter);
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
        q_msg =setHV("mca",getHVValue(ui->lineEdit_hv_value),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
        if(debug) cout<<getHVValue(ui->lineEdit_hv_value)<<endl;
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
 * @brief MainWindow::on_pushButton_l_5_clicked
 */
void MainWindow::on_pushButton_l_5_clicked()
{
    if(debug) cout<<"[LOG-DBG] "<<getLocalDateAndTime()<<" ================================"<<endl;
    QString q_msg;
    try
    {
        q_msg = setHV("mca",getHVValue(ui->lineEdit_hv_value,-5),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
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
        q_msg = setHV("mca",getHVValue(ui->lineEdit_hv_value,-10),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
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
        q_msg = setHV("mca",getHVValue(ui->lineEdit_hv_value,-50),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
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
        q_msg = setHV("mca",getHVValue(ui->lineEdit_hv_value,5),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
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
        q_msg = setHV("mca",getHVValue(ui->lineEdit_hv_value,10),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
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
        q_msg = setHV("mca",getHVValue(ui->lineEdit_hv_value,50),QString::number(getPMT(ui->lineEdit_pmt)).toStdString());
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
int MainWindow::parseConfigurationFile(QString filename)
{
    QFile configfile(filename);
    if (!configfile.open(QIODevice::ReadOnly)) {
        filename=openConfigurationFile();
        configfile.setFileName(filename);
        if (!configfile.open(QIODevice::ReadOnly)){
            if(debug) qDebug() << "No se puede abrir el archivo de configuración. Error: " << configfile.errorString();
            QMessageBox::critical(this,tr("Atención"),tr("No se puede abrir el archivo de configuración."));
            return MCAE::FILE_NOT_FOUND;
        }
    }

    QSettings settings(filename, QSettings::IniFormat);

    /* Parameters */
    AT = settings.value("SetUp/AT", "US").toInt();
    LowLimit = settings.value("SetUp/LowLimit", "US").toInt();

    /* Paths to the configuration files */

    QString head= getHead("config");
    QString root=settings.value("Paths/root", "US").toString();

    coefenerg = root+settings.value("Cabezal"+head+"/coefenerg", "US").toString();
    hvtable = root+settings.value("Cabezal"+head+"/hvtable", "US").toString();
    coefx = root+settings.value("Cabezal"+head+"/coefx", "US").toString();
    coefy = root+settings.value("Cabezal"+head+"/coefy", "US").toString();
    coefest = root+settings.value("Cabezal"+head+"/coefest", "US").toString();
    coefT = root+settings.value("Cabezal"+head+"/coefT", "US").toString();

    configfile.close();

    return MCAE::OK;
}

/**
 * @brief MainWindow::openConfigurationFile
 * @return
 */
QString MainWindow::openConfigurationFile()
{
    QString root="/media/arpet/pet/calibraciones/03-info/cabezales";
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de configuración"),
                                                    root,
                                                    tr("Configuración (*.ini);;Texto (*.txt)"));
    initfile = filename.toStdString();
    return filename;
}

/**
 * @brief MainWindow::getPaths
 */
void MainWindow::getPaths()
{
    QString filename=QString::fromStdString(initfile);
    parseConfigurationFile(filename);

    ui->textBrowser_triple_ventana->setText(coefest);
    ui->textBrowser_hv->setText(hvtable);
    ui->textBrowser_energia->setText(coefenerg);
    ui->textBrowser_posicion_X->setText(coefx);
    ui->textBrowser_posicion_Y->setText(coefy);
    ui->textBrowser_tiempos_cabezal->setText(coefT);

    ui->lineEdit_alta->setText(QString::number(AT));
    ui->lineEdit_limiteinferior->setText(QString::number(LowLimit));
}

/**
 * @brief MainWindow::SetLabelState
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
 * @brief MainWindow::SetButtonState
 * @param state
 * @param button
 * @param disable
 */
void MainWindow::SetButtonState(bool state, QPushButton * button, bool disable)
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
         arpet->portReadString(&msg,delimeter);
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
         arpet->portReadBufferString(&msg,buffer_size);
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
        string sended=msg + end;
        bytes_transfered = arpet->portWrite(&sended);
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
  QVector<QVector<double> >  hits(qlist.length(), QVector<double>(CHANNELS));
  for (unsigned int index=0; index < qlist.length(); index++)
  {
    hits[index].fill(0);
    qcp_pmt_parameters.insert(index, 1, getCustomPlotParameters());
  }
  setPMTVectorHits(hits);
}
/**
 * @brief MainWindow::setHeadCustomPlotEnvironment
 */
void MainWindow::setHeadCustomPlotEnvironment()
{
  QVector<QVector<double> >  hits(HEADS, QVector<double>(CHANNELS));
  for (unsigned int index=0; index < HEADS; index++)
  {
    hits[index].fill(0);
    qcp_head_parameters.insert(index, 1, getCustomPlotParameters());
  }
  setHeadVectorHits(hits);
}
/**
 * @brief MainWindow::getMultiplePlot
 * @param graph
 */
void MainWindow::getMultiplePlot(QCustomPlot *graph)
{
  graph->clearGraphs();
  if (arpet->getChannels().size()==0) return;

  for (int index=0;index<pmt_selected_list.length();index++)
  {
      addGraph(index, graph, CHANNELS_PMT, pmt_selected_list.at(index));
  }
  graph->rescaleAxes();
}
/**
 * @brief MainWindow::getHeadPlot
 * @param graph
 */
void MainWindow::getHeadPlot(QCustomPlot *graph)
{
    graph->clearGraphs();
    addGraph(HEAD, graph, CHANNELS, ui->comboBox_head_select_graph->currentText(), true);
    graph->rescaleAxes();
}
/**
 * @brief MainWindow::addGraph
 * @param index
 * @param graph
 * @param channels
 * @param graph_legend
 * @param head
 */
void MainWindow::addGraph(int index,  QCustomPlot *graph, int channels, QString graph_legend, bool head)
{
  channels_ui.resize(channels);
  channels_ui = arpet->getChannels();
  QVector<double> hits;
  QVector<int> param;

  if (head)
  {
    hits = getHeadVectorHits()[index];
    param = qcp_head_parameters[index];
  }
  else
  {
    hits = getPMTVectorHits()[index];
    param = qcp_pmt_parameters[index];
  }

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

/////// TESTING //////////////

/* Autocalib */

void MainWindow::on_pushButton_clicked()
{
    QList<int> checked_PMTs;

    // Recupero los PMT checkeados
    for(int i = 0;  i< PMTs ; i++ )
    {
        if (pmt_button_table[i]->isChecked())
        {
            checked_PMTs.append(i);
        }
    }

    // Recupero el canal objetivo
    QString Canal_obj = ui->Canal_objetivo->text();



    // Debug
    for(int i=0; i<checked_PMTs.length();i++) { cout<<checked_PMTs[i]<<endl; }
    cout<<"Canal Objetivo:"<<Canal_obj.toStdString()<<endl;

}
