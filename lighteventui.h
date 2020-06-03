#ifndef LIGHTEVENTUI_H
#define LIGHTEVENTUI_H

#include <QDialog>

#define RD01_MASK 0x0001
#define RD02_MASK 0x0002
#define RD03_MASK 0x0004
#define RD04_MASK 0x0008
#define RD05_MASK 0x0010
#define RD06_MASK 0x0020
#define RD07_MASK 0x0040
#define RD08_MASK 0x0080
#define RD09_MASK 0x0100
#define RD10_MASK 0x0200
#define RD11_MASK 0x0400
#define RD12_MASK 0x0800
#define RD13_MASK 0x1000

namespace Ui {
class lightEventUi;
}

class lightEventUi : public QDialog
{
    Q_OBJECT

public:
    explicit lightEventUi(QWidget *parent = 0);
    ~lightEventUi();

    void setTime(quint64 i_time);
    quint64 getTime();

    void setConfig(quint16 i_config);
    quint16 getConfig(void);


private:
    Ui::lightEventUi *ui;
};

#endif // LIGHTEVENTUI_H
