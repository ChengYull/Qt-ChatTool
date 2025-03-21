#ifndef WIDGETUI_H
#define WIDGETUI_H

#include <QWidget>
#include "functool.h"
#include <QTimer>
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QJsonArray m_messages;
    QString m_wholeMsg = "";
    QString m_record = "";
    QJsonArray m_tools;
    QString m_pendingMsg;
    QTimer m_updateTimer;
private slots:
    void on_pushButton_clicked();

    void receiveMsg(QString msg);
    void requestEnd(QJsonArray toolCalls, bool isFunctionCall);

private:
    Ui::Widget *ui;
};
#endif // WIDGETUI_H
