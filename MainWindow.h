#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothSocket>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyDescriptor>
#include <QBluetoothUuid>
#include <QTimer>

#include "Bluetooth.h"
#include "Terminal.h"
#include "Logger.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    enum State{
        DISCONNECTED,
        CONNECTED,
        READY,
    };

    State state             = DISCONNECTED;
    QTimer *timeoutTimer    = nullptr;
    Bluetooth *bluetooth    = nullptr;
    Logger *logger          = nullptr;

    QString terminalData;       //Keeps track of data written to the terminal window

    void startConnectTimeoutTimer();

public slots:
    void refreshDeviceList();
    void handleBluetoothConnect();
    void handleBluetoothDisconnect();
    void handleTransmitReady();
    void collectData();
    void sendUserInput(char c);

private slots:
    void connectionTimeout();
    void on_RefreshDevicesButton_released();
    void on_ConnectButton_released();
    void on_EchoTerminalCheck_toggled(bool checked);
    void on_DisplayInHexCheck_toggled(bool checked);
    void on_BrowseButton_released();
    void on_StartStopLoggingButton_released();
    void on_LogRawDataCheck_toggled(bool checked);
    void on_actionCapture_Terminal_triggered();
    void on_actionStart_Logging_triggered();
    void on_actionStop_Logging_triggered();
    void on_OvevrwritePromptCheck_toggled(bool checked);
};

#endif // MAINWINDOW_H
