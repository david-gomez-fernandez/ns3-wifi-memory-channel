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

#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/string.h"

#include "ns3/hidden-markov-error-model.h"

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HiddenMarkovErrorModel");
NS_OBJECT_ENSURE_REGISTERED (HiddenMarkovErrorModel);

const bool g_debug = false;  //Temporal solution (only for debugging)

TypeId
HiddenMarkovErrorModel::GetTypeId(void) {
	static TypeId tid = TypeId ("ns3::HiddenMarkovErrorModel")
	.SetParent<ErrorModel> ()
	.AddConstructor<HiddenMarkovErrorModel> ()
	.AddAttribute ("RanVar",
			"Random variable which determine a packet to be successfully received or not",
			RandomVariableValue (UniformVariable (0.0, 1.0)),
			MakeRandomVariableAccessor (&HiddenMarkovErrorModel::m_ranvar),
			MakeRandomVariableChecker ())
	.AddAttribute ("AverageFrameDuration",
			"Average time (in microseconds) between two consecutive frames (used to model the exponential interarrival process)",
			DoubleValue (2000.0),
			MakeDoubleAccessor (&HiddenMarkovErrorModel::m_averageInterFrameDuration),
			MakeDoubleChecker<double> ())
	.AddAttribute("InitialState",
			"Initial simulation state (Default 0)",
			IntegerValue(0),
			MakeIntegerAccessor(&HiddenMarkovErrorModel::m_currentState),
			MakeIntegerChecker<int>())
	.AddAttribute ("HiddenStates",
			"Number of hidden states at the Markov's chain",
			IntegerValue (2),
			MakeIntegerAccessor (&HiddenMarkovErrorModel::m_hiddenStates),
			MakeIntegerChecker<int> ())
	.AddAttribute ("ErrorUnit",
		   "Type of simulation (frames or time)",
	       EnumValue (EU_TIME),
	       MakeEnumAccessor (&HiddenMarkovErrorModel::m_unit),
	       MakeEnumChecker (EU_BYTE, "EU_BYTE",
	                        EU_PKT, "EU_PKT",
	                        EU_BIT, "EU_BIT",
	                        EU_TIME, "EU_TIME"))
	.AddAttribute("TransitionMatrixFileName",
			"Name of the file which contains the transition matrix (proprietary format)",
			StringValue("HMM_16states/HMM_09_TR_1.txt"),
			MakeStringAccessor(&HiddenMarkovErrorModel::m_transitionMatrixFileName),
			MakeStringChecker())
	.AddAttribute("EmissionMatrixFileName",
			"Name of the file which contains the emission matrix (proprietary format)",
			StringValue("HMM_16states/HMM_09_EMIS_1.txt"),
			MakeStringAccessor(&HiddenMarkovErrorModel::m_emissionMatrixFileName),
			MakeStringChecker())
	.AddTraceSource ("HiddenMarkovErrorModelRxTrace",
			"Packet tracing",
			MakeTraceSourceAccessor (&HiddenMarkovErrorModel::m_rxTrace))
	       ;
  return tid;
}

HiddenMarkovErrorModel::HiddenMarkovErrorModel() : m_currentState (0),
		m_started(false)
{
	NS_LOG_FUNCTION_NOARGS();
//	m_hiddenStates = 2;
//	m_unit = EU_TIME;
//	m_averageInterFrameDuration = 2000; 					//Inter-frame average duration (microseconds)
//	m_ranvar = UniformVariable(0.0, 1.0);
//
//	m_transitionMatrixFileName = "HMM_16states/HMM_09_TR_1.txt";
//	m_emissionMatrixFileName = "HMM_16states/HMM_09_EMIS_1.txt";
	//Read coefficients files and load the corresponding parameters
//	GetCoefficients();
}

HiddenMarkovErrorModel::~HiddenMarkovErrorModel()
{
	NS_LOG_FUNCTION_NOARGS();
	if (m_transitionMatrix.size() > 0)
		m_transitionMatrix.clear();
	if (m_emissionMatrix.size() > 0)
		m_emissionMatrix.clear();
	if (m_meanDurationVector.size() > 0)
		m_meanDurationVector.clear();
}


void HiddenMarkovErrorModel::SetTransitionMatrixFileName (string transitionMatrixFileName)
{
	NS_LOG_FUNCTION_NOARGS();
	m_transitionMatrixFileName = transitionMatrixFileName;
}

string HiddenMarkovErrorModel::GetTransitionMatrixFileName (void) const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_transitionMatrixFileName;
}

void HiddenMarkovErrorModel::SetEmissionMatrixFileName (string emissionMatrixFileName)
{
	NS_LOG_FUNCTION_NOARGS();
	m_emissionMatrixFileName = emissionMatrixFileName;
}

string HiddenMarkovErrorModel::GetEmissionMatrixFileName (void) const
{
	NS_LOG_FUNCTION_NOARGS();
	return m_emissionMatrixFileName;
}

u_int8_t HiddenMarkovErrorModel::GetHiddenStates() const
{
	NS_LOG_FUNCTION_NOARGS();
    return m_hiddenStates;
}

void HiddenMarkovErrorModel::SetHiddenStates(u_int8_t hiddenStates)
{
	NS_LOG_FUNCTION_NOARGS();
    this->m_hiddenStates = hiddenStates;
}

double HiddenMarkovErrorModel::GetAverageFrameDuration() const
{
	NS_LOG_FUNCTION_NOARGS();
    return m_averageInterFrameDuration;
}

void HiddenMarkovErrorModel::SetAverageFrameDuration(double averageFrameDuration)
{
	NS_LOG_FUNCTION_NOARGS();
    this->m_averageInterFrameDuration = averageFrameDuration;
}

ErrorUnit HiddenMarkovErrorModel::GetUnit() const
{
    return m_unit;
}

void HiddenMarkovErrorModel::SetUnit(ErrorUnit unit)
{
    this->m_unit = unit;
}

int HiddenMarkovErrorModel::GetCurrentState() const
{
    return m_currentState;
}

void HiddenMarkovErrorModel::SetCurrentState(int state)
{
    this->m_currentState = state;
}


void HiddenMarkovErrorModel::Enable()
{
	NS_LOG_FUNCTION_NOARGS ();
	m_enable = true;
}

void HiddenMarkovErrorModel::Disable()
{
	NS_LOG_FUNCTION_NOARGS ();
	m_enable = false;
}

bool HiddenMarkovErrorModel::IsEnabled() const
{
	NS_LOG_FUNCTION_NOARGS ();
	return m_enable;
}

std::string HiddenMarkovErrorModel::GetCwd() {
	NS_LOG_FUNCTION_NOARGS();
	char buf[FILENAME_MAX];
	char* succ = getcwd(buf, FILENAME_MAX);
	if (succ)
		return std::string(succ);
	return ""; 						// raise a flag, throw an exception, ...
}

bool HiddenMarkovErrorModel::GetCoefficients ()
{
	NS_LOG_FUNCTION(m_transitionMatrixFileName <<  m_emissionMatrixFileName);

	string transitionMatrixPath;
	string emissionMatrixPath;

	fstream transitionMatrixFile;
	fstream emissionMatrixFile;
	char line[256];

	int  rowNumber,i,j;												//Auxiliar counters
	double coefficient;
	vector<double> CoefficientsVector;

	//If invoked in a second time), flush the previous maps content
	if (m_transitionMatrix.size())
		m_transitionMatrix.clear();
	if (m_emissionMatrix.size())
		m_emissionMatrix.clear();
	if (m_meanDurationVector.size())
		m_meanDurationVector.clear();

	coefSetIter_t iter_;

	transitionMatrixPath = GetCwd() + "/src/hidden-markov-error-model/configs/" + m_transitionMatrixFileName;
	emissionMatrixPath = GetCwd() + "/src/hidden-markov-error-model/configs/" + m_emissionMatrixFileName;

	transitionMatrixFile.open((const char *) transitionMatrixPath.c_str(), ios::in);
	emissionMatrixFile.open((const char *) emissionMatrixPath.c_str(), ios::in);

	rowNumber = 0;

	if (transitionMatrixFile) {
		while (transitionMatrixFile.getline(line, 256)) {
			if (rowNumber == 0) { 											//First item in file--> Number of states in the Hidden Markov Chain
				m_states = atoi(line);
			} else { 														//Rest of values are the coefficient of the channel model
				j = 0;
				for (i = 0; i < m_states; i++) {
					//First value at position '0'
					if (i == 0) {
						coefficient = atof(line);
						CoefficientsVector.push_back(coefficient);
						j++;
					} else {
						while (line[j] != ' ' && line[j] != '\t') { 		//Look for "white" spaces between Coefficients
							j++;
						}
						coefficient = atof(line + j);
						j++;
						CoefficientsVector.push_back(coefficient);
					}
				}
				m_transitionMatrix.insert(pair<int, vector <double> > (rowNumber - 1, CoefficientsVector));   //rowNumber shifted 1 position
				CoefficientsVector.clear();

				//As seen in the analytical studio, the probability to hold on the same state is calculated as follows:
				//N_i = 1 / (1 - a_ii)

				m_meanDurationVector.push_back (1 / (1 - (m_transitionMatrix[rowNumber - 1][rowNumber -1 ])));
			}
			rowNumber++;
		}
	}

	else {
		NS_LOG_ERROR("File (HMM)" << transitionMatrixPath << " not found: Please fix");
		return false;
	}

	//Reset the rowNumber counter
	rowNumber = 0;

	if (emissionMatrixFile) {
		while (emissionMatrixFile.getline(line, 256)) {
			j = 0;
			for (i = 0; i < m_hiddenStates; i++) {
				//First value at position '0'
				if (i == 0) {
					coefficient = atof(line);
					CoefficientsVector.push_back(coefficient);
					j++;
				} else {
					while (line[j] != ' ' && line[j] != '\t') { 		//Look for "white" spaces between Coefficients
						j++;
					}
					coefficient = atof(line + j);
					j++;
					CoefficientsVector.push_back(coefficient);
				}
			}
			m_emissionMatrix.insert(pair<int, vector <double> > ((int) rowNumber, CoefficientsVector));
			CoefficientsVector.clear();
			rowNumber ++;

		}
	} else {
		NS_LOG_ERROR("File (HMM)" << emissionMatrixPath << " not found: Please fix");
		return false;
	}

	//DEBUGGING
#ifdef NS3_LOG_ENABLE
	if(g_debug)
	{

		//Print the m_transitionMatrix map
		printf("---Transition Matrix---\n");
		for (iter_ = m_transitionMatrix.begin(); iter_ != m_transitionMatrix.end(); iter_ ++ )
		{
			for (i = 0; i < (int) (iter_->second).size(); i++)
			{
				printf("%f  ", (iter_->second)[i]);
			}
			printf("\n");
		}

		//Print the m_emissionMatrix map
		printf("---Emission Matrix---\n");
		for (iter_ = m_emissionMatrix.begin(); iter_ != m_emissionMatrix.end();
				iter_++) {
			for (i = 0; i < (int) (iter_->second).size(); i++) {
				printf("%f  ", (iter_->second)[i]);
			}
			printf("\n");
		}


		//Print the m_meanDurationVector
		printf("---Mean Duration within each state (in frames)---\n");
		for (j = 0; j < (int) m_meanDurationVector.size(); j++ )
		{
			printf("%f\n", m_meanDurationVector[j]);
		}
	}

#endif   //NS3_LOG_ENABLE

	transitionMatrixFile.close();
	emissionMatrixFile.close();

	return true;
}

bool HiddenMarkovErrorModel::DoCorrupt(Ptr<Packet> packet)
{
	NS_LOG_FUNCTION(this);
	bool corruptedPacket;
	WifiMacHeader hdr;
	LlcSnapHeader llcHdr;
	Ipv4Header ipv4Hdr;
	UdpHeader udpHdr;
	TcpHeader tcpHdr;

	Ptr<Packet> pktCopy = packet->Copy();

	pktCopy->RemoveHeader(hdr);

	//In a time-based simulation, start the timers with the first reception
	if (m_started == false && m_unit == EU_TIME)
	{
		m_started = true;
		InitializeTimer();
		NS_LOG_DEBUG("Timer initialized");
	}

	//Decide whether the frame is correct or not according to the frame type (data, TCP or broadcast/control)
	//Force packet with length < 128 to be always correct
	if (hdr.IsData() && !hdr.GetAddr1().IsBroadcast())
	{
		//We have split the packet decision into the following three conditions:
		// - ARP frames --> Always correct
		// - TCP ACK --> Always correct
		// - Data frames --> Legacy HMM decision process
		pktCopy->RemoveHeader(llcHdr);

		switch (llcHdr.GetType())
		{
		case 0x0806:			//ARP
			corruptedPacket = false;
			break;
		case 0x0800:			//IP packet
			pktCopy->RemoveHeader(ipv4Hdr);
			switch (ipv4Hdr.GetProtocol())
			{
			case 6:				//TCP
				pktCopy->RemoveHeader(tcpHdr);

				//Data segments --> To be errored
				if (pktCopy->GetSize() > 4)
					corruptedPacket = Decide ();
				else
					corruptedPacket = false;
				break;
			case 17:			//UDP
				corruptedPacket =  Decide();
				break;
			default:
				NS_LOG_ERROR ("Protocol not implemented yet (IP) --> " << ipv4Hdr.GetProtocol());
				break;
			}
			break;
			default:
				NS_LOG_ERROR ("Protocol not implemented yet (LLC) --> " << llcHdr.GetType());
				break;
		}
	}

	//Force 802.11 ACKs, broadcast and control/management frames to be correct
	else if (hdr.IsAck())
	{
		corruptedPacket = false;

	}
	else if (hdr.IsCtl() || hdr.IsMgt() || (hdr.GetAddr1()).IsBroadcast())
	{
		corruptedPacket = false;
	}
	else
	{
		corruptedPacket = false;
	}

	//Packet-based simulation--> Change state attempt
	if (m_unit != EU_TIME)
		ChangeState();

	//Tracing and callbacks
	m_rxTrace (packet, Simulator::Now(),corruptedPacket, m_currentState);
	if (!m_rxCallback.IsNull())
		m_rxCallback (packet, Simulator::Now(),corruptedPacket, m_currentState);

	return corruptedPacket;
}

bool HiddenMarkovErrorModel::Decide ()
{
	NS_LOG_FUNCTION_NOARGS ();
	bool corruptedPacket;
	//Two posibilities:
	//Time --> Check into the emission matrix (current state)
	//Frames --> The same criteria as the time-based one and a possible state change
	if (m_ranvar.GetValue() < (m_emissionMatrix[m_currentState])[0]) //First column in emission matrix --> Error probability (state i)
	{
		NS_LOG_LOGIC("CORRUPT! (" << this << ") (" << Simulator::Now().GetSeconds() << ") State: " << m_currentState);
		corruptedPacket = true; 			//Frame received with errors
	}
	else
	{
		NS_LOG_LOGIC("CORRECT! (" << this << ") (" << Simulator::Now().GetSeconds() << ") State: " << m_currentState);
		corruptedPacket = false;			//Frame received successfully
	}

	return corruptedPacket;
}

void HiddenMarkovErrorModel::DoReset (void)
{
	NS_LOG_FUNCTION_NOARGS();
	m_currentState = 0;
}

bool HiddenMarkovErrorModel::ChangeState(void) {
	NS_LOG_FUNCTION("Object" << this << "State" << m_currentState << "Time" << Simulator::Now().GetSeconds());
	u_int16_t i, maxState;
	double transitionProbability, max;
	max = -1;
	for (i = 0; (int) i < (m_transitionMatrix[m_currentState]).size(); i++) {
		transitionProbability = (m_transitionMatrix[m_currentState])[i]	* m_ranvar.GetValue();
		//Different possibilities, depending on the type of simulation chosen:
		//EU_TIME: One call to this method brings about necessarily a state change (called after every average state stay duration)
		//Otherwise: As called at each frame reception, it may hold the same state
		if (m_unit == EU_TIME)  		//&& (transitionProbability > max) && (i != m_currentState))   //Time-based --> MUST change state
			{
			if ((transitionProbability > max) && (i != m_currentState)) {
				max = transitionProbability;
				maxState = i;
			}
		} else {
			if (transitionProbability > max) {
				max = transitionProbability;
				maxState = i;
			}
		}
	}
	//Did actually make a state change??
	if (m_currentState != maxState) {
		NS_LOG_DEBUG( "(" << m_currentState << ") --> (" << maxState << ") (" << this << ")" );
		m_currentState = maxState;
		return true;
	}
	return false;
}

void HiddenMarkovErrorModel::InitializeTimer()
{
	NS_LOG_FUNCTION(this);
	double nextTimeout;
	double nextTimeoutMeanValue;

	//Set the next timeout
	nextTimeoutMeanValue = m_meanDurationVector[m_currentState] * m_averageInterFrameDuration;
	ExponentialVariable expVar(nextTimeoutMeanValue);
	nextTimeout = expVar.GetValue();

	NS_LOG_INFO("(" << Simulator::Now().GetSeconds() << ") - Next timeout " << nextTimeoutMeanValue   \
				<< " --> " << nextTimeout << " (" << m_currentState << ")");
	Simulator::Schedule(MicroSeconds(nextTimeout),&HiddenMarkovErrorModel::TimerHandler, this);
}

void HiddenMarkovErrorModel::TimerHandler()
{
	NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds());
	double nextTimeout;
	double nextTimeoutMeanValue;

	//Once the timeout is reached, check if the states changes

	//Set the next timeout
	nextTimeoutMeanValue = m_meanDurationVector[m_currentState] * m_averageInterFrameDuration;
	ExponentialVariable expVar(nextTimeoutMeanValue);
	nextTimeout = expVar.GetValue();

	ChangeState();
	NS_LOG_INFO("(" << Simulator::Now().GetSeconds() << ") - Next timeout " << nextTimeoutMeanValue   \
			<< " --> " << nextTimeout << " (" << m_currentState << ")");
	Simulator::Schedule(MicroSeconds(nextTimeout),&HiddenMarkovErrorModel::TimerHandler, this);
}

void HiddenMarkovErrorModel::SetRxCallback(HiddenMarkovErrorModelRxCallback_t callback)
{
	NS_LOG_FUNCTION_NOARGS();
	m_rxCallback = callback;
}













