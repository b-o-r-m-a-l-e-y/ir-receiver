#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "slavethread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void startSlave();
    void activateRunButton();
    void processTimeout(const QString &s);
    void processError(const QString &s);
    void showRequest(const QString &s);
    void processState(const QString &s);
    void processBytesCtr(const QString &s);

private slots:
    void on_connectButton_clicked();

private:
    Ui::MainWindow *ui;
    SlaveThread m_thread;
    QString comResponse;
    int m_transactionCount = 0;
};
#endif // MAINWINDOW_H
