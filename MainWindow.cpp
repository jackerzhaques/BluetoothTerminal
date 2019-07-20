#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>

//TODO: Migrate bluetooth into the "Bluetooth" class.
//I left off at populating a list of bluetooth devices on the UI.
//The next thing I need to do is implement connecting to a bluetooth device by name.

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //this->startDeviceDiscovery();

    /*
     * Bluetooth instance
     *
     * This is an instance of the bluetooth class for
     * communication between the UART friend and this program
     */
    this->bluetooth = new Bluetooth(this);

    connect(bluetooth, SIGNAL(deviceListAvailable()), this, SLOT(refreshDeviceList()));
    connect(bluetooth, SIGNAL(deviceConnected()), this, SLOT(handleBluetoothConnect()));
    connect(bluetooth, SIGNAL(deviceDisconnected()), this, SLOT(handleBluetoothDisconnect()));
    connect(bluetooth, SIGNAL(deviceTransmitReady()), this, SLOT(handleTransmitReady()));
    connect(bluetooth, SIGNAL(dataAvailable()), this, SLOT(collectData()));

    this->bluetooth->refreshDeviceList();


    /*
     * Timeout Timer
     *
     * This timer is used to timeout device connectinos that take too long
     */
    timeoutTimer = new QTimer(this);
    connect(timeoutTimer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));

    /*
     * Terminal
     *
     * The terminal is the widget responsible for displaying text and handling
     * data enterred by the user.
     */
    connect(ui->terminal, SIGNAL(textEnterred(char)), this, SLOT(sendUserInput(char)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startConnectTimeoutTimer()
{
    if(!timeoutTimer){
        timeoutTimer = new QTimer(this);
        connect(timeoutTimer, SIGNAL(timeout()), this, SLOT(connectionTimeout()));
    }
    timeoutTimer->start(5000);
}

void MainWindow::refreshDeviceList()
{
    QStringList devices = this->bluetooth->getDeviceList();
    ui->BluetoothDevicesBox->clear();
    ui->BluetoothDevicesBox->addItems(devices);
}

void MainWindow::handleBluetoothConnect()
{
    this->timeoutTimer->stop();
    QString txt = "Connected to " + bluetooth->getDeviceName();
    ui->StatusLabel->setText(txt);
    ui->ConnectButton->setText("Disconnect");
    this->state = CONNECTED;
}

void MainWindow::handleBluetoothDisconnect()
{
    QString txt = "Disconnected from " + bluetooth->getDeviceName();
    ui->StatusLabel->setText(txt);
    ui->ConnectButton->setText("Connect");
    this->state = DISCONNECTED;
}

void MainWindow::handleTransmitReady()
{
    qDebug() << "ready!";
    this->state = READY;
    bluetooth->write("Death and despair!");
}

void MainWindow::collectData()
{
    ui->terminal->addText(bluetooth->readAll(), true);
}

void MainWindow::sendUserInput(char c)
{
    bluetooth->write(c);
}

void MainWindow::connectionTimeout()
{
    qDebug() << "Conn timeout";
    this->state = DISCONNECTED;
    ui->StatusLabel->setText("Failed to connect to device");
}

void MainWindow::on_RefreshDevicesButton_released()
{
    bluetooth->refreshDeviceList();
}

void MainWindow::on_ConnectButton_released()
{
    if(this->state == DISCONNECTED){
        QString deviceName = ui->BluetoothDevicesBox->currentText();
        if(!deviceName.isEmpty()){
            ui->StatusLabel->setText("Connecting...");
            this->startConnectTimeoutTimer();
            bluetooth->connectToDevice(deviceName);
        }
    }
    else{
        bluetooth->disconnectFromDevice();
    }
}

void MainWindow::on_EchoTerminalCheck_toggled(bool checked)
{
    ui->terminal->enableEcho(checked);
}

void MainWindow::on_DisplayInHexCheck_toggled(bool checked)
{
    if(checked){
        ui->terminal->setDisplayMode(Terminal::HEX);
    }
    else{
        ui->terminal->setDisplayMode(Terminal::ASCII);
    }
}
