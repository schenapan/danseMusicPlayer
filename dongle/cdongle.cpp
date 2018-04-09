#include "cdongle.h"
#include "log.h"

#define OPEN_UART_TIMEOUT 4000 // timeout pour re-tenter d'ouvrir une connection UART

CDongle::CDongle(QObject *parent) :
  QObject(parent)
{
  /** open uart connection */
  // start one shot timer for testing to open later
  QTimer::singleShot(200, this, SLOT(SlotOpenUart()));

  /** connect to error message */
  connect( &uart, SIGNAL(SignalIOError()),
           this, SLOT(SlotUartIOError()));
  connect( &uart, SIGNAL(SignalUnknownError()),
           this, SLOT(SlotUartIOError()));

  /** connect to receive message */
  connect( &uart, SIGNAL(SignalRxMessage(QList<quint8>)),
           this, SLOT(SlotRxMessage(QList<quint8>)) );
}

CDongle::~CDongle(void)
{
  uart.Close();
}

/**
 * @brief OpenUart : private function to open uart
 */
void CDongle::OpenUart( void )
{
  fw_version.clear();
  if( false == uart.Open() )
  {
    // log msg
    LOGGER(logTRACE) << "OpenUart FAILED";

    // start one shot timer for testing to open later
    QTimer::singleShot(OPEN_UART_TIMEOUT, this, SLOT(SlotOpenUart()));
  }
  else
  {
    // log msg
    LOGGER(logTRACE) << "OpenUart SUCCESS";

    // first configure stack and verify ezsp protocol
    QList<quint8> l_data;
    l_data.append(6);
    SendEzspCmd( EZSP_VERSION, l_data );
  }
}

/**
 * @brief SlotOpenUart : call by timeout timer to open uart connection
 */
void CDongle::SlotOpenUart( void )
{
  OpenUart();
}

/**
 * @brief SlotUartIOError
 */
void CDongle::SlotUartIOError( void )
{
  uart.Close();
  fw_version.clear();
  emit SignalDongleConnect( false );

  OpenUart();
}

/**
 * @brief SlotRxMessage : connect to receive messge function of uart
 * @param i_rx_msg : ezsp_mode message to process
 */
void CDongle::SlotRxMessage(QList<quint8> i_rx_msg )
{
    QString l_str;

    foreach(quint8 l_data, i_rx_msg )
    {
      l_str.append( QString::number(l_data,16).toUpper().rightJustified(2,'0') + " ");
    }

    LOGGER(logTRACE) << "SlotRxMessage  : " +  l_str;

    EzspProcess( i_rx_msg );

}

void CDongle::EzspProcess( QList<quint8> i_rx_msg )
{
  switch( (quint8)(i_rx_msg.at(2)) )
  {
    case EZSP_VERSION:
    {
        // check protocol version
        if( 6 == i_rx_msg.at(3) )
        {
          // check if first connection
          if( 0 == fw_version.size() )
          {
            fw_version.append(i_rx_msg.at(3));
            fw_version.append(i_rx_msg.at(4));
            emit SignalDongleConnect( true );
          }
        }
    }
    break;

    case EZSP_MFGLIB_RX_HANDLER:
    {
      // emit signal
      emit SignalMfglibRxHdl( i_rx_msg.mid(3) );
    }
    break;
  }
}


void CDongle::ResetUart( void )
{
  uart.Close( true );
  fw_version.clear();

  emit SignalDongleConnect( false );

  OpenUart();
}


/** --- LOW ACCESS --- */

/**
 * @brief SendEzspCmd : sending ezsp command to dongle : WARNING it's a very low level function
 */
void CDongle::SendEzspCmd(EEzspCmd i_cmd, QList<quint8> i_payload )
{
  QList<quint8> l_msg;
  l_msg.append(i_cmd);
  l_msg.append(i_payload);

  uart.WriteMsg( l_msg );
}
