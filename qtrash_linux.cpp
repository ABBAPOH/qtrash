#include "qtrash.h"
#include "qtrash_p.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "qdriveinfo.h"
#include "qtrashfileinfo_p.h"

static inline QString homePath()
{
    return QString(qgetenv("HOME"));
}

static inline void fixBashShortcuts(QString &s)
{
    if (s.startsWith('~'))
        s = QString(qgetenv("HOME")) + (s).mid(1);
}

static QString xdgSingleDir(const QString &envVar, const QString &def, bool createDir)
{
    QString s(qgetenv(envVar.toAscii()));

    if (!s.isEmpty())
        fixBashShortcuts(s);
    else
        s = QString("%1/%2").arg(qgetenv("HOME"), def);

    QDir d(s);
    if (createDir && !d.exists()) {
        if (!d.mkpath("."))
            qWarning() << QString("Can't create %1 directory.").arg(d.absolutePath());
    }

    return d.absolutePath();
}

static inline QString dataHome(bool createDir)
{
    return xdgSingleDir("XDG_DATA_HOME", ".local/share", createDir);
}

static inline QString getHomeTrash()
{
    return dataHome(true) + QLatin1Char('/') + QLatin1String("Trash");
}

static inline QString getDriveAdminTrash(const QString &drive)
{
    return drive + QLatin1Char('/') + QLatin1String(".Trash");
}

static inline QString getUserId()
{
    return QString::number(getuid(), 10);
}

static inline QString getDriveUserTrash(const QString &drive)
{
    return drive + QLatin1Char('/') + QLatin1String(".Trash-") + getUserId();
}

static inline QString infoPath(const QString &trash)
{
    return trash + QLatin1Char('/') + QLatin1String("info");
}

static inline QString infoPath(const QString &trash, const QString &fileName)
{
    return trash + QLatin1Char('/') + QLatin1String("info") + QLatin1Char('/') + fileName + ".trashinfo";
}

static inline QString filesPath(const QString &trash)
{
    return trash + QLatin1Char('/') + QLatin1String("files");
}

static bool testDir(const QString &dir)
{
    QFileInfo info(dir);
    // TODO : check permissions
    return info.exists() && info.isDir();
}

static bool checkTrashSubdirs(const QString &trashPath)
{
    return testDir(infoPath(trashPath)) && testDir(filesPath(trashPath));
}

static bool testDriveAdminTrash(const QString &rootTrashDir)
{
// (1) Administrator-created $topdir/.Trash directory

//    const QString rootTrashDir = topdir + QString::fromLatin1("/.Trash");
//    const QByteArray rootTrashDir_c = QFile::encodeName( rootTrashDir );
//    // Can't use QFileInfo here since we need to test for the sticky bit
//    uid_t uid = getuid();
//    KDE_struct_stat buff;
//    const unsigned int requiredBits = S_ISVTX; // Sticky bit required
//    if ( KDE_lstat( rootTrashDir_c, &buff ) == 0 ) {
//        if ( (S_ISDIR(buff.st_mode)) // must be a dir
//             && (!S_ISLNK(buff.st_mode)) // not a symlink
//             && ((buff.st_mode & requiredBits) == requiredBits)
//             && (::access(rootTrashDir_c, W_OK) == 0) // must be user-writable
//            ) {
//            const QString trashDir = rootTrashDir + QLatin1Char('/') + QString::number( uid );
//            const QByteArray trashDir_c = QFile::encodeName( trashDir );
//            if ( KDE_lstat( trashDir_c, &buff ) == 0 ) {
//                if ( (buff.st_uid == uid) // must be owned by user
//                     && (S_ISDIR(buff.st_mode)) // must be a dir
//                     && (!S_ISLNK(buff.st_mode)) // not a symlink
//                     && (buff.st_mode & 0777) == 0700 ) { // rwx for user
//                    return trashDir;
//                }
//                kDebug() << "Directory " << trashDir << " exists but didn't pass the security checks, can't use it";
//            }
//            else if ( createIfNeeded && initTrashDirectory( trashDir_c ) ) {
//                return trashDir;
//            }
//        } else {
//            kDebug() << "Root trash dir " << rootTrashDir << " exists but didn't pass the security checks, can't use it";
//        }
//    }

    return false;
}

static bool testDriveUserTrash(const QString &trashPath)
{
    // (2) $topdir/.Trash-$uid
    QFileInfo trashInfo(trashPath);

    if (!trashInfo.exists())
        return false;

    bool ok = true;
    ok &= trashInfo.isDir();
    ok &= !trashInfo.isSymLink();
    ok &= (trashInfo.permissions() & 0x700) == 0x700;

    if (ok)
        ok &= checkTrashSubdirs(trashPath);

    return ok;
}

static bool initTrashDirectory(const QString &trash)
{
}

static QString getDriveTrash(const QString &topdir, bool createIfNeeded)
{
    // (1) Administrator-created $topdir/.Trash directory
    QString adminTrash = getDriveAdminTrash(topdir);
    if (testDriveAdminTrash(adminTrash))
        return adminTrash;

    // (2) $topdir/.Trash-$uid
    QString userTrash = getDriveUserTrash(topdir);
    if (testDriveUserTrash(userTrash))
        return userTrash;

    if (createIfNeeded && initTrashDirectory(userTrash))
        return userTrash;

    return QString();
}

static QString trashPathForTrashedFile(const QString &file)
{
    QString result = file;
    result = QFileInfo(result).path();
    result = QFileInfo(result).path();
    return result;
}

#include <QtCore/QSettings>
static void readInfoFile(const QString &infoPath, QTrashFileInfoData &data)
{
    if (!QFileInfo(infoPath).exists())
        return;

    QSettings info(infoPath, QSettings::IniFormat);
    info.beginGroup(QLatin1String("Trash Info"));

    data.originalPath = QString::fromUtf8(QByteArray::fromPercentEncoding(info.value(QLatin1String("Path")).toString().toAscii()).data());
    if (!QFileInfo(data.originalPath).isAbsolute())
        data.originalPath = QDriveInfo(infoPath).rootPath() + '/' + data.originalPath;
    data.deletionDateTime = QDateTime::fromString(info.value(QLatin1String("DeletionDate")).toString(), Qt::ISODate);
}

static bool writeInfoFile(const QString &infoPath, QTrashFileInfoData &data)
{
//    if (!QFileInfo(infoPath).exists())
//        return;

//    QSettings info(infoPath, QSettings::IniFormat);
//    info.setIniCodec("utf-8");
//    info.beginGroup(QLatin1String("Trash Info"));

//    info.setValue(QLatin1String("Path"), QString::fromAscii(data.originalPath.replace(" ", "%20").toUtf8().data()));
//    info.setValue(QLatin1String("DeletionDate"), data.deletionDateTime.toString(Qt::ISODate));
//    return info.status() == QSettings::NoError;

    QFile file(infoPath);
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
//            qWarning() << Q_FUNC_INFO << "Could not write to file" << path;
            return false;
    }

    QTextStream out(&file);
    out << "[Trash Info]" << endl;
    out << "Path=" << data.originalPath.replace(" ", "%20") << endl;
    out << "DeletionDate=" << data.deletionDateTime.toString(Qt::ISODate) << endl;
//    for ( QStringList::const_iterator s = data.begin(); s != data.end(); s++ ) {
//            out << *s << "\n";
//    }
    return true;

}

static QString getNewFileName(const QString &trash, const QString &file)
{
    QDir dir(trash);
    QFileInfo info(file);
    QString baseName = info.baseName();
    QString suffix = info.suffix();
    QString newName = baseName + "." + suffix;

    for(int i = 1; dir.exists(newName); i++ )
        newName = baseName + " " + QString::number(i) + "." + suffix;

    return newName;
}

QTrashPrivate::QTrashPrivate()
{
}

QString findTrash(const QString &origPath)
{
    QString homeTrash = getHomeTrash();
    QDriveInfo homeDrive(homeTrash);
    QDriveInfo fileDrive(origPath);

    if (fileDrive == homeDrive)
        return homeTrash;

    return getDriveTrash(fileDrive.rootPath(), true);
}


bool QTrash::moveToTrash(const QString &path, QString *trashPath)
{
    QString localPath = path;
    QString trash = findTrash(path);

    if (trash.isEmpty())
        return false;

    if (trash != getHomeTrash())
        localPath = path.mid(QDriveInfo(path).rootPath().length());

    QTrashFileInfoData data;
    data.originalPath = localPath;
    data.deletionDateTime = QDateTime::currentDateTime();
//    data.size = QFileInfo(path.size());
    QString filesPath = ::filesPath(trash);
    QString newName = getNewFileName(filesPath, path);
    QString infoPath = ::infoPath(trash, newName);

    bool ok = QFile::rename(path, filesPath + '/' + newName);
    if (!ok)
        return false;

    ok &= writeInfoFile(infoPath, data);
    if (trashPath)
        *trashPath = filesPath + '/' + newName;

    return ok;
}

bool QTrash::restore(const QString &trashPath)
{
    QString fileName = QFileInfo(trashPath).fileName();
    QString trash = trashPathForTrashedFile(trashPath);
    QString infoPath = ::infoPath(trash, fileName);
    QTrashFileInfoData data;
    readInfoFile(infoPath, data);

    if (data.originalPath.isEmpty())
        return false;

    bool ok = QFile::rename(trashPath, data.originalPath);
    if (!ok)
        return false;

    // if this fails, it is ok, we already restored file
    QFile::remove(infoPath);

    return ok;
}

bool QTrash::remove(const QString &trashPath)
{
    Q_D(QTrash);

    bool ok = d->removePath(trashPath);
    if (ok) {
        ok = QFile::remove(infoPath(trashPathForTrashedFile(trashPath), QFileInfo(trashPath).fileName()));
    }

    return ok;
}

QTrashFileInfoList QTrash::files(const QString &trash) const
{
    QTrashFileInfoList result;

    QString filesPath = ::filesPath(trash);
    QString infoPath = ::infoPath(trash);
    QDir filesDir(filesPath);

    foreach (const QString &file, filesDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
        QTrashFileInfoData data;

        QString filePath = filesDir.absoluteFilePath(file);
        data.path = filePath;
        readInfoFile(infoPath + '/' + file + ".trashinfo", data);
        result.append(QTrashFileInfo(data));
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

QStringList QTrash::trashes() const
{
    QStringList result;

    QString homeTrash = getHomeTrash();
    if (QFileInfo(homeTrash).exists())
        result.append(homeTrash);

    foreach (const QDriveInfo &drive, QDriveInfo::drives()) {
        QString rootPath = drive.rootPath();
        QString adminTrash = getDriveAdminTrash(rootPath);
        if (testDriveAdminTrash(adminTrash))
            result.append(adminTrash);

        QString userTrash = getDriveUserTrash(rootPath);
        if (testDriveUserTrash(userTrash))
            result.append(userTrash);
    }

    return result;
}

void QTrash::clearTrash(const QString &trash)
{
}

void QTrash::clearTrash()
{
}
