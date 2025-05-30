#ifndef SCPINODE_H
#define SCPINODE_H

#include <QStandardItem>
#include "scpiobject.h"


const QString scpiNodeType[] = { "Node",
                                 "Query",
                                 "Command",
                                 "Command+Par" };

/**
  @brief base class to provide scpi nodes for objects
  */
class cSCPINode: public QStandardItem {
public:
    /**
      @b pointer to the real object that this node represents
      */
    ScpiObject* m_pSCPIObject;
    /**
      @b Initialise the const variables
      @param[in] sNodeName name of the node
      @param t type of the node
      @param[in] pSCPIObject object that handles the command
      */
    cSCPINode(const QString& sNodeName, quint8 t, ScpiObject* pSCPIObject);
    /**
      @b Returns the node's type
      @relates SCPI::scpiNodeType
      */
    quint8 getType();
    /**
      @b Sets the node's type
      */
    void setType(quint8 type);
    /**
      @b Returns the node's QStandardItem type (UserType)
      */
    virtual int type();
    /**
      @b Returns the node's name if role = Qt::DisplayRole, and a description of node's type if role = Qt::ToolTipRole
      */
    //virtual QVariant data(int role = Qt::UserRole + 1 );
    virtual QVariant data ( int role = Qt::UserRole + 1 ) const override;
private:
    /**
      @b Readable name of the node used in the model as displayRole
      */
    QString m_sNodeName;
    /**
      @b node type
      @see SCPI::eSCPINodeType
      */
    quint8 m_nType;

};

#endif // SCPINODE_H
