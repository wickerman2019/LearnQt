#include "mainwindow.h"
#include "ui_mainwindow.h"

static QByteArray in_block;
static QString host="localhost";
static quint16 port=80;
static QDate RequstDate;
static bool isDBonnect=false;
static QDate SendDate;
static QTime SendTime;
static qint16 ResultCode;


bool ConnectDB()
{
    //Подключаем базу данных
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("D:\\MyProjects\\QT\\LearnQt\\log.db");
    bool rc = db.open();
    if(!rc) qDebug() << "Cannot open database:" << db.lastError();
    return rc;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //При создании главного окна производим установку начальных параметров
    //из настроек, сохранённых в INI-файле
    QSettings *settings = new QSettings("settings.ini",QSettings::IniFormat);
    host = settings->value("conf/host","localhost").toString();
    port = settings->value("conf/port",80).toUInt();
    RequstDate = settings->value("conf/requestdate",QDate::currentDate()).toDate();
    ui->edHost->setText(host);
    ui->edPort->setValue(port);
    ui->dateEdit->setDate(RequstDate);
}

void ThreadReadSocket::run()
{
    QString request="Hello";
    SendDate=QDate::currentDate();
    SendTime=QTime::currentTime();
    ResultCode=0;
    socket = new QTcpSocket();
    //Connect to socket
    socket->connectToHost(host,port);
    if(!socket->waitForConnected(5000)){
        qDebug() << "Could not connected to server";
        ResultCode=-1;
    }
    else {
        //Send request
        block = request.toLocal8Bit();
        if(socket->write(block, block.size()) < 0){
            qDebug() << "Could not write request to socket";
            ResultCode=-2;
        }
        if (socket->waitForBytesWritten(3000)){
            qDebug() << "Request is sended";
            //Read data from socket
            in_block.clear();
            for(;;)
            if(!socket->waitForReadyRead(3000)) {
                qDebug() << "No more data";
                break;
            } else {
                block = socket->readAll();
                in_block.append(block);
            }
            if (in_block.size()==0) ResultCode=-4;
            else ResultCode=0;
        }
        else {
            qDebug() << "Could not send request";
            ResultCode=-3;
        }
        socket->disconnectFromHost();
    }
    emit readyData();
}

void MainWindow::slotReadyData()
{
    qDebug() << "Data is ready.";
    if(!isDBonnect) isDBonnect = ConnectDB();
    QSqlQuery query;
    query.prepare("INSERT INTO log"
                     "( f_date, f_time, f_result) "
                          "VALUES("
                          ":f_date, :f_time, :f_result"
                          ")");

    if(isDBonnect){
        query.bindValue(":f_date",SendDate.toString("yyyy-MM-dd"));
        query.bindValue(":f_time",SendTime.toString("hh:mm:ss"));
        query.bindValue(":f_result",ResultCode);
        query.exec();
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnRun_clicked()
{    
    host = ui->edHost->text();
    port = ui->edPort->value();
    threadReadSocket = new ThreadReadSocket();
    connect(threadReadSocket,SIGNAL(readyData()),this,SLOT(slotReadyData()));
    threadReadSocket->start();
}

void MainWindow::on_edHost_editingFinished()
{
    QSettings *settings = new QSettings("settings.ini",QSettings::IniFormat);
    host = ui->edHost->text();
    settings->setValue("conf/host",host);
    settings->sync();
}

void MainWindow::on_edPort_editingFinished()
{
    QSettings *settings = new QSettings("settings.ini",QSettings::IniFormat);
    port = ui->edPort->value();
    settings->setValue("conf/port",port);
    settings->sync();
}

void MainWindow::on_btnLog_clicked()
{
    RequstDate = ui->dateEdit->date();
    if(!isDBonnect) isDBonnect = ConnectDB();
    if(isDBonnect){
        QSqlTableModel *model = new QSqlTableModel;
        model->setTable("log");
        model->setFilter(QString("f_date = '%1'").arg(RequstDate.toString("yyyy-MM-dd")));
        model->select();
        model->setHeaderData(0,Qt::Horizontal,QObject::tr("Дата"));
        model->setHeaderData(1,Qt::Horizontal,QObject::tr("Время"));
        model->setHeaderData(2,Qt::Horizontal,QObject::tr("Код выполнения"));
        ui->tableView->setModel(model);
        ui->tableView->resizeColumnsToContents();
        ui->tableView->show();
    }
}

void MainWindow::on_dateEdit_dateChanged(const QDate &date)
{
    QSettings *settings = new QSettings("settings.ini",QSettings::IniFormat);
    RequstDate = date;
    settings->setValue("conf/requestdate",RequstDate);
    settings->sync();
}
