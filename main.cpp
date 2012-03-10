#include <QtGui/QApplication>
#include "qtrashwindow.h"

#include "qtrash.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    bool ok = true;
    QString newPath;

#if defined(Q_OS_MAC)
    QFile f("/Users/arch/file.bin");
#elif  defined(Q_OS_WIN)
    QFile f("C:/Documents and Settings/arch/file.bin");
#endif
    f.open(QFile::WriteOnly);
    f.close();

    QTrash trash;

    ok = trash.moveToTrash(f.fileName(), &newPath);
    qDebug() << "moveToTrash" << ok << newPath;

//    QString trashPath = QFileInfo(newPath).path();
    QString trashPath = "C:/RECYCLER/S-1-5-21-1177238915-1214440339-839522115-1003";
    QFile infoFile(trashPath + "/INFO2");
    qDebug() << "open trash path" << infoFile.open(QFile::ReadOnly);

    qDebug() << "trashPath" << trashPath;

    QByteArray arr = infoFile.readAll();
//    qDebug() << arr.toHex();

    infoFile.close();

    QFile f2("C:/Documents and Settings/arch/INFO2");
    f2.open(QFile::WriteOnly);
    f2.write(arr);
    f2.close();

//    qDebug() << "opening for writing" << infoFile.open(QFile::WriteOnly);
//    infoFile.write(arr);
//    infoFile.close();

//    qDebug() << "moving file" << f2.fileName() << "to trash :" << ok << "new path is" << newPath;

//    ok = trash.remove(newPath);
//    qDebug() << "removing file" << newPath << ok;

//    qDebug() << "trash contents" << trash.files();

//    foreach (QString file, trash.listFiles()) {
//        QFileInfo info(file);
//        qDebug() << info.fileName() << info.isDir();
//        if (info.isDir()) {
//            QDir d(file);
//            qDebug() << d.entryList(/*QDir::NoDotAndDotDot*/);
//            foreach (QString file, d.entryList(/*QDir::NoDotAndDotDot*/)) {
//                if (file != "." && file != "..")
//                qDebug() << "removing" << QFile::remove(d.absoluteFilePath(file));
//            }
////            qDebug() << "removing dir" << d.rmdir(".");
//        } else {
//            QFile f(file);
//            f.open(QFile::ReadOnly);
//            qDebug() << "contents of a file" << f.readAll();
////            qDebug() << "removing file" << QFile::remove(file);
//        }
//    }

//    ok = trash.restore(newPath);
//    qDebug() << "restoring file" << newPath << ok;

    QTrashWindow w;
    w.show();

    return a.exec();
}
