#include "cash.h"
#include "log.h"

/**
 * The receive timeout settings - min/initial/max - defined in milliseconds
 */
#define T_RX_ACK_MIN  400
#define T_RX_ACK_INIT 1600
#define T_RX_ACK_MAX 3200

#define ASH_CANCEL_BYTE     0x1A
#define ASH_FLAG_BYTE       0x7E
#define ASH_SUBSTITUTE_BYTE 0x18
#define ASH_XON_BYTE        0x11
#define ASH_OFF_BYTE        0x13
#define ASH_TIMEOUT         -1

#define ASH_MAX_LENGTH 131

CAsh::CAsh(QObject *parent) : QObject(parent)
{
  in_msg.clear();
  ackNum = 0;
  frmNum = 0;
//  reTx = 0;
  seq_num = 0;
  stateConnected = false;
  timer = new QTimer();
  connect( timer,SIGNAL(timeout()),
           this, SLOT(SlotTimeout()) );
}

QByteArray CAsh::resetNCPFrame(void)
{
  ackNum = 0;
  frmNum = 0;
//  reTx = 0;
  seq_num = 0;
  stateConnected = false;
  QByteArray lo_msg;

  timer->stop();

  lo_msg.append(0xC0);

  quint16 crc = computeCRC(lo_msg);
  lo_msg.append((quint8)(crc>>8));
  lo_msg.append((quint8)(crc&0xFF));

  lo_msg = stuffedOutputData(lo_msg);

  lo_msg.insert( 0, ASH_CANCEL_BYTE );

  // start timer
  timer->start( T_RX_ACK_INIT );

  return lo_msg; // { 0x1A, 0xC0, 0x38, 0xBC, 0x7E }
}

QByteArray CAsh::AckFrame(void)
{
  QByteArray lo_msg;

//  LOGGER(logTRACE) << "Send AckFrame : " << QString::number(ackNum,16).toUpper().rightJustified(2,'0');

  lo_msg.append(0x80+ackNum);

  quint16 crc = computeCRC(lo_msg);
  lo_msg.append((quint8)(crc>>8));
  lo_msg.append((quint8)(crc&0xFF));

  lo_msg = stuffedOutputData(lo_msg);

  // start timer
  timer->start( T_RX_ACK_INIT );

  return lo_msg; // { 0x1A, 0xC0, 0x38, 0xBC, 0x7E }
}

QByteArray CAsh::DataFrame(QList<quint8> i_data)
{
  QByteArray lo_msg;

//  LOGGER(logTRACE) << "Send DataFrame with ack : " << QString::number(ackNum,16).toUpper().rightJustified(2,'0');


  lo_msg.append((frmNum << 4) + /*reTx +*/ ackNum );
  frmNum = (frmNum + 1) & 0x07;
//  reTx = 0;

  if( 0 != i_data.at(0) )
  {
    // WARNING for all frames except "VersionRequest" frame, add exteded header
    i_data.insert(0,0);
    i_data.insert(0,0xFF);
  }

  // insert frm control
  i_data.insert(0,0);
  // insert seq number
  i_data.insert(0,seq_num++);


  i_data = dataRandomise(i_data,0);

  foreach (quint8 val, i_data) {
    lo_msg.append(val);
  }

  quint16 crc = computeCRC(lo_msg);
  lo_msg.append((quint8)(crc>>8));
  lo_msg.append((quint8)(crc&0xFF));

  lo_msg = stuffedOutputData(lo_msg);

  // start timer
  timer->start( T_RX_ACK_INIT );

/*
  QString l_str;
  foreach(quint8 l_data, lo_msg )
  {
    l_str.append( QString::number(l_data,16).toUpper().rightJustified(2,'0') + " ");
  }
  LOGGER(logTRACE) << "Send DataFrame  : " +  l_str;
*/

  return lo_msg;
}

QList<quint8> CAsh::decode(QList<quint8> *i_data)
{
  bool inputError = false;
  QList<quint8> lo_msg;
  quint8 val;

  while( !i_data->isEmpty() && lo_msg.isEmpty() )
  {
    val = i_data->takeFirst();
    switch( val )
    {
      case ASH_CANCEL_BYTE:
          // Cancel Byte: Terminates a frame in progress. A Cancel Byte causes all data received since the
          // previous Flag Byte to be ignored. Note that as a special case, RST and RSTACK frames are preceded
          // by Cancel Bytes to ignore any link startup noise.
          in_msg.clear();
          inputError = false;
          break;
      case ASH_FLAG_BYTE:
          // Flag Byte: Marks the end of a frame.When a Flag Byte is received, the data received since the
          // last Flag Byte or Cancel Byte is tested to see whether it is a valid frame.
          //LOGGER(logTRACE) << "<-- RX ASH frame: VIEW ASH_FLAG_BYTE";
          if (!inputError && !in_msg.isEmpty()) {
            if( in_msg.length() >= 3 )
            {
              // Remove byte stuffing
              bool escape = false;
              foreach(quint8 data, in_msg) {
                  if (escape) {
                      escape = false;
                      if ((data & 0x20) == 0) {
                          data = (quint8) (data + 0x20);
                      } else {
                          data = (quint8) (data & 0xDF);
                      }
                  } else if (data == 0x7D) {
                      escape = true;
                      continue;
                  }
                  lo_msg.append(data);
              }

              // Check CRC
              if (computeCRC(lo_msg) != 0) {
                  lo_msg.clear();
              }
              else
              {
                if ((lo_msg.at(0) & 0x80) == 0) {
                  // DATA;
                  /*
                  QString l_str;
                  foreach(quint8 l_data, lo_msg )
                  {
                    l_str.append( QString::number(l_data,16).toUpper().rightJustified(2,'0') + " ");
                  }
                  LOGGER(logTRACE) << "<-- RX ASH DATA Frame  : " +  l_str;
                  */

                  // update ack number, use incoming frm number
                  ackNum = ((lo_msg.at(0)>>4&0x07) + 1) & 0x07;


                  lo_msg = dataRandomise(lo_msg,1);

                  if( 0xFF == lo_msg.at(2) )
                  {
                    // WARNING for all frames except "VersionRequest" frame, add exteded header
                    lo_msg.removeAt(2);
                    lo_msg.removeAt(2);
                  }

                }
                else if ((lo_msg.at(0) & 0x60) == 0x00) {
                  // ACK;
                  //LOGGER(logTRACE) << "<-- RX ASH ACK Frame !! ";
                  lo_msg.clear();
                  timer->stop();
                  emit info(ASH_ACK);
                }
                else if ((lo_msg.at(0) & 0x60) == 0x20) {
                  // NAK;
                  frmNum = lo_msg.at(0) & 0x07;
//                  reTx = 0x04;

                  LOGGER(logTRACE) << "<-- RX ASH NACK Frame !! : 0x" << QString::number(lo_msg.at(0),16).toUpper().rightJustified(2,'0');
                  lo_msg.clear();
                  timer->stop();
                  emit info(ASH_NACK);
                }
                else if (lo_msg.at(0) == 0xC0) {
                  // RST;
                  lo_msg.clear();
                  LOGGER(logTRACE) << "<-- RX ASH RST Frame !! ";
                }
                else if (lo_msg.at(0) == 0xC1) {
                  // RSTACK;
                  LOGGER(logTRACE) << "<-- RX ASH RSTACK Frame !! ";

                  lo_msg.clear();
                  if( !stateConnected )
                  {
                    /** \todo : add some test to verify it is a software reset and ash protocol version is 2 */
                    timer->stop();
                    stateConnected = true;
                  }
                }
                else if (lo_msg.at(0) == 0xC2) {
                  // ERROR;
                  LOGGER(logTRACE) << "<-- RX ASH ERROR Frame !! ";
                  lo_msg.clear();
                }
                else
                {
                  LOGGER(logTRACE) << "<-- RX ASH Unknown !! ";
                  lo_msg.clear();
                }
              }
            }
            else
            {
              LOGGER(logTRACE) << "<-- RX ASH too short !! ";
            }
          }
          in_msg.clear();
          inputError = false;
          break;
      case ASH_SUBSTITUTE_BYTE:
          // Substitute Byte: Replaces a byte received with a low-level communication error (e.g., framing
          // error) from the UART.When a Substitute Byte is processed, the data between the previous and the
          // next Flag Bytes is ignored.
          inputError = true;
          break;
      case ASH_XON_BYTE:
          // XON: Resume transmissionUsed in XON/XOFF flow control. Always ignored if received by the NCP.
          break;
      case ASH_OFF_BYTE:
          // XOFF: Stop transmissionUsed in XON/XOFF flow control. Always ignored if received by the NCP.
          break;
      case ASH_TIMEOUT:
          break;
      default:
          if (in_msg.length() >= ASH_MAX_LENGTH) {
              in_msg.clear();
              inputError = true;
          }
          in_msg.append(val);
          break;
    }

  }
/*
  QString l_str;
  foreach(quint8 l_data, lo_msg )
  {
    l_str.append( QString::number(l_data,16).toUpper().rightJustified(2,'0') + " ");
  }
  LOGGER(logTRACE) << "<-- RX ASH return lo_msg  : " +  l_str;
*/
  return lo_msg;
}


quint16 CAsh::computeCRC( QByteArray i_msg )
{
  quint16 lo_crc = 0xFFFF; // initial value
  quint16 polynomial = 0x1021; // 0001 0000 0010 0001 (0, 5, 12)

  for (int cnt = 0; cnt < i_msg.length(); cnt++) {
      for (int i = 0; i < 8; i++) {
          bool bit = ((i_msg.at(cnt) >> (7 - i) & 1) == 1);
          bool c15 = ((lo_crc >> 15 & 1) == 1);
          lo_crc <<= 1;
          if (c15 ^ bit) {
              lo_crc ^= polynomial;
          }
      }
  }

  lo_crc &= 0xffff;

  return lo_crc;
}

quint16 CAsh::computeCRC( QList<quint8> i_msg )
{
  quint16 lo_crc = 0xFFFF; // initial value
  quint16 polynomial = 0x1021; // 0001 0000 0010 0001 (0, 5, 12)

  for (int cnt = 0; cnt < i_msg.length(); cnt++) {
      for (int i = 0; i < 8; i++) {
          bool bit = ((i_msg.at(cnt) >> (7 - i) & 1) == 1);
          bool c15 = ((lo_crc >> 15 & 1) == 1);
          lo_crc <<= 1;
          if (c15 ^ bit) {
              lo_crc ^= polynomial;
          }
      }
  }

  lo_crc &= 0xffff;

  return lo_crc;
}

QByteArray CAsh::stuffedOutputData(QByteArray i_msg)
{
  QByteArray lo_msg;

  for (int cnt = 0; cnt < i_msg.length(); cnt++) {
      switch (i_msg.at(cnt)) {
          case 0x7E:
              lo_msg.append( 0x7D );
              lo_msg.append( 0x5E );
              break;
          case 0x7D:
              lo_msg.append( 0x7D );
              lo_msg.append( 0x5D );
              break;
          case 0x11:
              lo_msg.append( 0x7D );
              lo_msg.append( 0x31 );
              break;
          case 0x13:
              lo_msg.append( 0x7D );
              lo_msg.append( 0x33 );
              break;
          case 0x18:
              lo_msg.append( 0x7D );
              lo_msg.append( 0x38 );
              break;
          case 0x1A:
              lo_msg.append( 0x7D );
              lo_msg.append( 0x3A );
              break;
          default:
              lo_msg.append( i_msg.at(cnt) );
              break;
      }
  }
  lo_msg.append( 0x7E );


  return lo_msg;
}

QList<quint8> CAsh::dataRandomise(QList<quint8> i_data, quint8 start)
{
  QList<quint8> lo_data;

  // Randomise the data
  quint8 rand = 0x42;
  for (quint8 cnt = start; cnt < i_data.length(); cnt++) {
      lo_data.append(i_data.at(cnt) ^ rand);

      if ((rand & 0x01) == 0) {
          rand = rand >> 1;
      } else {
          rand = (rand >> 1) ^ 0xb8;
      }
  }

  return lo_data;
}


void CAsh::SlotTimeout(void)
{
  if( !stateConnected )
  {
    emit info(ASH_RESET_FAILED);
  }
}
