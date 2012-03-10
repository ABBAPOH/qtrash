#include "info2_p.h"

#include <QFile>
#include <QBuffer>
#include <QDebug>
#include <QDir>

enum {
  LEGACY_FILENAME_OFFSET  = 0x4 - 4,
  RECORD_INDEX_OFFSET     = 0x108 - 4,
  DRIVE_LETTER_OFFSET     = 0x10C - 4,
  FILETIME_OFFSET         = 0x110 -4,
  FILESIZE_OFFSET         = 0x118 - 4,
  UNICODE_FILENAME_OFFSET = 0x11C - 4,
  RECORD_LENGTH           = 0x320
};

static QDataStream & operator << (QDataStream &s, const INFO2Header &h)
{
    s << h.reserved0;
    s << h.reserved1;
    s << h.reserved2;
    s << h.recordSize;
    s << h.totalLogicalSize;
    return s;
}

QDataStream & operator >> (QDataStream &s, INFO2Header &h)
{
    s >> h.reserved0;
    s >> h.reserved1;
    s >> h.reserved2;
    s >> h.recordSize;
    s >> h.totalLogicalSize;
    return s;
}

QDataStream & operator >> (QDataStream &s, INFO2Record &r)
{
    r.localName = s.device()->read(RECORD_INDEX_OFFSET - LEGACY_FILENAME_OFFSET);
    s >> r.number;
    s >> r.drive;
    s >> r.deletionTime;
    s >> r.fileSize;
    r.unicodeName = QString::fromUtf16((ushort*)s.device()->read(RECORD_LENGTH - UNICODE_FILENAME_OFFSET).data());
    return s;
}

QDataStream & operator << (QDataStream &s, const INFO2Record &r)
{
    QByteArray name = r.localName;
    name += QByteArray(RECORD_INDEX_OFFSET - LEGACY_FILENAME_OFFSET - r.localName.length(), 0);
    Q_ASSERT(name.length() == RECORD_INDEX_OFFSET - LEGACY_FILENAME_OFFSET);
    s.device()->write(name.data(), name.length());

    s << r.number;
    s << r.drive;
    s << r.deletionTime;
    s << r.fileSize;

    QByteArray unicodeName = QByteArray((char*)r.unicodeName.data(), r.unicodeName.length()*2);
    unicodeName += QByteArray(RECORD_LENGTH - UNICODE_FILENAME_OFFSET - r.unicodeName.length()*2, 0);
    s.device()->write(unicodeName.data(), unicodeName.length());

    return s;
}

#include <QDateTime>
#include <qendian.h>

QDateTime winTimeToDateTime(quint64 wintime)
{
//    wintime = qFromLittleEndian<qint64>(wintime);
//    quint32 low = wintime & 0xffffffff;
//    quint32 high = wintime >> 32;
    double dbl = /*low << 32 + high;*/wintime;
    dbl *= 1.0e-7;
    dbl -= 11644473600ll;
    return QDateTime::fromMSecsSinceEpoch(dbl*1000);
}

quint64 getDelTime()
{
    QDateTime dt = QDateTime::currentDateTime();
    quint64 res = dt.toMSecsSinceEpoch()/1000;
    res += 11644473600ll;
    res /= 1.0e-7;
    return res;
}

void updateInfo(const QString &trash, const QString &exclude)
{
    // exclude path is in format D<drive><number>
    QString fileName = QFileInfo(exclude).fileName();
    int drive = fileName.at(1).unicode() - QChar(QLatin1Char('A')).unicode();
    int dotIndex = fileName.indexOf(QLatin1Char('.'));
    int number = fileName.mid(2, dotIndex - 2).toInt();
    qDebug() << fileName.mid(2, dotIndex - 2).toInt() << number;

    QFile f(trash + "/INFO2");
    QBuffer buf;
    buf.open(QBuffer::WriteOnly);
    qDebug() << "testInfo" << "open" << f.open(QFile::ReadOnly);

    QDataStream s(&f);
    s.setByteOrder(QDataStream::LittleEndian);

    QDataStream s2(&buf);
    s2.setByteOrder(QDataStream::LittleEndian);

    INFO2Header h;
    s >> h;
    s2 << h;

    qDebug() << "header" << h.reserved0 << h.reserved1 << h.reserved2 << h.recordSize;

    while (!f.atEnd()) {
        INFO2Record r;
        s >> r;
//        qDebug() << "record" << r.number << r.reserved1 << QString::fromAscii(r.localName) << r.unicodeName << r.drive << winTimeToDateTime(r.deletionTime).toString() << r.fileSize;
//        if (QDir::fromNativeSeparators(r.unicodeName) != exclude)
        if (r.number != number && r.drive != drive)
            s2 << r;
        else
            qDebug() << "skipping" << QDir::fromNativeSeparators(r.unicodeName);
    }
    buf.close();
    buf.open(QBuffer::ReadOnly);
    f.close();

//    Q_ASSERT(buf.size() == f.size());
    qDebug() << buf.size();

    f.open(QFile::WriteOnly);
    f.write(buf.data());
}

void updateInfo2(QString file, QString &newPath)
{
    QString trash = "C:/RECYCLER/S-1-5-21-1177238915-1214440339-839522115-1003";
    QFile f(trash + "/INFO2");
    QBuffer buf;
    buf.open(QBuffer::WriteOnly);
    qDebug() << "testInfo" << "open" << f.open(QFile::ReadOnly);

    QDataStream s(&f);
    s.setByteOrder(QDataStream::LittleEndian);

    QDataStream s2(&buf);
    s2.setByteOrder(QDataStream::LittleEndian);

    INFO2Header h;
    s >> h;
    h.totalLogicalSize = 127;
    s2 << h;

    quint32 number = 127;

    qDebug() << "header" << h.reserved0 << h.reserved1 << h.reserved2 << h.recordSize << h.totalLogicalSize;

    while (!f.atEnd()) {
        INFO2Record r;
        s >> r;
        number = qMax(number, r.number);
        qDebug() << "record" << r.number << QString::fromAscii(r.localName) << r.unicodeName << r.drive << winTimeToDateTime(r.deletionTime).toString() << r.fileSize;
            s2 << r;
    }

    INFO2Record nr;
    nr.localName = /*getShortPathName*/(QDir::toNativeSeparators(file)).toAscii();
    nr.drive = 2;
    nr.unicodeName = QDir::toNativeSeparators(file);
    nr.deletionTime = getDelTime();
    nr.number = number + 1;
    nr.fileSize = 0;

    qDebug() << "new record" << nr.number << QString::fromAscii(nr.localName) << nr.unicodeName << nr.drive << winTimeToDateTime(nr.deletionTime).toString() << nr.fileSize;
    s2 << nr;

    newPath = trash + "/" + QString("Dc%1.%2").arg(nr.number).arg(QFileInfo(file).suffix());

    buf.close();
    buf.open(QBuffer::ReadOnly);
    f.close();

    Q_ASSERT(buf.size() == f.size() + 800);
    qDebug() << buf.size();

    f.open(QFile::WriteOnly);
    f.write(buf.data());
}
