/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "slavethread.h"

#include <QSerialPort>
#include <QTime>
#include <QFile>
#include <QTextStream>

SlaveThread::SlaveThread(QObject *parent) :
    QThread(parent)
{
    rawFile.setFileName("outRawFile.bin");
    rawFile.open(QIODevice::WriteOnly | QIODevice::Text);
}

SlaveThread::~SlaveThread()
{
    rawFile.close();
    m_mutex.lock();
    m_quit = true;
    m_mutex.unlock();
    wait();
}

void SlaveThread::startSlave(const QString &portName, int waitTimeout, const QString &response)
{
    const QMutexLocker locker(&m_mutex);
    m_portName = portName;
    m_waitTimeout = waitTimeout;
    m_response = response;
    if (!isRunning())
        start();
}

void SlaveThread::run()
{
    bool currentPortNameChanged = false;

    m_mutex.lock();
    QString currentPortName;
    if (currentPortName != m_portName) {
        currentPortName = m_portName;
        currentPortNameChanged = true;
    }

    int currentWaitTimeout = m_waitTimeout;
    QString currentRespone = m_response;
    m_mutex.unlock();
    QSerialPort serial;

    serial.setPortName(currentPortName);
    if (!serial.open(QIODevice::ReadWrite)) {
        emit error(tr("Can't open %1, error code %2")
                   .arg(m_portName).arg(serial.error()));
        return;
    }
    emit changeState("Serial port opened");
    parserState = Idle;
    bytesCtr = 0;

    while (!m_quit) {
        if (serial.waitForReadyRead(currentWaitTimeout)) {
            //serial.read(&buffer, 1);
            //emit request(tr("Data received"));
            // read request
            bytesCtr += serial.bytesAvailable();
            receivedData = serial.readAll();
            while (serial.waitForReadyRead(10))
            {
                bytesCtr += serial.bytesAvailable();
                receivedData += serial.readAll();
                emit updateBytes(tr("Bytes received: %1").arg(QString::number(bytesCtr)));
                switch (parserState) {
                case Idle:
                    if (bytesCtr>0){
                        parserState = Preamble;
                    }
                    break;
                case Preamble:
                    if (bytesCtr>=4) {
                        QByteArray tmp = receivedData.left(4);
                        if (tmp == PREAMBLE) {
                            parserState=Length;
                            emit changeState("Preamble Catched");
                        }
                    }
                    break;
                case Length:
                    if (bytesCtr>=8) {
                        QByteArray size = receivedData.mid(4, 4);
                        fileSize = size.toInt();
                        parserState = Data;
                        emit changeState(tr("File size %1. Receiving Data").arg(fileSize));
                    }
                    break;
                case Data:
                    if (bytesCtr>=8+fileSize) {
                        parserState = CRC;
                        emit changeState("End of Data");
                    }
                    break;
                case CRC:
                    if (bytesCtr>=8+fileSize+16) {
                        parserState = Received;
                        emit changeState("CRC catched");
                    }
                    break;
                case Received:
                    break;
                }
            }
            emit request(tr("Data received"));
            rawFile.write(receivedData);
        }
    }

}
