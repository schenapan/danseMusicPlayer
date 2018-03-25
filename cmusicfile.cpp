#include "cmusicfile.h"

#define ATTR_NAME "name"
#define ATTR_URL "url"

CMusicFile::CMusicFile(QUrl i_url, QObject *parent) : QObject(parent)
{
    name = i_url.fileName();
    url = i_url;
}

CMusicFile::CMusicFile(QDomElement i_element, QObject *parent) : QObject(parent)
{
    name = i_element.attribute(ATTR_NAME);
    url = i_element.attribute(ATTR_URL);
}

void CMusicFile::fillElement(QDomElement *ip_element)
{
    ip_element->setAttribute(ATTR_NAME, name);
    ip_element->setAttribute(ATTR_URL, url.toString());
}
