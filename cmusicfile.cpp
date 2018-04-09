#include "cmusicfile.h"

#define ATTR_NAME "name"
#define ATTR_URL "url"
#define ATTR_DURATION "duration"
#define ATTR_LIGHT_ENABLE "light_enable"

#define BAL_LIGHT_EVENT "light_event"
#define ATTR_LIGHT_EVENT_TIME "time"
#define ATTR_LIGHT_EVENT_CONFIG "config"



CMusicFile::CMusicFile(QUrl i_url, bool i_light_enable, QObject *parent) : QObject(parent)
{
    name = i_url.fileName();
    url = i_url;
    light_enable = i_light_enable;
    duration = 0;
    light_status = NULL;
    light_status_size = 0;
}

CMusicFile::CMusicFile(QDomElement i_element, QObject *parent) : QObject(parent)
{
    name = i_element.attribute(ATTR_NAME);
    url = i_element.attribute(ATTR_URL);
    duration = i_element.attribute(ATTR_DURATION).toULongLong();
    if(i_element.attribute(ATTR_LIGHT_ENABLE)=="TRUE")
    {
        light_enable = true;

        // fill evant list
        QDomNode noeud = i_element.firstChild();
        light_event_map.clear();
        while(!noeud.isNull())
        {
            QDomElement element = noeud.toElement();

            if(!element.isNull())
            {
                if( element.tagName() == BAL_LIGHT_EVENT )
                {
                    light_event_map.insert(timeToValue(element.attribute(ATTR_LIGHT_EVENT_TIME)),
                                           configToValue(element.attribute(ATTR_LIGHT_EVENT_CONFIG)));
                }
            }
            noeud = noeud.nextSibling();
        }

        // build configuration
        buildLightConfiguration();
    }
    else
    {
        light_enable = false;
    }
}

CMusicFile::~CMusicFile()
{
    if(NULL!=light_status){ delete light_status; }
}

void CMusicFile::setLightEnable(bool i_enable)
{
    light_enable=i_enable;
    buildLightConfiguration();
}

bool CMusicFile::setDuration(quint64 i_duration)
{
    bool o_change = false;

    if( i_duration != duration )
    {
        duration = i_duration;
        buildLightConfiguration();
        o_change = true;
    }

    return o_change;
}

quint16 CMusicFile::getLightStatusAt(quint64 i_time)
{
    quint16 lo_status= 0;

    if( (NULL != light_status) && (i_time<light_status_size) )
    {
        lo_status = light_status[i_time];
    }
    return lo_status;
}

bool CMusicFile::addEvent(quint64 i_time, quint16 i_status)
{
    bool lo_change = false;

    if( i_time<light_status_size )
    {
        light_event_map.insert(i_time,i_status);
        buildLightConfiguration();
        lo_change = true;
    }

    return lo_change;
}

bool CMusicFile::removeEvent(QString i_event)
{
    QStringList l_str_list = i_event.split(" - ");

    light_event_map.remove(timeToValue(l_str_list.at(0)));
    buildLightConfiguration();

    return true;
}

QStringList CMusicFile::getEventList(void)
{
    QStringList o_list;

    QMapIterator<quint64, quint16> i(light_event_map);
    while (i.hasNext()) {
        i.next();
        o_list.append(mapToString(i.key(),i.value()));
    }

    return o_list;
}

void CMusicFile::fillElement(QDomElement *ip_element, QDomDocument *dom_doc)
{
    ip_element->setAttribute(ATTR_NAME, name);
    ip_element->setAttribute(ATTR_URL, url.toString());
    ip_element->setAttribute(ATTR_DURATION, QString::number(duration));
    if(light_enable)
    {
        // check if some order exist
        if( light_event_map.isEmpty() )
        {
            ip_element->setAttribute(ATTR_LIGHT_ENABLE, "FALSE");
        }
        else
        {
            ip_element->setAttribute(ATTR_LIGHT_ENABLE, "TRUE");

            // save config
            QMapIterator<quint64, quint16> i(light_event_map);
            while (i.hasNext()) {
                i.next();
                QDomElement l_elmt = dom_doc->createElement(BAL_LIGHT_EVENT);
                l_elmt.setAttribute(ATTR_LIGHT_EVENT_TIME, timeToString(i.key()));
                l_elmt.setAttribute(ATTR_LIGHT_EVENT_CONFIG, configToString(i.value()));
                ip_element->appendChild(l_elmt);
            }
        }
    }
    else
    {
        ip_element->setAttribute(ATTR_LIGHT_ENABLE, "FALSE");
    }

}



/****************************************************************************
 * PRIVATE
 ***************************************************************************/


void CMusicFile::buildLightConfiguration(void)
{
    light_status_size = 0;
    if(NULL!=light_status){ delete light_status; }
    if( light_enable )
    {
        if( 0 != duration )
        {
            light_status_size = duration/100;
            light_status = new quint16[light_status_size];
            memset(light_status,0,light_status_size);

            quint16 last_value = 0;
            bool l_continue = true;
            QMapIterator<quint64, quint16> i(light_event_map);
            if(i.hasNext())
            {
                i.next();
                for(quint64 loop_time=0; loop_time<light_status_size; loop_time++)
                {
                    if((loop_time>=i.key()) && l_continue)
                    {
                        last_value = i.value();
                        if(i.hasNext())
                        {
                            i.next();
                        }
                        else
                        {
                            l_continue = false;
                        }
                    }
                    light_status[loop_time] = last_value;
                }
            }
       }
    }
}

QString CMusicFile::mapToString(quint64 i_time, quint16 i_config)
{
    QString o_str;

    qint64 seconds = i_time / 10;
    qint64 minutes = seconds / 60;
    quint64 dixiemme = i_time-(seconds*10);
    seconds -= minutes * 60;

    o_str.append( QStringLiteral("%1:%2:%3")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'))
        .arg(dixiemme, 3, 10, QLatin1Char('0')) );
    o_str.append(" - ");
    for(int loop=0; loop<16; loop++)
    {
        o_str.append(QString::number((i_config>>loop&0x1)));
    }

    return o_str;
}

QString CMusicFile::timeToString(quint64 i_time)
{
    qint64 seconds = i_time / 10;
    qint64 minutes = seconds / 60;
    quint64 dixiemme = i_time-(seconds*10);
    seconds -= minutes * 60;

    return QStringLiteral("%1:%2:%3")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'))
        .arg(dixiemme, 1, 10, QLatin1Char('0'));
}

quint64 CMusicFile::timeToValue(QString i_time)
{
    quint64 o_time;
    QStringList l_timeStr = i_time.split(":");
    quint64 seconds = l_timeStr.at(0).toUShort()*60;
    seconds += l_timeStr.at(1).toUShort();

    o_time = l_timeStr.at(2).toUShort() + (seconds*10) ;

    return o_time;
}

QString CMusicFile::configToString(quint16 i_config)
{
    QString o_str;

    for(int loop=0; loop<16; loop++)
    {
        o_str.append(QString::number((i_config>>loop&0x1)));
    }

    return o_str;
}

quint16 CMusicFile::configToValue(QString i_config)
{
    quint16 o_config = 0;

    for(int loop=0; loop<16; loop++)
    {
        o_config |= (i_config.at(loop).digitValue()<<loop);
    }


    return o_config;
}
