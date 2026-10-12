#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace ns3;

/*
 * TCP Congestion Window Trace
 */
static void
CwndTracer(Ptr<OutputStreamWrapper> stream,
            uint32_t oldCwnd,
            uint32_t newCwnd)
{
    *stream->GetStream()
        << Simulator::Now().GetSeconds()
        << " "
        << newCwnd
        << std::endl;
}

/*
 * Connect CWND trace
 */
static void
TraceCwnd(uint32_t nodeId, uint32_t socketId)
{
    AsciiTraceHelper ascii;

    Ptr<OutputStreamWrapper> stream =
        ascii.CreateFileStream("cwnd.dat");

    Config::ConnectWithoutContext(
        "/NodeList/" + std::to_string(nodeId) +
        "/$ns3::TcpL4Protocol/SocketList/" +
        std::to_string(socketId) +
        "/CongestionWindow",
        MakeBoundCallback(&CwndTracer, stream));
}

int
main(int argc, char *argv[])
{
    /*
     * Use TCP NewReno
     */
    Config::SetDefault(
        "ns3::TcpL4Protocol::SocketType",
        StringValue("ns3::TcpNewReno"));

    /*
     * TCP settings
     */
    Config::SetDefault(
        "ns3::TcpSocket::SegmentSize",
        UintegerValue(1024));

    Config::SetDefault(
        "ns3::TcpSocket::InitialCwnd",
        UintegerValue(20));

    Config::SetDefault(
        "ns3::TcpSocket::SndBufSize",
        UintegerValue(1024 * 1024));

    Config::SetDefault(
        "ns3::TcpSocket::RcvBufSize",
        UintegerValue(1024 * 1024));

    /*
     * ------------------------------------------------
     * 1. NODE CREATION
     * ------------------------------------------------
     */
    NodeContainer nodes;
    nodes.Create(2);

    /*
     * ------------------------------------------------
     * 2. POINT-TO-POINT NETWORK
     * ------------------------------------------------
     */
    PointToPointHelper p2p;

    p2p.SetDeviceAttribute(
        "DataRate",
        StringValue("2Mbps"));

    p2p.SetChannelAttribute(
        "Delay",
        StringValue("10ms"));

    /*
     * Small queue to create congestion
     */
    p2p.SetQueue(
        "ns3::DropTailQueue<Packet>",
        "MaxSize",
        QueueSizeValue(QueueSize("10p")));

    NetDeviceContainer devices;
    devices = p2p.Install(nodes);

    /*
     * ------------------------------------------------
     * 3. INTERNET STACK
     * ------------------------------------------------
     */
    InternetStackHelper stack;
    stack.Install(nodes);

    /*
     * ------------------------------------------------
     * 4. IP CONFIGURATION
     * ------------------------------------------------
     */
    Ipv4AddressHelper address;

    address.SetBase(
        "10.1.1.0",
        "255.255.255.0");

    Ipv4InterfaceContainer interfaces;
    interfaces = address.Assign(devices);

    /*
     * ------------------------------------------------
     * 5. TCP RECEIVER
     * ------------------------------------------------
     */
    uint16_t port = 5000;

    PacketSinkHelper sink(
        "ns3::TcpSocketFactory",
        InetSocketAddress(
            Ipv4Address::GetAny(),
            port));

    ApplicationContainer sinkApp =
        sink.Install(nodes.Get(1));

    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(10.0));

    /*
     * ------------------------------------------------
     * 6. TCP SENDER
     * ------------------------------------------------
     *
     * OnOffHelper is used here so that the output
     * can report the sender rate as 5 Mbps.
     */
    OnOffHelper source(
        "ns3::TcpSocketFactory",
        InetSocketAddress(
            interfaces.GetAddress(1),
            port));

    source.SetAttribute(
        "DataRate",
        StringValue("5Mbps"));

    source.SetAttribute(
        "PacketSize",
        UintegerValue(1024));

    source.SetAttribute(
        "OnTime",
        StringValue("ns3::ConstantRandomVariable[Constant=1]"));

    source.SetAttribute(
        "OffTime",
        StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer sourceApp =
        source.Install(nodes.Get(0));

    sourceApp.Start(Seconds(2.0));
    sourceApp.Stop(Seconds(9.0));

    /*
     * ------------------------------------------------
     * 7. TERMINAL REPORT - HEADER
     * ------------------------------------------------
     */
    std::cout << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "             TCP NETWORK SIMULATION               " << std::endl;
    std::cout << "==================================================" << std::endl;

    /*
     * Network information
     */
    std::cout << "\n[1] NETWORK DETAILS" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    std::cout << std::left
              << std::setw(22) << "Sender Node IP"
              << ": " << interfaces.GetAddress(0)
              << std::endl;

    std::cout << std::left
              << std::setw(22) << "Receiver Node IP"
              << ": " << interfaces.GetAddress(1)
              << std::endl;

    std::cout << std::left
              << std::setw(22) << "Connection"
              << ": Point-to-Point"
              << std::endl;

    std::cout << std::left
              << std::setw(22) << "Bandwidth"
              << ": 2 Mbps"
              << std::endl;

    std::cout << std::left
              << std::setw(22) << "Propagation Delay"
              << ": 10 ms"
              << std::endl;

    std::cout << std::left
              << std::setw(22) << "TCP Variant"
              << ": NewReno"
              << std::endl;

    std::cout << std::left
              << std::setw(22) << "Simulation Duration"
              << ": 10 seconds"
              << std::endl;

    /*
     * Application information
     */
    std::cout << "\n[2] TRAFFIC CONFIGURATION" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    std::cout << std::left
              << std::setw(22) << "Application"
              << ": TCP OnOff"
              << std::endl;

    std::cout << std::left
              << std::setw(22) << "Sender Rate"
              << ": 5 Mbps"
              << std::endl;

    std::cout << std::left
              << std::setw(22) << "Packet Size"
              << ": 1024 bytes"
              << std::endl;

    std::cout << std::left
              << std::setw(22) << "Destination Port"
              << ": " << port
              << std::endl;

    /*
     * ------------------------------------------------
     * 8. BUFFER DEMONSTRATION
     * ------------------------------------------------
     */
    int bufferSize = 5;
    int buffer = 0;

    std::cout << "\n[3] BUFFER ACTIVITY EXAMPLE" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    std::cout << "This section demonstrates buffer handling." << std::endl;
    std::cout << "It is a logical example and is not the" << std::endl;
    std::cout << "measured NS-3 queue occupancy." << std::endl;

    std::cout << std::endl;

    std::cout << std::left
              << std::setw(8)  << "Pkt"
              << std::setw(15) << "Before"
              << std::setw(15) << "After"
              << std::setw(20) << "Result"
              << std::endl;

    std::cout << "--------------------------------------------------"
              << std::endl;

    for (int i = 1; i <= 50; i++)
    {
        int before = buffer;

        if (buffer < bufferSize)
        {
            buffer++;

            std::cout << std::left
                      << std::setw(8)  << i
                      << std::setw(15)
                      << (std::to_string(before) + "/5")
                      << std::setw(15)
                      << (std::to_string(buffer) + "/5")
                      << std::setw(20)
                      << "Packet accepted"
                      << std::endl;
        }
        else
        {
            buffer = buffer - 2;

            std::cout << std::left
                      << std::setw(8)  << i
                      << std::setw(15)
                      << (std::to_string(before) + "/5")
                      << std::setw(15)
                      << (std::to_string(buffer) + "/5")
                      << std::setw(20)
                      << "Full - 2 released"
                      << std::endl;
        }
    }

    std::cout << "--------------------------------------------------"
              << std::endl;

    /*
     * ------------------------------------------------
     * 9. CWND TRACING
     * ------------------------------------------------
     */
    Simulator::Schedule(
        Seconds(2.001),
        &TraceCwnd,
        0,
        0);

    std::cout << "\n[4] TCP CONGESTION WINDOW" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "CWND trace file : cwnd.dat" << std::endl;
    std::cout << "Graph output    : tcp-cwnd.png" << std::endl;

    /*
     * ------------------------------------------------
     * 10. RUN SIMULATION
     * ------------------------------------------------
     */
    std::cout << "\n[5] SIMULATION STATUS" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    std::cout << "Initializing NS-3 environment..." << std::endl;
    std::cout << "Starting packet transmission..." << std::endl;
    std::cout << "Monitoring TCP congestion window..." << std::endl;

    Simulator::Stop(Seconds(10.0));

    Simulator::Run();

    /*
     * ------------------------------------------------
     * 11. TOTAL DATA RECEIVED
     * ------------------------------------------------
     */
    Ptr<PacketSink> packetSink =
        DynamicCast<PacketSink>(
            sinkApp.Get(0));

    uint64_t totalBytes = packetSink->GetTotalRx();

    /*
     * ------------------------------------------------
     * 12. FINAL REPORT
     * ------------------------------------------------
     */
    std::cout << "\n[6] SIMULATION RESULT" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    std::cout << std::left
              << std::setw(28) << "Execution status"
              << ": SUCCESS"
              << std::endl;

    std::cout << std::left
              << std::setw(28) << "Total bytes received"
              << ": " << totalBytes
              << " bytes"
              << std::endl;

    std::cout << std::left
              << std::setw(28) << "CWND data points"
              << ": Stored in cwnd.dat"
              << std::endl;

    std::cout << std::left
              << std::setw(28) << "Simulation end"
              << ": 10.0 seconds"
              << std::endl;

    std::cout << "\n==================================================" << std::endl;
    std::cout << "           SIMULATION FINISHED SUCCESSFULLY       " << std::endl;
    std::cout << "==================================================" << std::endl;

    Simulator::Destroy();

    return 0;
}
