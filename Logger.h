#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    void setLogFile(QString path);
    QString getLogFilePath();

    bool isLogging();

    void logInHex(bool enabled);
    bool isHexLoggingEnabled();

    void promptWhenOverwriting(bool enabled);

signals:
    void loggingStarted();
    void loggingStopped();

public slots:
    void startLogging();
    void stopLogging();
    void log(QString text);

private:
    bool logging                        = false;
    bool logInHexEnabled                = false;
    bool promptWhenOverwritingEnabled   = false;
    QString logFilePath                 = "C:\\BluetoothLogs\\log.txt";   //Default log file path
    QFile *logFile                      = nullptr;
    QString logText;    //Log file buffer

    bool promptOverwrite();
    void writeToFile(QString text);
    QStringList convertTextToHex(QString text);
    void addHexToBuffer(QStringList hex);

    //Preserved states.
    //This lets the user change states without interrupting the current logging process
    bool preserved_logInHexEnabled          = false;
    void preserveStates();
};

#endif // LOGGER_H
