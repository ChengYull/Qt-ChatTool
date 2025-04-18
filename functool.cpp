#include "functool.h"
#include <QDateTime>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include "widgetUI.h">
void FuncTool::newTool(const QString &name, const QString &description, const QJsonObject &params)
{
    QJsonObject function;
    function["name"] = name;
    function["description"] = description;
    function["parameters"] = params;
    m_toolTamplate["function"] = function;
    m_tools.append(m_toolTamplate);
}

// 获取天气
QString FuncTool::getWeather(const QJsonObject &arguments)
{
    QString city = arguments["city"].toString();
    QJsonObject result;
    if("苏州" == city){
        result["weather"] = "晴天";
        result["temperature"] = "19℃";
    }else if("杭州" == city){
        result["weather"] = "晴天";
        result["temperature"] = "21℃";
    }else if("北京" == city){
        result["weather"] = "阴天";
        result["temperature"] = "18℃";
    }else{
        result = QJsonObject();
    }
    return QString(QJsonDocument(result).toJson(QJsonDocument::Indented));;
}

// 获取时间
QString FuncTool::getTime(const QJsonObject &arguments)
{
    // 获取当前的日期和时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString info = "Current Date and Time:" + currentDateTime.toString();
    return info;
}

// 保存记录
QString FuncTool::saveMessages(const QJsonObject &arguments)
{
    QString result = "";
    QString exeDir = QCoreApplication::applicationDirPath();
    QString jsonPath = exeDir + "/messages.json";
    QJsonArray messageArr = ChatPro::Get()->m_messages;
    messageArr.removeLast();
    messageArr.removeLast();
    QJsonObject messages;
    messages["messages"] = messageArr;
    if(messages["messages"].toArray().count() <= 1){
        result = "我们刚开始对话，没有对话记录";
        return result;
    }
    QByteArray jsonData = ChatPro::Get()->qJsonObjectToQByteArray(messages);
    QFile file(jsonPath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        result = "文件打开失败:" + jsonPath;
        return result;
    }
    // 写入数据
    file.write(jsonData);
    file.close();
    result = "成功保存记录 路径：" + jsonPath;
    return result;
}
// 读取记录
QString FuncTool::readMessages()
{
    QString result = "";
    QString messagesPath = QCoreApplication::applicationDirPath() + "/messages.json";
    if(!QFile::exists(messagesPath)){
        result = "当前还没有记录哦";
        return result;
    }
    QFile file(messagesPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        result = "找到了记录文件:" + messagesPath + "但是好像打不开";
        return result;
    }
    QByteArray messages = file.readAll();
    QJsonObject jsMessages = ChatPro::Get()->qByteArrayToQJsonObject(messages);
    if(jsMessages.isEmpty()){
        result = "记录文件为空";
        return result;
    }
    file.close();
    QJsonObject lastMsg = ChatPro::Get()->m_messages.last().toObject();
    ChatPro::Get()->m_messages = jsMessages["messages"].toArray();
    ChatPro::Get()->m_messages.append(lastMsg);

    result = "成功加载了记录";
    return result;
}

QString FuncTool::executeFunction(const QString &name, const QJsonObject &arguments)
{
    qDebug() << "调用工具" << name;
    if(name == "get_weather")
        return getWeather(arguments);
    else if(name == "get_time")
        return getTime(arguments);
    else if(name == "save_messages")
        return saveMessages(arguments);
    else if(name == "read_messages")
        return readMessages();
    else
        return "函数请求失败";
}

FuncTool::FuncTool() {
    // 参数设置
    QString param = R"(
    {
        "type": "",
        "properties": {},
        "required": []
    })";
    // 无参数
    QString none_param = R"(
    {
        "type": "",
        "properties": {},
        "required": []
    })";
    m_noneParam = ChatPro::Get()->qStringToQJsonObject(none_param);

    QString tamplate = R"(
    {
        "type": "function",
        "function": {
            "description": "获取城市的天气信息",
            "name": "get_weather",
            "parameters": {
                "type": "object",
                "properties": {
                    "city": {
                        "type": "string",
                        "enum": ["杭州", "北京", "苏州"],
                        "description": "城市名"
                    }
                },
                "required": ["city"]
            }
        }
    })";
    // 获取天气
    m_toolTamplate = ChatPro::Get()->qStringToQJsonObject(tamplate);
    m_tools.append(m_toolTamplate);

    // 获取时间
    newTool("get_time", "用户需要获取时间时使用", m_noneParam);

    // 保存记录
    newTool("save_messages", "当用户需要保存记录时调用", m_noneParam);

    // 读取记录
    newTool("read_messages", "当用户需要读取记录时调用", m_noneParam);

}
