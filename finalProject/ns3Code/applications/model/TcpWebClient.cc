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
/**
 * Model for Web Browser Client. Implements behavior described in the Jeffay
 * "Tuning RED for Web Traffic" Paper. Simulates real human behavior
 * @author Tyler Lisowski
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

//define UNIQUE component name and ensured its registered
NS_LOG_COMPONENT_DEFINE ("TcpWebClientApplication");

NS_OBJECT_ENSURE_REGISTERED (TcpWebClient);

//set NS3 attributes to allow access to have them set dynamically
//also sets default values
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

/**
 * InitializeModelDistributions- reads distributions from provided file and sets CDF curves
 */
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
	//get number of pages to be fetched by this browser instance by sampling CDF
	m_totalPagesToFetch=(uint32_t)totalPagesToFetchGenerator->GetValue();
	//NS_LOG_FUNCTION("TOTAL PAGES: " << m_totalPagesToFetch);
}

/**
 * Constructor- initializes class variables
 */
TcpWebClient::TcpWebClient ()
{
  m_sent = 0;
  m_sendEvent = EventId ();
  InitializeModelDistributions();
}

TcpWebClient::~TcpWebClient()
{
}
//Sets remote IP address and port for different address types
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

//Inherited method; just logs and does normal method (really no need for it)
void
TcpWebClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

//Sets up application and schedules first events (HAS TO SCHEDULE EVENTS)
void 
TcpWebClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  StartNewServerConnection(true);
}

//callback for when TCP connection successfully closed
void 
TcpWebClient::HandleSuccessfulClose(Ptr<Socket> socket){
	NS_LOG_FUNCTION (this);
}

//callback for when error occurs when closing TCP connection
void
TcpWebClient::HandleErrorClose(Ptr<Socket> socket){
	NS_LOG_FUNCTION (this);
}

//Stops application and ensures all resources are cleared
void
TcpWebClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  //Close every socket and make sure the receive callbacks are null
  for(uint32_t i=0;i<m_primarySockets.size();i++){
	  m_primarySockets[i]->Close();
	  m_primarySockets[i]->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
	  m_primarySockets[i]=0;
  }
  m_primarySockets.clear();
  //Do the same for the secondary sockets
  for(uint32_t i=0;i<m_secondarySockets.size();i++){
	  m_secondarySockets[i]->Close();
	  m_secondarySockets[i]->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
	  m_secondarySockets[i]=0;
  }
  //TODO: HAVE LIST OF FUTURE EVENTS TO CANCEL
  //cancel any events (NEED TO KEEP LIST OF SCHEDULED EVENTS NOT DONE HERE)
  Simulator::Cancel(m_sendEvent);
}

//Schedules callback that will initate new primary connection
void 
TcpWebClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  NS_LOG_FUNCTION("PRIMARY: " << m_primarySockets.size());
  NS_LOG_FUNCTION("Secondary: " << m_secondarySockets.size());
  m_sendEvent = Simulator::Schedule (dt, &TcpWebClient::StartNewServerConnection, this, true);
}

//Starts a new connection (either primary or secondary) and simulates sending request
void TcpWebClient::StartNewServerConnection(bool isPrimary){

	//NS_LOG_FUNCTION (this << isPrimary);
	//create new TCP socket
	TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
	Ptr<Socket> newSocket = Socket::CreateSocket (GetNode(), tid);
    // Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
    if (newSocket->GetSocketType () != Socket::NS3_SOCK_STREAM &&
    		newSocket->GetSocketType () != Socket::NS3_SOCK_SEQPACKET)
      {
        NS_FATAL_ERROR ("Using TcpWebClient with an incompatible socket type. "
                        "BulkSend requires SOCK_STREAM or SOCK_SEQPACKET. "
                        "In other words, use TCP instead of UDP.");
      }
    NS_LOG_FUNCTION (this << newSocket);
    //reserve port on the machine
    newSocket->Bind();
    //initiate connection to remote machine
    newSocket->Connect(InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
    //set callbacks to handle successful exchange or failure
    newSocket->SetConnectCallback (
              MakeCallback (&TcpWebClient::ConnectionSucceeded, this),
              MakeCallback (&TcpWebClient::ConnectionFailed, this));
    //set callback for when data is received over channel
    newSocket->SetRecvCallback (MakeCallback (&TcpWebClient::HandleRead, this));
    NS_LOG_FUNCTION ("IS PRIMARY: " << isPrimary);
    //push new socket back in proper category
    if(isPrimary){
    	m_primarySockets.push_back(newSocket);
    	m_primarySocketsDataRemaining.push_back(1);
    }
    else{
    	m_secondarySockets.push_back(newSocket);
    	m_secondarySocketsDataRemaining.push_back(1);
    }
}

//sends data over the TCP socket with the specified request size and response size
//response size used by Web server to set request size
void 
TcpWebClient::Send (uint32_t requestSize, uint32_t responseSize, Ptr<Socket> socketToSend)
{
  NS_LOG_FUNCTION (this);

  //NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;

  //allocates amount of data with reques size (in bytes)
  uint8_t * packetData= new uint8_t[requestSize];
  //puts request size in first 4 bytes
  packetData[0]=(requestSize & 0xff000000) >> 24;
  packetData[1]=(requestSize & 0x00ff0000) >> 16;
  packetData[2]=(requestSize & 0x0000ff00) >> 8;
  packetData[3]=(requestSize & 0x000000ff);
  //puts response size in next 4 bytes
  packetData[4]=(responseSize & 0xff000000) >> 24;
  packetData[5]=(responseSize & 0x00ff0000) >> 16;
  packetData[6]=(responseSize & 0x0000ff00) >> 8;
  packetData[7]=(responseSize & 0x000000ff);
  //creates a packet with the specified data
  p = Create<Packet> (packetData,requestSize);
  //clear packetData since packet created
  delete[] packetData;
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);
  //send the packet
  socketToSend->Send(p);
  //NS_LOG_FUNCTION("" << socketToSend << p->GetSize());
  //increment the number of packets that have been sent
  ++m_sent;
  //update last sent time (to calculate response time)
  if(m_primarySockets.size()!=0){
	  if(socketToSend==m_primarySockets[0])
		  m_timeOfLastSentPacket=Simulator::Now().GetSeconds();
  }
}
//callback for handling reading data out of the socket
void
TcpWebClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  uint32_t totalDataReceived=0;
  //JUST receive data from the socket (dont need to store any of it at all)
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
  //check to see if all the data has been received for the socket (response fully sent)
  if(m_primarySockets.size()!=0){
	  //if primary socket exists, there is no secondary connections and data has to be from it
	  if(socket==m_primarySockets[0]){
		  m_primarySocketsDataRemaining[0]-=totalDataReceived;
		  NS_LOG_FUNCTION("TOTAL DATA REMAINING" << m_primarySocketsDataRemaining[0]);
		  //if received all data, spin off concurrent connections to fetch web objects
		  if(m_primarySocketsDataRemaining[0]==0){
			  //socket->Close(); (CAUSED ERRORS)
			  //erase data for primary socket
			  m_primarySockets.erase(m_primarySockets.begin());
			  m_primarySocketsDataRemaining.erase(m_primarySocketsDataRemaining.begin());
			  //spin up secondary sockets
			  uint32_t numOfSecondarySocketsToSpin=0;
			  if(m_maxConncurrentSockets>m_numFilesToFetch)
				  numOfSecondarySocketsToSpin=m_numFilesToFetch;
			  else
				  numOfSecondarySocketsToSpin=m_maxConncurrentSockets;
			  NS_LOG_FUNCTION("START NUM FILES:" << m_numFilesToFetch);
			  //spin off each new connection
			  for(uint32_t i=m_secondarySockets.size();i<numOfSecondarySocketsToSpin;i++){
				  StartNewServerConnection(false);
			  }
		  }
	  }
  }
  //otherwise check to see which secondary socket data is from
  for(uint32_t i=0;i<m_secondarySockets.size();i++){
	  if(socket==m_secondarySockets[i]){
		  m_secondarySocketsDataRemaining[i]-=totalDataReceived;
		  //if all data received, see if need to spin new connection to get another web object
		  if(m_secondarySocketsDataRemaining[i]==0){
			  m_numFilesToFetch--;
			  NS_LOG_FUNCTION("NUM FILES" << m_numFilesToFetch);
			  //socket->Close();
			  //clean up data around socket
			  m_secondarySockets.erase(m_secondarySockets.begin()+i);
			  m_secondarySocketsDataRemaining.erase(m_secondarySocketsDataRemaining.begin()+i);
			  //spin up new connection if not all objects have been fetched or actively being fetched
			  if(m_numFilesToFetch>m_secondarySockets.size() && m_secondarySockets.size()<m_maxConncurrentSockets){
				  StartNewServerConnection(false);
			  }
			  //otherwise wait think time and schedule new connection
			  else if(m_numFilesToFetch==0){
				  //YOU HAVE FETCHED ALL FILES, WAIT THINKTIME AND THEN START NEW PRIMARY CONNECTION
				  RequestDataStruct a;
				  //calculate response time
				  a.requestExecutionTime=Simulator::Now().GetSeconds()-m_timeOfLastSentPacket;
				  a.requestStart=m_timeOfLastSentPacket;
				  m_responseTimes.push_back(a);
				  //think time decreased by a factor of 10 (as in experiment)
				  double thinkT=thinkTimeGenerator->GetValue()/10;
				  Time thinkTime= Seconds(thinkT);
				  //decrement number of pages that need to be fetched
				  m_totalPagesToFetch--;
				  NS_LOG_FUNCTION("TOT PAGES REM: " << m_totalPagesToFetch);
				  if(m_totalPagesToFetch>0){
					  //wait think time and schedule next transmit
					  ScheduleTransmit(thinkTime);
				  }
			  }
		  }
		  break;
	  }
  }
}

//callback for when the connection succeeds
//sets up the response and request size for the sockets
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
  //schedule transmit of the request over the socket
  Send(requestSize,responseSize,socket);
}

//callback for when TCP connection fails
void TcpWebClient::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

//returns response time tracker for every request
std::vector<RequestDataStruct> TcpWebClient::getResponseTimes(){
	return m_responseTimes;
}

} // Namespace ns3
