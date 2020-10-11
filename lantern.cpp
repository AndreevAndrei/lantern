#include <QtGui>
#include <QTime>
#include <QMessageBox>
#include <QGraphicsEffect>

#include "lantern.h"

Lantern::Lantern():
    tcpSocket(new QTcpSocket(this))
{
    ui.setupUi(this);
    statusBar()->hide();

    // initialize style
    QFile file(":/qss/qdarkstyle.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);

    // extra form setting
    ui.le_port->setValidator(new QIntValidator(1, 65535, this));

    // main
    connect(ui.pb_connect, &QAbstractButton::clicked, this, &Lantern::connectionClick);
    connect(tcpSocket, &QIODevice::readyRead, this, &Lantern::readData);
    connect(tcpSocket, &QIODevice::readChannelFinished, this, &Lantern::forcedCloseConnection);
    connect(tcpSocket, &QAbstractSocket::errorOccurred, this, &Lantern::displayError);
}

void Lantern::connectionClick()
{
    // form preparation
    ui.pb_connect->setEnabled(false);
    inProgress = !inProgress;
    ui.pb_connect->setText(inProgress?tr("Disconnect"):tr("Connect"));
    ui.cb_ip->setEnabled(!inProgress);
    ui.le_port->setEnabled(!inProgress);

    tcpSocket->abort();

    if (inProgress)
    {
        connectionName = QString("%1:%2").arg(ui.cb_ip->currentText(), ui.le_port->text());
        Log(tr("Tring to connect to '%1'.").arg(connectionName));
        setWindowTitle(tr("Lantern").append(" <"+connectionName+">"));
        tcpSocket->connectToHost(ui.cb_ip->currentText(), ui.le_port->text().toInt());
    }
    else
    {
        connectionName = "";
        setWindowTitle(tr("Lantern"));
        ui.le_state_text->setText(tr("UNKNOWN"));
        ui.le_color_text->setText(tr("UNKNOWN"));
        delete ui.le_color->graphicsEffect();
        ui.le_color->update();
    }

    ui.pb_connect->setEnabled(true);
}

void Lantern::readData()
{
    // read raw data
    QByteArray rawData = tcpSocket->readAll();
    QString stringData = QString("0x").append(rawData.toHex().data());
    Log(QString("<%1>: '%2'").arg(connectionName, stringData));

    // handling data
    LanternPacket packet;
    int i = 0;
    packet.type = (byte)rawData.data()[i++];
    packet.length = ((byte)rawData.data()[i++] << 8) + (byte)rawData.data()[i++];
    if(packet.type == 0x12) // DATA: LANTERN ON
    {
        ui.le_state_text->setText(tr("ON"));
    }
    else if(packet.type == 0x13) // DATA: LANTERN OFF
    {
        ui.le_state_text->setText(tr("OFF"));
    }
    else if(packet.type == 0x20 && packet.length == 0x03) // DATA: LANTERN CHANGE COLOR
    {
        packet.color.setRed((byte)rawData.data()[i++]);
        packet.color.setGreen((byte)rawData.data()[i++]);
        packet.color.setBlue((byte)rawData.data()[i++]);

        delete ui.le_color->graphicsEffect();
        QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect;
        effect->setColor(packet.color);
        ui.le_color->setGraphicsEffect(effect);
        ui.le_color_text->setText(packet.color.name().toUpper());
    }
}

void Lantern::forcedCloseConnection()
{
    Log((tr("Forced close connection '%1'.").arg(connectionName)));
    if (inProgress)
        connectionClick();
}

void Lantern::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Lantern Connection"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Lantern Connection"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Lantern Connection"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }
}

void Lantern::Log(QString text)
{
    auto t = QDateTime::currentDateTimeUtc().toString("hh:mm:ss.zzz");
    ui.le_log->append(QString("[%1] %2").arg(t, text));
}
