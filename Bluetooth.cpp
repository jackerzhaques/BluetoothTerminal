#include "Bluetooth.h"

#include <QDateTime>

Bluetooth::Bluetooth(QObject *parent) : QObject(parent)
{
    this->logger = new Logger(this);
    logger->setLogFile(this->LogFilePath);
    logger->startLogging();
}

Bluetooth::~Bluetooth()
{

    if(this->service){
        delete this->service;
        this->service = nullptr;
        qDebug() << "del";
    }

    if(this->m_control){
        this->m_control->disconnectFromDevice();
        delete this->m_control;
        this->m_control = nullptr;
        qDebug() << "del";
    }

    this->logMessage("Destroying bluetooth object...");
    this->logger->stopLogging();
}

void Bluetooth::refreshDeviceList()
{
    this->discoveredDevices.clear();

    QBluetoothDeviceDiscoveryAgent *discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    discoveryAgent->setLowEnergyDiscoveryTimeout(5000);
    connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));

    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

QStringList Bluetooth::getDeviceList()
{
    QStringList devices;

    for(QBluetoothDeviceInfo device : discoveredDevices){
        devices.append(device.name());
    }

    return devices;
}

QString Bluetooth::getDeviceName()
{
    return device.name();
}

void Bluetooth::setDeviceByName(QString device)
{
    for(int i = 0; i < discoveredDevices.size(); i++){
        if(discoveredDevices[i].name() == device){
            this->device = discoveredDevices.at(i); //Set class device info variable
        }
    }

    this->logMessage(QString("Device set to %1").arg(this->device.name()));
}

QByteArray Bluetooth::readAll()
{
    QByteArray data = this->dataBuffer;
    this->dataBuffer.clear();
    return data;
}

QString Bluetooth::getLine(QString terminator)
{
    QString line;

    qDebug() << dataBuffer;

    QString buffer = QString::fromLocal8Bit(dataBuffer);

    if(buffer.contains(terminator)){
        line = buffer.split(terminator).at(0);

        //Remove line from buffer
        dataBuffer.remove(0, line.size());
    }

    qDebug() << line;
    qDebug() << dataBuffer;

    return line;
}

QStringList Bluetooth::getAllLines(QString terminator)
{
    QStringList lines;

    //Split data into lines
    lines = QString::fromLocal8Bit(dataBuffer).split(terminator);

    //Remove data from buffer
    dataBuffer.clear();

    return lines;
}

void Bluetooth::clearBuffer()
{
    this->dataBuffer.clear();
}

void Bluetooth::write(QByteArray data)
{
    QString logString = "Writing data.. '%1' (%2 bytes)";
    logString = logString.arg(QString::fromUtf8(data)).arg(data.size());
    this->logMessage(logString);

    if(m_control){
        //Service discovered.
        QBluetoothUuid uuid = QBluetoothUuid(UART_TX_UUID);
        QLowEnergyCharacteristic hrChar = service->characteristic(uuid);
        if(hrChar.isValid()){
            service->writeCharacteristic(hrChar, data);
        }
        else{
            this->logMessage("Failed to write data");
        }
    }
}

void Bluetooth::write(const QString &data)
{
    write(data.toLocal8Bit());
}

void Bluetooth::write(const char data[])
{
    write(QByteArray::fromRawData(data, static_cast<int>(strlen(data))));
}

void Bluetooth::write(QStringList data)
{
    for(QString str : data){
        write(str.toLocal8Bit());
    }
}

void Bluetooth::write(const char data)
{
    write(QString("%1").arg(data));
}

void Bluetooth::connectToDevice()
{
    this->logMessage(QString("Connecting to device %1...").arg(device.name()));
    m_control = QLowEnergyController::createCentral(device, this);
    connect(m_control, SIGNAL(error(QLowEnergyController::Error)),
            this, SLOT(handleError(QLowEnergyController::Error)));
    connect(m_control, SIGNAL(serviceDiscovered(QBluetoothUuid)),
            this, SLOT(serviceDiscovered(QBluetoothUuid)));
    connect(m_control, SIGNAL(discoveryFinished()),
            this, SLOT(serviceScanDone()));
    connect(m_control, SIGNAL(connected()), this, SLOT(handleDeviceConnection()));
    connect(m_control, SIGNAL(disconnected()), this, SLOT(handleDeviceDisconnection()));

    m_control->connectToDevice();
}

void Bluetooth::connectToDevice(QString device)
{
    setDeviceByName(device);
    connectToDevice();
}

void Bluetooth::disconnectFromDevice()
{
    m_control->disconnectFromDevice();

    delete service;
    service = nullptr;
}

void Bluetooth::logMessage(QString message)
{
    if(this->logger->isLogging()){
        QString str;
        QDateTime time = QDateTime::currentDateTime();
        str += time.currentDateTime().toString(Qt::ISODateWithMs);
        str += " - ";
        str += message;
        str += "\r\n";  //Only \n needed for most editors. Notepad needs \r\n
        logger->log(str);
        qDebug() << str;
    }
}

void Bluetooth::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    this->discoveredDevices.append(device);
    emit deviceListAvailable();
}

//This function is called whenever a GATT service is discovered
void Bluetooth::serviceDiscovered(QBluetoothUuid uuid)
{
    if(uuid == UARTuuid){
        service = m_control->createServiceObject(uuid, this);

        if(service){
            qDebug() << "service started";
            connect(service, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(handleServiceStateChange(QLowEnergyService::ServiceState)));
            connect(service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this, SLOT(handleCharacteristicChange(QLowEnergyCharacteristic, QByteArray)));
            connect(service, SIGNAL(descriptorWritten(QLowEnergyDescriptor, QByteArray)), this, SLOT(handleDescriptorWrite(QLowEnergyDescriptor, QByteArray)));
            connect(service, SIGNAL(characteristicRead(QLowEnergyCharacteristic, QByteArray)), this, SLOT(handleCharacteristicChange(QLowEnergyCharacteristic, QByteArray)));
            service->discoverDetails();
        }
        else{
            qDebug() << "Failed to start UART service";
        }
    }
}

//This function is called whenever a GATT service scan is finished
void Bluetooth::serviceScanDone()
{
    //Do nothing
}

void Bluetooth::handleDeviceConnection()
{
    this->logMessage("Successfully connected");
    this->logMessage("Discovering services...");
    m_control->discoverServices();
    emit deviceConnected();
}

void Bluetooth::handleDeviceDisconnection()
{
    this->logMessage("Disconnected from device");
    emit deviceDisconnected();
}

void Bluetooth::handleServiceStateChange(QLowEnergyService::ServiceState state)
{
    switch(state){
        case QLowEnergyService::DiscoveringServices:
            this->logMessage("Service state changed to 'Discovering'");
            //Discovering services, do nothing
            break;
        case QLowEnergyService::ServiceDiscovered:
        //Scope needed for inner variables
        {
            this->logMessage("Service state changed to 'Discovered'");

            //Service discovered.
            QBluetoothUuid txUuid = QBluetoothUuid(UART_TX_UUID);
            QLowEnergyCharacteristic txChar = service->characteristic(txUuid);
            if(txChar.isValid()){
                this->logMessage("UART TX service discovered");
                service->writeCharacteristic(txChar, "Test data");
                emit deviceTransmitReady();
            }
            else{
                qDebug() << "TX Data not found.";
            }

            QBluetoothUuid rxUuid = QBluetoothUuid(UART_RX_UUID);
            QLowEnergyCharacteristic rxChar = service->characteristic(rxUuid);
            if(rxChar.isValid()){
                this->logMessage("UART RX Service discovered");
            }
            else{
                qDebug() << "RX Data not found.";
            }

            QLowEnergyDescriptor m_notificationDescTx = rxChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);

            if(m_notificationDescTx.isValid()){
                service->writeDescriptor(m_notificationDescTx, QByteArray::fromHex("0100"));
                this->logMessage("Notification desc");
            }

            break;
        }
        default:
            break;
    }
}

void Bluetooth::handleCharacteristicChange(QLowEnergyCharacteristic, QByteArray data)
{
    this->logMessage(QString("Received data: '%1'").arg(QString::fromUtf8(data)));
    this->dataBuffer.append(data);
    emit dataAvailable();
}

void Bluetooth::handleDescriptorWrite(QLowEnergyDescriptor, QByteArray data)
{
    //Do nothing
    this->logMessage(QString("Descriptor written with data: %1").arg(QString::fromUtf8(data)));
}

void Bluetooth::handleError(QLowEnergyController::Error error)
{
    qDebug() << "err";
    QString errorText =
            "Error received (Code: %1)"
            " %2";
    errorText = errorText.arg(error).arg(this->m_control->errorString());
    this->logMessage(errorText);
}

void Bluetooth::handleServiceError(QLowEnergyService::ServiceError error)
{
    QString errorText = "Error received (Code: %1)";
    errorText = errorText.arg(error);
    this->logMessage(errorText);
}
