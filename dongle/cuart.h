#ifndef CUART_H
#define CUART_H

#include <QObject>
#include <QQueue>
#include <QTimer>

#include "cash.h"

#include <QSerialPort>
#include <QSerialPortInfo>


/**
 * \brief The CUart class :
 *  - implement uart ascii protocol for Zigbee USB Modem Dongle
 *  - manage erreur from QTSerial Class
 *  - manage autodection of USB dongle
 */

class CUart : public QObject
{
  Q_OBJECT
public:
  explicit CUart(QObject *parent = 0);
  ~CUart( void );

  /**
   * @brief Open : open uart connection with usb dongle
   * @return TRUE if a valid connection to dongle has been done
   */
  bool Open( void );

  /**
   * @brief Close : close uart connection with usb dongle
   */
  void Close(bool i_clear_waiting_msg = false );

  /**
   * @brief WriteMsg : write message to uart port for dongle
   * @param i_msg : message to write, uart dongle protocol is not include
   * @return true if message is enqueue without error
   */
  bool WriteMsg( QList<quint8> i_msg );

  /**
   * @brief GetBaudrate : return current baudrate in use
   * @return bauderate value
   */
  quint32 GetBaudrate( void );

  /**
   * @brief GetUartPortName  : return name of current open port
   * @return "" if not valid
   */
  QString GetUartPortName( void );

private:
  QSerialPort *serial_port;
  QQueue<QList<quint8>> waiting_msg;
  QList<quint8> incomming_ascii_msg; /** message currently in reception in ascii */
  QTimer ack_timeout_timer;
  int8_t wait_ack_timeout;  /** timeout in second before emit an error signal due to timeout */
  CAsh *ash;


  /**
   * @brief SendMsg : private function to send next msg of the queue if possible
   */
  void SendMsg( void );

  /**
   * @brief SerialPortReadAll : usefull function to convert QByteArray from QTserial ReadAll to QList<quint8>
   * @return reading data from serial port
   */
  QList<quint8> SerialPortReadAll(void);

  /**
   * @brief SerialPortWrite : usefull function to convert writen byte from QList to QBytearray
   * @param i_data data to write
   * @return number of byte writen
   */
  quint64 SerialPortWrite( QList<quint8> i_data );

  /**
   * @brief ComputeChecksum : compute dongle checksum of data list
   * @param i_data : data concerned by checksum
   * @return checksum
   */
  uint8_t ComputeChecksum(QList<quint8> *payload);

  /**
   * @brief ByteToAscii : convert 4 LSB bits of byte to ascii character
   * @param byte : byte to convert
   * @return ascii charater
   */
   uint8_t ByteToAscii( uint8_t byte );

   /**
    * @brief AsciiToByte convert ascii character to 4 LSB bits of byte
    * @param l_ascii
    * @return
    */
   uint8_t AsciiToByte( uint8_t l_ascii );

   /**
    * @brief try to detect if an Modem dongle in boot mode is present on current open uart.
    * @return true if it's the case
    */
   bool CheckForBootModemDongle( void );

   /**
    * @brief try to detect if an Modem dongle in boot mode is present on current open uart.
    * @return true if it's the case
    */
   bool CheckForModemDongle( void );

   /**
    * @brief try to detect if an XModem boot dongle is present on current open uart.
    * @return true if it's the case
    */
   bool CheckForBootXModemDongle( void );

   bool CheckForEzspUsbNcp( void );


signals:
  /**
   * @brief SignalNotOpenError : This error occurs when an operation is executed that can only be successfully performed if the device is open.
   */
  void SignalNotOpenError(void);

  /**
   * @brief SignalIOError : I/O error signal (dongle remove or have a problem)
   */
  void SignalIOError(void);

  /**
   * @brief SignalUnknownError : An unidentified error occurred.
   */
  void SignalUnknownError(void);

  /**
   * @brief SignalRxCommandError : emit when a receive uart message was not success
   */
  void SignalRxCommandError( void );

  /**
   * @brief SignalRxMessage : emit when a message is ready to process
   */
  void SignalRxMessage( QList<quint8> i_msg );

private slots:
  /**
   * @brief SlotSerialPortError : connect to QtSerialPort error signal
   * @param i_error : serial port error
   */
  void SlotSerialPortError(QSerialPort::SerialPortError i_error);

  /**
   * @brief SlotSerialPortReadyRead : connect to QtSerialPort readyread signal
   */
  void SlotSerialPortReadyRead( void );

  /**
   * @brief SlotWaitAckTimeout : slot call by timeout timer of a waiting acknowledge
   */
  void SlotWaitAckTimeout( void );

  void SlotAshInfo(EAshInfo i_info);
};

#endif // CUART_H
