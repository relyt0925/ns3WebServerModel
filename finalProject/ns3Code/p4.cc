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
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include <string>
#include <fstream>
#include <vector>
#include <numeric>


#define END_TIME 2000.0
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("P4Example");

int 
main (int argc, char *argv[])
{
  bool verbose = false;
  double minTh=5;
  double maxTh=15;
  uint32_t numSourceFlows=2;
  double weightFactor=.002;
  double maxDropProb=50;
  uint32_t queueSize=640000;
  uint64_t bottleneckRate=100;
  uint32_t recieverWindowSize=32000;
  double linkDelay=.0005;
  std::string queueType="";
  bool isDropTail=true;
  uint32_t numNodes=1;
  std::string csv="";
  //add command line parameters
  CommandLine cmd;
  cmd.AddValue ("minTh", "Queue length threshold for triggering probabilistic drops", minTh);
  cmd.AddValue ("maxTh", "Queue length threshold for triggering forced drops.", maxTh);
  cmd.AddValue ("weightFactor", "Weighting factor for the average queue length computation.",weightFactor);
  cmd.AddValue ("maxDropProb", "The maximum probability of performing an early drop as percent (50=50%).",maxDropProb);
  cmd.AddValue ("queueSize", "Queue Size at Bottleneck Link", queueSize);
  cmd.AddValue ("verbose", "Enable Informational Logging", verbose);
  cmd.AddValue("nFlows","Number of Flows on one node from source -> Receiver",numSourceFlows);
  cmd.AddValue("isDropTail","Boolean to toggle between Droptail and Red (True=DropTail)",isDropTail);
  cmd.AddValue("bottleneckRate","Rate of bottleneckLink in Mbps",bottleneckRate);
  cmd.AddValue ("receiverWindowSize", "TCP Advertised Reciever Window Size", recieverWindowSize);
  cmd.AddValue("linkDelays","Delays of all links in system in seconds",linkDelay);
  cmd.AddValue("nNodes","Number of Nodes in the Simulation",numNodes);
  cmd.AddValue("csv","Csv filename for run",csv);
  cmd.Parse (argc,argv);


  //get in terms of Mbps
  bottleneckRate*=1000000;

  //all values are inital defaults right now
  if(isDropTail){
	  queueType="ns3::DropTailQueue";
	  Config::SetDefault("ns3::DropTailQueue::Mode",EnumValue(ns3::DropTailQueue::QUEUE_MODE_BYTES));
	  Config::SetDefault("ns3::DropTailQueue::MaxBytes",UintegerValue(queueSize));

  }
  else{
	  queueType="ns3::RedQueue";
	  Config::SetDefault("ns3::RedQueue::Mode", EnumValue(ns3::RedQueue::QUEUE_MODE_BYTES));
	  Config::SetDefault("ns3::RedQueue::MinTh", DoubleValue(minTh));
	  Config::SetDefault("ns3::RedQueue::MaxTh",DoubleValue(maxTh));
	  Config::SetDefault("ns3::RedQueue::QueueLimit",UintegerValue(queueSize));
	  Config::SetDefault("ns3::RedQueue::QW",DoubleValue(weightFactor));
	  Config::SetDefault("ns3::RedQueue::LInterm",DoubleValue(maxDropProb));
  }
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(ns3::TcpNewReno::GetTypeId()));
  //Make sure packets lost bc of queues along the way not at hosts
  Config::SetDefault("ns3::TcpSocket::SndBufSize",UintegerValue(0xffffffff));
  Config::SetDefault("ns3::UdpSocket::RcvBufSize",UintegerValue(recieverWindowSize));
  Config::SetDefault ("ns3::TcpSocketBase::WindowScaling", BooleanValue (false));

  if (verbose)
  {
	  LogComponentEnable ("TcpWebClientApplication", LOG_LEVEL_FUNCTION);
	  LogComponentEnable ("TcpWebServerApplication", LOG_LEVEL_FUNCTION);
  }

  //type of links in the topology
  PointToPointHelper tenMbpsLink;
  tenMbpsLink.SetDeviceAttribute ("DataRate", StringValue("10Mbps"));
  tenMbpsLink.SetChannelAttribute ("Delay", TimeValue(Seconds(linkDelay)));
  tenMbpsLink.SetQueue(queueType);
  PointToPointHelper hundredMbpsLink;
  hundredMbpsLink.SetDeviceAttribute("DataRate",StringValue("100Mbps"));
  hundredMbpsLink.SetChannelAttribute("Delay",TimeValue(Seconds(linkDelay)));
  hundredMbpsLink.SetQueue(queueType);
  PointToPointHelper bottleNeckLink;
  bottleNeckLink.SetDeviceAttribute("DataRate",DataRateValue(DataRate(bottleneckRate)));
  bottleNeckLink.SetChannelAttribute("Delay",TimeValue(Seconds(linkDelay)));
  bottleNeckLink.SetQueue(queueType);


  //note lwo leftmost arguments can just make one coherent pipe (with a forwarding spot)
  //PointToPointDumbbellHelper d(numSourceFlows,tenMbpsLink,1,hundredMbpsLink,hundredMbpsLink);
  PointToPointDumbbellHelper d(numNodes,tenMbpsLink,1,hundredMbpsLink,hundredMbpsLink);
  InternetStackHelper stack;
  d.InstallStack(stack);
  // Assign IP Addresses
  d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.0.0", "255.255.255.0"),Ipv4AddressHelper ("10.3.0.0", "255.255.255.0"),
  Ipv4AddressHelper ("10.5.0.0", "255.255.255.0"));

  //note 2nd arg and last arg form pipe to other side
  //PointToPointDumbbellHelper serverSide(1,hundredMbpsLink,numSourceFlows,tenMbpsLink,hundredMbpsLink);
  PointToPointDumbbellHelper serverSide(1,hundredMbpsLink,numNodes,tenMbpsLink,hundredMbpsLink);

  serverSide.InstallStack(stack);
  serverSide.AssignIpv4Addresses (Ipv4AddressHelper ("10.7.0.0", "255.255.255.0"),Ipv4AddressHelper ("10.9.0.0", "255.255.255.0"),
  Ipv4AddressHelper ("10.11.0.0", "255.255.255.0"));

  NodeContainer nodes;
  nodes.Add(d.GetRight(0));
  nodes.Add(serverSide.GetLeft(0));
  NetDeviceContainer devices;
  devices = bottleNeckLink.Install(nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.6.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  //create sink to get packets
  uint16_t tcpStartSinkPort = 80;
  ApplicationContainer sinkApps;
  //std::cout << "SETTING SERVERS " << std::endl;
  for(uint32_t i=0;i<serverSide.RightCount();i++){
	  TcpWebServerHelper server("ns3::TcpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),tcpStartSinkPort));
	  ApplicationContainer sinkApp = server.Install(serverSide.GetRight(i));
	  sinkApp.Start(Seconds(0.0));
	  sinkApp.Stop(Seconds (END_TIME));
	  sinkApps.Add(sinkApp);
  }
  //std::cout << "DONE SETTING SERVERS " << std::endl;
  ApplicationContainer sourceApps;
  //install multiple apps on the one machine (each w/ different source ports)
  RngSeedManager::SetSeed(11223344);
  Ptr<UniformRandomVariable> randGenerator = CreateObject<UniformRandomVariable>();
  randGenerator->SetAttribute("Stream",IntegerValue(6110));
  randGenerator->SetAttribute("Min",DoubleValue(0.0));
  randGenerator->SetAttribute("Max",DoubleValue(0.1));
  std::vector<double> startTimes;
  for(uint32_t i=0;i<d.LeftCount();i++)
  {
	  for(uint16_t j=0;j<numSourceFlows;j++){
		  TcpWebClientHelper source (Address(serverSide.GetRightIpv4Address(i)),tcpStartSinkPort);
		  ApplicationContainer sourceApp = source.Install(d.GetLeft(i));
		  double startTime=randGenerator->GetValue();
		  startTimes.push_back(startTime);
		  sourceApp.Start(Seconds(startTime));
		  sourceApp.Stop(Seconds(END_TIME));
		  sourceApps.Add(sourceApp);
	  }
  }
  //populate routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  Simulator::Stop(Seconds(END_TIME));
  Simulator::Run();
  if(csv==""){
	  for(uint32_t i=0;i<sourceApps.GetN();i++){
		  //print out the number of received bytes
		  Ptr<TcpWebClient> source1 = DynamicCast<TcpWebClient>(sourceApps.Get(i));
		  std::vector<RequestDataStruct> responseTimes=source1->getResponseTimes();
		  //std::cout << "Response Time for Flow " << i <<std::endl;
		  for(uint32_t j=0;j<responseTimes.size();j++){
			  std::cout << "Request Start Time," <<  responseTimes[j].requestStart << ",";
			  std::cout << "Request Execution Time," << responseTimes[j].requestExecutionTime << std::endl;
		  }
	  }
  }
  else{
	  for(uint32_t i=0;i<sourceApps.GetN();i++){
		  std::ofstream fileOUT(csv.c_str(), std::ios::app);
		  //print out the number of received bytes
		  Ptr<TcpWebClient> source1 = DynamicCast<TcpWebClient>(sourceApps.Get(i));
		  std::vector<RequestDataStruct> responseTimes=source1->getResponseTimes();
		  //fileOUT << "Response Time for Flow " << i << std::endl;
		  for(uint32_t j=0;j<responseTimes.size();j++){
			  fileOUT << responseTimes[j].requestStart << ",";
			  fileOUT << responseTimes[j].requestExecutionTime << std::endl;
		  }
	  }
  }
  Simulator::Destroy();
  return 0;
}
