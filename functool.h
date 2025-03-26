#ifndef FUNCTOOL_H
#define FUNCTOOL_H
#include "chatpro.h"
class FuncTool : public QObject
{
        Q_OBJECT
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
    QString saveMessages(const QJsonObject &arguments);
    QString readMessages();
    QString executeFunction(const QString &name,  const QJsonObject &arguments);
private:
    QJsonArray m_tools;
    QJsonObject m_toolTamplate;
    QJsonObject m_noneParam;
protected:
    FuncTool();

signals:
    void loadMessages(QJsonObject);
};

#endif // FUNCTOOL_H
