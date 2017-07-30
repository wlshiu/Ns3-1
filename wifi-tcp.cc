/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015, IMDEA Networks Institute
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
 * Author: Hany Assasa <hany.assasa@gmail.com>
.*
 * This is a simple example to test TCP over 802.11n (with MPDU aggregation enabled).
 *
 * Network topology:
 *
 *   Ap    STA
 *   *      *
 *   |      |
 *   n1     n2
 *
 * In this example, an HT station sends TCP packets to the access point. 
 * We report the total throughput received during a window of 100ms. 
 * The user can specify the application data rate and choose the variant
 * of TCP i.e. congestion control algorithm to use.
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"

#include <math.h>
NS_LOG_COMPONENT_DEFINE ("wifi-tcp");

using namespace ns3;

Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */



uint32_t payloadSize;                       /* Transport layer payload size in bytes. */
uint64_t MaxBytes;
std::string dataRate;  

std::string ans_dataRate;
double ans_averageThroughput;
double ans_delay;
int ans_allbytes;

double distance = 50.0;

double speed_x=10;



//處理顯示Throughput
void
CalculateThroughput ()
{
  Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
double cur = (sink->GetTotalRx() - lastTotalRx) * (double) 8/1e5;     /* Convert Application RX Packets to MBits. */
// std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
// std::cout<<"my\t"<<sink->GetTotalRx()<<"all\t"<<MaxBytes<<"\n";
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (1), &CalculateThroughput);
  
if(cur!=0){


 
 double averageThroughput = ((sink->GetTotalRx() * 8) / (1e6  * now.GetSeconds ()));

ans_averageThroughput=averageThroughput;
ans_dataRate=dataRate.substr(0,dataRate.length()-4);
ans_delay=now.GetSeconds ()-1.0001;
ans_allbytes=sink->GetTotalRx();
// std::cout <<dataRate.substr(0,dataRate.length()-4)<<" "<<averageThroughput<<" "<<now.GetSeconds ()-1.0001<<" "<<MaxBytes*8/1024/1024<<std::endl;
 Simulator::Schedule (MilliSeconds (1), &CalculateThroughput);

 

 }
}

std::string int2str(int &i) {
std::string s;
  std::stringstream ss(s);
  ss << i;

  return ss.str();
}


 
/*處理顯示位置與速度*/

//只能顯示隨機路線的位置與速度
//固定位置與速度部份能直接換算
static void 
CourseChange (std::string foo, Ptr<const MobilityModel> mobility)
{
  Vector pos = mobility->GetPosition ();
  Vector vel = mobility->GetVelocity ();
 if(vel.x!=0){ 
  std::cout << Simulator::Now () << ", model=" << mobility << ", Position: x=" << pos.x << ", y=" << pos.y<< "; Velocity: x=" << vel.x << ", y=" << vel.y<< std::endl;
 }


}




int wifi(int argc, char *argv[]){



                 /* Application layer datarate. */
  std::string tcpVariant = "ns3::TcpNewReno";        /* TCP variant type. */
  std::string phyRate = "HtMcs7";                    /* Physical layer bitrate. */
  double simulationTime = 100;                        /* Simulation time in seconds. */
  bool pcapTracing = false;                          /* PCAP Tracing is enabled or not. */

  /* Command line argument parser setup. */
  CommandLine cmd;
  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("dataRate", "Application data ate", dataRate);
  cmd.AddValue ("tcpVariant", "Transport protocol to use: TcpTahoe, TcpReno, TcpNewReno, TcpWestwood, TcpWestwoodPlus ", tcpVariant);
  cmd.AddValue ("phyRate", "Physical layer bitrate", phyRate);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("pcap", "Enable/disable PCAP Tracing", pcapTracing);
  cmd.Parse (argc, argv);

  /* No fragmentation and no RTS/CTS */
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("999999"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));

  /* Configure TCP Options */
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));

  WifiMacHelper wifiMac;
  WifiHelper wifiHelper;
  wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

  /* Set up Legacy Channel */
  YansWifiChannelHelper wifiChannel ;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));

  /* wifi硬體設定Setup Physical Layer */
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.Set ("TxPowerStart", DoubleValue (10.0));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (10.0));
  wifiPhy.Set ("TxPowerLevels", UintegerValue (1));
  wifiPhy.Set ("TxGain", DoubleValue (0));
  wifiPhy.Set ("RxGain", DoubleValue (0));
  wifiPhy.Set ("RxNoiseFigure", DoubleValue (10));
  wifiPhy.Set ("CcaMode1Threshold", DoubleValue (-79));
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-79 + 3));
  wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode", StringValue (phyRate),
                                      "ControlMode", StringValue ("HtMcs0"));



/*產生節點*/
  NodeContainer networkNodes;
  networkNodes.Create (2);
  Ptr<Node> apWifiNode = networkNodes.Get (0);
  Ptr<Node> staWifiNode = networkNodes.Get (1);





  /* Configure AP（把wifi設備放入節點） */
  Ssid ssid = Ssid ("network");
  wifiMac.SetType ("ns3::ApWifiMac",
                    "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevice;
  apDevice = wifiHelper.Install (wifiPhy, wifiMac, apWifiNode);

  /* Configure STA */
  wifiMac.SetType ("ns3::StaWifiMac",
                    "Ssid", SsidValue (ssid));

  NetDeviceContainer staDevices;
  staDevices = wifiHelper.Install (wifiPhy, wifiMac, staWifiNode);

  /* Mobility model(設定各個節點移動模型) */
  
//走直線


/*
    
 MobilityHelper mobility;
 mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue ("10.0"),
                                  "Y", StringValue ("100.0"),
                                  "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=30]"));
mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
 mobility.Install (apWifiNode);
 apWifiNode->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (22, 0, 0));
*/




//設定使用者的wifi


 MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue ("10.0"),
                                  "Y", StringValue ("100.0"),
                                  "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=30]"));
 mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");

 mobility.Install (staWifiNode);

  
 //apWifiNode->GetObject<MobilityModel> ()->SetPosition (Vector (222, 222, 0));

 //這裡設定固定速度 行走向量 Vector (speed_x, 0, 0) (速度,x向量,y向量)
  staWifiNode->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed_x, 0, 0));





//底下功能 能走隨機路線 但我設定速度0讓它靜止
//wifi ap

  MobilityHelper mobility2;

  mobility2.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue ("100.0"),
                                  "Y", StringValue ("100.0"),
                                  "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=30]"));
  mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                              "Mode", StringValue ("Time"),
                              "Time", StringValue ("1s"),
                              "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"),
                              "Bounds", StringValue ("0|200|0|200"));

  mobility2.Install (apWifiNode);











  /* Internet stack */
  InternetStackHelper stack;
  stack.Install (networkNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface;
  apInterface = address.Assign (apDevice);
  Ipv4InterfaceContainer staInterface;
  staInterface = address.Assign (staDevices);

  /* Populate routing table */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* Install TCP Receiver on the access point */
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer sinkApp = sinkHelper.Install (apWifiNode);
  sink = StaticCast<PacketSink> (sinkApp.Get (0));

  /* Install TCP/UDP Transmitter on the station */
  OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (apInterface.GetAddress (0), 9)));
  server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  server.SetAttribute ("MaxBytes", UintegerValue (MaxBytes));
  ApplicationContainer serverApp = server.Install (staWifiNode);

  /* Start Applications */
  sinkApp.Start (Seconds (0.0));
  serverApp.Start (Seconds (1.0));
  Simulator::Schedule (Seconds (1.0001), &CalculateThroughput);

  /* Enable Traces */
  
      wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
      wifiPhy.EnablePcap ("AccessPoint", apDevice);
      wifiPhy.EnablePcap ("Station", staDevices);
   
   


Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
                   MakeCallback (&CourseChange));

 



  /* Start Simulation */
  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();
 Simulator::Destroy ();
  
std::cout <<ans_dataRate<<" "<<ans_averageThroughput<<" "<<ans_delay<<" "<<ans_allbytes<<" "<<(double)ans_delay-(double)ans_allbytes*8/std::stod(ans_dataRate)/1024/1024<<std::endl;

//std::cout <<"*************************************"<<std::endl;

return 0;
}





int
main(int argc, char *argv[])
{

//控制的變數參考 

  //設定每一個封包多大
  payloadSize = 1024;                       /* Transport layer payload size in bytes. */
  //設定多少個封包 注意封包設定太大會有模擬器跑完卻還沒送完問題所以要再增大模擬器時間
  int package=50;
  MaxBytes= payloadSize*package;
  //設定datarate
  dataRate = "1000Mbps";  
/******************************************************************/





//輸出的格式 
std::cout<<"DataRate(Mbps) AverageThroughput(Mbit/s) Delay(sec) ALLDATA(Bytes) deviation-sec（s）\n";






speed_x=110;
wifi(argc,argv);
return 0;

for(int sp=1;sp<100;sp++){
  speed_x=sp;
wifi(argc,argv);

}


return 0;








for(int change=1;change<50;change++){
int bit=1;
  for(int j=1;j<=change;j++){
    bit=1*bit+1;

  }
  dataRate = int2str(bit)+"Mbps";

wifi(argc,argv);

}


  return 0;

}
