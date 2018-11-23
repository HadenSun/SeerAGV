#include "mainwindow.h"
#include "ui_mainwindow.h"

//#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QString::fromUtf8("仓储调度"));                  //修改标题
    setAutoFillBackground(true);                //修改背景颜色，必须有这条语句
    setPalette(QPalette(QColor(255,204,1)));
    QPixmap pixmap("C:\\Users\\ayshx\\Documents\\AGV\\SeerAGV\\SeerAGV\\DHL.jpg");                  //加载logo
    QPixmap fitpixmap = pixmap.scaled(480, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->label->setPixmap(fitpixmap);
    showMaximized();
    id_number = _ID_NUMBER;

    freshTable();

    socket = new QTcpSocket(this);
    isReceved = 1;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connectButton_clicked()
{
    if(!socket->isOpen())
    {
        //TCP连接
        int port = _PORT;
        socket = new QTcpSocket(this);
        socket->abort();
        socket->connectToHost(QHostAddress(_IP),port);

        //连接槽

        connect(socket,SIGNAL(readyRead()),this,SLOT(socket_readPendingDatagrams()));
        connect(socket,SIGNAL(disconnected()),this,SLOT(socket_disconnect()));
        connect(socket,SIGNAL(connected()),this,SLOT(socket_connected()));

        //ui更新
        //ui->connectButton->setText(QString::fromUtf8("断开"));

    }
    else
    {
        //TCP关闭
        socket->close();
        socket->abort();
        disconnect(socket,SIGNAL(readyRead()),this,SLOT(socket_readPendingDatagrams()));

        //UI更新
        //ui->connectButton->setText(QString::fromUtf8("连接"));
    }
}

void MainWindow::socket_readPendingDatagrams()
{
    //接受数据
    isReceved = 1;

    while (socket->bytesAvailable()>0)
    {
        char buf[1024];
        int length=socket->bytesAvailable();
        socket->read(buf, length);
        buf[length] = 0;
        QString msg;

        if(length != 16)
        {
            char* outChar = buf+16;
            QString str = QString("%1").arg(outChar);
            msg+=str+"\n";

            QByteArray qstr = str.toLatin1();
            QJsonParseError json_error;
            QJsonDocument jsonDoc(QJsonDocument::fromJson(qstr,&json_error));

            if(json_error.error!= QJsonParseError::NoError)
            {
                QMessageBox::warning(NULL,"Warning",QString::fromUtf8("Json解析错误"),QMessageBox::Yes);
                break;
            }

            QJsonObject rootObj = jsonDoc.object();
            QStringList keys = rootObj.keys();
            for(int i = 0; i < keys.size(); i++)
            {
                msg += keys.at(i) + " : " + rootObj.value(keys.at(i)).toString() + "\n";
            }
            QMessageBox::information(NULL,"information",msg,QMessageBox::Yes);
        }
    }
}

void MainWindow::socket_connected()
{
    ui->connectButton->setText(QString::fromUtf8("断开"));
    isReceved = 1;
}

void MainWindow::socket_disconnect()
{
    QMessageBox::warning(NULL,"Title",QString::fromUtf8("连接断开"),QMessageBox::Yes);
    ui->connectButton->setText(QString::fromUtf8("连接"));
}

int MainWindow::packetCreate(int APICode, int packetId, char* jsonData,char *data)
{
    int len = 0;
    ProtocolHeader *protolHeader = (ProtocolHeader*)data;
    protolHeader->m_sync = 90;
    protolHeader->m_version = 1;
    protolHeader->m_number = (uint16_t)packetId << 8 | (packetId>>8 & 0x0F);
    protolHeader->m_type = (uint16_t)APICode << 8 | (APICode>>8 & 0x0F);
    protolHeader->m_reserved[0] = 0;
    protolHeader->m_reserved[1] = 0;
    protolHeader->m_reserved[2] = 0;
    protolHeader->m_reserved[3] = 0;
    protolHeader->m_reserved[4] = 0;
    protolHeader->m_reserved[5] = 0;
    len = strlen(jsonData);
    uint32_t l = (uint32_t)len;
    protolHeader->m_length =  ((l & 0x00FF) << 24) | ((l & 0xff00) << 8);
    memcpy(data+16,jsonData,strlen(jsonData));
    data[16+len] = 0;

    return strlen(jsonData) + 16;
}


void MainWindow::on_deleButton_clicked()
{
    QPushButton *btn = (QPushButton*)sender();
    QModelIndex index = ui->tableWidget->indexAt(btn->pos());
    ui->tableWidget->removeRow(index.row());
    //QMessageBox::information(NULL,"title",QString::number(index.row()),QMessageBox::Yes);

}

void MainWindow::on_takeButton_clicked()
{
    if(!socket->isOpen())
    {
        QMessageBox::warning(NULL,"Warning",QString::fromUtf8("请连接！"),QMessageBox::Yes);
    }
    else if(!isReceved)
    {
        QMessageBox::warning(NULL,"Warning",QString::fromUtf8("等待上一条命令执行！"),QMessageBox::Yes);
    }
    else
    {
        QPushButton *btn = (QPushButton*)sender();
        QString position = btn->property("position").toString();
        int row = ui->tableWidget->indexAt(btn->pos()).row();
        ui->tableWidget->item(row,2)->setText(QString::fromUtf8("取货"));


        char data[100];
        char json[100];

        if(position == "A")
        {
            char json_data[] = "{\"name\":\"tasks1_2\"}";
            memcpy(json,json_data,strlen(json_data));
            json[strlen(json_data)] = 0;
        }
        else if(position == "B")
        {
            char json_data[] = "{\"name\":\"tasks1_3\"}";
            memcpy(json,json_data,strlen(json_data));
            json[strlen(json_data)] = 0;
        }
        else if(position == "C")
        {
            char json_data[] = "{\"name\":\"tasks1_4\"}";
            memcpy(json,json_data,strlen(json_data));
            json[strlen(json_data)] = 0;
        }
        else if(position == "D")
        {
            char json_data[] = "{\"name\":\"tasks1_5\"}";
            memcpy(json,json_data,strlen(json_data));
            json[strlen(json_data)] = 0;
        }
        int len = packetCreate(3106,12,json,data);
        socket->write(data,len);

        isReceved = 0;
    }

    //QMessageBox::information(NULL,"title",position,QMessageBox::Yes);
}

void MainWindow::on_deliverButton_clicked()
{
    if(!socket->isOpen())
    {
        QMessageBox::warning(NULL,"Warning",QString::fromUtf8("请连接！"),QMessageBox::Yes);
    }
    else if(!isReceved)
    {
        QMessageBox::warning(NULL,"Warning",QString::fromUtf8("等待上一条命令执行！"),QMessageBox::Yes);
    }
    else
    {
        QPushButton *btn = (QPushButton*)sender();
        QString position = btn->property("position").toString();
        int row = ui->tableWidget->indexAt(btn->pos()).row();
        ui->tableWidget->item(row,2)->setText(QString::fromUtf8("送货"));


        char data[100];
        char json[100];

        if(position == "A")
        {
            char json_data[] = "{\"name\":\"tasks2_1\"}";
            memcpy(json,json_data,strlen(json_data));
            json[strlen(json_data)] = 0;
        }
        else if(position == "B")
        {
            char json_data[] = "{\"name\":\"tasks3_1\"}";
            memcpy(json,json_data,strlen(json_data));
            json[strlen(json_data)] = 0;
        }
        else if(position == "C")
        {
            char json_data[] = "{\"name\":\"tasks4_1\"}";
            memcpy(json,json_data,strlen(json_data));
            json[strlen(json_data)] = 0;
        }
        else if(position == "D")
        {
            char json_data[] = "{\"name\":\"tasks5_1\"}";
            memcpy(json,json_data,strlen(json_data));
            json[strlen(json_data)] = 0;
        }
        int len = packetCreate(3106,12,json,data);
        socket->write(data,len);

        isReceved = 0;
    }

    //QMessageBox::information(NULL,"title",position,QMessageBox::Yes);
}

void MainWindow::freshTable()
{
    //表格部分
    QStringList header;
    header<<QString::fromUtf8(" 订单编号 ")<<" 库位 "<<"模式"<<"  取货  "<<"  送货  "<<"  控制  ";

    int count = ui->tableWidget->rowCount();
    if(count == 0)
    {
        ui->tableWidget->setColumnCount(6);
        ui->tableWidget->setColumnWidth(0,1100);
        ui->tableWidget->setColumnWidth(1,400);
        ui->tableWidget->setColumnWidth(2,400);
        ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);    //禁止编辑
        ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);
        ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableWidget->setHorizontalHeaderLabels(header);
        ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    }

    if(count < 40)
    {
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        for(int i = count; i < 40;i++)
        {
            ui->tableWidget->insertRow(i);              //插入行
            ui->tableWidget->setRowHeight(i,100);       //设置行高

            QDateTime data = QDateTime::currentDateTime();
            QString data_string = data.toString("yyyyMMdd");
            data_string += QString("%1").arg(id_number,4,10,QLatin1Char('0'));
            id_number++;
            ui->tableWidget->setItem(i,0,new QTableWidgetItem(data_string));

            QChar position = 'A' + qrand()%_POINT_NUMBER;
            ui->tableWidget->setItem(i,1,new QTableWidgetItem(position));

            ui->tableWidget->setItem(i,2,new QTableWidgetItem("等待"));

            QPushButton *takeButton = new QPushButton();
            takeButton->setText("取货");
            takeButton->setProperty("position",position);
            takeButton->setProperty("row",i);
            connect(takeButton,SIGNAL(clicked()),this,SLOT(on_takeButton_clicked()));
            ui->tableWidget->setCellWidget(i,3,takeButton);

            QPushButton *deliverButton = new QPushButton();
            deliverButton->setText("送货");
            deliverButton->setProperty("position",position);
            connect(deliverButton,SIGNAL(clicked()),this,SLOT(on_deliverButton_clicked()));
            ui->tableWidget->setCellWidget(i,4,deliverButton);

            QPushButton *deleButton = new QPushButton();
            deleButton->setText("删除");
            connect(deleButton,SIGNAL(clicked()),this,SLOT(on_deleButton_clicked()));
            ui->tableWidget->setCellWidget(i,5,deleButton);

        }

    }
}

void MainWindow::on_refreshButton_clicked()
{
    freshTable();
}



void MainWindow::on_resetButton_clicked()
{
    socket->close();
    socket->abort();
    isReceved = 1;

    for(int i=0;i<ui->tableWidget->rowCount();i++)
    {
        ui->tableWidget->item(i,2)->setText(QString::fromUtf8("等待"));
    }

    freshTable();
}

void MainWindow::on_autoButton_clicked()
{
    if(!socket->isOpen())
    {
        QMessageBox::warning(NULL,"Warning",QString::fromUtf8("请连接！"),QMessageBox::Yes);
    }
    else if(!isReceved)
    {
        QMessageBox::warning(NULL,"Warning",QString::fromUtf8("等待上一条命令执行！"),QMessageBox::Yes);
    }
    else
    {
        char data[100];
        char json[100];

        char json_data[] = "{\"name\":\"tasks_re\"}";
        memcpy(json,json_data,strlen(json_data));
        json[strlen(json_data)] = 0;

        int len = packetCreate(3106,12,json,data);
        socket->write(data,len);

        isReceved = 0;
    }
}
