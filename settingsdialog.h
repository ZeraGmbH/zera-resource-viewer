#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    QString getIPAddress();
    void setIPAddress(QString strIPAdress);
    quint16 getPort();
    void setPort(quint16 ui16Port);
private:
    Ui::SettingsDialog *m_pUI;
};

#endif // SETTINGSDIALOG_H
