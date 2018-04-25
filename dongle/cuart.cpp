#include "cuart.h"

#include "ezsp_constant.h"
#include "log.h"

// ********************************************************
// INFOS TECHNIQUES SUR LA LIAISON
//
// taille des trames :
// trame ezsp en reception -> 130 octets (fixe)
//
// datasheet uart cp 2102 :
// input buffer : 576 bytes
// output buffer : 640 bytes
//
// ********************************************************

// FTDI Basic usb to uart bridge
#define FTDI_VENDOR_IDENTIFIER 0x0403
#define FTDI_PRODUCT_IDENTIFIER 0x6001

// Silicon labs cp2102 usb to uart bridge
#define VENDOR_IDENTIFIER 0x10c4
#define PRODUCT_IDENTIFIER 0xea60

// Silicon labs USB NCP
#define NCP_VENDOR_IDENTIFIER 0x10c4
#define NCP_PRODUCT_IDENTIFIER 0x0003

//defines
//-------
#define STX 0x02
#define ETX 0x03
//-------
#define WAIT_ACK_TIMEOUT 1000 // timer to manage ack timeout in ms
#define WAIT_ACK_TIMEOUT_VALUE 5 // timeout for a waiting ack in second
//-------
//[Error]
#define USART_ERR_SUCCESS       0x0U
#define USART_ERR_ERROR         0x1U
#define USART_ERR_TIMEOUT       0x2U
#define USART_ERR_UNDEFINE_CMD  0x3U


CUart::CUart(QObject *parent) :
  QObject(parent)
{
  ash = NULL;
  serial_port = NULL;
  wait_ack_timeout = 0;
  ack_timeout_timer.start(WAIT_ACK_TIMEOUT);
  connect( &ack_timeout_timer,SIGNAL(timeout()),
           this, SLOT(SlotWaitAckTimeout()) );
}

CUart::~CUart( void )
{
  if( NULL != serial_port )
  {
    delete serial_port;
    serial_port = NULL;
  }
}

/**
 * @brief Open : open an uart connection with usb dongle
 * @return TRUE if a valid connection to dongle has been done
 */
bool CUart::Open( void )
{
  bool lo_success = false;

  if( (NULL != serial_port) && (true == serial_port->isOpen()))
  {
    lo_success = true;
  }
  else
  {
      QSerialPortInfo lp_serial_info;

      // search a valid dongle on uart port
      for( int loop=0; loop<QSerialPortInfo::availablePorts().count(); loop++ )
      {
        lp_serial_info = QSerialPortInfo::availablePorts().at(loop);

        if( ((lp_serial_info.vendorIdentifier() == VENDOR_IDENTIFIER) &&
             (lp_serial_info.productIdentifier() == PRODUCT_IDENTIFIER)) ||
            ((lp_serial_info.vendorIdentifier() == FTDI_VENDOR_IDENTIFIER) &&
                       (lp_serial_info.productIdentifier() == FTDI_PRODUCT_IDENTIFIER))||
            ((lp_serial_info.vendorIdentifier() == NCP_VENDOR_IDENTIFIER) &&
                       (lp_serial_info.productIdentifier() == NCP_PRODUCT_IDENTIFIER))
          )
        {
          // create port
          serial_port = new QSerialPort(lp_serial_info);

          LOGGER(logTRACE) << "lp_serial_info port : " << lp_serial_info.portName();

          if( NULL != serial_port )
          {
            lo_success = CheckForEzspUsbNcp();

            // end port loop
            if( true == lo_success )
            {
              loop = QSerialPortInfo::availablePorts().count();
              serial_port->clear();
              // send waiting message
              SendMsg();
            }
            else
            {
              serial_port->close();
              delete serial_port;
              serial_port = NULL;
            }
          }
        }
      }

  }

  return lo_success;
}


/**
 * @brief Close : close uart connection with usb dongle
 */
void CUart::Close( bool i_clear_waiting_msg )
{
  if( NULL != serial_port )
  {
    serial_port->close();
  }
  if( true == i_clear_waiting_msg ){ waiting_msg.clear(); wait_ack_timeout = 0; }
}

/**
 * @brief WriteMsg : write message to uart port for dongle
 * @param i_msg : message to write, uart dongle protocol is not include
 * @return true if message is enqueue without error
 */
bool CUart::WriteMsg(QList<quint8> i_msg )
{
  waiting_msg.enqueue( i_msg );
  SendMsg();
  return true;
}
/**
 * @brief GetBaudrate : return current baudrate in use
 * @return bauderate value
 */
quint32 CUart::GetBaudrate( void )
{
  quint32 lo_bauderate = 0;

  if( NULL != serial_port )
  {
    lo_bauderate = serial_port->baudRate();
  }

  return lo_bauderate;
}

/**
 * @brief GetUartPortName  : return name of current open port
 * @return "" if not valid
 */
QString CUart::GetUartPortName( void )
{
  QString lo_port_name = "";

  if( NULL != serial_port )
  {
    lo_port_name = serial_port->portName();
  }

  return lo_port_name;}

/**
 * @brief SlotSerialPortError : connect to QtSerialPort error signal
 * @param i_error : serial port error
 */
void CUart::SlotSerialPortError(QSerialPort::SerialPortError i_error)
{
  // log msg
  // LOGGER(logTRACE) << "SlotSerialPortError : " << QString::number(i_error);

  switch( i_error )
  {
    case QSerialPort::NotOpenError:
    {
      // This error occurs when an operation is executed that can only be successfully performed if the device is open.
      // emit not open signal
      emit SignalNotOpenError();
    }
    break;

    case QSerialPort::WriteError: // An I/O error occurred while writing the data.
    case QSerialPort::ReadError:  // An I/O error occurred while reading the data.
    case QSerialPort::ResourceError:  // An I/O error occurred when a resource becomes unavailable, e.g. when the device is unexpectedly removed from the system.
    {
      // emit I/O error signal
      emit SignalIOError();
    }
    break;

    case QSerialPort::TimeoutError:	//	A timeout error occurred.
    {
      // emit uknown error
      emit SignalUnknownError();
    }
    break;

    case QSerialPort::UnknownError:	//	An unidentified error occurred.
    {
      // emit uknown error
      emit SignalUnknownError();
    }
    break;
  }
}

/**
 * @brief CUart::SlotSerialPortReadyRead : connect to QtSerialPort readyread signal
 */
void CUart::SlotSerialPortReadyRead( void )
{
  // read incomming data and process it
  QList<quint8> l_in_data = SerialPortReadAll();
  int8_t l_data;

    while( 0 != l_in_data.size() )
    {
      // send incomming data to ash decoder
      QList<quint8> lo_msg;
      if( NULL != ash )
      {
        lo_msg = ash->decode(&l_in_data);
      }


      // send incomming mesage to application
      if( !lo_msg.isEmpty() )
      {
        // assume this is an ack of current command
        if( (EZSP_CB_HANDLER != lo_msg.at(1)) &&
            (0 != wait_ack_timeout)
          )
        {
          waiting_msg.dequeue();
          wait_ack_timeout = 0;
        }

        // send ack
        QByteArray l_msg = ash->AckFrame();
        serial_port->write( l_msg.constData(), l_msg.size() );

        // send next message
        SendMsg();

        emit SignalRxMessage( lo_msg );
      }
      else
      {
        //LOGGER(logTRACE) << "<-- SlotSerialPortReadyRead : lo_msg EMPTY !! ";
      }

      //l_in_data.clear();
      l_in_data.append( SerialPortReadAll() );
    }
}

/**
 * @brief SlotWaitAckTimeout : slot call by timeout timer of a waiting acknowledge
 */
void CUart::SlotWaitAckTimeout( void )
{
  if( 0 != wait_ack_timeout )
  {
    wait_ack_timeout -= 1;

    if( 0 == wait_ack_timeout )
    {
      // acknowledge not arrived -> error
      SlotSerialPortError(QSerialPort::UnknownError);
      // log msg
      LOGGER(logTRACE) << "SlotWaitAckTimeout";
    }
  }
}

/**
 * @brief SendMsg : private function to send next msg of the queue if possible
 */
void CUart::SendMsg( void )
{
  if( !waiting_msg.isEmpty() )
  {
      if( (0 == wait_ack_timeout) &&
          (NULL != serial_port) &&
          (true == serial_port->isOpen())
        )
      {
        QByteArray l_out_msg = ash->DataFrame( waiting_msg.head() );

        // write
        if( l_out_msg.size() != serial_port->write( l_out_msg ) )
        {
          // error
          SlotSerialPortError(QSerialPort::UnknownError);
        }
        else
        {
          // debug
          QString l_debug_msg;
          QList<quint8> l_msg = waiting_msg.head();
          for(int loop=0;loop<l_msg.size();loop++)
          {
              l_debug_msg.append(QString::number(l_msg.at(loop),16).toUpper().rightJustified(2,'0') + " ");
          }
          LOGGER(logTRACE) << "uart SendMsg : " << l_debug_msg;

          // success, wait ack and start timeout timer
          wait_ack_timeout = WAIT_ACK_TIMEOUT_VALUE;
        }
      }
  }
}

/**
 * @brief SerialPortReadAll : usefull function to convert QByteArray from QTserial ReadAll to QList<quint8>
 * @return reading data from serial port
 */
QList<quint8> CUart::SerialPortReadAll( void )
{
  QList<quint8> lo_data;
  QByteArray l_tmp_data = serial_port->readAll();

  // convert to QList
  foreach( quint8 ll_data, l_tmp_data )
  {
    lo_data.append(ll_data);
  }

  return lo_data;
}

/**
 * @brief SerialPortWrite : usefull function to convert writen byte from QList to QBytearray
 * @param i_data data to write
 * @return number of byte writen
 */
quint64 CUart::SerialPortWrite( QList<quint8> i_data )
{
  quint64 lo_write_data;
  QByteArray l_msg;

  foreach( quint8 ll_data, i_data )
  {
    l_msg.append(ll_data);
  }

  lo_write_data = serial_port->write( l_msg );

  return lo_write_data;
}

/**
 * @brief ComputeChecksum : compute dongle checksum of data list
 * @param i_data : data concerned by checksum
 * @return checksum
 */
uint8_t CUart::ComputeChecksum(QList<quint8> *payload)
{
  // la longueur est utilisee dans le calcul du crc
  // rajouter +1 pour l'octet ou sera ecrit le crc
  uint8_t sum = payload->size() + 1;

  for (int i = 0; i < payload->size(); i++)
  {
    sum += payload->at(i);
  }

  uint8_t result = ~sum;
  result += 1;

  return result;
}

/**
 * @brief ByteToAscii : convert 4 LSB bits of byte to ascii character
 * @param byte : byte to convert
 * @return ascii charater
 */
uint8_t CUart::ByteToAscii( uint8_t byte )
{
  if( byte < 0x0AU )
  {
    byte += 0x30;
  }
  else
  {
    byte -= 0x0AU;
    byte += 0x61U;
  }

  return byte;
}

/**
 * @brief AsciiToByte convert ascii character to 4 LSB bits of byte
 * @param l_ascii
 * @return
 */
uint8_t CUart::AsciiToByte( uint8_t l_ascii )
{
  if( (l_ascii > 0x60U) && (l_ascii < 0x67U) )
  {
    l_ascii -= 0x61U;
    l_ascii += 0x0AU;
  }
  else if( (l_ascii > 0x40U) && (l_ascii < 0x47U) )
  {
    l_ascii -= 0x41U;
    l_ascii += 0x0AU;
  }
  else if( (l_ascii > 0x2FU) && (l_ascii < 0x3AU) )
  {
    l_ascii -= 0x30U;
  }
  else
  {
    // par defaut on met une valeur coherente
    l_ascii = 0U;
  }

  return (l_ascii&0x0FU);
}

bool CUart::CheckForEzspUsbNcp( void )
{
  bool lo_success = false;

  if( true == serial_port->open(QIODevice::ReadWrite) )
  {
    bool configured = true;
    configured &= serial_port->setBaudRate(QSerialPort::Baud57600);
    configured &= serial_port->setFlowControl(QSerialPort::SoftwareControl);
    configured &= serial_port->setParity(QSerialPort::NoParity);
    configured &= serial_port->setDataBits(QSerialPort::Data8);
    configured &= serial_port->setStopBits(QSerialPort::OneStop);

    if( true == configured )
    {
      // first send RST frame
      //unsigned char l_rst_frame[] = { 0x1A, 0xC0, 0x38, 0xBC, 0x7E };
      //serial_port->write((const char*)l_rst_frame, 5);

      // do a ash reset
      if( NULL != ash ){ delete ash; }

      ash = new CAsh();
      connect( ash, SIGNAL(info(EAshInfo)), SLOT(SlotAshInfo(EAshInfo)) );

      QByteArray l_msg = ash->resetNCPFrame();
      serial_port->write( l_msg.constData(), l_msg.size() );

      if( true == serial_port->waitForReadyRead(2000) )
      {
        // read buffer
        QList<quint8> in_data = SerialPortReadAll();
        //if( in_data.count() >= 6 )
        {
          /*
          if( (in_data.at(0) == 0x1A) &&
              (in_data.at(1) == 0xC1) &&
              (in_data.at(2) == 0x02) &&
              (in_data.at(3) == 0x0B)
            )
            */
          ash->decode(&in_data);
          if(ash->isConnected())
          {
            lo_success = true;
            // connect error signal
            connect( serial_port, SIGNAL(error(QSerialPort::SerialPortError)),
                     SLOT(SlotSerialPortError(QSerialPort::SerialPortError)) );
            // connect receive byte signal
            connect( serial_port, SIGNAL(readyRead()),
                     SLOT(SlotSerialPortReadyRead()) );


          }
          else
          {
            disconnect( ash, SIGNAL(info(EAshInfo)), this, SLOT(SlotAshInfo(EAshInfo)) );
            delete ash;
            ash = NULL;
          }
        }
      }
    }

    if( false == lo_success ){ serial_port->close(); }
  }

  return lo_success;
}

void  CUart::SlotAshInfo(EAshInfo i_info)
{
  LOGGER(logTRACE) << "SlotAshInfo : " << i_info;

  if( ASH_NACK == i_info )
  {
    // re-send last message
    wait_ack_timeout = 0;
    SendMsg();
  }

}

