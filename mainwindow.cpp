#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lighteventui.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->grpMusic->setEnabled(false);

    connect(&mediaPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::updatePosition);
    connect(&mediaPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::updateDuration);
    connect(&mediaPlayer, &QMediaObject::metaDataAvailableChanged, this, &MainWindow::updateInfo);
    connect(&mediaPlayer, &QMediaPlayer::stateChanged, this, &MainWindow::updateState);

    mediaPlayer.setNotifyInterval(100);

    // dongle
    connect(&dongle, SIGNAL(SignalDongleConnect(bool)), this, SLOT(SlotDongleConnect(bool)));


    // get file list
    ui->listWidget->addItems(fileList.getListName());
    p_current_file = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --- Static

QStringList MainWindow::supportedMimeTypes()
{
    QStringList result = QMediaPlayer::supportedMimeTypes();
    if (result.isEmpty())
        result.append(QStringLiteral("audio/mpeg"));
    return result;
}

QString MainWindow::formatTime(qint64 timeMilliSeconds)
{
    qint64 seconds = timeMilliSeconds / 1000;
    const qint64 minutes = seconds / 60;
    timeMilliSeconds = ((timeMilliSeconds-(seconds*1000))/100)*100;
    seconds -= minutes * 60;
    return QStringLiteral("%1:%2:%3")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'))
        .arg(timeMilliSeconds, 3, 10, QLatin1Char('0'));
}

// --


void MainWindow::playUrl(const QUrl &url)
{
    ui->grpMusic->setEnabled(true);
    if (url.isLocalFile()) {
        const QString filePath = url.toLocalFile();
        setWindowFilePath(filePath);
        ui->lblInfo->setText(QDir::toNativeSeparators(filePath));
        fileName = QFileInfo(filePath).fileName();
    } else {
        setWindowFilePath(QString());
        ui->lblInfo->setText(url.toString());
        fileName.clear();
    }
    mediaPlayer.setMedia(url);
    mediaPlayer.play();
}


void MainWindow::updatePosition(qint64 position)
{
    ui->sliMusic->setValue(position);
    ui->lblTime->setText(MainWindow::formatTime(position));

    if( (0!=position) && (position == mediaPlayer.duration())) {
        if( ui->chkAutoPlay->isChecked() ) {
            if( (ui->listWidget->currentRow()+1) < ui->listWidget->count() )
            {
                ui->listWidget->setCurrentRow(ui->listWidget->currentRow()+1);
            }
        }
    }

    if( p_current_file->isLightEnable() )
    {
        // send light configuration
        setLightStatus(p_current_file->getLightStatusAt(position/100));
    }
}

void MainWindow::updateDuration(qint64 duration)
{
    ui->sliMusic->setRange(0, duration);
    ui->sliMusic->setEnabled(duration > 0);
    updateInfo();

}

void MainWindow::updateInfo()
{
    QStringList info;
    if (!fileName.isEmpty())
        info.append(fileName);
    if (mediaPlayer.isMetaDataAvailable()) {
        QString author = mediaPlayer.metaData(QStringLiteral("Author")).toString();
        if (!author.isEmpty())
            info.append(author);
        QString title = mediaPlayer.metaData(QStringLiteral("Title")).toString();
        if (!title.isEmpty())
            info.append(title);
        if( p_current_file->setDuration(mediaPlayer.duration()) )
        {
            fileList.saveList();
        }
    }
    info.append(formatTime(mediaPlayer.duration()));
    ui->lblInfo->setText(info.join(tr(" - ")));
}

void MainWindow::updateState(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::PlayingState) {
        ui->btnPlay->setToolTip(tr("Pause"));
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    } else {
        ui->btnPlay->setToolTip(tr("Play"));
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
}

void MainWindow::on_btnPlay_clicked()
{
    // play files
    /*if (mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia)
        on_btnOpen_clicked();
    else*/
    if (mediaPlayer.state() == QMediaPlayer::PlayingState)
        mediaPlayer.pause();
    else
        mediaPlayer.play();
}

void MainWindow::on_sliMusic_valueChanged(int value)
{
    // avoid seeking when the slider value change is triggered from updatePosition()
    if (qAbs(mediaPlayer.position() - value) > 99)
        mediaPlayer.setPosition(value);

}

void MainWindow::on_sliVolume_valueChanged(int value)
{
    mediaPlayer.setVolume(value);
}

void MainWindow::on_btnAdd_clicked()
{
    // open a file
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open File"));
    fileDialog.setMimeTypeFilters(MainWindow::supportedMimeTypes());
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
    {
        // add an entry to file list
        QUrl l_tmp = fileDialog.selectedUrls().constFirst();
        fileList.addFile(l_tmp);
        ui->listWidget->clear();
        ui->listWidget->addItems(fileList.getListName());
    }
}

void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    // load selected item to player
    if( currentRow >= 0 )
    {
        p_current_file = fileList.getMusicFile(ui->listWidget->item(currentRow)->text());
        if( NULL != p_current_file )
        {
            playUrl(p_current_file->getUrl());
            ui->chkEnableLight->setChecked(p_current_file->isLightEnable());
            ui->grpLight->setEnabled(p_current_file->isLightEnable());
            ui->lstLightEvent->clear();
            ui->lstLightEvent->addItems(p_current_file->getEventList());
        }
    }
}

void MainWindow::on_btnRemove_clicked()
{
    QListWidgetItem *l_item = ui->listWidget->currentItem();

    if( NULL != l_item) {
        int l_item_idx = ui->listWidget->currentIndex().row();
        fileList.removeFile(l_item_idx);
        ui->listWidget->clear();
        ui->listWidget->addItems(fileList.getListName());
    }
}

void MainWindow::on_chkEnableLight_clicked(bool checked)
{
    if( NULL != p_current_file )
    {
        p_current_file->setLightEnable(checked);
        ui->grpLight->setEnabled(checked);
        fileList.saveList();
    }
}


void MainWindow::on_btnLightAdd_clicked()
{
    lightEventUi *dial = new lightEventUi(this);
    quint64 l_time = mediaPlayer.position()/100;

    dial->setModal(true);

    dial->setTime(l_time);
    dial->setConfig(p_current_file->getLightStatusAt(l_time));


    if( dial->exec() == QDialog::Accepted )
    {
        if( p_current_file->addEvent(dial->getTime(),dial->getConfig()) )
        {
            fileList.saveList();
            ui->lstLightEvent->clear();
            ui->lstLightEvent->addItems(p_current_file->getEventList());
        }
    }

    delete dial;
}

void MainWindow::on_btnLightRemove_clicked()
{
    QListWidgetItem *l_item = ui->lstLightEvent->currentItem();

    if( NULL != l_item) {
        p_current_file->removeEvent(l_item->text());
        fileList.saveList();
        ui->lstLightEvent->clear();
        ui->lstLightEvent->addItems(p_current_file->getEventList());
    }
}

//------ DONGLE -----
void MainWindow::SlotDongleConnect(bool i_connect)
{
    if( i_connect )
    {
        ui->lblDongle->setText("Dongle Radio version : " + QString::number(dongle.GetFwVersion().at(0)) + "." + QString::number(dongle.GetFwVersion().at(1)));

        // configure mfglib for sending message
        QList<quint8> l_data;
        l_data.append(0);
        dongle.SendEzspCmd(EZSP_MFGLIB_START,l_data);
        l_data.clear();
        l_data.append(3);
        dongle.SendEzspCmd(EZSP_MFGLIB_SET_POWER,l_data);
        l_data.clear();
        l_data.append(15);
        dongle.SendEzspCmd(EZSP_MFGLIB_SET_CHANNEL,l_data);
    }
    else
    {
        ui->lblDongle->setText("Aucun Dongle Radio");
    }
}

/***********************************************************************************
 *  PRIVATE
 **********************************************************************************/

void MainWindow::setLightStatus(quint16 i_status)
{
        ui->rdState00->setChecked(i_status&RD01_MASK);
        ui->rdState01->setChecked(i_status&RD02_MASK);
        ui->rdState02->setChecked(i_status&RD03_MASK);
        ui->rdState03->setChecked(i_status&RD04_MASK);
        ui->rdState04->setChecked(i_status&RD05_MASK);
        ui->rdState05->setChecked(i_status&RD06_MASK);
        ui->rdState06->setChecked(i_status&RD07_MASK);
        ui->rdState07->setChecked(i_status&RD08_MASK);
        ui->rdState08->setChecked(i_status&RD09_MASK);
        ui->rdState09->setChecked(i_status&RD10_MASK);
        ui->rdState10->setChecked(i_status&RD11_MASK);
        ui->rdState11->setChecked(i_status&RD12_MASK);
}

