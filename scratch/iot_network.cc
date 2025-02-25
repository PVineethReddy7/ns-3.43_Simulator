#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IotNetworkExample");

int main(int argc, char *argv[])
{
    Time::SetResolution(Time::NS);

    // Create 5 IoT nodes
    NodeContainer nodes;
    nodes.Create(5);

    // Install LR-WPAN devices (802.15.4)
    LrWpanHelper lrWpan;
    NetDeviceContainer devices = lrWpan.Install(nodes);
    lrWpan.EnablePcap("iot_network", devices);

    // Assign a PAN ID to the devices
    for (uint32_t i = 0; i < devices.GetN(); i++) {
        Ptr<NetDevice> device = devices.Get(i);
        Ptr<lrwpan::LrWpanNetDevice> lrWpanDevice = DynamicCast<lrwpan::LrWpanNetDevice>(device);
        if (lrWpanDevice) {
            lrWpanDevice->GetMac()->SetPanId(0x1234); // PAN ID
        }
    }

    // Install mobility model
    MobilityHelper mobility;

    // **Node 0 (Server) is stationary**
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes.Get(0));

    // **Nodes 1-4 move randomly within a 200x200 area**
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-100, 100, -100, 100)),
                              "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]"),
                              "Distance", DoubleValue(50.0));
    mobility.Install(NodeContainer(nodes.Get(1), nodes.Get(2), nodes.Get(3), nodes.Get(4)));

    // Install Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    // Set up a UDP Server on Node 0
    uint16_t serverPort = 9;
    UdpServerHelper udpServer(serverPort);
    ApplicationContainer serverApp = udpServer.Install(nodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(20.0));

    // Set up UDP Clients (Nodes 1-4 send messages to Node 0)
    for (uint32_t i = 1; i < nodes.GetN(); i++) {
        UdpClientHelper udpClient(interfaces.GetAddress(0), serverPort);
        udpClient.SetAttribute("MaxPackets", UintegerValue(10));
        udpClient.SetAttribute("Interval", TimeValue(Seconds(2.0)));
        udpClient.SetAttribute("PacketSize", UintegerValue(64));

        ApplicationContainer clientApp = udpClient.Install(nodes.Get(i));
        clientApp.Start(Seconds(2.0));
        clientApp.Stop(Seconds(20.0));
    }

    // Enable packet tracing
   // ✅ Corrected tracing setup
   AsciiTraceHelper ascii;
   Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("iot_network_udp.tr");
   internet.EnableAsciiIpv4All(stream);  // ✅ Enables UDP tracing
   


    // Enable NetAnim visualization
    AnimationInterface anim("iot_network.xml");

    // Enable NetAnim packet tracking
anim.EnablePacketMetadata(true);  // ✅ Show packet movement

// Assign colors to different nodes
anim.UpdateNodeColor(0, 255, 0, 0);   // Node 0 (Red - Server)
anim.UpdateNodeColor(1, 0, 0, 255);   // Node 1 (Blue)
anim.UpdateNodeColor(2, 0, 255, 0);   // Node 2 (Green)
anim.UpdateNodeColor(3, 255, 255, 0); // Node 3 (Yellow)
anim.UpdateNodeColor(4, 255, 0, 255); // Node 4 (Purple)

// Enable IP route tracking for better visualization
anim.EnableIpv4RouteTracking("iot_network_route.xml", Seconds(0), Seconds(10), Seconds(0.5));


    // Enable mobility tracking for NetAnim
    anim.SetMobilityPollInterval(Seconds(0.1));

    // Run simulation
    Simulator::Stop(Seconds(22.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
