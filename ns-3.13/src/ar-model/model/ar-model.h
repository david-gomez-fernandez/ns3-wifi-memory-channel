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

#ifndef AR_MODEL_H_
#define AR_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include <unistd.h>

#include "ns3/object.h"
#include "ns3/random-variable.h"

//AR model will play the role as the derived classes of both Error and Propagation Loss models
#include "ns3/error-model.h"
#include "ns3/mobility-model.h"
#include "ns3/propagation-loss-model.h"

#include "ns3/event-id.h"

//Needed to parse the packet content (BurstyErrorModel::ParsePacket)
#include "ns3/wifi-mac-header.h"
#include "ns3/llc-snap-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"

//Tracing
#include "ns3/traced-value.h"
#include "ns3/traced-callback.h"

using namespace std;
namespace ns3 {

class Packet;
class BurstyErrorModel;
typedef int32_t nsaddr_t;

struct prevValues_t {
	double 		snr;
	Time 		time;
};

typedef struct {
	double propagationSnr;
	double slowFading;
	double fastFading;
	double totalSnr;
} snrComposition_t;

class ChannelKey {
public:
	ChannelKey() {}
	ChannelKey (nsaddr_t tx, nsaddr_t rx) : m_tx(tx), m_rx(rx) {}
	virtual ~ChannelKey () {}

	bool operator < (const ChannelKey& key) const {
		if(m_tx < key.m_tx) {
			return true;
		} else if(m_tx == key.m_tx) {
			return(m_rx < key.m_rx);
		} else {
			return false;
		}
	}

	u_int32_t GetRx () const;
	u_int32_t GetTx () const;
	void SetRx (u_int32_t rx);
	void SetTx (u_int32_t tx);

private:
	int m_tx;
	int m_rx;
};

class ArChannelEntry: public Object {
public:
	/**
	 * Constructor
	 */
	ArChannelEntry ();
	/**
	 * \param order Auto Regressive filter order
	 * \
	 */
	ArChannelEntry (int order, double coherenceTime);

	/**
	 * Destructor
	 */
	~ArChannelEntry ();
	 /**
	  * When a new frame arrives, we need to update the vector that contains the AR order previous frames information; furthermore, we must
	  * update the ArChannelEntry oldest received frame timeout
	  * \param snr SNR read from the physical layer
	  * \param order Order of the AR filter
	  */
	 void UpdateSnr (double snr);

	 /**
	  *  \param previousSnr Vector which will hold the SNR values (window)
	  *  \returns The size of the input vector
	  */
	 int GetPreviousSnr (vector<double> &previousSnr);

	 /**
	  * \param coherenceTime Channel coherence
	  * \returns The instant the next timeout should be invoked
	  */
	 double GetNextTimeout();
	 /**
	  * \brief Handle the timeout event when the coherence timer expires
	  */
	 void HandleCoherenceTimeout ();

	 u_int32_t GetRx () const;
	 u_int32_t GetTx () const;
	 void SetRx (u_int32_t rx);
	 void SetTx (u_int32_t tx);

	 /**
	  *  \brief Print the captured packet queue (AR model window)
	  */
	 void DisplaySnrQueue (void);

private:

	 u_int32_t m_tx;
	 u_int32_t m_rx;

	 //Vector that stores the SNR and the timestamp of the overheard packets (Size set by the AR filter order)
	 vector<struct prevValues_t> m_previousSnr;

	 //One timer per ChannelEntry object
	 EventId m_coherenceTimeout;

	 int m_order;
	 double m_coherenceTime;
};


class ArModel: public PropagationLossModel
{
public:
	/**
	 * Attribute handler
	 */
	static TypeId GetTypeId (void);
	/**
	 * Default constructor
	 */
	ArModel ();

	/**
	 * Default destructor
	 */
	virtual ~ArModel ();

	/**
	 * \param name the name of the model to set
	 * \param n0 the name of the attribute to set
	 * \param v0 the value of the attribute to set
	 * \param n1 the name of the attribute to set
	 * \param v1 the value of the attribute to set
	 * \param n2 the name of the attribute to set
	 * \param v2 the value of the attribute to set
	 * \param n3 the name of the attribute to set
	 * \param v3 the value of the attribute to set
	 * \param n4 the name of the attribute to set
	 * \param v4 the value of the attribute to set
	 * \param n5 the name of the attribute to set
	 * \param v5 the value of the attribute to set
	 * \param n6 the name of the attribute to set
	 * \param v6 the value of the attribute to set
	 * \param n7 the name of the attribute to set
	 * \param v7 the value of the attribute to set
	 *
	 * Configure a propagation delay for this channel.
	 */
	void SetPropagationLoss (std::string name,
			std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
			std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
			std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
			std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
			std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
			std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
			std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
			std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());

	/**
	 * \brief Set the propagation loss model
	 * \param loss Pointer to the propagation loss model to set
	 */
	void SetPropagationLoss (Ptr<PropagationLossModel> loss);

	/**
	 * \brief Set the error model
	 * \param error Pointer to the error model to set
	 */
	void SetErrorModel (Ptr<BurstyErrorModel> error);

	/**
	 * \returns The Bursty error model linked to this class
	 */
	Ptr<BurstyErrorModel> GetErrorModel ();
	/**
	 * \param fileName Name of the file which holds the AR filter coefficient
	 */
	bool GetCoefficients (string fileName);

	/**
	 * \param key AR coefficient set map key value
	 * \param vectorPosition The position in which stays the desired coefficient
	 * \returns AR coefficient
	 */
	double GetArCoefficient (int key, int vectorPosition) const;

	/**
	 * \brief
	 * \param tx Transmitter index
	 * \param rx Reveiver index
	 * \returns Auto Regressive filter obtained value (dB)
	 */
	double GetCurrentArValue (Ptr<MobilityModel> sender, Ptr<MobilityModel> receiver) const;

private:


	//PropagationLossModel virtual (abstract) method --> Equivalent to ns-2 arModel::Pr (PacketStamp *t_, PacketStamp *r_, WirelessPhy *ifp_)
	virtual double DoCalcRxPower (double txPowerDbm,
			Ptr<MobilityModel> a,
			Ptr<MobilityModel> b) const;

	/* AR mode parameters */
	int m_order;               /* Order of the AR filter  */
	double m_variance;         /* Input noise variance    */

	double m_ffVariance;       /* Fast Fading variance    */

	double m_coherenceTime;    /* Coherence time          */
	bool m_symmetry;            /* Am I using symmetry?    */

	/* Other parameters of the channel */
	double m_pathLossExp;      /* path loss exponent      */
	double m_stdDevDb;         /* standard deviation (dB) */

	double m_noise;				/* Noise Power used to model the SNR */

	RandomVariable m_ranvar;

	/* All the channels (one per tx/rx pair) */
	typedef map<ChannelKey, Ptr <ArChannelEntry> > channelSet_t;
	typedef channelSet_t::const_iterator channelSetIter_t;

	channelSet_t m_channelSetMap;

	/* The coeficients of the AR model */
	typedef map<int, vector<double> > coefSet_t;
	typedef coefSet_t::const_iterator coefSetIter_t;

	coefSet_t m_arCoefficientsMap;

	//Generic propagation model
	Ptr<PropagationLossModel> m_propagationLoss;

	//BEAR model
	Ptr<BurstyErrorModel> m_errorModel;

	//Coefficient file name
	string m_coefficientsFile;

protected:
	/**
	 * \return The current path (in string format)
	 */
	static std::string GetCwd ();

};

//Error model selection
typedef enum {
	NO_ERROR_MODEL,
	BEAR_MODEL,
	SHADOWING_MODEL
} errorModelOption_t;

//Parameter to carry out the error decision
typedef struct {
	double a;
	double b;
	double c;
	int lowThreshold;
	int highThreshold;
} burstyErrorModelParams_t;

enum PacketType{
	IEEE_80211_ACK,
	IEEE_80211_NODATA,
	ARP_PACKET,
	UDP_DATA,
	TCP_DATA
};

typedef struct {
	WifiMacHeader wifiHdr;
	LlcSnapHeader llcHdr;
	Ipv4Header ipv4Hdr;
	TcpHeader tcpHdr;
	UdpHeader udpHdr;
	u_int16_t payloadLength;
	PacketType type;

} packetInfo_t;

class BurstyErrorModel: public ErrorModel
{
public:
	/**
	 * arg1: packet received successfully
	 * arg2: packet timestamp
	 * arg3: Boolean that represents whether a packet has been succesfully received or not
	 * arg4: snr (global) of packet
	 * arg5: snr due to the AR model (Slow fading)
	 * arg6: Fast Fading related SNR
	 */
	typedef Callback<void,Ptr<Packet>, Time, bool, double, double, double> BearRxCallback_t;

	/**
	 * Attribute handler
	 */
	static TypeId GetTypeId (void);
	/**
	 * Default constructor
	 */
	BurstyErrorModel ();
	/**
	 * Default destructor
	 */
	virtual ~BurstyErrorModel ();
	/**
	 * \brief Apply a logistic function to decide whether a data frame is correct or not
	 * \param packet The packet received
	 * \returns True if the packet is corrupted
	 */
	bool CorruptDataFrame (Ptr<Packet> packet);
	/**
	 * \brief Apply a logistic function to decide whether an ack (TCP) frame is correct or not
	 * \param packet The packet received
	 * \returns True if the packet is corrupted
	 */
	bool CorruptAckFrame (Ptr<Packet> packet);
	/**
	 * \brief Apply a logistic function to decide whether a broadcast/control/management frame is correct or not
	 * \param packet The packet received
	 * \returns True if the packet is corrupted
	 */
	bool CorruptBcastCtrlFrame (Ptr<Packet> packet);

	/**
	 * \brief Use the logistic function to obtain the FER
	 * \param params Struct that holds the logistic function parameters
	 * \returns The FER value
	 */
	double GetBearFer (burstyErrorModelParams_t *params);

	/**
	 * \param rxComponent Value obtained from the propagation loss model
	 * \param slowFadingComponent Auto regressive component that tries to emulate the Slow Fading wireless channel behaviour
	 * \param fastFadingComponent Random value which represent Fast Fading
	 */
	void SetUnwrappedSnr (double propagationSnr, double slowFadingComponent, double fastFadingComponent);

	//Callback invoked when a packet is received by the error model object
	void SetRxCallback (BearRxCallback_t callback);

	/**
	 * \param packet Packet received by the node
	 * \returns Struct which contains the info relative to the packet
	 */
	packetInfo_t ParsePacket (Ptr<Packet> packet);

private:
	//ErrorModel virtual (abstract) methods
	virtual bool DoCorrupt (Ptr<Packet>);
	virtual void DoReset (void);

	RandomVariable m_ranvar;

	//Logistic function configuration parameters
	burstyErrorModelParams_t m_dataLogParams;
	burstyErrorModelParams_t m_ackLogParams;
	burstyErrorModelParams_t m_bcastCtrlLogParams;

	//Choose among the different options (0- No model, 1- BEAR model, 2- Shadowing model)
	errorModelOption_t m_arModel;

	snrComposition_t m_unwrappedSnr;

	// Tracing
	/**
	 * The trace source fired when a packet ends the reception process from
	 * the medium.
	 *
	 * \see class CallBackTraceSource
	 */
	TracedCallback<Ptr<const Packet>, Time, bool, double, double, double> m_rxTrace;
	BearRxCallback_t m_rxCallback;

};


}  //namespace ns3
#endif /* AR_MODEL_H_ */
