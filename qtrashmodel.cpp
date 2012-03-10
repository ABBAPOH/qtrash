#include "qtrashmodel.h"

QTrashModel::QTrashModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_trash(new QTrash(this))
{
    rebuild();
}

int QTrashModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

int QTrashModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_files.count();
}

QVariant QTrashModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0: return tr("Name");
            case 1: return tr("Deletion time");
            case 2: return tr("Size");
            case 3: return tr("Original path");
            }
        }
    }

    return QVariant();
}

QVariant QTrashModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();
    if (role == Qt::DisplayRole) {
        switch (column) {
        case 0: return m_files.at(row).originalName();
        case 1: return m_files.at(row).deletionDateTime();
        case 2: return m_files.at(row).size();
        case 3: return m_files.at(row).originalPath();
        }
    }

    return QVariant();
}

void QTrashModel::restore(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    m_trash->restore(m_files.at(index.row()).path());
    rebuild();
}

void QTrashModel::remove(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    m_trash->remove(m_files.at(index.row()).path());
    rebuild();
}

void QTrashModel::clearTrash()
{
    m_trash->clearTrash();
    rebuild();
}

void QTrashModel::rebuild()
{
    beginResetModel();
    m_files = m_trash->files();
    endResetModel();
}
