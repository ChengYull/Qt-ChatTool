#ifndef CHATPRO_H
#define CHATPRO_H

#include <QObject>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QList>
enum MessageType{
    SYSTEM_MESSAGE,
    USER_MESSAGE,
    ASSISTANT_MESSAGE,
    TOOL_MESSAGE
};

class ChatPro : public QObject
{
    Q_OBJECT
public:
    // 单例模式
    static ChatPro *Get(){
        static ChatPro cp;
        return &cp;
    }
    // 构造请求头
    QNetworkRequest buildRequestHeader();
    // 构造请求体
    QByteArray buildRequestBody(const QJsonArray &messages, const QJsonArray &tools);
    // 数据流转Json对象
    QJsonObject qByteArrayToQJsonObject(const QByteArray &data);
    // QString转Json对象
    QJsonObject qStringToQJsonObject(const QString &qStr);
    // Json对象转数据流
    QByteArray qJsonObjectToQByteArray(const QJsonObject &obj);
    // 响应
    QNetworkReply* getReply(const QJsonArray &messages, const QJsonArray &tools);
    // 构建信息（系统、用户、助手、工具）
    QJsonObject buildMessage(const QString &message, MessageType type,
                                 const QJsonArray &toolCalls = QJsonArray(),
                                 const QString &name = "",
                                 const QString &id = "");
    // 解析流式输出
    QJsonArray parseChunkResponse(const QByteArray &resp);
    // 获取信息
    QJsonObject getMessage(const QJsonObject &chunk);
    // 获取内容
    QString getContent(const QJsonObject &message);
    // 从多条不完整的ToolCalls字段中解析出完整的字段
    QJsonArray parsetoolCallPieces(const QList<QJsonArray> &toolCalls);

    void ConnectReply(const QJsonArray &messages, const QJsonArray &tools);
private:
    // 网络管理器
    QNetworkAccessManager *manager;
    QString m_url = "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions";
    QString m_api_key = "sk-a90528958d5d4abb8621ef0886f85f7f1";
    QString m_model = "qwen-max";
    QList<QJsonArray> m_toolCallPieces;
    bool m_isFunction = false;
protected:
    ChatPro();
    ~ChatPro();
signals:

    void receiveContent(QString);
    void respEnd(QJsonArray, bool);
};

#endif // CHATPRO_H
