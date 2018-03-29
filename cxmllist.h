#ifndef CXMLLIST_H
#define CXMLLIST_H

#include <QObject>

#include "cmusicfile.h"

class CXmlList : public QObject
{
    Q_OBJECT
public:
    explicit CXmlList(QObject *parent = nullptr);
    ~CXmlList();

    QStringList getListName();

    void addFile(QUrl i_url, bool i_light_enable = false);
    void removeFile(int i_file_idx);
    CMusicFile *getMusicFile(QString i_filemane);
    void setLightEnable(QUrl i_url, bool i_checked);


    void saveList();

signals:

public slots:

private:
    QList<CMusicFile *> musicFileList;

};

#endif // CXMLLIST_H
