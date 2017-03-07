#include "inc/SetPreferences.h"
#include "ui_SetPreferences.h"

/**
 * @brief SetPreferences::SetPreferences
 *
 * Constructor de la clase.
 * @param parent
 */
SetPreferences::SetPreferences(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SetPreferences)
{
  ui->setupUi(this);
}
/**
 * @brief SetPreferences::accept
 * @overload
 */
void SetPreferences::accept()
{
  debconsole = ui->checkBox_Debug->isChecked();
  QDialog::accept();
}
/**
 * @brief SetPreferences::reject
 * @overload
 */
void SetPreferences::reject()
{
  ui->checkBox_Debug->setChecked(debconsole);
  QDialog::reject();
}
/**
 * @brief SetPreferences::~SetPreferences
 *
 * Destructor de la clase.
 */
SetPreferences::~SetPreferences()
{
  delete ui;
}
