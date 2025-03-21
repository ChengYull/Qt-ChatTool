#ifndef FUNCTOOL_H
#define FUNCTOOL_H
#include "chatpro.h"
class FuncTool
{
public:

    static FuncTool* Get(){
        static FuncTool ft;
        return &ft;
    }
    QJsonArray Tools(){
        return m_tools;
    }
    void newTool(const QString &name, const QString &description, const QJsonObject &params);

    QString getWeather(const QJsonObject &arguments);
    QString getTime(const QJsonObject &arguments = QJsonObject());
    QString executeFunction(const QString &name,  const QJsonObject &arguments);
private:
    QJsonArray m_tools;
    QJsonObject m_toolTamplate;
protected:
    FuncTool();
};

#endif // FUNCTOOL_H
