#ifndef RESOURCEVIEWER_H
#define RESOURCEVIEWER_H

#include <QWidget>
#include <QPushButton>
#include <QStandardItemModel>
#include "loghelper.h"

namespace Ui
{
    class ScpiViewer;
}

class ResourceViewerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResourceViewerWidget(QWidget *parent = 0);

signals:
    /**
     * @brief Public signal thrown when user presses 'Update'-button
     */
    void forwardUpdateResources();

    /**
     * @brief Public signal thown when user presses 'Send'-button
     * @param strSCPI The SCPI command found in command text edit
     */
    void forwardSendSCPI(QString strSCPI);

public slots:
    /**
     * @brief Set the model for tree view
     * @param model
     */
    void setScpiModel(QStandardItemModel* model);

    /**
     * @brief Unset the model for tree view. This slot should be called before passing to a new model
     */
    void unsetScpiModel();
    void AppendLogString(QString strSCPIResponse, LogHelper::enLogTypes logType);
private slots:
    /**
     * @brief Handler for "Send" button sending command found in ResourceViewerWidget::m_pUI->leCmd
     */
    void onSendScpiCmd();
    void onSendScpiQuery();

    /**
     * @brief In case the selected item is a leaf, the SCPI-command for the item is created
     * @param modelIndex reperesents selected item
     */
    void onTvDoubleClick(QModelIndex modelIndex);

private:
    /**
     * @brief user interface wrapper
     */
    Ui::ScpiViewer *m_pUI;
};

#endif // RESOURCEVIEWER_H
