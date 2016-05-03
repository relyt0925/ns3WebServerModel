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
#include "TcpWebClient.h"

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/http-distributions.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpWebClientApplication");

NS_OBJECT_ENSURE_REGISTERED (TcpWebClient);

TypeId
TcpWebClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpWebClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<TcpWebClient> ()
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&TcpWebClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&TcpWebClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
	.AddAttribute("MaxConcurrentSockets","Maximum Number of Concurrent Sockets",
				  UintegerValue(4),MakeUintegerAccessor(&TcpWebClient::m_maxConncurrentSockets),
				  MakeUintegerChecker<uint32_t>())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&TcpWebClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

void TcpWebClient::InitializeModelDistributions(){
	  numFilesToFetchGenerator = CreateObject<EmpiricalRandomVariable> ();
	  thinkTimeGenerator = CreateObject<EmpiricalRandomVariable> ();
	  primaryRequestSizeGenerator= CreateObject<EmpiricalRandomVariable> ();
	  secondaryRequestSizeGenerator= CreateObject<EmpiricalRandomVariable> ();
	  primaryResponseSizeGenerator= CreateObject<EmpiricalRandomVariable> ();
	  secondaryResponseSizeGenerator= CreateObject<EmpiricalRandomVariable> ();
	  totalPagesToFetchGenerator= CreateObject<EmpiricalRandomVariable> ();
	for(uint32_t i=0; i<sizeof(httpDist::consecutivePages)/sizeof(httpDist::intd_t);i++){
		totalPagesToFetchGenerator->CDF(httpDist::consecutivePages[i].i,httpDist::consecutivePages[i].d);
	}
	for(uint32_t i=0; i<sizeof(httpDist::thinkTime)/sizeof(httpDist::dd_t);i++){
		thinkTimeGenerator->CDF(httpDist::thinkTime[i].d1,httpDist::thinkTime[i].d2);
	}
	for(uint32_t i=0; i<sizeof(httpDist::primaryRequest)/sizeof(httpDist::intd_t);i++){
		primaryRequestSizeGenerator->CDF(httpDist::primaryRequest[i].i,httpDist::primaryRequest[i].d);
	}
	for(uint32_t i=0; i<sizeof(httpDist::secondaryRequest)/sizeof(httpDist::intd_t);i++){
		secondaryRequestSizeGenerator->CDF(httpDist::secondaryRequest[i].i,httpDist::secondaryRequest[i].d);
	}
	for(uint32_t i=0; i<sizeof(httpDist::primaryReply)/sizeof(httpDist::intd_t);i++){
		primaryResponseSizeGenerator->CDF(httpDist::primaryReply[i].i,httpDist::primaryReply[i].d);
	}
	for(uint32_t i=0; i<sizeof(httpDist::secondaryReply)/sizeof(httpDist::intd_t);i++){
		secondaryResponseSizeGenerator->CDF(httpDist::secondaryReply[i].i,httpDist::secondaryReply[i].d);
	}
	for(uint32_t i=0; i<sizeof(httpDist::filesPerPage)/sizeof(httpDist::intd_t);i++){
		numFilesToFetchGenerator->CDF(httpDist::filesPerPage[i].i,httpDist::filesPerPage[i].d);
	}
	m_totalPagesToFetch=(uint32_t)totalPagesToFetchGenerator->GetValue();
	//m_totalPagesToFetch=20;
	NS_LOG_FUNCTION("TOTAL PAGES: " << m_totalPagesToFetch);
}

TcpWebClient::TcpWebClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_sendEvent = EventId ();
  InitializeModelDistributions();
  NS_LOG_FUNCTION("FINESHED");
}

TcpWebClient::~TcpWebClient()
{
  NS_LOG_FUNCTION (this);
}

void 
TcpWebClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
TcpWebClient::SetRemote (Ipv4Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = Address (ip);
  m_peerPort = port;
}

void 
TcpWebClient::SetRemote (Ipv6Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = Address (ip);
  m_peerPort = port;
}

void
TcpWebClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
TcpWebClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  StartNewServerConnection(true);
}

void 
TcpWebClient::HandleSuccessfulClose(Ptr<Socket> socket){
	NS_LOG_FUNCTION (this);
}

void
TcpWebClient::HandleErrorClose(Ptr<Socket> socket){
	NS_LOG_FUNCTION (this);
}

void
TcpWebClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  for(uint32_t i=0;i<m_primarySockets.size();i++){
	  m_primarySockets[i]->Close();
	  m_primarySockets[i]->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
	  m_primarySockets[i]=0;
  }
  m_primarySockets.clear();
  for(uint32_t i=0;i<m_secondarySockets.size();i++){
	  m_secondarySockets[i]->Close();
	  m_secondarySockets[i]->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
	  m_secondarySockets[i]=0;
  }
  Simulator::Cancel (m_sendEvent);
}


void 
TcpWebClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  NS_LOG_FUNCTION("PRIMARY: " << m_primarySockets.size());
  NS_LOG_FUNCTION("Secondary: " << m_secondarySockets.size());
  m_sendEvent = Simulator::Schedule (dt, &TcpWebClient::StartNewServerConnection, this, true);
}

void TcpWebClient::StartNewServerConnection(bool isPrimary){

	NS_LOG_FUNCTION (this << isPrimary);
	TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
	Ptr<Socket> newSocket = Socket::CreateSocket (GetNode (), tid);
    // Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
    if (newSocket->GetSocketType () != Socket::NS3_SOCK_STREAM &&
    		newSocket->GetSocketType () != Socket::NS3_SOCK_SEQPACKET)
      {
        NS_FATAL_ERROR ("Using TcpWebClient with an incompatible socket type. "
                        "BulkSend requires SOCK_STREAM or SOCK_SEQPACKET. "
                        "In other words, use TCP instead of UDP.");
      }
    NS_LOG_FUNCTION (this << newSocket);
    newSocket->Bind();
    newSocket->Connect(InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
    newSocket->SetConnectCallback (
              MakeCallback (&TcpWebClient::ConnectionSucceeded, this),
              MakeCallback (&TcpWebClient::ConnectionFailed, this));
    newSocket->SetRecvCallback (MakeCallback (&TcpWebClient::HandleRead, this));
    NS_LOG_FUNCTION ("IS PRIMARY: " << isPrimary);
    if(isPrimary){
    	m_primarySockets.push_back(newSocket);
    	m_primarySocketsDataRemaining.push_back(1);
    }
    else{
    	m_secondarySockets.push_back(newSocket);
    	m_secondarySocketsDataRemaining.push_back(1);
    }
}


void 
TcpWebClient::Send (uint32_t requestSize, uint32_t responseSize, Ptr<Socket> socketToSend)
{
  NS_LOG_FUNCTION (this);

  //NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;
      //
      // If m_dataSize is zero, the client has indicated that it doesn't care
      // about the data itself either by specifying the data size by setting
      // the corresponding attribute or by not calling a SetFill function.  In
      // this case, we don't worry about it either.  But we do allow m_size
      // to have a value different from the (zero) m_dataSize.
      //
  uint8_t * packetData= new uint8_t[requestSize];
  packetData[0]=(requestSize & 0xff000000) >> 24;
  packetData[1]=(requestSize & 0x00ff0000) >> 16;
  packetData[2]=(requestSize & 0x0000ff00) >> 8;
  packetData[3]=(requestSize & 0x000000ff);
  packetData[4]=(responseSize & 0xff000000) >> 24;
  packetData[5]=(responseSize & 0x00ff0000) >> 16;
  packetData[6]=(responseSize & 0x0000ff00) >> 8;
  packetData[7]=(responseSize & 0x000000ff);
  p = Create<Packet> (packetData,requestSize);
  delete[] packetData;
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);
  socketToSend->Send(p);
  NS_LOG_FUNCTION("" << socketToSend << p->GetSize());

  ++m_sent;
/*
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
    */
  //update last sent time
  if(m_primarySockets.size()!=0){
	  if(socketToSend==m_primarySockets[0])
		  m_timeOfLastSentPacket=Simulator::Now().GetSeconds();
  }
}

void
TcpWebClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  uint32_t totalDataReceived=0;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }
      totalDataReceived+=packet->GetSize();
    }
  if(m_primarySockets.size()!=0){
	  if(socket==m_primarySockets[0]){
		  m_primarySocketsDataRemaining[0]-=totalDataReceived;
		  NS_LOG_FUNCTION("TOTAL DATA REMAINING" << m_primarySocketsDataRemaining[0]);
		  if(m_primarySocketsDataRemaining[0]==0){
			  //socket->Close();
			  m_primarySockets.erase(m_primarySockets.begin());
			  m_primarySocketsDataRemaining.erase(m_primarySocketsDataRemaining.begin());
			  //spin up secondary sockets
			  uint32_t numOfSecondarySocketsToSpin=0;
			  if(m_maxConncurrentSockets>m_numFilesToFetch)
				  numOfSecondarySocketsToSpin=m_numFilesToFetch;
			  else
				  numOfSecondarySocketsToSpin=m_maxConncurrentSockets;
			  NS_LOG_FUNCTION("START NUM FILES:" << m_numFilesToFetch);
			  for(uint32_t i=m_secondarySockets.size();i<numOfSecondarySocketsToSpin;i++){
				  StartNewServerConnection(false);
			  }
		  }
	  }
  }
  for(uint32_t i=0;i<m_secondarySockets.size();i++){
	  if(socket==m_secondarySockets[i]){
		  m_secondarySocketsDataRemaining[i]-=totalDataReceived;
		  if(m_secondarySocketsDataRemaining[i]==0){
			  m_numFilesToFetch--;
			  NS_LOG_FUNCTION("NUM FILES" << m_numFilesToFetch);
			  //socket->Close();
			  m_secondarySockets.erase(m_secondarySockets.begin()+i);
			  m_secondarySocketsDataRemaining.erase(m_secondarySocketsDataRemaining.begin()+i);
			  if(m_numFilesToFetch>m_secondarySockets.size() && m_secondarySockets.size()<m_maxConncurrentSockets){
				  StartNewServerConnection(false);
			  }
			  else if(m_numFilesToFetch==0){
				  //YOU HAVE FETCHED ALL FILES, WAIT THINKTIME AND THEN START NEW PRIMARY CONNECTION
				  RequestDataStruct a;
				  a.requestExecutionTime=Simulator::Now().GetSeconds()-m_timeOfLastSentPacket;
				  a.requestStart=m_timeOfLastSentPacket;
				  m_responseTimes.push_back(a);
				  //increase load (if not remove 10
				  double thinkT=thinkTimeGenerator->GetValue()/10;
				  Time thinkTime= Seconds(thinkT);
				  m_totalPagesToFetch--;
				  NS_LOG_FUNCTION("TOT PAGES REM: " << m_totalPagesToFetch);
				  if(m_totalPagesToFetch>0){
					  //thinkTime=Seconds(1.0);
					  ScheduleTransmit(thinkTime);
				  }
			  }
		  }
		  break;
	  }
  }
}

void TcpWebClient::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  uint32_t requestSize;
  uint32_t responseSize;
  if(m_primarySockets.size()!=0){
	  if(socket==m_primarySockets[0]){
		  NS_LOG_FUNCTION("CHECK PASSED");
		  //NS_LOG_FUNCTION (this << socket);
		  //sockets are a match and its a primary socket
		  //sample Distribution
		  //worked with constant values
		  requestSize=8+(uint32_t)primaryRequestSizeGenerator->GetValue();
		  responseSize=8+(uint32_t)primaryResponseSizeGenerator->GetValue();
		  NS_LOG_FUNCTION("PRIM RESPONSE SIZE: " << responseSize);
		  NS_LOG_FUNCTION("PRIM REQUEST SIZE: " << requestSize);
		  m_numFilesToFetch=(uint32_t)numFilesToFetchGenerator->GetValue();
		  m_primarySocketsDataRemaining[0]=responseSize;
		  //SECONDARY ALWAYS 0
		  //NS_LOG_FUNCTION("IN PRIMARY" << m_secondarySockets.size());
	  }
  }
  for(uint32_t i=0;i<m_secondarySockets.size();i++){
	  if(socket==m_secondarySockets[i]){
		  NS_LOG_FUNCTION("SECONDARY CHECK PASSED");
		  //NS_LOG_FUNCTION (this << socket);
		  //sample distribution here as well (Secondary Distribution)
		  requestSize=8+(uint32_t)secondaryRequestSizeGenerator->GetValue();
		  responseSize=8+(uint32_t)secondaryResponseSizeGenerator->GetValue();
		  m_secondarySocketsDataRemaining[i]=responseSize;
	  }
  }
  NS_LOG_FUNCTION("ACT RESPONSE SIZE: " << responseSize);
  NS_LOG_FUNCTION("ACT REQUEST SIZE: " << requestSize);
  Send(requestSize,responseSize,socket);
}

void TcpWebClient::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}


std::vector<RequestDataStruct> TcpWebClient::getResponseTimes(){
	return m_responseTimes;
}

} // Namespace ns3
