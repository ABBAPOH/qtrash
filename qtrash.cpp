#include "qtrash.h"
#include "qtrash_p.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

bool QTrashPrivate::removePath(const QString &path)
{
    bool result = true;
    QFileInfo info(path);
    if (info.isDir()) {
        QDir dir(path);
        foreach (const QString &entry, dir.entryList(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot)) {
            result &= removePath(dir.absoluteFilePath(entry));
        }
        if (!info.dir().rmdir(info.fileName()))
            return false;
    } else {
        result = QFile::remove(path);
    }
    return result;
}

QTrash::QTrash(QObject *parent) :
    QObject(parent),
    d_ptr(new QTrashPrivate)
{
}

QTrash::~QTrash()
{
    delete d_ptr;
}

//QStringList QTrash::trashes() const
//{
//    Q_D(const QTrash);

//    d->ensurePopulated();

//    QStringList result;

//    foreach (const QTrashInfo &trash, d->trashes) {
//        result.append(trash.path);
//    }

//    return result;
//}

//bool QTrash::moveToTrash(const QString &path, QString *trashPath)
//{
//}

//bool QTrash::restore(const QString &trashPath)
//{
//}

//bool QTrash::remove(const QString &trashPath)
//{
//}

//QStringList QTrash::listFiles()
//{
//}

//void QTrash::clearTrash()
//{
//}
