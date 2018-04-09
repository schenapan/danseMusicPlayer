#ifndef CASH_H
#define CASH_H

#include <QObject>
#include <QTimer>

typedef enum {
  ASH_RESET_FAILED,
  ASH_ACK,
  ASH_NACK
}EAshInfo;

class CAsh : public QObject
{
  Q_OBJECT
private:
  quint8 ackNum;
  quint8 frmNum;
  bool stateConnected;
  quint8 seq_num;
  QTimer *timer;
  QList<quint8> in_msg;

  quint16 computeCRC( QByteArray i_msg );
  QByteArray stuffedOutputData(QByteArray i_msg);
  QList<quint8> dataRandomise(QList<quint8> i_data, quint8 start);
  quint16 computeCRC( QList<quint8> i_msg );

public:
  explicit CAsh(QObject *parent = 0);

  QByteArray resetNCPFrame(void);
  QByteArray AckFrame(void);
  QByteArray DataFrame(QList<quint8> i_data);

  QList<quint8> decode(QList<quint8> *i_data);

  bool isConnected(void){ return stateConnected; }

signals:
  void info(EAshInfo i_info);

public slots:
  void SlotTimeout(void);
};

#endif // CASH_H
