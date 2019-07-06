#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtWidgets>
#include <QDataStream>
#include <QtEndian>

#include <QPainter>
#include <QPaintEvent>

#include <QString>
#include <iostream>

#define X_ZERO 350
#define Y_ZERO 400

typedef struct ObjectStruct{
    quint16 rangeIndex;
    quint16 dopplerIndex;
    quint16 peakVal;
    qint16 x;
    qint16 y;
    qint16 z;
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
    QTimer* timer;
    void plotObjects(QPainter &p);
    void plotting();
    uint counter = 0;
    ~Widget();

private:
    Ui::Widget *ui;

    QByteArray serialData;

    QSerialPort *radar;

    static const quint16 radar_vendor_ID = 1105;
    static const quint16 radar_product_ID = 48883;
    QString radar_port_name;
    bool radar_is_available;

    QByteArray magicWord = QByteArray("\x02\x01\x04\x03\x06\x05\x08\x07", 8); // 2 1 4 3 6 5 8 7

    int start, current;
    uint numberObjects;
    quint16 qValue;
    bool ok;

    ObjectStruct objects[100];

private slots:
    bool readSerial();
    void paintEvent(QPaintEvent *e);




};

#endif // WIDGET_H
