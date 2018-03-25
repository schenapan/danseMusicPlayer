#include "cxmllist.h"

#include <QtXml>
#include <QtDebug>

#define DANSE_LIST_FILENAME "danseMusicList.xml"
#define BAL_MUSIC_FILE "music_file"

CXmlList::CXmlList(QObject *parent) : QObject(parent)
{
    QDomDocument dom_doc("danseMusicList");
    QFile xml_doc(DANSE_LIST_FILENAME);

    if( !xml_doc.open(QIODevice::ReadOnly))
    {
        qDebug() << "Erreur à l'ouverture du document XML\n" << "Le document XML n'a pas pu être ouvert. Vérifiez que le nom est le bon et que le document est bien placé";
    }
    else
    {
        if (!dom_doc.setContent(&xml_doc)) // Si l'on n'arrive pas à associer le fichier XML à l'objet DOM.
        {
            qDebug() << "Erreur à l'ouverture du document XML\n" << "Le document XML n'a pas pu être attribué à l'objet QDomDocument.";
        }
        else
        {
            QDomElement dom_element = dom_doc.documentElement();
            QDomNode noeud = dom_element.firstChild();
            while(!noeud.isNull())
            {
                QDomElement element = noeud.toElement(); // On utilise cette propriété afin de transformer le nœud en éléments. Cela nous permet aussi de récupérer l'élément ou nœud courant.

                if(!element.isNull())
                {
                    qDebug() << "Le nom de la balise est " << element.tagName();
                    if( element.tagName() == BAL_MUSIC_FILE )
                    {
                        musicFileList.append(new CMusicFile(element));
                    }
                }
                noeud = noeud.nextSibling();
            }
        }
        xml_doc.close(); // Dans tous les cas, on doit fermer le document XML : on n'en a plus besoin, tout est compris dans l'objet DOM.
    }
}

CXmlList::~CXmlList()
{
    foreach (CMusicFile *file, musicFileList) {
        delete file;
    }
}

void CXmlList::saveList()
{
    QFile xml_doc(DANSE_LIST_FILENAME);
    QDomDocument dom_doc("danseMusicList");
    QDomElement dom_element = dom_doc.createElement("danse_music");// dom_doc.documentElement();
    dom_doc.appendChild(dom_element);
    foreach (CMusicFile *file, musicFileList) {
        QDomElement l_element = dom_doc.createElement(BAL_MUSIC_FILE);
        file->fillElement(&l_element);
        dom_element.appendChild(l_element);
    }
    if( xml_doc.open(QIODevice::WriteOnly) )
    {
        QTextStream stream(&xml_doc);

        stream << dom_doc.toString();

        xml_doc.close();
    }
}

QStringList CXmlList::getListName()
{
    QStringList lo_list;

    foreach (CMusicFile *file, musicFileList) {
        lo_list.append(file->getName());
    }

    return lo_list;
}

void CXmlList::addFile(QUrl i_url)
{
    musicFileList.append(new CMusicFile(i_url));
    saveList();
}

void CXmlList::removeFile(int i_file_idx)
{
    if(musicFileList.size()>i_file_idx)
    {
        musicFileList.removeAt(i_file_idx);
        saveList();
    }
}

QUrl CXmlList::getUrl(QString i_filemane)
{
    foreach (CMusicFile *file, musicFileList) {
        if( file->getName() == i_filemane ) {
            return file->getUrl();
        }
    }
}
