#ifndef QTRASHMODEL_H
#define QTRASHMODEL_H

#include <QtCore/QAbstractTableModel>

#include "qtrash.h"

class QTrashModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit QTrashModel(QObject *parent = 0);

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;

    void restore(const QModelIndex &index);
    void remove(const QModelIndex &index);
    void clearTrash();

signals:

public slots:

private:
    void rebuild();

private:
    QTrash *m_trash;
    QTrashFileInfoList m_files;
};

#endif // QTRASHMODEL_H
