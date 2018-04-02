#include "cmusicfile.h"

#define ATTR_NAME "name"
#define ATTR_URL "url"
#define ATTR_DURATION "duration"
#define ATTR_LIGHT_ENABLE "light_enable"

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
        light_enable = false;
        // create empty ram
        clearLightStatus();
        // fill with value
        if( 0 != duration)
        {
            QStringList l_config = i_element.text().split(" ");

            if( l_config.size() == light_status_size )
            {
                for(quint64 loop; loop<light_status_size; loop++)
                {
                    QString l_value = l_config.at(loop);
                    light_status[loop] = l_value.toUShort(NULL,16);
                }

                light_enable = true;
            }

        }
    }
    else{light_enable = false;}
}

CMusicFile::~CMusicFile()
{
    if(NULL!=light_status){ delete light_status; }
}

void CMusicFile::setLightEnable(bool i_enable)
{
    light_enable=i_enable;
    if( light_enable && (NULL==light_status) )
    {
        clearLightStatus();
    }
}

bool CMusicFile::setDuration(quint64 i_duration)
{
    bool o_change = false;

    if( i_duration != duration )
    {
        duration = i_duration;
        clearLightStatus();
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

bool CMusicFile::setLightStatusAt(quint64 i_time, quint16 i_status)
{
    bool lo_change = false;
    if( (NULL != light_status) &&
            (i_time<light_status_size) &&
            (light_status[i_time] != i_status) )
    {
        light_status[i_time] = i_status;
        lo_change = true;
    }
    return lo_change;
}

void CMusicFile::fillElement(QDomElement *ip_element)
{
    ip_element->setAttribute(ATTR_NAME, name);
    ip_element->setAttribute(ATTR_URL, url.toString());
    ip_element->setAttribute(ATTR_DURATION, QString::number(duration));
    if(light_enable){
        // check if some order exist
        if( (NULL != light_status) && (0!=light_status_size) )
        {
            quint16 l_tmp[light_status_size];
            memset(l_tmp,0,light_status_size);
            if( memcmp(l_tmp,light_status,light_status_size) != 0 )
            {
                ip_element->setAttribute(ATTR_LIGHT_ENABLE, "TRUE");

                // save config
                QString l_config;
                for( quint64 loop=0; loop<light_status_size; loop++)
                {
                    l_config.append(QString::number(light_status[loop],16) + " ");
                }
                ip_element->setNodeValue(l_config);
            }
            else
            {
                ip_element->setAttribute(ATTR_LIGHT_ENABLE, "FALSE");
            }
        }
        else
        {
            ip_element->setAttribute(ATTR_LIGHT_ENABLE, "FALSE");
        }
    }
    else{ip_element->setAttribute(ATTR_LIGHT_ENABLE, "FALSE");}

}



/****************************************************************************
 * PRIVATE
 ***************************************************************************/


void CMusicFile::clearLightStatus(void)
{
    if(NULL!=light_status){ delete light_status; }
    light_status_size = 0;
    if( light_enable )
    {
        if( 0 != duration )
        {
            light_status_size = duration/100;
            light_status = new quint16[light_status_size];
            memset(light_status,0,light_status_size);
        }
    }
}
