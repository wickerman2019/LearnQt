#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QSettings>
#include <QtSql>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ThreadReadSocket : public QThread
{
    Q_OBJECT
private:
    QTcpSocket* socket;
    QByteArray block;
public:
    ThreadReadSocket() {}
    ~ThreadReadSocket() {}
    void run();
signals:
    void readyData();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    ThreadReadSocket* threadReadSocket;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void slotReadyData();

    void on_btnRun_clicked();

    void on_edHost_editingFinished();

    void on_edPort_editingFinished();

    void on_btnLog_clicked();

    void on_dateEdit_dateChanged(const QDate &date);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
