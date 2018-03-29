#include "cmusicfile.h"

#define ATTR_NAME "name"
#define ATTR_URL "url"
#define ATTR_LIGHT_ENABLE "light_enable"

CMusicFile::CMusicFile(QUrl i_url, bool i_light_enable, QObject *parent) : QObject(parent)
{
    name = i_url.fileName();
    url = i_url;
    light_enable = i_light_enable;
}

CMusicFile::CMusicFile(QDomElement i_element, QObject *parent) : QObject(parent)
{
    name = i_element.attribute(ATTR_NAME);
    url = i_element.attribute(ATTR_URL);
    if(i_element.attribute(ATTR_LIGHT_ENABLE)=="TRUE"){light_enable = true;}
    else{light_enable = false;}
}

void CMusicFile::fillElement(QDomElement *ip_element)
{
    ip_element->setAttribute(ATTR_NAME, name);
    ip_element->setAttribute(ATTR_URL, url.toString());
    if(light_enable){ip_element->setAttribute(ATTR_LIGHT_ENABLE, "TRUE");}
    else{ip_element->setAttribute(ATTR_LIGHT_ENABLE, "FALSE");}
}
