#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QTcpSocket>

#include "ui_lantern.h"

class StyleSheetEditor;

class Lantern : public QMainWindow
{
    Q_OBJECT

public:
    Lantern();
    void Log(QString text);

private slots:
    void connectionClick();
    void readData();
    void forcedCloseConnection();
    void displayError(QAbstractSocket::SocketError socketError);

private:
    bool inProgress = false;
    QString connectionName = "";
    QTcpSocket *tcpSocket = nullptr;
    Ui::MainWindow ui;
};

#pragma pack(1)
struct LanternPacket
{
    byte type;
    ushort length;

    QColor color;
};

#endif
