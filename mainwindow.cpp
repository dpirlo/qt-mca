#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include <QString>

/* TOMARLOS COMO EJEMPLO PARA ENVIO Y RECEPCIÓN DEL SERIE */
#include <QtSerialPort>
QSerialPort serial;
/*********************************************************/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->comboBox_port->addItems(availablePortsName());

}

MainWindow::~MainWindow()
{
    delete ui;
    serial.close();         //EJEMPLO: Libero el puerto cuando cierro el programa
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->textEdit->clear();    
}

void MainWindow::on_pushButton_triple_ventana_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_triple_ventana->setText(fileName);
}

void MainWindow::on_pushButton_hv_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_hv->setText(fileName);
}

void MainWindow::on_pushButton_energia_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_energia->setText(fileName);
}


void MainWindow::on_pushButton_posicion_X_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_posicion_X->setText(fileName);
}

void MainWindow::on_pushButton_posicion_Y_clicked()
{
    QString fileName = openConfigurationFile();
    ui->textBrowser_posicion_Y->setText(fileName);
}

void MainWindow::on_pushButton_salir_clicked()
{
    QApplication::quit();
}

void MainWindow::on_pushButton_salir_graficos_clicked()
{
     QApplication::quit();
}

/* Métodos generales del entorno gráfico */

/**
 * @brief MainWindow::openConfigurationFile
 * @return
 */
QString MainWindow::openConfigurationFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de configuración"),
                                                    QDir::homePath(),
                                                    tr("Texto (*.txt)"));
    return filename;
}

/**
 * @brief MainWindow::SetLabelState
 * @param state
 * @param label
 */
void MainWindow::setLabelState(bool state, QLabel *label)
{
    QPalette palette;

    if (!state)
    {
        palette.setColor(QPalette::Background,Qt::green);
        label->setPalette(palette);
    }
    else
    {
        palette.setColor(QPalette::Background,Qt::red);
        label->setPalette(palette);
    }
    return;
}

/**
 * @brief MainWindow::availablePortsName
 * @return
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

/* TOMARLOS COMO EJEMPLO PARA LOS ESTADOS EN EL CABEZAL */

void MainWindow::on_pushButton_4_clicked()
{
      setLabelState(true,ui->label_cabezal_estado);

}

void MainWindow::on_pushButton_5_clicked()
{
      setLabelState(false,ui->label_cabezal_estado);
}

/*********************************************************/

/* TOMARLOS COMO EJEMPLO PARA ENVIO Y RECEPCIÓN DEL SERIE */

int MainWindow::on_pushButton_clicked()
{

    QMessageBox *mbox = new QMessageBox(this);

    try{
        QString port_name=ui->comboBox_port->currentText();
        mca.portConnect(port_name.toStdString().c_str(), 115200);
        mbox->setText("Conectado al puerto: " + port_name);
        mbox->exec();
    }
    catch(boost::system::system_error e)
        {
            mbox->setText(e.what());
            mbox->exec();
            return -1;
        }

    delete mbox;
    return 0;
}

void MainWindow::on_pushButton_2_clicked()
{
    QMessageBox *mbox = new QMessageBox(this);
    string sended = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
    string received;

    mca.portWrite(&sended);
    //mca.portRead(&received,sended.size()); //TODO: Ojo que bloquea, implemetar timeout

    cout<<received<<endl;
    mbox->setText(QString::fromStdString(received));
    mbox->exec();
}

void MainWindow::on_pushButton_7_clicked()
{
    QMessageBox *mbox = new QMessageBox(this);
       if(serial.isOpen())
       {
           serial.close();
           ui->pushButton_7->setText("Connect");
           disconnect(&serial,SIGNAL(readyRead()));
       }
       else
       {
           serial.setPortName("/dev/ttyUSB0");
           if (!serial.open(QIODevice::ReadWrite)){
               mbox->setText("No se pudo abrir el puerto");
               mbox->exec();
           }
           else
           {
               serial.setParity(QSerialPort::NoParity);
               serial.setDataBits(QSerialPort::Data8);
               serial.setBaudRate(115200,QSerialPort::AllDirections);
               serial.setFlowControl(QSerialPort::NoFlowControl);
               serial.setStopBits(QSerialPort::OneStop);
               serial.flush();
               connect(&serial,SIGNAL(readyRead()),this,SLOT(recibirdatosSerie()));
           }
           ui->pushButton_7->setText("Disconnect");
       }

       delete mbox;
}

void MainWindow::on_pushButton_8_clicked()
{

    if(serial.isOpen())
    {
        QString msg = "12345";
        serial.write(msg.toUtf8());
        connect(&serial,SIGNAL(readyRead()),this,SLOT(showNormal()));
    }
}

void MainWindow::recibirdatosSerie()
{
     QByteArray data = serial.readAll();
     ui->textEdit->append(QString::fromUtf8(data));
}

/*********************************************************/
