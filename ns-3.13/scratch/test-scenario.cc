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

#include "scratch-logging.h"				//Set the LOGGING options
#include "ns3/scenario-creator-module.h"
#include "ns3/simulation-singleton.h"
#include <ctime>

using namespace ns3;
using namespace std;

u_int32_t GetNumberOfSimulations (string fileName);

/**
 * Simple script to test the scenario-creator handler. User only need the following stuff:
 * 		- Raw file name where we have stored the scenario configuration values. NOTE: it MUST have the ".config" extension, but MUST NOT be included in the variable declaration.
 *
 * To run the script, just prompt a command similar to this one: ./waf --run "scratch/test-scenario --Configuration=network-coding-scenario"
 *
 * ENJOY!!
 */

int main (int argc, char *argv[])
{

	CommandLine cmd;
	char output [255];
	string traceFile;
	clock_t begin, end;

	//Default variables  --> Available scenarios: two-nodes, x and butterfly (the last two scenarios present as well three different error location policies)
	//Configuration file
	string configuration = "channel-characterization-scenario";

	//Scenario setup variables
	Ptr <ProprietaryTracing> propTracing;

	//Random seed related values
	u_int32_t runCounter;

	//Random variable generation (Random seed)
	SeedManager::SetSeed (3);

	//Activate the logging  (from the library scratch-logging.h, just modify there those LOGGERS as wanted)
	EnableLogging ();

	//Command line options
	//Scenario configuration files
	cmd.AddValue ("Configuration", "Scenario configuration file (located in src/scenario-creator/config)", configuration);
	cmd.Parse (argc,argv);

	//We will instance both ConfigureScenario and ProprietaryTracing objects as SimulationSingletons

	for (runCounter = 1; runCounter <= GetNumberOfSimulations (configuration); runCounter ++)
	{
		begin = clock();

		//Create the scenario (auto-configured by the ConfigureScenario object)
		SimulationSingleton <ConfigureScenario>::Get ()->ParseConfigurationFile (configuration);
		SimulationSingleton <ConfigureScenario>::Get ()->Init ();
		propTracing = SimulationSingleton <ConfigureScenario>::Get ()->GetProprietaryTracing ();

		//Change the seed for each simulation run
		SeedManager::SetRun (runCounter + SimulationSingleton <ConfigureScenario>::Get ()->GetRunOffset ());

		//Set the tracing name as a function of the current run iteration
		traceFile = SimulationSingleton <ConfigureScenario>::Get ()->ComposeTraceFileName (runCounter);

		//Run the simulation
		Simulator::Stop (Seconds (1000.0));
		Simulator::Run ();
		end = clock ();

		//Print final statistics
		sprintf(output, "[%04.5f sec] - Run %d - %d/%d (FER = %f)", (double) (end - begin) / CLOCKS_PER_SEC,
				runCounter + SimulationSingleton <ConfigureScenario>::Get ()->GetRunOffset (),
				SimulationSingleton <ConfigureScenario>::Get ()->GetProprietaryTracing () -> GetCorrectPackets (),
				SimulationSingleton <ConfigureScenario>::Get ()->GetProprietaryTracing ()-> GetTotalPackets (),
				(double)  ((double) SimulationSingleton <ConfigureScenario>::Get ()-> GetProprietaryTracing() -> GetTotalPackets()
						- (double) SimulationSingleton <ConfigureScenario>::Get ()-> GetProprietaryTracing() -> GetCorrectPackets())
						/ (double) SimulationSingleton <ConfigureScenario>::Get ()-> GetProprietaryTracing() -> GetTotalPackets() );
		printf("%s\n", output);

		Simulator::Destroy ();

	} // end for

	return 0;
} 	//end main

/**
 * Read the configuration file in order to get the number of simulations to create the main loop
 */
u_int32_t GetNumberOfSimulations (string fileName)
{
	ConfigurationFile config;
	string temp;
	config.LoadConfig (config.SetConfigFileName("/src/scenario-creator/config/", fileName));
	config.GetKeyValue("SCENARIO", "RUN", temp);

	return (u_int32_t) atoi (temp.c_str());
}
