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
 *		   Ramón Agüero Calvo <ramon@tlmat.unican.es>
 */

#ifndef SCRATCH_LOGGING_H_
#define SCRATCH_LOGGING_H_


namespace ns3 {

void EnableLogging () {
//	LOGGING   --> Uncomment desired files to show log messages
//	-- Physical layer
//	LogComponentEnable("WifiPhy", LOG_ALL);
//	LogComponentEnable("YansWifiPhy",LOG_DEBUG);
//	LogComponentEnable("YansWifiChannel", LOG_DEBUG);
//	LogComponentEnable("YansWifiHelper", LOG_ALL);
//	LogComponentEnable("PropagationLossModel", LOG_DEBUG);
//	LogComponentEnable("WifiPhyStateHelper", LOG_LEVEL);
//	LogComponentEnable("HiddenMarkovErrorModel", LOG_LOGIC);
//	-- Link Level
//	LogComponentEnable("MacLow", LOG_FUNCTION);
//	LogComponentEnable("WifiMacQueue", LOG_FUNCTION);
//	LogComponentEnable("WifiMacQueue", LOG_ALL);
//	LogComponentEnable("DcaTxop", LOG_FUNCTION);
//	LogComponentEnable("DcfManager", LOG_FUNCTION);
//	LogComponentEnable("RegularWifiMac",LOG_FUNCTION);
//	LogComponentEnable("RegularWifiMac",LOG_ALL);
//	LogComponentEnable("StaWifiMac",LOG_FUNCTION);
//	LogComponentEnable("AdhocWifiMac",LOG_ALL);
//	LogComponentEnable("WifiNetDevice", LOG_ALL);
//	-- Network layer
//	LogComponentEnable("Ipv4L3Protocol", LOG_LOGIC);
//	-- Transport layer
//	LogComponentEnable("UdpSocket", LOG_ALL);
//	LogComponentEnable("UdpSocketImpl", LOG_ALL);
//	-- Propagation Loss Models
//	LogComponentEnable("PropagationLossModel", LOG_ALL);
//	-- Error Models
//	LogComponentEnable("ErrorModel", LOG_DEBUG);
//	LogComponentEnable("HiddenMarkovErrorModel", LOG_INFO);
//	LogComponentEnable("ArModel", LOG_ALL);
//	LogComponentEnable("ArModel", LOG_LOGIC);
//	-- Test unit
//	LogComponentEnable("Experiment", LOG_DEBUG);

//	--	Other levels
//	LogComponentEnable("Socket", LOG_ALL);
//	LogComponentEnable("PacketSocket", LOG_ALL);
//	LogComponentEnable("UdpSocketImpl", LOG_ALL);
//	LogComponentEnable("TcpSocketBase", LOG_ALL);
//	LogComponentEnable("TcpReno", LOG_ALL);
//	LogComponentEnable("TcpNewReno", LOG_ALL);
//	LogComponentEnable("UdpSocketImpl", LOG_LOGIC);

//	-- Routing
//	LogComponentEnable("Ipv4StaticRouting", LOG_ALL);

//		 --Application
//	LogComponentEnable ("TcpSocket", LOG_ALL);
//	LogComponentEnable ("OnOffApplication", LOG_ALL);
//	LogComponentEnable ("PacketSink", LOG_ALL);
//	LogComponentEnable ("TcpL4Protocol", LOG_ALL);
//	LogComponentEnable ("UdpL4Protocol", LOG_ALL);
//	LogComponentEnable ("Ipv4L3Protocol", LOG_ALL);

}


} //namespace ns3
#endif /* SCRATCH_LOGGING_H_ */
