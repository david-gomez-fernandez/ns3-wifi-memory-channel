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

#ifndef PROPRIETARY_TRACING_H_
#define PROPRIETARY_TRACING_H_

#include "ns3/object.h"
#include "ns3/random-variable.h"
#include "ns3/traced-value.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/core-module.h"
#include "ns3/wifi-module.h"

//Needed to parse the packet content (BurstyErrorModel::ParsePacket)
#include "ns3/wifi-mac-header.h"
#include "ns3/llc-snap-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"

#include "ns3/bear-propagation-loss-model.h"
#include "ns3/bear-error-model.h"

#include <math.h>

namespace ns3{

enum TransportProtocol_t{
	TCP_PROTOCOL,
	UDP_PROTOCOL,
	NSC_TCP_PROTOCOL,
	MPTCP_PROTOCOL
};

class ProprietaryTracing: public Object
{
public:
	/**
	 * Default constructor
	 */
	ProprietaryTracing();
	/**
	 * Default destructor
	 */
	~ProprietaryTracing();

	/**
	 * Set the name of the file which will be used to store the results of the simulations
	 * \param fileName Name of the trace file
	 */
	void OpenTraceFile (string fileName);
	/**
	 * Close the trace file
	 */
	void CloseTraceFile ();

	/**
	 * \return The transport protocol (EnumValue  TransportProtocol_t)
	 */
	inline TransportProtocol_t GetTransportProtocol() {return m_transportProtocol;}

	/**
	 * \param transportProtocol The transport protocol (EnumValue  TransportProtocol_t)
	 */
	inline void SetTransportProtocol (TransportProtocol_t transportProtocol) {m_transportProtocol = transportProtocol;}

	//Statistics
	inline u_int32_t GetCorrectPackets () {return m_totalDataCorrectPackets;}
	inline u_int32_t GetCorruptedPackets () {return m_totalDataCorruptedPackets;}
	inline u_int32_t GetTotalPackets () {return m_totalDataPackets;}

	//Getters/Setters
	inline bool GetWriteToFile () {return m_writeToFile;}
	inline void SetWriteToFile (bool flag) {m_writeToFile = flag;}

	
	/**
	 * \brief Upper Layer parser (Raw content overheard from the channel)
	 * \param packet The received packet
	 * \return Structure which contains the information relative to the data extracted from the different headers
	 */
	packetInfo_t ParsePacket (Ptr<const Packet> packet);

	/**
	 * Default trace callback (invoked at YansWifiPhy::EndReceive) for those frames which are corrupted
	 * \param packet
	 * \param error
	 * \param snr
	 * \param nodeId
	 */
	void DefaultPhyRxTrace (Ptr<Packet> packet, bool error, double snr, int nodeId);

	
	/**
	 * \brief Conversion from Mac48Address format to
	 * \param mac Mac address in Mac48Address format
	 * \return The MAC address in string format
	 */
	std::string ConvertMacToString(Mac48Address mac);

	/**
	 * Print the packet information into the trace file
	 * \param packetInfo The struct that contains the information relative to the packet
	 * \param nodeId The receiver node's ID
	 * \param error Flag that defines whether the error is correct or not
	 * \param lastField Variable parameter, which depends of the type of channel simulated (i.e. HMM state, SNR, etc.)
	 */
	void PrintPacketData (packetInfo_t packetInfo, int nodeId, bool error, double lastField);

	/**
	 * \brief Print line to the corresponding file
	 * \param line String to record into the file
	 */
	void TraceToFile (string line);

private:
	bool m_writeToFile;

	//Lower layer statistics
	u_int32_t m_totalDataPackets;
	u_int32_t m_totalDataCorrectPackets;
	u_int32_t m_totalDataCorruptedPackets;

	TransportProtocol_t m_transportProtocol;

	fstream m_file;							//File to store the trace (YansWifiPhy level)
	fstream m_applicationLevelTracing;		//File to store the trace (Application level
};

}  //End namespace ns3

#endif /* PROPRIETARY_TRACING_H_ */
