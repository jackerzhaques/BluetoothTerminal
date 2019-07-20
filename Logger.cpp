#include "Logger.h"

#include <QDebug>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>

Logger::Logger(QObject *parent) : QObject(parent)
{
    this->logFile = new QFile();
}

Logger::~Logger()
{
    if(logFile->isOpen()){
        qDebug() << "Logger: Closing logger forcefully...";
        this->stopLogging();
    }

    delete this->logFile;
}

void Logger::setLogFile(QString path)
{
    this->logFilePath = path;
}

QString Logger::getLogFilePath()
{
    return this->logFilePath;
}

bool Logger::isLogging()
{
    return this->logging;
}

void Logger::log(QString text)
{
    if(this->logFile->isOpen()){
        if(this->preserved_logInHexEnabled){
            QStringList hexVals = convertTextToHex(text);
            addHexToBuffer(hexVals);
        }
        else{
            this->logText.append(text);
        }

        writeToFile(this->logText);
    }
}

bool Logger::promptOverwrite()
{
    bool overwriteFile = false;

    QString promptString = QString("You are attempting to overwrite a log file (%1).\n"
                                   "Do you wish to continue?").arg(logFilePath);
    int ret = QMessageBox::warning(nullptr,
                                   "Overwrite Confirmation",
                                   promptString,
                                   QMessageBox::Ok | QMessageBox::Cancel);

    switch(ret){
        case QMessageBox::Ok:
            overwriteFile = true;
            break;
        default:
            overwriteFile = false;
            break;
    }

    return overwriteFile;
}

void Logger::writeToFile(QString text)
{
    if(logFile->isOpen()){
        logFile->resize(0);
        logFile->write(text.toUtf8());
        logFile->flush();
    }
}

QStringList Logger::convertTextToHex(QString text)
{
    const static std::string asciiLookup = "0123456789ABCDEF";
    QStringList hexList;

    for(QChar qc : text){
        QString hex;
        char c = qc.toLatin1();

        //Preserve newlines (don't convert to hex)
        if(c == '\r' || c == '\n'){
            hex = c;
        }
        else{
            char upper = asciiLookup[(c & 0xF0) >> 4];
            char lower = asciiLookup[c & 0x0F];
            hex.append(upper);
            hex.append(lower);
        }

        hexList.append(hex);

    }

    return hexList;
}

void Logger::addHexToBuffer(QStringList hex)
{
    //Append each hex character to the buffer
    //If the character is a newline, append it as is
    //If the character is not a newline, append it as is, but add a comma before it if there is data on the same line

    for(QString val : hex){
        if(val == "\r" || val == "\n"){
            logText.append(val);
        }
        else{
            if(!logText.endsWith("\r") && !logText.endsWith("\n") && !logText.isEmpty()){
                logText.append(',');
            }

            logText.append(val);
        }
    }
}

void Logger::preserveStates()
{
    this->preserved_logInHexEnabled         = logInHexEnabled;
}

void Logger::logInHex(bool enabled)
{
    this->logInHexEnabled = enabled;
}

bool Logger::isHexLoggingEnabled()
{
    return logInHexEnabled;
}

void Logger::promptWhenOverwriting(bool enabled)
{
    this->promptWhenOverwritingEnabled = enabled;
}

void Logger::startLogging()
{
    //Create the directory if it does not exist
    QDir dir = QFileInfo(logFilePath).absoluteDir();
    if(!dir.exists()){
        qDebug() << "Logger: log file path does not exist. Creating path...";
        dir.mkpath(dir.path());
    }

    //Attempt to open the log file
    logFile->setFileName(this->logFilePath);

    //Prompt for an overwrite if applicable
    bool overwriteFile = false;
    if(promptWhenOverwritingEnabled){
        //Check if file exists
        if(logFile->exists()){
            overwriteFile = this->promptOverwrite();
        }
        else{
            overwriteFile = true;
        }
    }
    else{
        overwriteFile = true;
    }

    if(overwriteFile){
        logFile->open(QIODevice::WriteOnly | QIODevice::Truncate);

        if(logFile->isOpen()){
            qDebug() << "Logger: Logging started.";
            this->logging = true;
            this->logText.clear();
            this->preserveStates();
            emit loggingStarted();
        }
    }
}

void Logger::stopLogging()
{
    if(logFile->isOpen()){
        qDebug() << "Logger: Logging stopped.";
        logFile->close();
        this->logging = false;
    }
}
