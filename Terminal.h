#ifndef TERMINAL_H
#define TERMINAL_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>
#include <QKeyEvent>

class Terminal : public QTextEdit
{
    Q_OBJECT
public:
    Terminal(QWidget *parent);

    enum DisplayMode{
        ASCII,
        HEX,
    };

    void addText(QString text, bool incoming);
    void addText(QByteArray text, bool incoming);

    QString getText();
    QString getFormattedText();

    void enableEcho(bool enable);
    void setDisplayMode(DisplayMode mode);

signals:
    void textEnterred(char c);

private:
    void keyPressEvent(QKeyEvent *e) override;

    QString asciiText;
    DisplayMode displayMode = ASCII;
    bool echoEnabled = false;

    void updateTerminal(QString text);
    QString asciiTextToHex(QString text);
};

#endif // TERMINAL_H
