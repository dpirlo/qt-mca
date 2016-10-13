#include "SetPreferences.h"
#include "ui_SetPreferences.h"

SetPreferences::SetPreferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetPreferences)
{
    ui->setupUi(this);
}

bool SetPreferences::GetDegugConsoleValue()
{
    return debconsole;
}

void SetPreferences::accept()
{
   debconsole = ui->checkBox_Debug->isChecked();
   QDialog::accept();
}

void SetPreferences::reject()
{
   ui->checkBox_Debug->setChecked(debconsole);
   QDialog::reject();
}

SetPreferences::~SetPreferences()
{
    delete ui;
}


