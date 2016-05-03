/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TCP_WEB_CLIENT_H
#define TCP_WEB_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/random-variable-stream.h"
#include <vector>
namespace ns3 {

class Socket;
class Packet;
typedef struct
{
  double requestStart;
  double requestExecutionTime;
} RequestDataStruct;

/**
 * \ingroup udpecho
 * \brief A Udp Echo client
 *
 * Every packet sent should be returned by the server and received here.
 */
class TcpWebClient : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  TcpWebClient ();

  virtual ~TcpWebClient ();

  /**
   * \brief set the remote address and port
   * \param ip remote IPv4 address
   * \param port remote port
   */
  void SetRemote (Ipv4Address ip, uint16_t port);
  /**
   * \brief set the remote address and port
   * \param ip remote IPv6 address
   * \param port remote port
   */
  void SetRemote (Ipv6Address ip, uint16_t port);
  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetRemote (Address ip, uint16_t port);



  std::vector<RequestDataStruct> getResponseTimes();

protected:
  virtual void DoDispose (void);

private:

  void ConnectionSucceeded (Ptr<Socket> socket);


  void ConnectionFailed (Ptr<Socket> socket);

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Schedule the next packet transmission
   * \param dt time interval between packets.
   */
  void ScheduleTransmit (Time dt);

  void Send (uint32_t requestSize, uint32_t responseSize, Ptr<Socket> socketToSend);
  /**
   * \brief Send a packet
   */
  //void Send (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

  void StartNewServerConnection(bool isPrimary);

  void HandleSuccessfulClose(Ptr<Socket> socket);
  void HandleErrorClose(Ptr<Socket> socket);
  void InitializeModelDistributions();


  uint32_t m_sent; //!< Counter for sent packets
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet
  std::vector<RequestDataStruct> m_responseTimes;
  double m_timeOfLastSentPacket;
  std::vector< Ptr<Socket> > m_primarySockets;
  std::vector< Ptr<Socket> > m_secondarySockets;
  std::vector<uint32_t> m_primarySocketsDataRemaining;
  std::vector<uint32_t> m_secondarySocketsDataRemaining;
  uint32_t m_maxConncurrentSockets;
  uint32_t m_numFilesToFetch;
  uint32_t m_totalPagesToFetch;
  Ptr<EmpiricalRandomVariable> numFilesToFetchGenerator ;
  Ptr<EmpiricalRandomVariable> thinkTimeGenerator;
  Ptr<EmpiricalRandomVariable> primaryRequestSizeGenerator;
  Ptr<EmpiricalRandomVariable> secondaryRequestSizeGenerator;
  Ptr<EmpiricalRandomVariable> primaryResponseSizeGenerator;
  Ptr<EmpiricalRandomVariable> secondaryResponseSizeGenerator;
  Ptr<EmpiricalRandomVariable> totalPagesToFetchGenerator;
  /// Callbacks for tracing the packet Tx events
  TracedCallback<Ptr<const Packet> > m_txTrace;
};

} // namespace ns3

#endif /* UDP_ECHO_CLIENT_H */
