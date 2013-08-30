#include <QMessageBox>
#include "resourceviewer.h"
#include "ui_resource-viewer.h"

ResourceViewerWidget::ResourceViewerWidget(QWidget *parent) :
    QWidget(parent), m_pUI(new Ui::ScpiViewer)
{
    m_pUI->setupUi(this);
    connect(m_pUI->pbUpdate, SIGNAL(clicked()), this, SIGNAL(forwardUpdateResources()));
    connect(m_pUI->pbSend, SIGNAL(clicked()), this, SLOT(onSendSCPI()));
    connect(m_pUI->tvScpiNodeViewer, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onTvDoubleClick(QModelIndex)));
}

void ResourceViewerWidget::onSendSCPI()
{
    forwardSendSCPI(m_pUI->leCmd->text());
}

void ResourceViewerWidget::onTvDoubleClick(QModelIndex modelIndex)
{
    // only leafs update SCPI command
    if(!modelIndex.child(0,0).isValid())
    {
        QModelIndex runningIndex;
        QString strSCPICmd;
        for(runningIndex = modelIndex; runningIndex.isValid(); runningIndex=runningIndex.parent())
        {
            if(!strSCPICmd.isEmpty())
                strSCPICmd = ":" + strSCPICmd;
            strSCPICmd = runningIndex.data().toString() + strSCPICmd;
        }
        m_pUI->leCmd->setText(strSCPICmd);
    }
}

void ResourceViewerWidget::setScpiModel(QStandardItemModel *model)
{
    m_pUI->tvScpiNodeViewer->setModel(model);
}

void ResourceViewerWidget::unsetScpiModel()
{
    m_pUI->tvScpiNodeViewer->setModel(0);
}

void ResourceViewerWidget::AppendLogString(QString strSCPIResponse, LogHelper::enLogTypes logType)
{
    m_pUI->tbResponse->append(LogHelper::formatMessage(strSCPIResponse, logType));
}
