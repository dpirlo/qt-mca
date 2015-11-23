#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include <QtSerialPort>


QSerialPort serial;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->espCabezal->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectPlottables  );
    ui->espCabezal->xAxis->setRange(-8, 8);
    ui->espCabezal->yAxis->setRange(-5, 5);
    ui->espCabezal->axisRect()->setupFullAxesBox();

    ui->espCabezal->plotLayout()->insertRow(0);
    ui->espCabezal->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->espCabezal, "Multi MCA"));
    ui->espCabezal->xAxis->setLabel("Energia");
    ui->espCabezal->yAxis->setLabel("Cuentas");

    ui->espCabezal->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->espCabezal->legend->setFont(legendFont);
    ui->espCabezal->legend->setSelectedFont(legendFont);
    ui->espCabezal->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

}

MainWindow::~MainWindow()
{
    delete ui;
    serial.close();  //libero el puerto cuando cierro el programa
}

void MainWindow::recibirdatosSerie()
{
     QByteArray data = serial.readAll();
     ui->textEdit->append(QString::fromUtf8(data));
}

void MainWindow::on_pushButton_clicked()
{

    QMessageBox *mbox = new QMessageBox(this);
    if(serial.isOpen())
    {
        serial.close();
        ui->pushButton->setText("Conectar");
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
            serial.setBaudRate(921600,QSerialPort::AllDirections);
            serial.setFlowControl(QSerialPort::NoFlowControl);
            serial.flush();
            connect(&serial,SIGNAL(readyRead()),this,SLOT(recibirdatosSerie()));
            ui->pushButton->setText("Desconectar");
        }

    }

    delete mbox;
}

void MainWindow::on_pushButton_2_clicked()
{
    if(serial.isOpen())
    {
        QString msg = "Hola!!!";
        serial.write(msg.toUtf8());
        connect(&serial,SIGNAL(readyRead()),this,SLOT(Graficaalgo()));
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->textEdit->clear();
}


void MainWindow::Graficaalgo(){

    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0; // x goes from 0 to 2
      y[i] = x[i]*x[i];  // let's plot a quadratic function
    }
    ui->espCabezal->addGraph();
    ui->espCabezal->graph(0)->setData(x, y);

    ui->espCabezal->xAxis->setRange(0, 2);
    ui->espCabezal->yAxis->setRange(0, 2);

}


void MainWindow::on_pushButton_triple_ventana_clicked()
{
    QString fileName = OpenConfigurationFile();
    ui->textBrowser_triple_ventana->setText(fileName);
}

void MainWindow::on_pushButton_hv_clicked()
{
    QString fileName = OpenConfigurationFile();
    ui->textBrowser_hv->setText(fileName);
}

void MainWindow::on_pushButton_energia_clicked()
{
    QString fileName = OpenConfigurationFile();
    ui->textBrowser_energia->setText(fileName);
}


void MainWindow::on_pushButton_posicion_X_clicked()
{
    QString fileName = OpenConfigurationFile();
    ui->textBrowser_posicion_X->setText(fileName);
}

void MainWindow::on_pushButton_posicion_Y_clicked()
{
    QString fileName = OpenConfigurationFile();
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

QString MainWindow::OpenConfigurationFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Abrir archivo de configuración"),
                                                    QDir::homePath(),
                                                    tr("Texto (*.txt)"));
    return filename;
}

void MainWindow::SetLabelState(bool state, QLabel *label)
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


/* TOMARLOS COMO EJEMPLO PARA LOS ESTADOS EN EL CABEZAL */

void MainWindow::on_pushButton_4_clicked()
{
      SetLabelState(true,ui->label_cabezal_estado);

}

void MainWindow::on_pushButton_5_clicked()
{
    SetLabelState(false,ui->label_cabezal_estado);
}

/*********************************************************/

