#ifndef CMUSICFILE_H
#define CMUSICFILE_H

#include <QObject>

#include <QtXml>

class CMusicFile : public QObject
{
    Q_OBJECT
public:
    explicit CMusicFile(QUrl i_url, QObject *parent = nullptr);
    explicit CMusicFile(QDomElement i_element, QObject *parent = nullptr);

    QString getName(){ return name;}
    QUrl getUrl(){return url;}

    void fillElement(QDomElement *ip_element);

signals:

public slots:

private:
    QString name;
    QUrl url;
};

#endif // CMUSICFILE_H
