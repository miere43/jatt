#include "session_list_model.h"

#include <QVariant>
#include "common.h"

SessionListModel::SessionListModel()
{
}

int SessionListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() != 0) {
        return m_sessions.count();
    } else {
        qWarning() << __FUNCTION__ << "- parent.column() =" << parent.column();
        return 0;
    }
}

QVariant SessionListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        //qWarning() << __FUNCTION__ << "- role =" << role;
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (index.column() != 0) {
            qWarning() << __FUNCTION__ << "- index.column() =" << index.row();
            return QVariant();
        }
        if (index.row() < 0 || index.row() > m_sessions.count()) {
            qWarning() << __FUNCTION__ << "- index.row() =" << index.row();
            return QVariant();
        }
        return QVariant(m_sessions.at(index.row())->name);
    }
    else if (role == Qt::UserRole) {
        // UserRole returns pointer to session at specified row.
        if (index.row() < 0 || index.row() > m_sessions.count()) {
            qWarning() << __FUNCTION__ << "- get UserRole row out of bounds:" << index.row();
            return QVariant();
        }
        Session* session = m_sessions[index.row()];
        return qVariantFromValue((void*)session);
    } else {
        return QVariant();
    }
}

//QVariant SessionListModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int role) const
//{
//    if (role != Qt::DisplayRole) {
//        return QVariant();
//    }
//    return tr("Name");
//}

void SessionListModel::appendSession(Session *session)
{
    Q_ASSERT(session);

    QModelIndex index = QModelIndex();
    int count = rowCount(index);
    this->beginInsertRows(index, count, count);

    m_sessions.append(session);

    this->endInsertRows();
}

//void SessionListModel::sendDataChangedHint(Session *session)
//{
//    if (session == nullptr)
//    {
//        APP_ERRSTREAM << "session == nullptr";
//        return;
//    }

//    for (int i = 0; i < m_sessions.)
//    for (const Session* dataSession : m_sessions)
//    {
//        if (dataSession == session)
//        {
//            emit dataChanged(Q)
//        }
//    }
//}
