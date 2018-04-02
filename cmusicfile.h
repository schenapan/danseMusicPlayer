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
    ~CMusicFile();

    QString getName(){ return name;}
    QUrl getUrl(){return url;}
    bool setDuration(quint64 i_duration);
    bool isLightEnable(){return light_enable;}
    void setLightEnable(bool i_enable);

    /**
     * @brief getLightStatusAt
     * @param i_time: in 1/100éme de seconde
     * @return état des lumières au temps demandé
     */
    quint16 getLightStatusAt(quint64 i_time);

    /**
     * @brief setLightStatusAt
     * @param i_time: in 1/100éme de seconde
     * @param i_status: état des lumières
     */
    bool setLightStatusAt(quint64 i_time, quint16 i_status);


    void fillElement(QDomElement *ip_element);

signals:

public slots:

private:
    QString name;
    QUrl url;
    quint64 duration;
    bool light_enable;
    quint16 *light_status;
    quint64 light_status_size;

    void clearLightStatus(void);
};

#endif // CMUSICFILE_H
