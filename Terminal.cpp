#include "Terminal.h"

#include <QDebug>

Terminal::Terminal(QWidget *parent) : QTextEdit(parent)
{
    this->setStyleSheet("background-color: black; color: white;");
}

//TODO: Implement text coloring
void Terminal::addText(QString text, bool incoming)
{
    asciiText.append(text);
    this->updateTerminal(asciiText);
    emit textAdded(text);
}

//TODO: Implement text coloring
void Terminal::addText(QByteArray text, bool incoming)
{
    asciiText.append(text);
    this->updateTerminal(asciiText);
    emit textAdded(text);
}

QString Terminal::getText()
{
    return this->toPlainText();
}

QString Terminal::getFormattedText()
{
    return this->toHtml();
}

void Terminal::enableEcho(bool enable)
{
    this->echoEnabled = enable;
}

void Terminal::setDisplayMode(Terminal::DisplayMode mode)
{
    this->displayMode = mode;
    this->updateTerminal(this->asciiText);
}

void Terminal::keyPressEvent(QKeyEvent *e)
{
    QString key = e->text();

    char c = 0;

    //Handle special keys
    //All other keys transmit as is
    if(e->key() == Qt::Key_Backspace){
        //TODO: Fix backspace not working
        //c = '\b';
    }
    else if( !key.isEmpty() ){ //Key could be a modifier (i.e. shift). Do not send this.

        c = key.at(0).toLatin1();
    }

    //If converted correctly send the character
    if(c){
        emit textEnterred(c);

        if(echoEnabled){
            addText(key, false);
        }
    }
}

void Terminal::updateTerminal(QString text)
{
    if(this->displayMode == ASCII){
        this->setText(text);
    }
    else{
        this->setText(asciiTextToHex(text));
    }
}

QString Terminal::asciiTextToHex(QString text)
{
    const static std::string asciiLookup = "0123456789ABCDEF";
    //Replace each character with its hex equivalent and a space
    QString hex;

    for(QChar qc : text){
        char c = qc.toLatin1();
        char lower, higher;
        lower = asciiLookup[c & 0x0F];
        higher = asciiLookup[(c & 0xF0) >> 4];
        hex += higher;
        hex += lower;
        hex += " ";
    }

    return hex;
}
