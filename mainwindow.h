#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtWidgets>
#include <QMediaPlayer>

#include "cxmllist.h"
#include "dongle/cdongle.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static QStringList supportedMimeTypes();
    static QString formatTime(qint64 timeMilliSeconds);

private slots:
    void playUrl(const QUrl& url);
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void updateInfo();
    void updateState(QMediaPlayer::State state);

    void on_btnPlay_clicked();

    void on_sliMusic_valueChanged(int value);

    void on_sliVolume_valueChanged(int value);

    void on_btnAdd_clicked();

    void on_listWidget_currentRowChanged(int currentRow);

    void on_btnRemove_clicked();

    void on_chkEnableLight_clicked(bool checked);


    void on_btnLightAdd_clicked();

    void on_btnLightRemove_clicked();

    void SlotDongleConnect(bool i_connect);

private:
    Ui::MainWindow *ui;

    QString fileName;
    QMediaPlayer mediaPlayer;
    CXmlList fileList;
    CMusicFile *p_current_file;
    CDongle dongle;

    void setLightStatus(quint16 i_status);

};

#endif // MAINWINDOW_H
