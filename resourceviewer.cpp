#include <QMessageBox>
#include "resourceviewer.h"
#include "ui_resource-viewer.h"

ResourceViewerWidget::ResourceViewerWidget(QWidget *parent) :
    QWidget(parent), m_pUI(new Ui::ScpiViewer)
{
    m_pUI->setupUi(this);
    connect(m_pUI->pbUpdate, &QPushButton::clicked, this, &ResourceViewerWidget::forwardUpdateResources);
    connect(m_pUI->pbSendCmd, &QPushButton::clicked, this, &ResourceViewerWidget::onSendScpiCmd);
    connect(m_pUI->pbSendQuery, &QPushButton::clicked, this, &ResourceViewerWidget::onSendScpiQuery);
    connect(m_pUI->tvScpiNodeViewer, &QTreeView::doubleClicked, this, &ResourceViewerWidget::onTvDoubleClick);
}

void ResourceViewerWidget::onSendScpiCmd()
{
    emit forwardSendSCPI(m_pUI->leCmd->text());
}

void ResourceViewerWidget::onSendScpiQuery()
{
    emit forwardSendSCPI(m_pUI->leCmd->text() + "?");
}

void ResourceViewerWidget::onTvDoubleClick(QModelIndex modelIndex)
{
    // only leafs update SCPI command
    if(!modelIndex.child(0,0).isValid()) {
        QModelIndex runningIndex;
        QString strSCPICmd;
        for(runningIndex = modelIndex; runningIndex.isValid(); runningIndex=runningIndex.parent()) {
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
