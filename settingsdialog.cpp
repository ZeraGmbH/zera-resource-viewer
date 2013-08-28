#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_pUI(new Ui::SettingsDialog)
{
    m_pUI->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete m_pUI;
}

QString SettingsDialog::getIPAddress()
{
    return m_pUI->leIPAddress->text();
}

void SettingsDialog::setIPAddress(QString strIPAdress)
{
    m_pUI->leIPAddress->setText(strIPAdress);
}

quint16 SettingsDialog::getPort()
{
    return m_pUI->sbPortNo->value();
}

void SettingsDialog::setPort(quint16 ui16Port)
{
    m_pUI->sbPortNo->setValue(ui16Port);
}
