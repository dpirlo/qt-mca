#include "Validate.h"
#include "ui_Validate.h"

Validate::Validate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Validate)
{
    ui->setupUi(this);
}

Validate::~Validate()
{
    delete ui;
}

void Validate::accept()
{
    QDialog::accept();
}

void Validate::reject()
{
    QDialog::reject();
}

void Validate::PlotHits(QVector<double> hits,QVector<double> channels)
{
    channels_ui.resize(channels.length());
    channels_ui = channels;

    ui->specValidate->clearGraphs();
    ui->specValidate->addGraph();
    //ui->specValidate->graph()->setName("Validación de calibración gruesa");
    ui->specValidate->graph()->setAdaptiveSampling(true);
    ui->specValidate->graph()->addData(channels_ui,hits);
    //ui->specValidate->legend->setVisible(true);
    ui->specValidate->rescaleAxes();
    ui->specValidate->replot();
}
