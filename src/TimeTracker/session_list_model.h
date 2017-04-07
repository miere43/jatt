#ifndef SESSION_LIST_MODEL_H
#define SESSION_LIST_MODEL_H

#include "core_types.h"
#include <QAbstractListModel>
#include <QList>

struct Session;

class SessionListModel : public QAbstractListModel
{
public:
    QVector<Session*> m_sessions;

    SessionListModel();

    void appendSession(Session* session);
    //void sendDataChangedHint(Session* session);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const;
    //QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif // SESSION_LIST_MODEL_H
