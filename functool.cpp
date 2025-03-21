#include "functool.h"
#include <QDateTime>
void FuncTool::newTool(const QString &name, const QString &description, const QJsonObject &params)
{
    QJsonObject function;
    function["name"] = name;
    function["description"] = description;
    function["parameters"] = params;
    m_toolTamplate["function"] = function;
    m_tools.append(m_toolTamplate);
}

QString FuncTool::getWeather(const QJsonObject &arguments)
{
    QString city = arguments["city"].toString();
    QJsonObject result;
    if("苏州" == city){
        result["weather"] = "晴天";
        result["temperature"] = "11℃";
    }else if("杭州" == city){
        result["weather"] = "晴天";
        result["temperature"] = "15℃";
    }else if("北京" == city){
        result["weather"] = "阴天";
        result["temperature"] = "9℃";
    }else{
        result = QJsonObject();
    }
    return QString(QJsonDocument(result).toJson(QJsonDocument::Indented));;
}

QString FuncTool::getTime(const QJsonObject &arguments)
{
    // 获取当前的日期和时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString info = "Current Date and Time:" + currentDateTime.toString();
    return info;
}

QString FuncTool::executeFunction(const QString &name, const QJsonObject &arguments)
{
    if(name == "get_weather")
        return getWeather(arguments);
    else if(name == "get_time")
        return getTime(arguments);
    else
        return "";
}

FuncTool::FuncTool() {
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
    m_toolTamplate = ChatPro::Get()->qStringToQJsonObject(tamplate);
    m_tools.append(m_toolTamplate);

    QString timeParam = R"(
    {
        "type": "",
        "properties": {},
        "required": []
    })";
    QJsonObject timeParams = ChatPro::Get()->qStringToQJsonObject(timeParam);
    newTool("get_time", "用户需要获取时间时使用", timeParams);

}
