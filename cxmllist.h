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

    void addFile(QUrl i_url);
    void removeFile(int i_file_idx);
    QUrl getUrl(QString i_filemane);


signals:

public slots:

private:
    QList<CMusicFile *> musicFileList;

    void saveList();


};

#endif // CXMLLIST_H
