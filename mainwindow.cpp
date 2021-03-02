#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QSerialPortInfo"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
        ui->comPortsComboBox->addItem(info.portName());

    setWindowTitle(tr("IR Reciever"));

    connect(&m_thread, &SlaveThread::request, this,&MainWindow::showRequest);
    connect(&m_thread, &SlaveThread::error, this, &MainWindow::processError);
    connect(&m_thread, &SlaveThread::text, this, &MainWindow::processText);
    connect(&m_thread, &SlaveThread::changeState, this, &MainWindow::processState);
    connect(&m_thread, &SlaveThread::updateBytes, this, &MainWindow::processBytesCtr);
    connect(&m_thread, &SlaveThread::configureProgressBar, this, &MainWindow::configureProgressBar);
    connect(&m_thread, &SlaveThread::updateProgressBar, this, &MainWindow::updateProgressBar);

    updateProgressBar(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::startSlave()
{
    ui->connectButton->setEnabled(false);
    m_thread.startSlave(ui->comPortsComboBox->currentText(),
                        10000,
                        comResponse);
}

void MainWindow::showRequest(const QString &s)
{
    /*
    ui->textEdit->setText(tr("Traffic, transaction #%1:"
                               "\n\r-request: %2"
                               "\n\r-response: %3")
                            .arg(++m_transactionCount)
                            .arg(s)
                            .arg(comResponse));
    */
    //ui->statusLabel->setText(tr("Receiving data"));
}

void MainWindow::processError(const QString &s)
{
    activateRunButton();
    ui->statusLabel->setText(tr("Status: Not running, %1.").arg(s));
}

void MainWindow::processText(const QString &s)
{
    ui->textEdit->setText(tr("Received Bytes: %1").arg(s));
}

void MainWindow::processState(const QString &s)
{
    ui->statusLabel->setText(s);
}

void MainWindow::processBytesCtr(const QString &s)
{
    ui->BytesCtrLabel->setText(s);
}

void MainWindow::updateProgressBar(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::activateRunButton()
{
    ui->connectButton->setEnabled(true);
}

void MainWindow::configureProgressBar(int value)
{
    ui->progressBar->setRange(0, value);
}

void MainWindow::on_connectButton_clicked()
{
    startSlave();
}
