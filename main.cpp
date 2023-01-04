#include <QApplication>
#include <QSettings>
#include "resourceviewer.h"
#include "scpiclient.h"
#include "settingsdialog.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QSettings settings("Zera GmbH", "resource-viewer");
    SettingsDialog settingsDialog;
    settingsDialog.setIPAddress(settings.value("IPAddress", "localhost").toString());
    settingsDialog.setPort(settings.value("Port", 6312).toInt());

    if(settingsDialog.exec() == QDialog::Accepted) {
        QString strIPAdress = settingsDialog.getIPAddress();
        quint16 ui16Port = settingsDialog.getPort();

        settings.setValue("IPAddress", strIPAdress);
        settings.setValue("port", ui16Port);

        ResourceViewerWidget w;
        ScpiClient* client = ScpiClient::getInstance();

        QObject::connect(client, &ScpiClient::signalModelAvailable, &w, &ResourceViewerWidget::setScpiModel);
        QObject::connect(client, &ScpiClient::signalModelDeleted, &w, &ResourceViewerWidget::unsetScpiModel, Qt::DirectConnection);
        QObject::connect(client, &ScpiClient::signalAppendLogString, &w, &ResourceViewerWidget::AppendLogString);
        QObject::connect(&w, &ResourceViewerWidget::forwardUpdateResources, client, &ScpiClient::signalUpdateModel);
        QObject::connect(&w, &ResourceViewerWidget::forwardSendSCPI, client, &ScpiClient::signalSendSCPIMessage);

        w.show();
        client->start(strIPAdress, ui16Port);
        return app.exec();
    }
    return 0;
}
