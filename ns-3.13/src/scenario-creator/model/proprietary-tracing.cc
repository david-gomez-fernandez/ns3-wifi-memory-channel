/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Universidad de Cantabria
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
 * Author: David Gómez Fernández <dgomez@tlmat.unican.es>
 *         Ramón Agüero Calvo <ramon@tlmat.unican.es>
 */

#include "proprietary-tracing.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ProprietaryTracing");
NS_OBJECT_ENSURE_REGISTERED(ProprietaryTracing);

ProprietaryTracing::ProprietaryTracing ()
{
    NS_LOG_FUNCTION(this);
    m_totalDataPackets = 0;
    m_totalDataCorrectPackets = 0;
    m_totalDataCorruptedPackets = 0;
}

ProprietaryTracing::~ProprietaryTracing ()
{
    NS_LOG_FUNCTION(this);
    //We can harness the call to the destructor and print out the brief statistics

    //...
    if (m_file.is_open())
    	m_file.close();
}

void ProprietaryTracing::OpenTraceFile (string fileName)
{
    char buf[FILENAME_MAX];
    string path = string(getcwd(buf, FILENAME_MAX)) + "/traces/" + fileName;
    NS_LOG_FUNCTION(this << path);
    m_file.open(path.c_str(), fstream::out);

    sprintf(buf, "%16s %8s %5s %18s %18s %6s %6s %16s %16s %6s %8s %8s %12s %12s %6s %8s %13s",
            "Time", "Node_ID", "CRC", "MAC_SRC", "MAC_DST", "RETX", "SN", "IP_SRC", "IP_DST", "PROT", "SRC_PORT", "DST_PORT", "TCP_SN", "TCP_Ack", "Flags", "Length", "SNR/State");

    TraceToFile(buf);
}

void ProprietaryTracing::CloseTraceFile()
{
    NS_LOG_FUNCTION(this);
    if (m_file.is_open())
        m_file.close();

}

packetInfo_t ProprietaryTracing::ParsePacket (Ptr<const Packet> packet)
{
    //	NS_LOG_FUNCTION(this << packet);

    packetInfo_t packetInfo;
    Ptr<Packet> pktCopy = packet->Copy();

    pktCopy->RemoveHeader(packetInfo.wifiHdr);

    if (packetInfo.wifiHdr.IsData()) {
        pktCopy->RemoveHeader(packetInfo.llcHdr);
        switch (packetInfo.llcHdr.GetType()) {
            case 0x0806: //ARP
                packetInfo.type = ARP_PACKET;
                break;
            case 0x0800: //IP packet
                pktCopy->RemoveHeader(packetInfo.ipv4Hdr);
                switch (packetInfo.ipv4Hdr.GetProtocol()) {
                    case 6: //TCP
                        pktCopy->RemoveHeader(packetInfo.tcpHdr);
                        packetInfo.type = TCP_DATA;

                        break;
                    case 17: //UDP
                        pktCopy->RemoveHeader(packetInfo.udpHdr);
                        packetInfo.type = UDP_DATA;
                        break;
                    default:
                        NS_LOG_ERROR("Protocol not implemented yet (IP) --> " << packetInfo.llcHdr.GetType());
                        break;
                }
                break;
            default:
                NS_LOG_ERROR("Protocol not implemented yet (LLC) --> " << packetInfo.llcHdr.GetType());
                break;
        }
    }
    else if (packetInfo.wifiHdr.IsAck())
    {
        packetInfo.type = IEEE_80211_ACK;
    }
    else // 802.11 Control/Management frame
    {
        packetInfo.type = IEEE_80211_NODATA;
    }

    packetInfo.payloadLength = pktCopy->GetSize() - 4; //Last four bytes are used for tagging

    return packetInfo;
}

void ProprietaryTracing::DefaultPhyRxTrace (Ptr<Packet> packet, bool error, double snr, int nodeId)
{
    NS_LOG_FUNCTION(this);
    //Update statistics
    if (packet->GetSize() > 500) {
        if (error == 1) {
            m_totalDataCorrectPackets++;
            m_totalDataPackets++;
        } else {
            m_totalDataCorruptedPackets++;
            m_totalDataPackets++;
        }
    }

    if (m_writeToFile)
    {
        PrintPacketData(ParsePacket(packet), nodeId, error, snr);
    }
}


void ProprietaryTracing::PrintPacketData (packetInfo_t packetInfo, int nodeId, bool error, double lastField)
{
    NS_LOG_FUNCTION(this);

    char line [255];
    u_int8_t source [4], destination [4];
    char sourceChar [32], destChar [32];

    switch (packetInfo.type) {
        case TCP_DATA:
            //Get the IP Addresses printable
            packetInfo.ipv4Hdr.GetSource().Serialize(source); //Get a string from Ipv4Address
            packetInfo.ipv4Hdr.GetDestination().Serialize(destination);
            sprintf(sourceChar, "%d.%d.%d.%d", source[0], source[1], source[2], source[3]);
            sprintf(destChar, "%d.%d.%d.%d", destination[0], destination[1], destination[2], destination[3]);

            sprintf(line, "%16f %8d %5d %18s %18s %6d %6d %16s %16s %6s %8d %8d %12d %12d %6X %8d %13.3f",
                    Simulator::Now().GetSeconds(), nodeId, error, ConvertMacToString(packetInfo.wifiHdr.GetAddr2()).c_str(), ConvertMacToString(packetInfo.wifiHdr.GetAddr1()).c_str(),
                    packetInfo.wifiHdr.IsRetry(), packetInfo.wifiHdr.GetSequenceNumber(),
                    sourceChar, destChar, "TCP", packetInfo.tcpHdr.GetSourcePort(), packetInfo.tcpHdr.GetDestinationPort(),
                    packetInfo.tcpHdr.GetSequenceNumber().GetValue(), packetInfo.tcpHdr.GetAckNumber().GetValue(),
                    packetInfo.tcpHdr.GetFlags(), packetInfo.payloadLength, lastField);
            TraceToFile(line);
            break;
        case UDP_DATA:
            //Get the IP Addresses printable
            packetInfo.ipv4Hdr.GetSource().Serialize(source); //Get a string from Ipv4Address
            packetInfo.ipv4Hdr.GetDestination().Serialize(destination);
            sprintf(sourceChar, "%d.%d.%d.%d", source[0], source[1], source[2], source[3]);
            sprintf(destChar, "%d.%d.%d.%d", destination[0], destination[1], destination[2], destination[3]);

            sprintf(line, "%16f %8d %5d %18s %18s %6d %6d %16s %16s %6s %8d %8d %12d %12d %6d %8d %13.3f",
                    Simulator::Now().GetSeconds(), nodeId, error, ConvertMacToString(packetInfo.wifiHdr.GetAddr2()).c_str(), ConvertMacToString(packetInfo.wifiHdr.GetAddr1()).c_str(),
                    packetInfo.wifiHdr.IsRetry(), packetInfo.wifiHdr.GetSequenceNumber(),
                    sourceChar, destChar, "UDP", packetInfo.udpHdr.GetSourcePort(), packetInfo.udpHdr.GetDestinationPort(),
                    0, 0, 0, packetInfo.payloadLength, lastField);
            TraceToFile(line);
            break;
        case ARP_PACKET:

            break;
        case IEEE_80211_ACK:

            break;        
        default:
            NS_LOG_ERROR("Unknown packet type --> " << packetInfo.type);
            break;
    }

}

std::string ProprietaryTracing::ConvertMacToString (Mac48Address mac)
{
    //	NS_LOG_FUNCTION(mac);
    u_int8_t temp[6];
    char result[24];
    mac.CopyTo(temp);

    sprintf(result, "%02X:%02X:%02X:%02X:%02X:%02X", temp[0], temp[1], temp[2], temp[3], temp[4], temp[5]);

    return std::string(result);
}

void ProprietaryTracing::TraceToFile(string line)
{
    NS_ASSERT_MSG(m_file.is_open(), "No trace file to write to");
    m_file << line << endl;
}
