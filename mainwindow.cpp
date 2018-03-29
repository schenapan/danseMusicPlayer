#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->btnPlay->setEnabled(false);

    connect(&mediaPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::updatePosition);
    connect(&mediaPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::updateDuration);
    connect(&mediaPlayer, &QMediaObject::metaDataAvailableChanged, this, &MainWindow::updateInfo);
    connect(&mediaPlayer, &QMediaPlayer::stateChanged, this, &MainWindow::updateState);

    mediaPlayer.setNotifyInterval(100);

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
    ui->btnPlay->setEnabled(true);
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
    }
}

void MainWindow::updateDuration(qint64 duration)
{
    ui->sliMusic->setRange(0, duration);
    ui->sliMusic->setEnabled(duration > 0);
//    ui->sliMusic->setPageStep(duration / 100);
//    ui->sliMusic->setSingleStep(duration / 100);
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

void MainWindow::on_btnOpen_clicked()
{
    // open a file
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open File"));
    fileDialog.setMimeTypeFilters(MainWindow::supportedMimeTypes());
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
        playUrl(fileDialog.selectedUrls().constFirst());

}

void MainWindow::on_btnPlay_clicked()
{
    // play files
    if (mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia)
        on_btnOpen_clicked();
    else if (mediaPlayer.state() == QMediaPlayer::PlayingState)
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
    p_current_file = fileList.getMusicFile(ui->listWidget->item(currentRow)->text());
    if( NULL != p_current_file )
    {
        playUrl(p_current_file->getUrl());
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
