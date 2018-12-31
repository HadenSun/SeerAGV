#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <stdint.h>
#include <stdlib.h>
#include <QMainWindow>
#include <QtNetwork/QtNetwork>
#include <QMessageBox>
#include <QDateTime>



#define _IP "192.168.10.107"
#define _PORT 19206
#define _POINT_NUMBER 2
#define _ID_NUMBER 2034
#define _VERTICAL



struct ProtocolHeader {
    uint8_t  m_sync;
    uint8_t  m_version;
    uint16_t m_number;
    uint32_t m_length;
    uint16_t m_type;
    uint8_t  m_reserved[6];
};



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void socket_readPendingDatagrams();


    void on_deleButton_clicked();
    void on_takeButton_clicked();
    void on_deliverButton_clicked();
    void on_refreshButton_clicked();
    void socket_disconnect();
    void socket_connected();

    void on_resetButton_clicked();

    void on_autoButton_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    int isReceved;          //是否收到回复
    int id_number;          //id序号

    int packetCreate(int APICode, int packetId, char* jsonData,char *data);
    void freshTable();
};

#endif // MAINWINDOW_H
