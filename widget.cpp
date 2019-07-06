#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

//    timer->setInterval(1000.0/24.0);

//    connect(timer, SIGNAL(timeout()), this, SLOT(plotting()));

    radar = new QSerialPort;
    radar_is_available = false;
    radar_port_name = "";
    serialData = "";


    qDebug() << magicWord.mid(0, 2).toHex().toInt(&ok, 16);

    //Connect to radar
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier())
        {
            if(serialPortInfo.vendorIdentifier() == radar_vendor_ID && serialPortInfo.productIdentifier() == radar_product_ID)
            {
                radar_port_name = serialPortInfo.portName();
                radar_is_available = true;
            }
        }
    }

    //Set up radar serial connection
    if(radar_is_available)
    {
        // format port for communication
        radar->setPortName(radar_port_name);
        radar->open(QSerialPort::ReadOnly);
        radar->setBaudRate(921600);
        radar->setDataBits(QSerialPort::Data8);
        radar->setParity(QSerialPort::NoParity);
        radar->setStopBits(QSerialPort::OneStop);
        radar->setFlowControl(QSerialPort::NoFlowControl);

        qDebug() << serialData.size();

        //stream.setDevice(radar);
        QDataStream stream(serialData);
        stream.setByteOrder(QDataStream::LittleEndian);

        QObject::connect(radar, SIGNAL(readyRead()), this, SLOT(readSerial()));
    }
    else{
        // print error message
        QMessageBox::warning(this, "Port Error", "Couldn't find the radar or port was unavailable");
    }



}

bool Widget::readSerial()
{
    serialData += radar->readAll();

    if(!currentlyParsing)
    {
        findMagicWord();
        parseData();
    }

    return true;
}

bool Widget::parseData()
{
    currentlyParsing = true;

    while(serialData.length() - current < 32)
    {
        radar->waitForReadyRead();
        serialData += radar->readAll();
    }

    current += 28;

    QByteArray numObjectBytes;
    numObjectBytes = serialData.mid(current, 4);

    numberObjects = qFromLittleEndian<quint32>(numObjectBytes);

    qDebug() << "Reading_#: " << ++counter;

    qDebug() << "Number_of_Objects: " << numberObjects;

    while(serialData.length() - current < 24)
    {
        radar->waitForReadyRead();
        serialData += radar->readAll();
    }

    current += 22;

    numObjectBytes = serialData.mid(current, 2);//Not really number of ojects bytes just using that so dont have to make another int
    qValue = qFromLittleEndian<quint16>(numObjectBytes);

    qDebug() << "Q-Value: " << qValue;

    current += 2;

    while((uint)serialData.length() - (uint)current < numberObjects*12)
    {

//        qDebug() << "Have: " << serialData.length() - current;
//        qDebug() << "Need: " << numberObjects*12;
//        qDebug() << "Waiting Number Objects";
        radar->waitForReadyRead();
        serialData += radar->readAll();
    }

    for(uint i = 0; i < numberObjects; i++)
    {
        qDebug() << "      Object_" << i+1;

        objects[i].rangeIndex = qFromLittleEndian<quint16>(serialData.mid(current,2));
        current += 4;

        qDebug() << "      Range_Index: " << objects[i].rangeIndex;

        objects[i].peakVal = qFromLittleEndian<quint16>(serialData.mid(current,2));
        current += 2;

        qDebug() << "      Peak_Value: " << objects[i].peakVal;

        objects[i].x = qFromLittleEndian<qint16>(serialData.mid(current,2));
        current += 2;

        qDebug() << "      X: " << (float)(objects[i].x)/pow(2,9);

        objects[i].y = qFromLittleEndian<qint16>(serialData.mid(current,2));
        current += 2;

        qDebug() << "      Y: " << (float)objects[i].y/pow(2,9);

        objects[i].z = qFromLittleEndian<qint16>(serialData.mid(current,2))/pow(2,9);
        current += 2;

        qDebug() << "      Z: " << (float)objects[i].z/pow(2,9);

    }

    repaint();

    current = start;

    //serialData = "";

    currentlyParsing = false;

    return true;
}

bool Widget::findMagicWord()
{
    if(serialData.length() < 16)
    {
        return false;
    }

    int temp = serialData.lastIndexOf(magicWord);

    if(start != temp)
    {
        start = temp;
        current = start;
    }

    qDebug() << "Found_Magic_Word: " << start;

    return true;
}

Widget::~Widget()
{    
    QString data = QString::fromStdString(serialData.toStdString());
//    qDebug() << data;
    qDebug() << serialData.toHex();
//    qDebug() << magicWord.toHex();

    if(radar->isOpen())
    {
        radar->close();
    }

    delete ui;
}

void Widget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    if(numberObjects > 0)
    {
        plotObjects(p);
    }
}

void Widget::plotObjects(QPainter &p)
{
    QRectF rectangle(X_ZERO-5, Y_ZERO-5, 10, 10);
    QBrush myBrush;

    for(uint i = 0; i < numberObjects; i++)
    {
        rectangle.moveCenter(QPoint(objects[i].x/pow(2,9)*30 + X_ZERO, Y_ZERO - objects[i].y/pow(2,9)*30));
        p.fillRect(rectangle, Qt::white);
    }
}
void Widget::plotting()
{
    repaint();
}
