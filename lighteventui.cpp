#include "lighteventui.h"
#include "ui_lighteventui.h"

lightEventUi::lightEventUi(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lightEventUi)
{
    ui->setupUi(this);
}

lightEventUi::~lightEventUi()
{
    delete ui;
}

void lightEventUi::setTime(quint64 i_time)
{
    qint64 seconds = i_time / 10;
    const qint64 minutes = seconds / 60;
    i_time = i_time-(seconds*10);
    seconds -= minutes * 60;

    ui->spDixiemme->setValue(i_time);
    ui->spSecondes->setValue(seconds);
    ui->spMinutes->setValue(minutes);
}

quint64 lightEventUi::getTime()
{
    quint64 o_time = 0;

    quint64 seconds = ui->spSecondes->value() + (ui->spMinutes->value()*60);

    o_time = ui->spDixiemme->value();
    o_time += seconds*10;

    return o_time;
}

void lightEventUi::setConfig(quint16 i_config)
{
    ui->rdState00->setChecked(i_config&RD01_MASK);
    ui->rdState01->setChecked(i_config&RD02_MASK);
    ui->rdState02->setChecked(i_config&RD03_MASK);
    ui->rdState03->setChecked(i_config&RD04_MASK);
    ui->rdState04->setChecked(i_config&RD05_MASK);
    ui->rdState05->setChecked(i_config&RD06_MASK);
    ui->rdState06->setChecked(i_config&RD07_MASK);
    ui->rdState07->setChecked(i_config&RD08_MASK);
    ui->rdState08->setChecked(i_config&RD09_MASK);
    ui->rdState09->setChecked(i_config&RD10_MASK);
    ui->rdState10->setChecked(i_config&RD11_MASK);
    ui->rdState11->setChecked(i_config&RD12_MASK);
    ui->rdState12->setChecked(i_config&RD13_MASK);

}

quint16 lightEventUi::getConfig(void)
{
    quint16 o_config = 0;

    o_config |= ui->rdState00->isChecked()<<0;
    o_config |= ui->rdState01->isChecked()<<1;
    o_config |= ui->rdState02->isChecked()<<2;
    o_config |= ui->rdState03->isChecked()<<3;
    o_config |= ui->rdState04->isChecked()<<4;
    o_config |= ui->rdState05->isChecked()<<5;
    o_config |= ui->rdState06->isChecked()<<6;
    o_config |= ui->rdState07->isChecked()<<7;
    o_config |= ui->rdState08->isChecked()<<8;
    o_config |= ui->rdState09->isChecked()<<9;
    o_config |= ui->rdState10->isChecked()<<10;
    o_config |= ui->rdState11->isChecked()<<11;
    o_config |= ui->rdState12->isChecked()<<12;

    return o_config;
}
