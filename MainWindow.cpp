#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Bluetooth Terminal");

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

    /*
     *
     * The logger class handles logging data to a file.
     *
     * This class allows logging data as text, hex, splitting log files (TX and RX files), and more.
     */
    logger = new Logger(this);
    connect(ui->terminal, SIGNAL(textAdded(QString)), logger, SLOT(log(QString)));

    //Apply default settings here
    ui->LogPathInput->setText(logger->getLogFilePath());
    ui->OvevrwritePromptCheck->setChecked(true);
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
    if(bluetooth){
        QStringList devices = this->bluetooth->getDeviceList();
        ui->BluetoothDevicesBox->clear();
        ui->BluetoothDevicesBox->addItems(devices);
    }
}

void MainWindow::handleBluetoothConnect()
{
    if(bluetooth){
        this->timeoutTimer->stop();
        QString txt = "Connected to " + bluetooth->getDeviceName();
        ui->StatusLabel->setText(txt);
        ui->ConnectButton->setText("Disconnect");
        this->state = CONNECTED;
    }
}

void MainWindow::handleBluetoothDisconnect()
{
    if(bluetooth){
        QString txt = "Disconnected from " + bluetooth->getDeviceName();
        ui->StatusLabel->setText(txt);
        ui->ConnectButton->setText("Connect");
        this->state = DISCONNECTED;
    }
}

void MainWindow::handleTransmitReady()
{
    if(bluetooth){
        qDebug() << "ready!";
        this->state = READY;
        bluetooth->write("Death and despair!");
    }
}

void MainWindow::collectData()
{
    if(bluetooth){
        ui->terminal->addText(bluetooth->readAll(), true);
    }
}

void MainWindow::sendUserInput(char c)
{
    if(bluetooth){
        bluetooth->write(c);
    }
}

void MainWindow::connectionTimeout()
{
    qDebug() << "Conn timeout";
    this->state = DISCONNECTED;
    ui->StatusLabel->setText("Failed to connect to device");
}

void MainWindow::on_RefreshDevicesButton_released()
{
    if(bluetooth){
        bluetooth->refreshDeviceList();
    }
}

void MainWindow::on_ConnectButton_released()
{
    if(bluetooth){
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

void MainWindow::on_BrowseButton_released()
{
    QString path = QFileDialog::getSaveFileName(this, "Log File",
                                                logger->getLogFilePath(),
                                                "Text Files (*.txt);;CSV Files (*.csv);;All Files (*.*)");

    logger->setLogFile(path);
}

void MainWindow::on_StartStopLoggingButton_released()
{
    if(logger->isLogging()){
        logger->stopLogging();
        ui->StartStopLoggingButton->setText("Start Logging");
    }
    else{
        logger->startLogging();
        if(logger->isLogging()){
            ui->StartStopLoggingButton->setText("Stop Logging");
        }
    }
}

void MainWindow::on_LogRawDataCheck_toggled(bool checked)
{
    logger->logInHex(checked);
}

void MainWindow::on_actionCapture_Terminal_triggered()
{
    //Capture the terminal by writing all of the terminal's contents to the logger
    if(!logger->isLogging()){
        logger->startLogging();
        if(logger->isLogging()){
            logger->log(ui->terminal->getText());
            logger->stopLogging();
        }
    }
}

void MainWindow::on_actionStart_Logging_triggered()
{
    logger->startLogging();
}

void MainWindow::on_actionStop_Logging_triggered()
{
    logger->stopLogging();
}

void MainWindow::on_OvevrwritePromptCheck_toggled(bool checked)
{
    logger->promptWhenOverwriting(checked);
}
