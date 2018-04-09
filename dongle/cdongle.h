#ifndef CDONGLE_H
#define CDONGLE_H

#include <QObject>

#include "ezsp_constant.h"
#include "cuart.h"

#include <QMetaType>

/**
 * \brief The CDongle class :
 *  - Manage UART connection at higher level
 *  - Manage EZSP Protocol
 */

class CDongle : public QObject
{
  Q_OBJECT
public:
  explicit CDongle(QObject *parent = 0);
  ~CDongle(void);

  /**
   * @brief IsConnect : check if a dongle is present
   * @return  true if a dongle is correctly connect
   */
  bool IsConnect(void){ return (!fw_version.isEmpty()); }

  /**
   * @brief GetFwVersion : obtain firmware version of currently connect dongle
   * @return firmware version, QList<quint8> is empty if dongle not connected
   */
  QList<quint8> GetFwVersion( void ) { return fw_version; }

  /**
   * @brief ResetUart
   */
  void ResetUart( void );


  /** --- LOW ACCESS --- */

  /**
   * @brief SendEzspCmd : sending ezsp command to dongle : WARNING it's a very low level function
   */
  void SendEzspCmd( EEzspCmd i_cmd, QList<quint8> i_payload = QList<quint8>() );

  CUart *GetUart( void ){ return &uart; }


private:
  /** \brief UART Object to communicate with USB Modem Dongle */
  CUart uart;

  /** \brief firmware versoin of connected dongle */
  QList<quint8> fw_version;


  /**
   * @brief OpenUart : private function to open uart
   */
  void OpenUart( void );

  void EzspProcess( QList<quint8> i_rx_msg );

signals:
  /**
   * @brief SignalDongleConnect :
   * @param i_connect
   */
  void SignalDongleConnect( bool i_connect );

  void SignalMfglibRxHdl( QList<quint8> l_msg );

public slots:
  /**
   * @brief SlotOpenUart : call by timeout timer to open uart connection
   */
  void SlotOpenUart( void );
  /**
   * @brief SlotUartIOError : call when an IO error occurs an uart connection
   */
  void SlotUartIOError( void );

  /**
   * @brief SlotRxMessage : connect to receive messge function of uart
   * @param i_rx_msg : ezsp_mode message to process
   */
  void SlotRxMessage( QList<quint8> i_rx_msg );
};


#endif // CDONGLE_H
