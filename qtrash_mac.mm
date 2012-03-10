#include "qtrash.h"
#include "qtrash_p.h"

#include <CoreFoundation/CFError.h>
#include <CoreServices/CoreServices.h>
#include <AppKit/NSWorkspace.h>
#include <Foundation/NSString.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "qdriveinfo.h"
#include "qtrashfileinfo_p.h"

QMap<QString, QTrashFileInfo> QTrashPrivate::deletedFiles;

QTrashPrivate::QTrashPrivate()
{
}

static void notifyFinder(const QString &trash)
{
    QByteArray utf8TrashPath = trash.toUtf8();

    // Notify Finder about moving file to trash
    // should i receive top trash dir or not?
    NSString *trashPath = [[NSString alloc] initWithBytes:utf8TrashPath.data()
            length:utf8TrashPath.length()
            encoding:NSUTF8StringEncoding];
    [[NSWorkspace sharedWorkspace] noteFileSystemChanged:trashPath];
    [trashPath release];
}

bool QTrash::moveToTrash(const QString &path, QString *newFilePath)
{
    Q_D(QTrash);

    QByteArray pathUtf8 = path.toUtf8();
    char *newPathUtf8 = 0;
    OSStatus result = FSPathMoveObjectToTrashSync(pathUtf8.constData(), &newPathUtf8, kFSFileOperationDoNotMoveAcrossVolumes);

    QString newPath = QString::fromUtf8(newPathUtf8);
    free(newPathUtf8);

    QTrashFileInfoData data;
    data.originalPath = path;
    data.path = newPath;
    data.deletionDateTime = QDateTime::currentDateTime();
    data.size = QFileInfo(newPath).size();
    d->deletedFiles.insert(newPath, QTrashFileInfo(data));

    notifyFinder(QFileInfo(newPath).path());

    if (newFilePath)
        *newFilePath = newPath;

    return result == noErr;
}

bool QTrash::restore(const QString &trashPath)
{
    Q_D(QTrash);

    QString originalPath = d->deletedFiles.value(trashPath).originalPath();
    if (originalPath.isEmpty())
        return false;

    bool ok = QFile::rename(trashPath, originalPath);
    if (ok) {
        d->deletedFiles.remove(trashPath);
        notifyFinder(QFileInfo(trashPath).path());
    }

    return ok;
}

bool QTrash::remove(const QString &trashPath)
{
    Q_D(QTrash);

    bool ok = d->removePath(trashPath);
    if (ok) {
        d->deletedFiles.remove(trashPath);
        notifyFinder(QFileInfo(trashPath).path());
    }

    return ok;
}

static QString getHomeTrash()
{
    QString homePath = QDir().homePath();
    return homePath + QLatin1Char('/') + QLatin1String(".Trash");
}

static QString getUserId()
{
    return QString::number(getuid(), 10);
}

static QString getDriveTrash(const QString &drive)
{
    return QString(QLatin1String("%1/.Trashes/%2")).arg(drive).arg(getUserId());
}

QStringList QTrash::trashes() const
{
    QStringList result;

    QString homeTrash = getHomeTrash();
    if (QFileInfo(homeTrash).exists())
        result.append(homeTrash);

    foreach (const QDriveInfo &drive, QDriveInfo::drives()) {
        QString driveTrash = getDriveTrash(drive.rootPath());
        QFileInfo info(driveTrash);
        if (info.exists() && (info.permissions() & 0x600) == 0x600) { // user can read and write to dir
            result.append(info.absoluteFilePath());
        }
    }

    return result;
}

QTrashFileInfoList QTrash::files(const QString &trash) const
{
    Q_D(const QTrash);

    QTrashFileInfoList result;
    QStringList paths;

    QDir trashDir(trash);
    foreach (const QString &file, trashDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
        QString filePath = trashDir.absoluteFilePath(file);
        paths.append(filePath);

        if (d->deletedFiles.contains(filePath)) {
            result.append(d->deletedFiles.value(filePath));
        } else {
            QTrashFileInfoData data;
            // we can't determine original path here
            data.path = filePath;
            data.size = QFileInfo(trashDir.absoluteFilePath(file)).size();
            result.append(QTrashFileInfo(data));
        }
    }

    // update deleted files if file was deleted from elsewhere
    foreach (const QString &path, d->deletedFiles.keys()) {
        if (path.startsWith(trash)) { // file within current trash
            if (!paths.contains(path)) {
                d->deletedFiles.remove(path);
            }
        }
    }

    return result;
}

QTrashFileInfoList QTrash::files() const
{
    QTrashFileInfoList result;

    foreach (const QString &trashPath, trashes()) {
        result.append(files(trashPath));
    }

    return result;
}

void QTrash::clearTrash()
{
    foreach (const QTrashFileInfo &info, files()) {
        remove(info.path());
    }
}

void QTrash::clearTrash(const QString &trash)
{
    foreach (const QTrashFileInfo &info, files(trash)) {
        remove(info.path());
    }

    notifyFinder(trash);
}
