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
 *
 * Author:  Tom Henderson (tomhend@u.washington.edu)
 */
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "TcpWebServer.h"
#include "ns3/uinteger.h"

namespace ns3 {
//unique component name
NS_LOG_COMPONENT_DEFINE ("TcpWebServerApplication");

NS_OBJECT_ENSURE_REGISTERED (TcpWebServer);

TypeId
TcpWebServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpWebServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<TcpWebServer> ()
    .AddAttribute ("Local",
                   "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&TcpWebServer::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol",
                   "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&TcpWebServer::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Rx",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&TcpWebServer::m_rxTrace),
                     "ns3::Packet::AddressTracedCallback")
	.AddTraceSource ("Tx", "A new packet is created and is sent",
					 MakeTraceSourceAccessor (&TcpWebServer::m_txTrace),
					 "ns3::Packet::TracedCallback")
	;
  return tid;
}

TcpWebServer::TcpWebServer ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_totalRx = 0;
  m_totalDataSent=0;
}

TcpWebServer::~TcpWebServer()
{
  NS_LOG_FUNCTION (this);
}

uint32_t TcpWebServer::GetTotalRx () const
{
  NS_LOG_FUNCTION (this);
  return m_totalRx;
}

Ptr<Socket>
TcpWebServer::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

//this returns all the accepted sockets
std::vector<Ptr<Socket> >
TcpWebServer::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}

//called when simulation ends
void TcpWebServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socketList.clear ();

  // chain up
  Application::DoDispose ();
}


// Application Methods
void TcpWebServer::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  // Create the socket and bind and listen
  if (!m_socket)
    {
	  //create and listen on port
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->Bind (m_local);
      m_socket->Listen ();
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
            }
        }
    }
  //set receive callback
  m_socket->SetRecvCallback (MakeCallback (&TcpWebServer::HandleRead, this));
  //set proper accept callback
  m_socket->SetAcceptCallback (
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&TcpWebServer::HandleAccept, this));
  //set proper close callback
  m_socket->SetCloseCallbacks (
    MakeCallback (&TcpWebServer::HandlePeerClose, this),
    MakeCallback (&TcpWebServer::HandlePeerError, this));
}

void TcpWebServer::StopApplication ()     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  if (m_socket)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  for(uint32_t i=0;i<m_socketList.size();i++){
	  m_socketList[i]->Close();
  }
  m_socketList.clear();
}

void TcpWebServer::HandleRead (Ptr<Socket> socket)
{
  //NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      m_totalRx += packet->GetSize ();
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4 ()
                       << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << Inet6SocketAddress::ConvertFrom(from).GetIpv6 ()
                       << " port " << Inet6SocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }


      m_rxTrace (packet, from);


     // Ptr<Packet> p = Create<Packet> (responseSize);
          //socket->Send (p);
      //m_txTrace (p);
      //socket->SendTo (p, 0, from);
      //socket->Close();
      //check which socket data received from
      for(uint32_t i=0;i<m_socketList.size();i++){
    	  if(socket==m_socketList[i]){
    		  //append packet data
    		  m_inTransitPackets[i]->AddAtEnd(packet);
    		  //if received first 8 bytes can get request size (seems to constantly get it every time data received
    		  //TODO: can be optimized by only getting it once (check if m_totalRequestSize initialized)
    		  if(m_inTransitPackets[i]->GetSize()-packet->GetSize()<8 && m_inTransitPackets[i]->GetSize()>=8){
    			  uint8_t* packetData= new uint8_t[m_inTransitPackets[i]->GetSize()];
    			  packet->CopyData(packetData,m_inTransitPackets[i]->GetSize());
    			  uint32_t requestSize= ((uint32_t)packetData[0]) << 24;
    			  requestSize= (requestSize + ((uint32_t)packetData[1]) ) << 16;
    			  requestSize= (requestSize + ((uint32_t)packetData[2]) ) << 8;
    			  requestSize= requestSize + ((uint32_t)packetData[3]);
    			  NS_LOG_FUNCTION("INIT PACKET REQUEST SIZE" << requestSize);
    			  m_totalRequestSize[i]=requestSize;
    			  delete[] packetData;
    		  }
    		  //then check to see if total data received
    		  if(m_totalRequestSize[i]==m_inTransitPackets[i]->GetSize()){
    			  //if so send response data
    			  NS_LOG_FUNCTION("CompleteResponseSize:" << m_inTransitPackets[i]->GetSize());
    			  uint8_t* packetData= new uint8_t[m_inTransitPackets[i]->GetSize()];
    			  m_inTransitPackets[i]->CopyData(packetData,m_inTransitPackets[i]->GetSize());
    			  //get total response size (next 4 bytes)
    			  uint32_t responseSize= ((uint32_t)packetData[4]) << 24;
    			  responseSize= responseSize + (((uint32_t)packetData[5])  << 16);
    			  responseSize= responseSize + (((uint32_t)packetData[6])  << 8);
    			  responseSize= responseSize + ((uint32_t)packetData[7]);
    			  //NS_LOG_FUNCTION("Response Size:" << responseSize);
    			  //create the packet (size is response size and send)
    			  Ptr<Packet> p = Create<Packet>(responseSize);
    			  socket->Send(p);
    			  socket->Close();
    			  //NS_LOG_INFO("DONEEEEE WITH PACKETTSSSS");
    			  //clear socket since we are finished sending data
    			  m_inTransitPackets.erase(m_inTransitPackets.begin()+i);
    			  m_totalRequestSize.erase(m_totalRequestSize.begin()+i);
    			  m_socketList.erase(m_socketList.begin()+i);
    		  }
    		  break;
    	  }
      }
    }
}

//callback if peer closes the tcp socket
void TcpWebServer::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
//callback for error on TCP socket
void TcpWebServer::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

//callback to handle a client connection
void TcpWebServer::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  //sets receive callback for the newly accepted socket
  s->SetRecvCallback (MakeCallback (&TcpWebServer::HandleRead, this));
  //add socket to active socket list and initialize total request data and request size
  //inTransitPackets acts as buffer to hold intermidary data until request all comes in
  //total Request size is used to know when all packet data arrives
  m_socketList.push_back (s);
  m_inTransitPackets.push_back(Create<Packet>());
  m_totalRequestSize.push_back(500000000);
}



} // Namespace ns3
