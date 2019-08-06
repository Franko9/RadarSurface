#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    myTimer->setInterval(2000);
    myTimer->start();

    radar = new QSerialPort;
    radar_is_available = false;
    radar_port_name = "";
    serialData = "";

    arduino = new QSerialPort;
    arduino_is_available = false;
    arduino_port_name = "";
    arduinoSerialData = "";


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
        radar->setPortName(radar_port_name);
        radar->open(QSerialPort::ReadOnly);
        radar->setBaudRate(921600);
        radar->setDataBits(QSerialPort::Data8);
        radar->setParity(QSerialPort::NoParity);
        radar->setStopBits(QSerialPort::OneStop);
        radar->setFlowControl(QSerialPort::NoFlowControl);

        QObject::connect(radar, SIGNAL(readyRead()), this, SLOT(readSerial()));
    }
    else{
        QMessageBox::warning(this, "Radar Port Error", "Couldn't find the radar or port was unavailable");
    }


    // Connect to arduino serial
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier())
        {

            if(serialPortInfo.vendorIdentifier() == arduino_vendor_ID && serialPortInfo.productIdentifier() == arduino_product_ID)
            {
                arduino_port_name = serialPortInfo.portName();
                arduino_is_available = true;
            }
        }
    }

    //Set up arduino serial connection
    if(arduino_is_available)
    {
        arduino->setPortName(arduino_port_name);
        arduino->open(QSerialPort::WriteOnly);
        arduino->setBaudRate(115200);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setFlowControl(QSerialPort::NoFlowControl);

        QObject::connect(myTimer, SIGNAL(timeout()), this, SLOT(writeSerial()));
    }
    else{
        QMessageBox::warning(this, "Arduino Port Error", "Couldn't find the arduino or port was unavailable");
    }


}

void Widget::displayError()
{
    QMessageBox::warning(this, "Port Error", "An error occured with the serial port");
}

bool Widget::readSerial()
{
    serialData += radar->readAll();

    if(!currentlyParsing)
    {
        if(findMagicWord())
        {
            parseData();
        }
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


    //qDebug() << "Reading #: " << ++counter;

    //qDebug() << "Number of Objects: " << numberObjects;

    while(serialData.length() - current < 24)
    {
        radar->waitForReadyRead();
        serialData += radar->readAll();
    }

    current += 22;

    numObjectBytes = serialData.mid(current, 2);//Not really number of ojects bytes just using that so dont have to make another int

    //qValue = qFromLittleEndian<quint16>(numObjectBytes);

    //qDebug() << "Q Value: " << qValue;

    current += 2;

    while((uint)serialData.length() - (uint)current < numberObjects*12)
    {
        radar->waitForReadyRead();
        serialData += radar->readAll();
    }

    for(uint i = 0; i < numberObjects; i++)
    {
        //qDebug() << "      Object: " << i+1;

        objects[i].rangeIndex = qFromLittleEndian<quint16>(serialData.mid(current,2));
        current += 4;

        //qDebug() << "      Range Index: " << objects[i].rangeIndex;

        objects[i].peakVal = qFromLittleEndian<quint16>(serialData.mid(current,2));
        current += 2;

        //qDebug() << "      Peak Value: " << objects[i].peakVal;

        objects[i].x = qFromLittleEndian<qint16>(serialData.mid(current,2));
        current += 2;

        //qDebug() << "      X: " << (float)(objects[i].x)/pow(2,9);

        objects[i].y = qFromLittleEndian<qint16>(serialData.mid(current,2));
        current += 2;

        //qDebug() << "      Y: " << (float)objects[i].y/pow(2,9);

        objects[i].z = qFromLittleEndian<qint16>(serialData.mid(current,2))/qValue;
        current += 2;

        //qDebug() << "      Z: " << (float)objects[i].z/pow(2,9);

        if(objects[i].x/qValue < 0.3 && objects[i].x/qValue > -0.3)
        {
            x = objects[i].x;
            y = objects[i].y;
            if(objects[i].peakVal/qValue < 6000)
            {
                cond = 1;
            }
            else{
                cond = 2;
            }
        }

    }

    if(numberObjects > 0)
    {
        ui->progressBar_1->setValue(objects[0].peakVal);
        ui->label_1->setText(QString::fromStdString(std::to_string(objects[0].peakVal)));
    }
    else
    {
        ui->progressBar_1->setValue(0);
        ui->label_1->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 1)
    {
        ui->progressBar_2->setValue(objects[1].peakVal);
        ui->label_2->setText(QString::fromStdString(std::to_string(objects[1].peakVal)));
    }
    else
    {
        ui->progressBar_2->setValue(0);
        ui->label_2->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 2)
    {
        ui->progressBar_3->setValue(objects[2].peakVal);
        ui->label_3->setText(QString::fromStdString(std::to_string(objects[2].peakVal)));
    }
    else
    {
        ui->progressBar_3->setValue(0);
        ui->label_3->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 3)
    {
        ui->progressBar_4->setValue(objects[3].peakVal);
        ui->label_4->setText(QString::fromStdString(std::to_string(objects[3].peakVal)));
    }
    else
    {
        ui->progressBar_4->setValue(0);
        ui->label_4->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 4)
    {
        ui->progressBar_5->setValue(objects[4].peakVal);
        ui->label_5->setText(QString::fromStdString(std::to_string(objects[4].peakVal)));
    }
    else
    {
        ui->progressBar_5->setValue(0);
        ui->label_5->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 5)
    {
        ui->progressBar_6->setValue(objects[5].peakVal);
        ui->label_6->setText(QString::fromStdString(std::to_string(objects[5].peakVal)));
    }
    else
    {
        ui->progressBar_6->setValue(0);
        ui->label_6->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 6)
    {
        ui->progressBar_7->setValue(objects[6].peakVal);
        ui->label_7->setText(QString::fromStdString(std::to_string(objects[6].peakVal)));
    }
    else
    {
        ui->progressBar_7->setValue(0);
        ui->label_7->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 7)
    {
        ui->progressBar_8->setValue(objects[7].peakVal);
        ui->label_8->setText(QString::fromStdString(std::to_string(objects[7].peakVal)));
    }
    else
    {
        ui->progressBar_8->setValue(0);
        ui->label_8->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 8)
    {
        ui->progressBar_9->setValue(objects[8].peakVal);
        ui->label_9->setText(QString::fromStdString(std::to_string(objects[8].peakVal)));
    }
    else
    {
        ui->progressBar_9->setValue(0);
        ui->label_9->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 9)
    {
        ui->progressBar_10->setValue(objects[9].peakVal);
        ui->label_10->setText(QString::fromStdString(std::to_string(objects[9].peakVal)));
    }
    else
    {
        ui->progressBar_10->setValue(0);
        ui->label_10->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 10)
    {
        ui->progressBar_11->setValue(objects[10].peakVal);
        ui->label_11->setText(QString::fromStdString(std::to_string(objects[10].peakVal)));
    }
    else
    {
        ui->progressBar_11->setValue(0);
        ui->label_11->setText(QString::fromStdString(std::to_string(0)));
    }

    if(numberObjects > 11)
    {
        ui->progressBar_12->setValue(objects[11].peakVal);
        ui->label_12->setText(QString::fromStdString(std::to_string(objects[11].peakVal)));
    }
    else
    {
        ui->progressBar_12->setValue(0);
        ui->label_12->setText(QString::fromStdString(std::to_string(0)));
    }

    repaint();

    //current = start;

    serialData = "";

    currentlyParsing = false;

    return true;
}

bool Widget::findMagicWord()
{

    int temp = serialData.lastIndexOf(magicWord);

    if(serialData.length() < 16 || temp == -1)
    {
        return false;
    }
    else{
        current = temp;
        //qDebug() << "Found Magic Word: " << start;
        return true;
    }
}

Widget::~Widget()
{    
//    QString data = QString::fromStdString(serialData.toStdString());
//    qDebug() << serialData.toHex();

    if(radar->isOpen())
    {
        radar->close();
    }

    if(arduino->isOpen())
    {
        arduino->close();
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
    QRectF rectangle(X_ZERO-5, Y_ZERO-5, 20, 20);
//    QBrush myBrush;
//    p.setBrush(myBrush);

    for(uint i = 0; i < numberObjects; i++)
    {
        rectangle.moveCenter(QPoint(objects[i].x/qValue*30 + X_ZERO, Y_ZERO - objects[i].y/qValue*30));

        if(true)//objects[i].x > -0.2 && objects[i].x < 0.2 && objects[i].y > 0.8 && objects[i].y < 1.2)
        {
            if(objects[i].peakVal < 6000 && objects[i].peakVal > 0)
            {
                p.fillRect(rectangle, Qt::green);//dry
            }
            else
            {
                p.fillRect(rectangle, Qt::blue);//wet
            }
        }
    }

}

void Widget::plotting()
{
    repaint();
}

void Widget::on_pushButton_3_clicked()
{
    qDebug() << "Found Magic Word: " << start;
    qDebug() << "Reading #: " << ++counter;
    qDebug() << "Number of Objects: " << numberObjects;
    qDebug() << "Q Value: " << qValue;
    for(uint i = 0; i < numberObjects; i++)
    {
        qDebug() << "      Object: " << i+1;
        qDebug() << "      Range Index: " << objects[i].rangeIndex;
        qDebug() << "      Peak Value: " << objects[i].peakVal;
        qDebug() << "      X: " << (float)(objects[i].x)/qValue;
        qDebug() << "      Y: " << (float)objects[i].y/qValue;
        qDebug() << "      Z: " << (float)objects[i].z/qValue;
    }
    QSound::play("/Users/macuser/Downloads/smw_kick.wav");
}

void Widget::writeSerial()
{
    qDebug() << "In Write Serial";
    qDebug() << "x: " << x;
    qDebug() << "y: " << y;
    qDebug() << "cond: " << cond;
    arduinoSerialData = "";
//    arduinoSerialData += QString::number(x).toUtf8();
//    arduinoSerialData += QString::number(y).toUtf8();
//    arduinoSerialData += cond;

    char *xval = (char*)&x;
    char *yval = (char*)&y;
    char *condval = (char*)&cond;

    arduinoSerialData.append(xval);
    arduinoSerialData.append(yval);
    arduinoSerialData.append(condval);

    qDebug() << arduinoSerialData.toHex();
    //QString data = "ABCDE";
    //arduinoSerialData = data.toUtf8();
    arduino->write(arduinoSerialData);
}

//void Widget::sendToArduino()
//{
//    arduino->write(QString::number(numberObjects).toStdString().c_str());
//    for(uint i = 0; i < numberObjects; i++)
//    {
//        arduino->write(QString::number(objects[i].x).toStdString().c_str());
//        arduino->write(QString::number(objects[i].y).toStdString().c_str());
//        arduino->write(QString::number(objects[i].peakVal).toStdString().c_str());
//    }

//}
