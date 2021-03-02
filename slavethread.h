#ifndef SLAVETHREAD_H
#define SLAVETHREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QFile>
#include <QCryptographicHash>


enum ParserState {
    Idle = 0,
    Preamble,
    Length,
    Data,
    CRC,
    Checking_CRC,
    CRC_Valid,
    CRC_Invalid,
    Received
};

class SlaveThread : public QThread
{
    Q_OBJECT

public:
    explicit SlaveThread(QObject *parent = nullptr);
    ~SlaveThread();

    void startSlave(const QString &portName, int waitTimeout, const QString &response);
    QByteArray receivedData;

signals:
    void error(const QString &s);
    void text(const QString &s);
    void changeState(const QString &s);
    void updateBytes(const QString &s);
    void updateProgressBar(int);
    void configureProgressBar(int);

private:
    void run() override;
    const QString preamble = QString("deadbeef").toUtf8();

    QString m_portName;
    QString m_response;
    int m_waitTimeout = 0;
    QMutex m_mutex;
    bool m_quit = false;
    ParserState parserState = Idle;
    QFile rawFile;
    qint64 fileSize = 0;
    qint64 bytesCtr = 0;

    QByteArray filledData;
    QByteArray image;

    QCryptographicHash md5Calc;
    QByteArray imageMd5 = 0;

    QByteArray receivedMd5;
    int receivedFilesCounter = 0;
};

#endif // SLAVETHREAD_H
