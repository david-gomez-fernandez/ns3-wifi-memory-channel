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

#include <math.h>
#include <fstream>
#include <stdio.h>

#include <ns3/node.h>
#include <ns3/node-list.h>
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/string.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/trace-source-accessor.h"

#include "ar-model.h"

const bool g_debug = false;  							//Temporal solution (only for debugging)

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ArModel");

u_int32_t ChannelKey::GetRx () const
{
    return m_rx;
}

u_int32_t ChannelKey::GetTx () const
{
    return m_tx;
}

void ChannelKey::SetRx (u_int32_t rx)
{
    m_rx = rx;
}

void ChannelKey::SetTx (u_int32_t tx)
{
    m_tx = tx;
}


NS_OBJECT_ENSURE_REGISTERED (ArChannelEntry);

ArChannelEntry::ArChannelEntry()
{
	NS_LOG_FUNCTION (this);
}

ArChannelEntry::ArChannelEntry(int order, double coherenceTime): m_order (order), m_coherenceTime (coherenceTime)
{
	NS_LOG_FUNCTION ("AR filter order " << order << " -- Coherence Time" << coherenceTime);
}

ArChannelEntry::~ArChannelEntry()
{
	NS_LOG_FUNCTION (this);
}

u_int32_t ArChannelEntry::GetRx () const
{
    return m_rx;
}

u_int32_t ArChannelEntry::GetTx () const
{
    return m_tx;
}

void ArChannelEntry::SetRx (u_int32_t rx)
{
    m_rx = rx;
}

void ArChannelEntry::SetTx (u_int32_t tx)
{
    m_tx = tx;
}

void ArChannelEntry::UpdateSnr (double snr)
{
	NS_LOG_FUNCTION (this);

	struct prevValues_t newSample;
	double newTimeout;
	newSample.time = Simulator::Now();

	//If the vector is empty, set the timer with the arrival of the first frame
	if (!m_previousSnr.size())
	{
		m_coherenceTimeout = Simulator::Schedule(MilliSeconds(m_coherenceTime), &ArChannelEntry::HandleCoherenceTimeout, this);
		NS_LOG_DEBUG("Timeout established at " << Simulator::Now().GetSeconds() + m_coherenceTime/1e3);
	}
	//If the vector is full (already holds AR model order values), we do erase the oldest one (the first one), and push back the newest one
	//at the end of the vector. It is worth highlighting, that if there is an active timer
	if(m_previousSnr.size() && m_previousSnr.size() == ((unsigned) m_order)) {
		m_previousSnr.erase(m_previousSnr.begin());
		if (m_coherenceTimeout.IsRunning())
		{
			m_coherenceTimeout.Cancel();
			//Update the new timer
			newTimeout = GetNextTimeout();
			m_coherenceTimeout = Simulator::Schedule(MilliSeconds(newTimeout), &ArChannelEntry::HandleCoherenceTimeout, this);
		}

		//Option not tested yet
//		else
//		{
//			newTimeout = GetNextTimeout();
//			m_coherenceTimeout = Simulator::Schedule(MilliSeconds(newTimeout), &ArChannelEntry::HandleCoherenceTimeout, this);
//		}
	}

	newSample.snr = snr;

	m_previousSnr.push_back(newSample);
	DisplaySnrQueue();
}

int ArChannelEntry::GetPreviousSnr(vector<double> &previousSnr)
{
	NS_LOG_FUNCTION_NOARGS();
	int i;

	for(i = 0 ; i < (int) m_previousSnr.size() ; i++) {
		previousSnr.push_back(m_previousSnr[i].snr);
	}

	return ((int) m_previousSnr.size());
}

double ArChannelEntry::GetNextTimeout()
{
	NS_LOG_FUNCTION(this);
	double timeout, currentTime, firstArrival;

	if(m_previousSnr.size())
	{
		firstArrival = m_previousSnr[0].time.GetMilliSeconds();
		currentTime = Simulator::Now().GetMilliSeconds();
		timeout = m_coherenceTime - (currentTime - firstArrival);

		NS_LOG_DEBUG ("Getting next timeout = " << m_coherenceTime << " - " << "(" 		\
				<< currentTime << " - " << firstArrival << ") = " << timeout << " --> " << Simulator::Now().GetSeconds() + timeout/1e3);
		//Maximum precision 1 msec
		if(timeout < 1)
			timeout = 1;

	} else {
		timeout = 0.0;
	}

	NS_LOG_DEBUG("Introducing a new timeout in " << timeout << " milliseconds");

	return timeout;
}

void ArChannelEntry::HandleCoherenceTimeout()
{
	NS_LOG_FUNCTION(Simulator::Now().GetSeconds());

//	DisplaySnrQueue();

	if(m_previousSnr.size())
	{
		m_previousSnr.erase(m_previousSnr.begin());
		m_coherenceTimeout = Simulator::Schedule(MilliSeconds(GetNextTimeout()), &ArChannelEntry::HandleCoherenceTimeout, this);
	}
	else
		NS_LOG_ERROR("There are not buffered samples");

//	DisplaySnrQueue();
}

void ArChannelEntry::DisplaySnrQueue()
{
	NS_LOG_FUNCTION(Simulator::Now().GetSeconds());
	u_int8_t i;
	char message[256];

	for (i=0; i < m_previousSnr.size(); i++)
	{
		sprintf(message, "%3d SNR = %2.6f Time = %4.5f", i, m_previousSnr[i].snr, m_previousSnr[i].time.GetSeconds());
		NS_LOG_DEBUG(message);
	}
}

NS_OBJECT_ENSURE_REGISTERED (ArModel);

TypeId
ArModel::GetTypeId(void) {
	static TypeId tid = TypeId ("ns3::ArModel")
	.SetParent<PropagationLossModel> ()
	.AddConstructor<ArModel> ()
	.AddAttribute ("RanVar",
			"Random variable which determine a packet to be successfully received or not",
			RandomVariableValue (UniformVariable (0.0, 1.0)),
			MakeRandomVariableAccessor (&ArModel::m_ranvar),
			MakeRandomVariableChecker ())
	.AddAttribute("ArFilterOrder",
			"Order of the Auto Regressive Filter",
			IntegerValue(4),
			MakeIntegerAccessor (&ArModel::m_order),
			MakeIntegerChecker<int> ())
	.AddAttribute("NoiseLevel",
			"Total noise power level (W)",
			DoubleValue(1.082e-6),
			MakeDoubleAccessor(&ArModel::m_noise),
			MakeDoubleChecker<double> ())
	.AddAttribute("ArFilterVariance",
			"AR model variance",
			DoubleValue(0.005),
			MakeDoubleAccessor(&ArModel::m_variance),
			MakeDoubleChecker <double> ())
	.AddAttribute("StandardDeviation",
			"AR model standard deviation (dB)",
			DoubleValue(2.6),
			MakeDoubleAccessor(&ArModel::m_stdDevDb),
			MakeDoubleChecker<double> ())
	.AddAttribute("FastFadingVariance",
			"Variance that model the Gaussian model inherent to Fast Fading",
			DoubleValue(2.8),
//			DoubleValue (1.67),
			MakeDoubleAccessor(&ArModel::m_ffVariance),
			MakeDoubleChecker<double> ())
	.AddAttribute("CoherenceTime",
			"Channel propagation coherence time (in milliseconds)",
			DoubleValue(5000.0),
			MakeDoubleAccessor(&ArModel::m_coherenceTime),
			MakeDoubleChecker<double> ())
	.AddAttribute("CoefficientsFile",
			"Name of the file that contains the AR model coefficients",
			StringValue("coefsAR.cfg"),
			MakeStringAccessor (&ArModel::m_coefficientsFile),
			MakeStringChecker ())
	       ;
  return tid;
}

ArModel::ArModel() : m_order (3),
		m_variance(0.005),
		m_ffVariance (2.8),
//		m_ffVariance (1.67),
		m_stdDevDb (2.6),
		m_ranvar (UniformVariable (0.0, 1.0))
{
	NS_LOG_FUNCTION(m_noise);
	GetCoefficients("coefsAR.cfg");

	int totalNodes;
	int i, j;

	ChannelKey key(0,0);
	ChannelKey keyAux(0,0);
	Ptr<ArChannelEntry> channel;

	channelSetIter_t iter;

	//Private variable initialization
	m_symmetry = true;
	m_coherenceTime = 10000.0;

	//// Input noise value
	//	m_noise = 1.082e-6;					//SNR = 10 dB @ 10 meters between nodes
	//	m_noise = 5.42338e-7;				//SNR = 13 dB @ 10 meters between nodes
	//  m_noise = 9.14617e-9;				//SNR = 15 dB @ 10 meters between nodes
	//	m_noise = 1.082e-9;					//SNR = 20 dB @ 10 meters between node

	//Default Propagation Loss
	Ptr<FriisPropagationLossModel> aux = CreateObject <FriisPropagationLossModel>();
	m_propagationLoss = aux;


	//Create the map which contains the scenario snapshot
	totalNodes = NodeList().GetNNodes();

	for( i = 0 ; i < totalNodes ; i ++ ) {
		for( j = 0; j < totalNodes ; j ++) {
			if( j != i ) {
				key.SetTx(i);
				key.SetRx(j);

				if(m_symmetry == 1) {
					if(i < j) {
						channel = CreateObject<ArChannelEntry> (m_order, m_coherenceTime);
						channel->SetTx(i);
						channel->SetRx(j);

					} else {
						keyAux.SetTx(j);
						keyAux.SetRx(i);
						iter = m_channelSetMap.find(keyAux);
						if(iter != m_channelSetMap.end())
							channel = iter->second;
					}
				}
				else {
					channel = CreateObject<ArChannelEntry> (m_order, m_coherenceTime);
					channel->SetTx(i);
					channel->SetRx(j);

				}
				//Timer handling -- Work in progress (Port from Ramon's Timer class)
				m_channelSetMap.insert(pair<ChannelKey, Ptr<ArChannelEntry> > (key, channel));
			}
		}
	}
}

ArModel::~ArModel ()
{
	NS_LOG_FUNCTION(this);
	if (m_arCoefficientsMap.size() > 0)
		m_arCoefficientsMap.clear();
	if (m_channelSetMap.size() > 0)
		m_channelSetMap.clear();
}

void ArModel::SetPropagationLoss (std::string type,
        std::string n0, const AttributeValue &v0,
        std::string n1, const AttributeValue &v1,
        std::string n2, const AttributeValue &v2,
        std::string n3, const AttributeValue &v3,
        std::string n4, const AttributeValue &v4,
        std::string n5, const AttributeValue &v5,
        std::string n6, const AttributeValue &v6,
        std::string n7, const AttributeValue &v7)
{
	NS_LOG_FUNCTION(this);
	ObjectFactory factory;
	factory.SetTypeId (type);
	factory.Set (n0, v0);
	factory.Set (n1, v1);
	factory.Set (n2, v2);
	factory.Set (n3, v3);
	factory.Set (n4, v4);
	factory.Set (n5, v5);
	factory.Set (n6, v6);
	factory.Set (n7, v7);
	m_propagationLoss = factory.Create<PropagationLossModel> ();
}

void ArModel::SetPropagationLoss (Ptr<PropagationLossModel> loss)
{
	NS_LOG_FUNCTION(this);
	m_propagationLoss = loss;
}

void ArModel::SetErrorModel(Ptr<BurstyErrorModel> error)
{
	NS_LOG_FUNCTION(this);
	m_errorModel= error;
}

Ptr<BurstyErrorModel> ArModel::GetErrorModel()
{
	NS_LOG_FUNCTION(this);
	return m_errorModel;
}

bool ArModel::GetCoefficients (string fileName)
{
	NS_LOG_FUNCTION (fileName);

	fstream arCoefficientsFile;
	int currentCoefficientNumber, i, j;
	char line[256];
	double arCurrentCoefficient;
	string arCoefficientFilePath;
	vector<double> arCoefficientsVector;

	coefSetIter_t iter; 							//Only for debugging

	//File handling
	arCoefficientFilePath = GetCwd() + "/src/network/utils/configs/" + fileName;
	arCoefficientsFile.open((const char *) arCoefficientFilePath.c_str(), ios::in);

	if (arCoefficientsFile)
	{
		while(arCoefficientsFile.getline(line, 256)) {
			currentCoefficientNumber = atoi(line);  				// This is the current number of coeficients
			j = 0;
			for( i=0 ; i <= currentCoefficientNumber ; i++) {
				// We need to go to the next coefficient
				while(line[j] != ' ' && line[j] != '\t') {
					j++;
				}
				while(line[j] == ' ' || line[j] == '\t') {
					j++;
				}
				arCurrentCoefficient = atof(line + j);
				arCoefficientsVector.push_back(arCurrentCoefficient);
			}

			m_arCoefficientsMap.insert(pair<int,vector<double> > (currentCoefficientNumber,arCoefficientsVector));
			arCoefficientsVector.clear();
		}
	}
	else
	{
		NS_LOG_ERROR("File (AR model) " << arCoefficientFilePath << " not found: Please fix");
		return false;
	}

	//DEBUGGING
	#ifdef NS3_LOG_ENABLE
		if (g_debug)
		{
			//Print the m_transitionMatrix map
			printf ("---AR Coefficients Vector---\n");
			for (iter = m_arCoefficientsMap.begin(); iter != m_arCoefficientsMap.end(); iter ++ )
			{
				printf ("KEY %2d  - ", iter->first);
				for (i = 0; i < (int) (iter->second).size(); i++)
				{
					printf ("%1.8f  ", (iter->second)[i]);
				}
				printf ("\n");
			}
		}

	#endif   //NS3_LOG_ENABLE

	arCoefficientsFile.close();

	return true;
}

double ArModel::DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
	NS_LOG_FUNCTION("---------NEW PACKET---------" << Simulator::Now().GetSeconds());
	double rxPowerDbm;
	double arOutput;
	double fastFadingRandomValue;
	double snr;
	NormalVariable fastFading (0.0, m_ffVariance);

	rxPowerDbm = m_propagationLoss->CalcRxPower(txPowerDbm, a, b);

	//Calculate the SNR
//	snr = rxPowerDbm - 10 * log10(m_noise);
	//Force the channel to get a fixed SNR
	snr = 10.0;

	/* Then we calculate the AR model output */
	arOutput = GetCurrentArValue (a, b);

	/* We also calculate the fast fading value */
	if(m_order) {
		fastFadingRandomValue = fastFading.GetValue();
	} else {
		fastFadingRandomValue = 0;
	}

	NS_LOG_DEBUG("Received packet (SNR = " << snr  << " dB) : TX power (" << txPowerDbm << " dBm) RX power (" << rxPowerDbm <<  		   \
			" dBm) + AR Output (" << arOutput << ") + Fast Fading (" << fastFadingRandomValue << ")");

	if (m_errorModel)
	{
		m_errorModel->SetUnwrappedSnr(snr, arOutput, fastFadingRandomValue);
	}
	else
		NS_LOG_WARN("No Bursty Error Model found");

	return  rxPowerDbm;
}

double ArModel::GetArCoefficient(int key, int vectorPosition) const
{

	double arCoefficient;
	coefSetIter_t iter = m_arCoefficientsMap.find(key);
	if(iter != m_arCoefficientsMap.end())
	{
		if (vectorPosition < (signed) iter->second.size())
			arCoefficient = (iter->second) [vectorPosition];
	}
	else {
		NS_LOG_ERROR ("AR coefficient not found");
		arCoefficient = 0.0;
	}

//	NS_LOG_FUNCTION(key << vectorPosition << arCoefficient);

	return arCoefficient;
}


double ArModel::GetCurrentArValue (Ptr<MobilityModel> sender, Ptr<MobilityModel> receiver) const
{

	bool txFound = false;
	bool rxFound = false;
	ChannelKey key;
	channelSetIter_t channelKeyIter;
	Ptr<ArChannelEntry> channel;
	u_int32_t i;
	int currentSize;
	double currentSnr;
	vector<double> previousSNR;
	NormalVariable randomArNoise (0.0, pow(m_stdDevDb,2));
	NormalVariable randomNoise (0.0, m_variance);

	//We need to get the transmitter and the receiver -- Pending work --> Search a smart solution
	for (i=0;i < NodeList().GetNNodes(); i ++)
	{
		Ptr<MobilityModel> temp = (NodeList().GetNode(i))->GetObject<MobilityModel> ();
		if ((temp == sender) && (txFound == false))
		{
			txFound = true;
			key.SetTx (i);
		}
		if ((temp == receiver) && (rxFound == false))
		{
			rxFound = true;
			key.SetRx (i);
		}
	}

	 NS_LOG_FUNCTION (key.GetTx() << key.GetRx());

	 channelKeyIter = m_channelSetMap.find(key);

	 if (channelKeyIter != m_channelSetMap.end())
		 channel = channelKeyIter->second;
	 else
		 channel = 0;

	 //If there is a channel defined, get the current SNR value
	 if (channel != 0)
	 {
		 currentSize = channel->GetPreviousSnr(previousSNR);
		 //If we have any packet buffered, use the Yule-Walker expression
		 if (currentSize && m_order)
		 {
			 currentSnr = randomNoise.GetValue();
			 #ifdef NS3_LOG_ENABLE
			 if (g_debug)
			 {
				 printf("Slow fading: SV[i] = ");
			 }
			 #endif //NS3_LOG_ENABLE
			 if (currentSize < m_order)
			 {

				 for (i=0; i < (unsigned) currentSize; i++)
				 {
					currentSnr -= previousSNR[i] * GetArCoefficient(currentSize, currentSize -i);
					#ifdef NS3_LOG_ENABLE
					if (g_debug)
					{
						printf("a [%d,%d] (%f) * SV [i-%d] (%f) ", currentSize , currentSize - i, GetArCoefficient(currentSize, currentSize -i),    \
								currentSize - i, previousSNR[i] );
						if ( i != (unsigned) currentSize - 1)
							printf("+ ");
					}
					#endif //NS3_LOG_ENABLE

				 }
				 #ifdef NS3_LOG_ENABLE
				 if (g_debug)
					 printf(" = %f\n", currentSnr);
				#endif //NS3_LOG_ENABLE
			 }
			 else
			 {
				 for (i=0; i < (unsigned) currentSize; i++)
				 {
					 currentSnr -= previousSNR[i] * GetArCoefficient(m_order, currentSize -i);
					 #ifdef NS3_LOG_ENABLE
					 if (g_debug)
					 {
						 printf("a [%d,%d] (%f) * SV [i-%d] (%f) ", m_order , currentSize - i, GetArCoefficient(m_order, currentSize -i),    \
								 currentSize - i, previousSNR[i] );
						 if ( i != (unsigned) currentSize - 1)
							 printf("+ ");
					 }
					 #endif //NS3_LOG_ENABLE

				 }
				 #ifdef NS3_LOG_ENABLE
				 if (g_debug)
					 printf(" = %f\n", currentSnr);
				 #endif //NS3_LOG_ENABLE
			 }
		 }


		 //When we have an empty queue (currentSize == 0) -> We set the Slow Varying value
		 //as a random number (Normal Variable with average centered at 0.0 and a variance
		 //equal to m_stdDevDb
		 else
		 {
			 currentSnr = randomArNoise.GetValue();
			 if (g_debug)
				 printf("Slow fading: SV[i] = %f\n", currentSnr);
		 }

		 if (m_order)
			 channel->UpdateSnr(currentSnr);
	 }
	 else
		 NS_LOG_ERROR("AR propagation loss model does not know the current tx/rx pair" << key.GetTx() << "/" << key.GetRx() );

	return currentSnr;
}


std::string ArModel::GetCwd () {
	NS_LOG_FUNCTION_NOARGS();
	char buf[FILENAME_MAX];
	char* succ = getcwd(buf, FILENAME_MAX);
	if (succ)
		return std::string(succ);
	return ""; 						// raise a flag, throw an exception, ...
}

NS_OBJECT_ENSURE_REGISTERED (BurstyErrorModel);

TypeId
BurstyErrorModel::GetTypeId(void) {
	static TypeId tid = TypeId ("ns3::BurstyErrorModel")
	.SetParent<ErrorModel> ()
	.AddConstructor<BurstyErrorModel> ()
	.AddAttribute("BearModel",
			"Flag to decide the error model decider to use",
			EnumValue(BEAR_MODEL),
			MakeEnumAccessor(&BurstyErrorModel::m_arModel),
			MakeEnumChecker(NO_ERROR_MODEL, "NO_ERROR_MODEL",
					BEAR_MODEL, "BEAR_MODEL",
					SHADOWING_MODEL, "SHADOWING_MODEL"))
	.AddAttribute ("RanVar",
			"Random variable which determine a packet to be successfully received or not",
			RandomVariableValue (UniformVariable (0.0, 1.0)),
			MakeRandomVariableAccessor (&BurstyErrorModel::m_ranvar),
			MakeRandomVariableChecker ())
	.AddTraceSource ("BearRxTrace",
			"Packet tracing",
	        MakeTraceSourceAccessor (&BurstyErrorModel::m_rxTrace))
		       ;
  return tid;
}

BurstyErrorModel::BurstyErrorModel()
{
	NS_LOG_FUNCTION_NOARGS();

	m_arModel = BEAR_MODEL;

	burstyErrorModelParams_t dataLogParams = {1.24, 0.366, 6.88, 3, 16};
	burstyErrorModelParams_t ackLogParams = {1.00, 0.886, 6.88, 0, 13};
	burstyErrorModelParams_t bcastCtrlLogParams = {1.9, 0.6, 0.0, 0, 10};
	m_dataLogParams = dataLogParams;
	m_ackLogParams = ackLogParams;
	m_bcastCtrlLogParams = bcastCtrlLogParams;
}

BurstyErrorModel::~BurstyErrorModel()
{
	NS_LOG_FUNCTION_NOARGS();
}

void BurstyErrorModel::SetRxCallback(BearRxCallback_t callback)
{
	NS_LOG_FUNCTION_NOARGS();
	m_rxCallback = callback;
}

bool BurstyErrorModel::DoCorrupt(Ptr<Packet> packet)
{
	NS_LOG_FUNCTION(m_unwrappedSnr.totalSnr);

	bool rxError;
	packetInfo_t packetInfo = ParsePacket(packet);

	//Decide whether the frame is correct or not according to the frame type (data, TCP or broadcast/control)
	if ((packetInfo.type == UDP_DATA || packetInfo.type == TCP_DATA) && (packetInfo.payloadLength > 4))		//Discard ACKs TCP
	{
		rxError = CorruptDataFrame(packet);
	}
	else if (packetInfo.type == TCP_DATA && packetInfo.payloadLength < 4 \
			&& (!(packetInfo.tcpHdr.GetFlags() & 0x02)	&& !(packetInfo.tcpHdr.GetFlags() & 0x01)))  			//TCP ACK particular logistic function
	{
		rxError = CorruptAckFrame(packet);

	}
	else if (packetInfo.wifiHdr.IsCtl() || packetInfo.wifiHdr.IsMgt() ||  \
			(packetInfo.wifiHdr.GetAddr1()).IsBroadcast() || \
			packetInfo.ipv4Hdr.GetDestination().IsBroadcast())
		rxError = CorruptBcastCtrlFrame(packet);
	else
		rxError = false;

	//Tracing and callbacks
	m_rxTrace (packet, Simulator::Now(), rxError, m_unwrappedSnr.propagationSnr, m_unwrappedSnr.slowFading, m_unwrappedSnr.fastFading);

	if (!m_rxCallback.IsNull ())
	{
		m_rxCallback (packet, Simulator::Now(), rxError, m_unwrappedSnr.propagationSnr, m_unwrappedSnr.slowFading, m_unwrappedSnr.fastFading);
	}
	return rxError;
}

bool BurstyErrorModel::CorruptDataFrame(Ptr<Packet>)
{
	NS_LOG_FUNCTION_NOARGS();
	double fer;
	double value;
	bool error;

	//Reception decision
		switch (m_arModel)	{

		case NO_ERROR_MODEL:
			fer = 0.0;
			break;
		case BEAR_MODEL:
			fer = GetBearFer(&m_dataLogParams);
			break;
		case SHADOWING_MODEL:
			if (m_unwrappedSnr.totalSnr < 9)
				fer = 1.0;
			else
				fer = 0.0;
			break;

		default:
			NS_LOG_ERROR("Error model not found... Please fix");
			fer = 0.0;
			break;
		}
		//Use a random value to decide if the frame is received correctly
		value = m_ranvar.GetValue();
		NS_LOG_DEBUG ("Random value (" << value << ") < FER (" << fer << ")");
		if (value  < fer)
		{
			NS_LOG_LOGIC("CORRUPT!!");
			error = true;
		}
		else
		{
			NS_LOG_LOGIC("CORRECT!!");
			error = false;
		}
		return error;
}

bool BurstyErrorModel::CorruptAckFrame(Ptr<Packet>)
{
	NS_LOG_FUNCTION_NOARGS();
	double fer;
//	double value;
	bool error;

	//Reception decision
	switch (m_arModel)	{

	case NO_ERROR_MODEL:
		fer = 0.0;
		break;
	case BEAR_MODEL:
//		fer = GetBearFer(&m_ackLogParams);
		fer = 0.0;			//Test version --> All ACK (TCP) are received correctly

		break;
	case SHADOWING_MODEL:
		if (m_unwrappedSnr.totalSnr < 8)
			fer = 1.0;
		else
			fer = 0.0;
		break;

	default:
		NS_LOG_ERROR("Error model not found... Please fix");
		fer = 0.0;
				break;
	}
	//Use a random value to decide if the frame is received correctly
//	value = m_ranvar.GetValue();
//	NS_LOG_UNCOND ("Random value (" << value << ") < FER (" << fer << ")");

	//Force ACK to be correct
//	return false;

	if (m_ranvar.GetValue() < fer)
		error = true;
	else
		error = false;

	return error;
}

bool BurstyErrorModel::CorruptBcastCtrlFrame(Ptr<Packet>)
{
	NS_LOG_FUNCTION_NOARGS();
	double fer;
	double value;
	bool error;
	//Reception decision
	switch (m_arModel)	{

	case NO_ERROR_MODEL:
		fer = 0.0;
		break;
	case BEAR_MODEL:
		fer = GetBearFer(&m_bcastCtrlLogParams);
		break;
	case SHADOWING_MODEL:
		if (m_unwrappedSnr.totalSnr < 1.7)
			fer = 1.0;
		else
			fer = 0.0;
		break;

	default:
		NS_LOG_ERROR("Error model not found... Please fix");
		fer = 0.0;
		break;
	}
	//Use a random value to decide if the frame is received correctly
	value = m_ranvar.GetValue();
	NS_LOG_DEBUG ("Random value (" << value << ") < FER (" << fer << ")");

	//// In order not to get an ARP message lost which cut the measurement out, force the broadcast frames to be correct
	return false;

	if (m_ranvar.GetValue() < fer)
		error = true;
	else
		error = false;

	return error;
}

double BurstyErrorModel::GetBearFer(burstyErrorModelParams_t *params)
{
	NS_LOG_FUNCTION_NOARGS();
	double fer;

	if (m_unwrappedSnr.totalSnr < params->lowThreshold)
		fer = 1;
	else if (m_unwrappedSnr.totalSnr < params->highThreshold)
	{
		fer = params->a / (1 + exp(params->b * (m_unwrappedSnr.totalSnr - params->c)));
		NS_LOG_DEBUG ("FER = " << params->a << " / (1 + e^(" << params->b << "* (" << m_unwrappedSnr.totalSnr << 				\
				" - " << params->c << "))) = " << fer);
	}
	else
		fer = 0;
	return fer;
}

void BurstyErrorModel::DoReset()
{
	NS_LOG_FUNCTION_NOARGS();
}

void BurstyErrorModel::SetUnwrappedSnr(double propagationSnr, double slowFadingComponent, double fastFadingComponent)
{
	NS_LOG_FUNCTION(propagationSnr << slowFadingComponent << fastFadingComponent);
	m_unwrappedSnr.propagationSnr = propagationSnr;
	m_unwrappedSnr.slowFading = slowFadingComponent;
	m_unwrappedSnr.fastFading = fastFadingComponent;

	m_unwrappedSnr.totalSnr = propagationSnr + slowFadingComponent + fastFadingComponent;
	//Do not allow non-positive SNRs
	if (m_unwrappedSnr.totalSnr < 0)
		m_unwrappedSnr.totalSnr = 0;
}

packetInfo_t BurstyErrorModel::ParsePacket (Ptr<Packet> packet)
{
	NS_LOG_FUNCTION(packet);

	packetInfo_t packetInfo;
	Ptr<Packet> pktCopy = packet->Copy();

	pktCopy->RemoveHeader(packetInfo.wifiHdr);

	if (packetInfo.wifiHdr.IsData())
	{
		pktCopy->RemoveHeader(packetInfo.llcHdr);
		switch (packetInfo.llcHdr.GetType())
		{
		case 0x0806:			//ARP
			packetInfo.type = ARP_PACKET;
			break;
		case 0x0800:			//IP packet
			pktCopy->RemoveHeader(packetInfo.ipv4Hdr);
			switch (packetInfo.ipv4Hdr.GetProtocol())
			{
			case 6:				//TCP
				pktCopy->RemoveHeader(packetInfo.tcpHdr);
				packetInfo.type = TCP_DATA;
				break;
			case 17:			//UDP
				pktCopy->RemoveHeader(packetInfo.udpHdr);
				packetInfo.type = UDP_DATA;
				break;
			default:
				NS_LOG_ERROR ("Protocol not implemented yet (IP header) --> " << packetInfo.ipv4Hdr.GetProtocol());
				break;
			}
			break;
		default:
			NS_LOG_ERROR ("Protocol not implemented yet (LLC header) --> " << packetInfo.llcHdr.GetType());
			break;
		}
	}
	else if (packetInfo.wifiHdr.IsAck())
	{
		packetInfo.type = IEEE_80211_ACK;
	}
	else			// 802.11 Control/Management frame
	{
		packetInfo.type = IEEE_80211_NODATA;
	}

	packetInfo.payloadLength = pktCopy->GetSize() - 4;								//Last four bytes are used for tagging

	return packetInfo;
}





