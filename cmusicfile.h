#ifndef CMUSICFILE_H
#define CMUSICFILE_H

#include <QObject>

#include <QtXml>

class CMusicFile : public QObject
{
    Q_OBJECT
public:
    explicit CMusicFile(QUrl i_url, bool i_light_enable=false, QObject *parent = nullptr);
    explicit CMusicFile(QDomElement i_element, QObject *parent = nullptr);

    QString getName(){ return name;}
    QUrl getUrl(){return url;}
    bool isLightEnable(){return light_enable;}
    void setLightEnable(bool i_enable){light_enable=i_enable;}

    void fillElement(QDomElement *ip_element);

signals:

public slots:

private:
    QString name;
    QUrl url;
    bool light_enable;
};

#endif // CMUSICFILE_H
