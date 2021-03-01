#ifndef SLAVETHREAD_H
#define SLAVETHREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QFile>

enum ParserState {
    Idle = 0,
    Preamble,
    Length,
    Data,
    CRC,
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
    char buffer;

signals:
    void request(const QString &s);
    void error(const QString &s);
    void timeout(const QString &s);
    void changeState(const QString &s);
    void updateBytes(const QString &s);

private:
    void run() override;
    QByteArray PREAMBLE = QByteArray::fromHex(QString("DEADBEEF").toUtf8());

    QString m_portName;
    QString m_response;
    int m_waitTimeout = 0;
    QMutex m_mutex;
    bool m_quit = false;
    ParserState parserState = Idle;
    QFile rawFile;
    qint64 fileSize = 0;
    qint64 bytesCtr = 0;

};

#endif // SLAVETHREAD_H
