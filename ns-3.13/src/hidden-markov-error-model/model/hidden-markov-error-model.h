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

#ifndef HIDDEN_MARKOV_ERROR_MODEL_H_
#define HIDDEN_MARKOV_ERROR_MODEL_H_

#include "ns3/object.h"
#include "ns3/random-variable.h"
#include "ns3/error-model.h"
#include <map>
#include <unistd.h>

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

/**
 * \brief Error model based on a Hidden Markov Chain, with N states and M observables. This model can work in either frames
 * or time simulations
 *
 */

class HiddenMarkovErrorModel: public ErrorModel {
public:
	/**
	 * arg1: packet received successfully
	 * arg2: packet timestamp
	 * arg3: Boolean that represents whether a packet has been succesfully received or not
	 * arg4: snr (global) of packet
	 * arg5: snr due to the AR model (Slow fading)
	 * arg6: Fast Fading related SNR
	 */
	typedef Callback<void,Ptr<Packet>, Time, bool, u_int16_t> HiddenMarkovErrorModelRxCallback_t;
	/**
	 * Attribute handler
	 */
	static TypeId GetTypeId (void);
	/**
	 * Default constructor
	 */
	HiddenMarkovErrorModel ();
	/**
	 * Default destructor
	 */
	virtual ~HiddenMarkovErrorModel ();
	/**
	 * \returns The number of hidden states in the Markov Chain
	 */
	u_int8_t GetHiddenStates () const;
	/**
	 *	\param hiddenStates The number of observables within each state at the Markov chain
	 */
	void SetHiddenStates (u_int8_t hiddenStates);

	double GetAverageFrameDuration () const;
	void SetAverageFrameDuration (double averageFrameDuration);

	/**
	 * \returns The simulation unit (Time or frames)
	 */
	ErrorUnit GetUnit () const;
	/**
	 * \param unit Enum value that represent the type of simulation to be run
	 */
	void SetUnit (ErrorUnit unit);
	/**
	 * \returns The current state at the Markov chain
	 */
	int GetCurrentState () const;
	/**
	 * \params state Desired state to set up into the model
	 */
	void SetCurrentState (int state);
	/**
	 * \params Transition matrix file name (default path not necessary)
	 */
	void SetTransitionMatrixFileName (string transitionMatrixFileName);
	/**
	 * \returns The name of the file which holds the transition matrix
	 */
	string GetTransitionMatrixFileName (void) const;
	/**
	 * \params Transition matrix file name (default path not necessary)
	 */
	void SetEmissionMatrixFileName (string emissionMatrixFileName);
	/**
	 * \returns The name of the file which holds the transition matrix
	 */
	string GetEmissionMatrixFileName (void) const;
	/**
	 * Enable the error model
	 */
	void Enable (void);
	/**
	 * Disable the error model
	 */
	void Disable (void);
	/**
	 * \returns true if error model is enabled; false otherwise
	 */
	bool IsEnabled (void) const;
	/**
	 * \returns true if the state is actually changed after the execution of the method
	 */
	bool ChangeState (void);
	/**
	 *	Methods that handles the timers (only in time-based simulations);
	 */
	void InitializeTimer (void);
	void TimerHandler (void);

	//Callback invoked when a packet is received by the error model object
	void SetRxCallback (HiddenMarkovErrorModelRxCallback_t callback);

	/**
	 * \returns -1 if an error happened during the file extraction, 0  otherwise
	 * \params
	 */
	bool GetCoefficients ();

	/**
	 * \returns Whether a frame is received correctly or not
	 */
	bool Decide();

private:

	virtual bool DoCorrupt (Ptr<Packet>);
	virtual void DoReset (void);

	RandomVariable m_ranvar;						//Decide whether the error model is enabled
	double m_averageInterFrameDuration;				//Average time between two consecutive frames (depends on the physical layer)
	ErrorUnit m_unit;								//Time/frames based simulations

	int m_currentState;								//Current position into the chain model
	double m_transitionStateTimer;					//When time-based simulation, this value handles the scheduler timing
	int m_states;									//N
	int m_hiddenStates;								//M  (Typically, M=2: 0- Correct reception, 1- Error)

	bool m_enable;									//Flag to allow the error model execution
	bool m_started;									//For time-based simulation, starts when the node receives its first packet

	/* The Coefficients of the HiddenMarkovErrorModel */
	typedef map<int, vector<double> > coefSet_t;
	typedef coefSet_t::iterator coefSetIter_t;

	coefSet_t m_transitionMatrix;					//Transition probabilities among the states (NxN)
	coefSet_t m_emissionMatrix;						//Output observables (Errored, correct) (NxM)
	vector <double> m_meanDurationVector;   		//Mean duration (in frames) within each state (Nx1)

	string m_transitionMatrixFileName;
	string m_emissionMatrixFileName;

	// Tracing
	/**
	 * The trace source fired when a packet ends the reception process from
	 * the medium.
	 *
	 * \see class CallBackTraceSource
	 */
	TracedCallback<Ptr<const Packet>, Time, bool, u_int16_t> m_rxTrace;
	HiddenMarkovErrorModelRxCallback_t m_rxCallback;

protected:
	/**
	 * \return The current path (in string format)
	 */
	static std::string GetCwd ();

};

}    ////namespace ns3
#endif /* HIDDEN_MARKOV_ERROR_MODEL_H_ */
