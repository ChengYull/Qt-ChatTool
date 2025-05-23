#include "widgetUI.h"
#include "ui_widgetUI.h"
#include <QScrollBar>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_tools = FuncTool::Get()->Tools();

    // 初始化定时器（避免在槽函数中更新）
    m_updateTimer.setInterval(100); // 100ms 合并一次更新
    connect(&m_updateTimer, &QTimer::timeout, this, [this]() {
        if (!m_pendingMsg.isEmpty()) {
            // 流式传输显示
            ui->textEdit_record->clear();
            ui->textEdit_record->setText(m_record + m_pendingMsg);
            QScrollBar *scrollBar = ui->textEdit_record->verticalScrollBar();
            scrollBar->setValue(scrollBar->maximum());
            m_pendingMsg.clear();
        }
    });
    m_updateTimer.start();


    // 连接收到信息槽函数
    // 收到普通信息
    connect(ChatPro::Get(), &ChatPro::receiveContent, this, &Widget::receiveMsg);
    // 请求结束
    connect(ChatPro::Get(), &ChatPro::respEnd, this, &Widget::requestEnd);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{

    QString input = ui->textEdit_input->toPlainText();
    ChatPro::Get()->m_messages.append(ChatPro::Get()->buildMessage(input, MessageType::USER_MESSAGE));

    ui->textEdit_input->clear();
    ui->textEdit_record->append("user: " + input);
    // 清空完整消息
    m_wholeMsg.clear();
    ui->textEdit_record->append("AI:");
    m_record = ui->textEdit_record->toPlainText();

    ChatPro::Get()->ConnectReply(ChatPro::Get()->m_messages, m_tools);

}

void Widget::receiveMsg(QString msg)
{
    qDebug() << "接收消息：" << msg;
    m_wholeMsg.append(msg);
    m_pendingMsg = m_wholeMsg;
}

void Widget::requestEnd(QJsonArray toolCalls, bool isFunctionCall)
{
    // 如果是函数调用
    if(isFunctionCall){
        // 如果没有函数名直接返回
        if(toolCalls.isEmpty() || toolCalls[0].toObject()["function"].toObject()["name"].isNull())
            return;
        ChatPro::Get()->m_messages.append(ChatPro::Get()->buildMessage("", MessageType::ASSISTANT_MESSAGE, toolCalls));
        foreach(const QJsonValue &tool, toolCalls){
            QJsonObject jsTool = tool.toObject();
            QString id = jsTool["id"].toString();
            QString name = jsTool["function"].toObject()["name"].toString();
            QJsonObject args = ChatPro::Get()->qStringToQJsonObject(jsTool["function"].toObject()["arguments"].toString());
            QString result = FuncTool::Get()->executeFunction(name, args);
            ChatPro::Get()->m_messages.append(ChatPro::Get()->buildMessage(result, MessageType::TOOL_MESSAGE, QJsonArray(), name, id));
        }
        // 递归调用
        ChatPro::Get()->ConnectReply(ChatPro::Get()->m_messages, m_tools);
    }else{
        // 普通消息
        ChatPro::Get()->m_messages.append(ChatPro::Get()->buildMessage(m_wholeMsg, MessageType::ASSISTANT_MESSAGE));
    }
}


