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
     * @param i_time: in 1/10éme de seconde
     * @return état des lumières au temps demandé
     */
    quint16 getLightStatusAt(quint64 i_time);

    /**
     * @brief addEvent
     * @param i_time: in 1/10éme de seconde
     * @param i_status: état des lumières
     */
    bool addEvent(quint64 i_time, quint16 i_status);

    bool removeEvent(QString i_event);

    QStringList getEventList(void);

    void fillElement(QDomElement *ip_element, QDomDocument *dom_doc);

signals:

public slots:

private:
    QString name;
    QUrl url;
    quint64 duration;
    bool light_enable;
    quint16 *light_status;
    quint64 light_status_size;
    QMap<quint64, quint16> light_event_map;


    void buildLightConfiguration(void);
    QString mapToString(quint64 i_time, quint16 i_config);
    QString timeToString(quint64 i_time);
    quint64 timeToValue(QString i_time);
    QString configToString(quint16 i_config);
    quint16 configToValue(QString i_config);
};

#endif // CMUSICFILE_H
