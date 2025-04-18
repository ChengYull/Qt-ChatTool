#include "chatpro.h"
#include <QMutex>
QNetworkRequest ChatPro::buildRequestHeader()
{
    // 构建请求头
    QUrl qUrl = QUrl(m_url);
    QNetworkRequest request(qUrl);
    request.setRawHeader("Authorization", ("Bearer " + m_api_key).toLocal8Bit());
    request.setRawHeader("Content-Type", "application/json");
    return request;
}

QByteArray ChatPro::buildRequestBody(const QJsonArray &messages, const QJsonArray &tools)
{
    // 构造请求体
    QJsonObject requestBody;
    requestBody.insert("model", m_model);
    requestBody.insert("messages", messages);
    // 流式输出
    requestBody.insert("stream", true);
    QJsonObject stream_options;
    stream_options.insert("include_usage", true);
    requestBody.insert("stream_options", stream_options);

    // 启用function call支持
    requestBody["tools"] = tools;
    QJsonDocument doc(requestBody);
    qDebug() << "Request:" << doc.toJson(QJsonDocument::Indented);
    return qJsonObjectToQByteArray(requestBody);
}

QJsonObject ChatPro::qByteArrayToQJsonObject(const QByteArray &data)
{
    return QJsonDocument::fromJson(data).object();
}

QJsonObject ChatPro::qStringToQJsonObject(const QString &qStr)
{
    QJsonDocument doc = QJsonDocument::fromJson(qStr.toUtf8());
    if (doc.isNull()) {
        qDebug() << "JSON 解析失败！请检查语法: " << qStr;
        return QJsonObject();
    }
    return doc.object();
}

QByteArray ChatPro::qJsonObjectToQByteArray(const QJsonObject &obj)
{
    return QJsonDocument(obj).toJson();
}

QNetworkReply* ChatPro::getReply(const QJsonArray &messages, const QJsonArray &tools)
{
    QNetworkRequest requestHeader = buildRequestHeader();
    QByteArray requestBody = buildRequestBody(messages, tools);
    return manager->post(requestHeader, requestBody);
}

QJsonObject ChatPro::buildMessage(const QString &message, MessageType type, const QJsonArray &toolCalls,
                                      const QString &name, const QString &id)
{
    QJsonObject jsonMsg;
    switch(type){
    case USER_MESSAGE:
        jsonMsg["role"] = "user";
        break;
    case ASSISTANT_MESSAGE:
        jsonMsg["role"] = "assistant";
        if(!toolCalls.empty())
            jsonMsg["tool_calls"] = toolCalls;
        break;
    case TOOL_MESSAGE:
        jsonMsg["role"] = "tool";
        jsonMsg["name"] = name;
        jsonMsg["tool_call_id"] = id;
        break;
    case SYSTEM_MESSAGE:
        jsonMsg["role"] = "system";
        break;
    default:
        break;
    }
    jsonMsg["content"] = message;
    return jsonMsg;
}

QJsonArray ChatPro::parseChunkResponse(const QByteArray &resp)
{
    QJsonArray msgArr;
    QStringList chunks = QString(resp).split("\n\n");
    for(QString &chunk : chunks){
        if(chunk.isEmpty() || chunk.contains("DONE")) continue;
        QJsonObject message = qStringToQJsonObject(chunk.mid(6));
        msgArr.append(message);
    }
    return msgArr;
}

QJsonObject ChatPro::getMessage(const QJsonObject &chunk)
{
    if(chunk.contains("choices")){
        return chunk["choices"].toArray()[0].toObject()["delta"].toObject();
    }
    return QJsonObject();
}

QString ChatPro::getContent(const QJsonObject &chunk)
{
    QJsonObject msg = getMessage(chunk);
    if(!msg.empty() && msg.contains("content")){
        return msg["content"].toString();
    }
    return "";
}
// 定义结构体存储每个工具调用的中间状态
struct ToolCallState {
    QString arguments;    // 正在拼接的arguments字符串
    QJsonObject metadata; // 存储id/type等固定字段
    bool isCompleted = false; // 标记是否完成
};
QJsonArray ChatPro::parsetoolCallPieces(const QList<QJsonArray> &toolCallsChunks)
{
    qDebug() << "分片个数：" << toolCallsChunks.size();
    // 存储每个index对应的工具调用状态
    QMap<int, ToolCallState> toolCallMap;
    // 遍历所有分块数据
    for (const QJsonArray &chunk : toolCallsChunks) {
        // 遍历当前分块中的每个工具调用
        for (const QJsonValue &toolCallVal : chunk) {
            QJsonObject toolCall = toolCallVal.toObject();
            int index = toolCall["index"].toInt(0); // (如果没获取到)默认为0

            // 处理arguments分块
            QJsonValue argsVal = toolCall["function"].toObject()["arguments"];
            // 直接跳过args为null的内容，因为第一条有可能是args为null的无效数据
            if(argsVal.isNull()){
                //qDebug() << toolCall;
                continue;
            }

            // 初始化或获取现有状态
            ToolCallState &state = toolCallMap[index];

            // 如果是首次遇到该index，保存metadata
            if (state.metadata.isEmpty()) {
                QJsonObject funcObj = toolCall["function"].toObject();
                state.metadata["id"] = toolCall["id"];
                state.metadata["type"] = toolCall["type"];
                state.metadata["function"] = QJsonObject{
                    {"name", funcObj["name"]}
                };
            }
            if (argsVal.isString())
                // 拼接字符串内容
                state.arguments += argsVal.toString();
            qDebug() << state.arguments;
            if (state.arguments.endsWith("}") && !QJsonDocument::fromJson(state.arguments.toUtf8()).isNull()) {
                state.isCompleted = true;
            }
        }
    }

    // 构建最终结果
    QJsonArray result;
    QMapIterator<int, ToolCallState> it(toolCallMap);
    while (it.hasNext()) {
        it.next();
        const ToolCallState &state = it.value();

        // 只处理已完成的调用
        if (state.isCompleted) {
            QJsonObject funcObj = state.metadata["function"].toObject();

            funcObj["arguments"] = state.arguments;
            qDebug() << "合并结果：" << state.arguments;

            // 构建完整工具调用对象
            QJsonObject toolCall;
            toolCall["id"] = state.metadata["id"];
            toolCall["type"] = state.metadata["type"];
            toolCall["function"] = funcObj;
            toolCall["index"] = it.key();

            result.append(toolCall);
        }
    }
    return result;
}

void ChatPro::ConnectReply(const QJsonArray &messages, const QJsonArray &tools)
{
    QNetworkReply* reply = ChatPro::Get()->getReply(messages, tools);
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        if (reply->error()) {
            qDebug() << "Error:" << reply->errorString();
            return;
        }
        QByteArray resp = reply->readAll();
        QJsonArray respArr = ChatPro::Get()->parseChunkResponse(resp);
        //qDebug() << "Response: " << respArr;
        //QScrollBar *scrollBar = ui->textEdit_record->verticalScrollBar();
        //qDebug() << "一次：";
        for(auto chunk : respArr){
            QJsonObject delta = ChatPro::Get()->getMessage(chunk.toObject());
            if(delta.empty()) continue;
            qDebug() << delta;
            if(delta.contains("tool_calls")){
                m_isFunction = true;
                m_toolCallPieces.append(delta["tool_calls"].toArray());
            }else{
                //qDebug() << "发送信号！============";
                emit receiveContent(ChatPro::Get()->getContent(chunk.toObject()));
            }
        }
    }, Qt::QueuedConnection);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error()) {
            qDebug() << "Error:" << reply->errorString();
            return;
        }
        reply->deleteLater();
        QJsonArray toolCalls = ChatPro::Get()->parsetoolCallPieces(m_toolCallPieces);
        emit respEnd(toolCalls, m_isFunction);

        // 清空调用函数的响应缓存
        m_toolCallPieces.clear();
        m_isFunction = false;
    });
}

ChatPro::ChatPro() {
    manager = new QNetworkAccessManager(this);
    m_messages.append(buildMessage("你是一个非常活泼的AI，回复要积极主动", MessageType::SYSTEM_MESSAGE));
}

ChatPro::~ChatPro() {}
