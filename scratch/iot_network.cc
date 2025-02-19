#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/lr-wpan-module.h"  
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/energy-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IotNetworkExample");

int main (int argc, char *argv[])
{
    // Set the simulation time resolution
    Time::SetResolution(Time::NS);

    // Create 5 IoT nodes
    NodeContainer nodes;
    nodes.Create(5);

    
    ns3::LrWpanHelper lrWpan;
    NetDeviceContainer devices = lrWpan.Install(nodes);
    lrWpan.EnablePcap("iot_network", devices);


    // Set up a grid-based mobility model (nodes placed at fixed positions)
    MobilityHelper mobility;
mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                              "MinX", DoubleValue(0.0),
                              "MinY", DoubleValue(0.0),
                              "DeltaX", DoubleValue(50.0),
                              "DeltaY", DoubleValue(50.0),
                              "GridWidth", UintegerValue(3),
                              "LayoutType", StringValue("RowFirst"));

// ✅ Make nodes move randomly
mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                          "Bounds", RectangleValue(Rectangle(-100, 100, -100, 100)),
                          "Distance", DoubleValue(30.0),   // Move 30 units
                          "Speed", StringValue("ns3::ConstantRandomVariable[Constant=5.0]")); // Speed = 5 m/s

mobility.Install(nodes);


    
    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        Ptr<ns3::energy::BasicEnergySource> energySource = CreateObject<ns3::energy::BasicEnergySource>();
        energySource->SetInitialEnergy(100.0);
        nodes.Get(i)->AggregateObject(energySource);  // Attach a unique energy source per node
    }
    
    // Enable network animation output
    AnimationInterface anim("iot_network.xml");
    anim.SetMobilityPollInterval(Seconds(1.0));

    lrWpan.EnablePcap("iot_network", devices);
anim.EnablePacketMetadata(true); // Enables packet tracking in NetAnim
Simulator::Stop(Seconds(10.0)); // Run for 10 seconds

    
    // Run simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
