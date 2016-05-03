/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/packet-sink.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"

#include <sstream>
#include <cmath>

#define MAX_BYTES 100000000
#define UDPINTERVAL 0.00320
#define END_TIME 3.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("P3Wifi");

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}


int 
main (int argc, char *argv[])
{
  bool verbose = true;
  std::string queueType="";
  //add command line parameters
  double gridLength;
  int numNodes=10;
  double txPower=900; //in mW
  bool isAODV=false;
  double trafficIntensity=0.1;
  //OfdmRate6Mbps is initail value?
  std::string phyMode ("DsssRate11Mbps");
  //double rss = -80;  // -dBm
  std::string csvFile="";
  CommandLine cmd;
  cmd.AddValue ("gridLength", "Grid Length in meters (gridLength by gridLength grid)", gridLength);
  cmd.AddValue ("numNodes", "Number of wireless Nodes", numNodes);
  cmd.AddValue ("txPower", "Transmit Power of each node", txPower);
  cmd.AddValue("isAODV","Boolean to toggle between AODV and OSLR Routing Protocols (True=AODV)",isAODV);
  cmd.AddValue("trafficIntensity","Trafic intensity on all the nodes",trafficIntensity);
  cmd.AddValue("csv","csv File Name",csvFile);
  cmd.AddValue("verbose","turn on debugging",verbose);
  cmd.Parse (argc,argv);

  //convert to dbm
  double txPowerPrint=txPower;
  txPower=10*log(txPower);
  //std::string fileName="p2Res_LD_"+std::to_string(bottleneckRate)+"_QW_"+std::to_string(queueSize)+"_RW_"
		  //+patch::to_string(recieverWindowSize)+"_RT_";
  //std::string rtt=std::to_string(linkDelay);
  //rtt.replace(rtt.find("."),1,"p");
  //fileName=fileName+rtt+".csv";
  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
		  StringValue (phyMode));



  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
	  wifi.EnableLogComponents();  // Turn on all Wifi logging
  //configure wifi parameters
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
		  "DataMode",StringValue (phyMode),"ControlMode",StringValue (phyMode));

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  //in grid default one
  wifiPhy.Set ("RxGain", DoubleValue (-10) );
  //SET TX POWER
  wifiPhy.Set("TxPowerStart",DoubleValue(txPower));
  wifiPhy.Set("TxPowerEnd",DoubleValue(txPower));
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");


  //add the channel
  wifiPhy.SetChannel (wifiChannel.Create());
  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
  wifiMac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, wifiStaNodes);


  //set mobility properly
  int64_t streamIndex = 0; // used to get consistent mobility across scenarios
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  std::string xAndYRandDist="ns3::UniformRandomVariable[Min=0.0|Max="+patch::to_string(gridLength)+"]";
  //"ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"
  pos.Set ("X", StringValue (xAndYRandDist));
  pos.Set ("Y", StringValue (xAndYRandDist));
  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator>();
  streamIndex += taPositionAlloc->AssignStreams(streamIndex);
  MobilityHelper mobility;
  mobility.SetPositionAllocator(taPositionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiStaNodes);

  //configure stack properly
  InternetStackHelper stack;
  //Set routing protocol
  Ipv4ListRoutingHelper list;
  if(isAODV){
	  AodvHelper aodv;
	  list.Add (aodv, 100);
  }
  else{
	  OlsrHelper olsr;
	  list.Add (olsr, 100);
  }
  stack.SetRoutingHelper(list);
  stack.Install(wifiStaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.0.0", "255.255.0.0");
  Ipv4InterfaceContainer staInterface;
  staInterface =address.Assign(devices);

  //RANDOMLY CHOOSE START AND END NODE FOR UDP
  //std::time(NULL)
  //constant to test
  std::srand(3);
  ApplicationContainer sourceApps;
  ApplicationContainer sinkApps;
  uint udpEchoPort=9;
  uint videoPacketSize=1316;
  for(uint32_t i=0;i<wifiStaNodes.GetN();i++){
	  bool validPeer=false;
	  uint32_t randPeer;
	  while(!validPeer){
		  randPeer=rand()%numNodes;
		  if(randPeer!=i)
			  validPeer=true;
	  }
	  //SET THE ON OFF APPS
	  OnOffHelper source ("ns3::UdpSocketFactory",InetSocketAddress(staInterface.GetAddress(randPeer),udpEchoPort));
	  double offTime= 1.0-trafficIntensity;
	  std::string onTimeString= "ns3::ConstantRandomVariable[Constant="+patch::to_string(trafficIntensity)+"]";
	  std::string offTimeString="ns3::ConstantRandomVariable[Constant="+patch::to_string(offTime)+"]";
	  source.SetAttribute("OnTime", StringValue(onTimeString));
	  source.SetAttribute ("OffTime", StringValue(offTimeString));
	  source.SetAttribute ("PacketSize", UintegerValue (videoPacketSize));
	  source.SetAttribute ("DataRate",StringValue ("3Mbps"));
	  ApplicationContainer videoApp= source.Install(wifiStaNodes.Get(i));
	  videoApp.Start(Seconds(0.0));
	  videoApp.Stop(Seconds(END_TIME));
	  sourceApps.Add(videoApp);

	  PacketSinkHelper server("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),udpEchoPort));
	  ApplicationContainer videoSink = server.Install(wifiStaNodes.Get(randPeer));
	  videoSink.Start(Seconds(0.0));
	  videoSink.Stop(Seconds(END_TIME));
	  sinkApps.Add(videoSink);
	  //avoid port collisions if on same node
	  udpEchoPort++;
  }
  //populate routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  Simulator::Stop(Seconds(END_TIME));
  Simulator::Run();
  uint32_t totalBytesSent=0;
  uint32_t totalBytesReceived=0;
  if(csvFile==""){
	  for(uint32_t i=0;i<sinkApps.GetN();i++){
		  Ptr<PacketSink> udpSink = DynamicCast<PacketSink>(sinkApps.Get(i));
		  uint32_t receivedBytes=udpSink->GetTotalRx();
		  Ptr<OnOffApplication> onoff1 = DynamicCast<OnOffApplication>(sourceApps.Get(i));
		  uint32_t bytesSent=onoff1->getTotalBytesSent();
		  //double streamUtilization=static_cast<double>(receivedBytes)/static_cast<double>(bytesSent);
		  //std::cout << "UDP Stream " << i << " Utilization is " << (streamUtilization*100) << std::endl;
		  totalBytesSent+=bytesSent;
		  totalBytesReceived+=receivedBytes;
	  }
	  std::cout << "gridLength," << gridLength << ",";
	  std::cout << "numNodes," << numNodes << ",";
	  std::cout << "txPower," << txPowerPrint << ",";
	  std::cout << "isAODV," << isAODV << ",";
	  std::cout << "trafficIntensity," << trafficIntensity << ",";
	  if(totalBytesReceived==0)
		  std::cout << "netEfficiency," << "0.0" << std::endl;
	  else
		  std::cout << "netEfficiency," << (((double)totalBytesReceived/(double)totalBytesSent)*100.0) << std::endl;
  }
  else{
	  std::ofstream fileOUT(csvFile.c_str(), std::ios::app);
	  for(uint32_t i=0;i<sinkApps.GetN();i++){
		  Ptr<PacketSink> udpSink = DynamicCast<PacketSink>(sinkApps.Get(i));
		  uint32_t receivedBytes=udpSink->GetTotalRx();
		  Ptr<OnOffApplication> onoff1 = DynamicCast<OnOffApplication>(sourceApps.Get(i));
		  uint32_t bytesSent=onoff1->getTotalBytesSent();
		  //double streamUtilization=static_cast<double>(receivedBytes)/static_cast<double>(bytesSent);
		  //std::cout << "UDP Stream " << i << " Utilization is " << (streamUtilization*100) << std::endl;
		  totalBytesSent+=bytesSent;
		  totalBytesReceived+=receivedBytes;
	  }
	  fileOUT <<  gridLength << ",";
	  fileOUT << numNodes << ",";
	  fileOUT << txPowerPrint << ",";
	  fileOUT << isAODV << ",";
	  fileOUT << trafficIntensity << ",";
	  if(totalBytesReceived==0)
		  fileOUT << "0" << std::endl;
	  else
		  fileOUT << (((double)totalBytesReceived/(double)totalBytesSent)*100.0) << std::endl;
  }
  Simulator::Destroy();
  return 0;
}

