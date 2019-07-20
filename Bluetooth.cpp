#include "Bluetooth.h"

Bluetooth::Bluetooth(QObject *parent) : QObject(parent)
{

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

    qDebug() << "Device set to " << this->device.name();
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
    if(m_control){
        //Service discovered.
        QBluetoothUuid uuid = QBluetoothUuid(UART_TX_UUID);
        QLowEnergyCharacteristic hrChar = service->characteristic(uuid);
        if(hrChar.isValid()){
            service->writeCharacteristic(hrChar, data);
        }
        else{
            qDebug() << "HR Data not found.";
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
    m_control = QLowEnergyController::createCentral(device, this);
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
    delete m_control;
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
            connect(service, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(handleServiceStateChange(QLowEnergyService::ServiceState)));
            connect(service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this, SLOT(handleCharacteristicChange(QLowEnergyCharacteristic, QByteArray)));
            connect(service, SIGNAL(descriptorWritten(QLowEnergyDescriptor, QByteArray)), this, SLOT(handleDescriptorWrite(QLowEnergyDescriptor, QByteArray)));
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
    m_control->discoverServices();
    emit deviceConnected();
}

void Bluetooth::handleDeviceDisconnection()
{
    emit deviceDisconnected();
}

void Bluetooth::handleServiceStateChange(QLowEnergyService::ServiceState state)
{
    switch(state){
        case QLowEnergyService::DiscoveringServices:
            //Discovering services, do nothing
            break;
        case QLowEnergyService::ServiceDiscovered:
        //Scope needed for inner variables
        {
            //Service discovered.
            QBluetoothUuid uuid = QBluetoothUuid(UART_TX_UUID);
            QLowEnergyCharacteristic hrChar = service->characteristic(uuid);
            if(hrChar.isValid()){
                service->writeCharacteristic(hrChar, "Slipknot!!!!");
                emit deviceTransmitReady();
            }
            else{
                qDebug() << "HR Data not found.";
            }
            break;
        }
        default:
            break;
    }
}

void Bluetooth::handleCharacteristicChange(QLowEnergyCharacteristic characteristic, QByteArray data)
{
   this->dataBuffer.append(data);
   emit dataAvailable();
}

void Bluetooth::handleDescriptorWrite(QLowEnergyDescriptor descriptor, QByteArray data)
{
    //Do nothing
}
