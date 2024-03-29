#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtWidgets>
#include <QDataStream>
#include <QtEndian>
#include <QTimer>

#include <QPainter>
#include <QPaintEvent>

#include <QString>
#include <iostream>

#include <QSound>

#include <QProgressBar>

#define X_ZERO 350
#define Y_ZERO 400

enum Condition{
    dry, wet, snow, ice
};

typedef struct ObjectStruct{
    quint16 rangeIndex;
    quint16 dopplerIndex;
    quint16 peakVal;
    qint16 x;
    qint16 y;
    qint16 z;
    Condition cond;
}ObjectStruct;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    bool findMagicWord();
    bool parseData();
    bool currentlyParsing = false;
    QTimer* myTimer = new QTimer;
    QTimer* plotTimer = new QTimer;
    QTimer* errorTimer = new QTimer;
    void plotObjects(QPainter &p);
    void establishComm();

    QProgressBar* progressBar[12];

    uint counter = 0;

    QDataStream *stream;

    ~Widget();


private:
    Ui::Widget *ui;

    QByteArray serialData;

    QSerialPort *radar;

    static const quint16 radar_vendor_ID = 1105;
    static const quint16 radar_product_ID = 48883;
    QString radar_port_name;
    bool radar_is_available;

    QSerialPort *arduino;
    bool arduino_is_available;
    QString arduino_port_name;
    QByteArray arduinoSerialData;

    static const quint16 arduino_vendor_ID = 9025;
    static const quint16 arduino_product_ID = 66;

    QByteArray magicWord = QByteArray("\x02\x01\x04\x03\x06\x05\x08\x07", 8); // 2 1 4 3 6 5 8 7

    int start, current;
    uint numberObjects;
    quint16 qValue = 512;
    bool ok;

    ObjectStruct objects[100];

    void sendToArduino();

    QColor colors[4] = {Qt::green, Qt::blue, Qt::gray, Qt::white};

    int timeNum = 0;
    bool isData = false;

    double xMin = 100.0, xMax = 100.0, yMin = 100.0, yMax = 100.0;

private slots:
    bool readSerial();
    void paintEvent(QPaintEvent *e);
    void displayError();
    void writeSerial();
    void plotting();
    //void checkConnected();
    void on_pushButton_3_clicked();
    void timerCheck();

};

#endif // WIDGET_H
