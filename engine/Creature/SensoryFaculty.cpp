// SensoryFaculty.cpp: implementation of the SensoryFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SensoryFaculty.h"
#include "Brain/Brain.h"
#include "../Agents/Agent.h"
#include "LinguisticFaculty.h"
#include "LifeFaculty.h"
#include "MotorFaculty.h"
#include "MusicFaculty.h"
#include "../App.h"
#include "../World.h"
#include "Brain/BrainScriptFunctions.h"
#include "Biochemistry/BiochemistryConstants.h"


#ifndef C2E_OLD_CPP_LIB
#include <sstream>
#endif

const int SIMPLEIDS = 1;		// ID of first SimpleObject ID (1-25) - 25 simpleobj genuses
const int COMPOUNDIDS = 26;		// ID of first CompObj ID (26-35) - 10 compobj genuses
const int CREATUREIDS = 36;		// ID of first Creature ID (36-39) - 4 creature genuses
const float visualRange = 512;

// 39 ("geat") is the error code, for
	// backwards compatibility.
const int SensoryFaculty::ourCatagoryIdError = 39;
int SensoryFaculty::ourNumCategories = -1;

std::vector<Classifier> SensoryFaculty::ourCategoryClassifiers;
std::vector<std::string> SensoryFaculty::ourCategoryNames;



enum categoryRepresentativeAlgorithms {
	PICK_NEAREST_IN_X_DIRECTION,
	PICK_A_RANDOM_ONE,
	PICK_NEAREST_IN_CURRENT_ROOM,
	PICK_NEAREST_TO_GROUND,
	PICK_RANDOM_NEAREST_IN_X_DIRECTION,

	NUM_CATEGORY_REPRESENTATIVE_ALGORITHMS
};

#define NO_RANDOM_NEAR_AGENTS 5
#define NEAR_RAND_VISUAL_RANGE 200



enum detailLobeOffsets {
    IP_IT_IS_BEING_CARRIED_BY_ME,
    IP_IT_IS_BEING_CARRIED_BY_SOMEONE_ELSE,
    IP_IT_NEARNESS,

	IP_IT_IS_CREATURE,

	IP_IT_IS_MYSIBLING,
	IP_IT_IS_MYPARENT,
	IP_IT_IS_MYCHILD,
	IP_IT_IS_OPPOSITESEX,

	IP_IT_IS_OF_THIS_SIZE,
	IP_IT_IS_SMELLING_THIS_MUCH,
	IP_IT_IS_FALLING,

	NUM_DETAIL_LOBE_OFFSETS
};


enum situationLobeOffsets {
	IP_AGE_LEVEL,
    IP_IN_VEHICLE,

	IP_CARRYING_SOMETHING,
	IP_BEING_CARRIED,
	IP_FALLING,
	IP_NEAR_OPPOSITE_SEX,

	IP_MUSIC_MOOD,
	IP_MUSIC_THREAT,

	IP_SELECTED_CREATURE,

	NUM_SITUATION_LOBE_OFFSETS
};




CREATURES_IMPLEMENT_SERIAL(SensoryFaculty)



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       SensoryFaculty
// Description: 
// ------------------------------------------------------------------------
SensoryFaculty::SensoryFaculty()
{
	// By now, SetupStaticVariables() should have been called by someone
	ASSERT(ourNumCategories != -1);

	myKnownAgents.resize(ourNumCategories);

	for (int i=0; i<ourNumCategories; i++)
	{
		myKnownAgents[i] = NULLHANDLE;
	}

	
	myNumDrives = theCatalogue.GetArrayCountForTag("drive_chemical_numbers");
	
	myDriveChemicals = new int[myNumDrives];
	for(int d =	0; d != myNumDrives; d++)
		myDriveChemicals[d] = atoi(theCatalogue.Get("drive_chemical_numbers", d));

	myAddedAFriendOnThisUpdate = false;

}


// ------------------------------------------------------------------------
// Function:    PostInit
// Class:       SensoryFaculty
// Description: 
// ------------------------------------------------------------------------
void SensoryFaculty::PostInit() 
{
	// must be done post init cos brain will not be initialised when init is run	
	// -1 cos you need 1 set of spare dendrites for migration to work (see genome notes)
	int forfSize = myCreature.GetCreatureReference().GetBrain()->GetLobeSize("forf")-1;
	if (forfSize >= 1)
	{
		myFriendsAndFoeHandles.resize(forfSize);
		myFriendsAndFoeMonikers.resize(forfSize);	
		myFriendsAndFoeLastEncounters.resize(forfSize);	
	}
}




// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       SensoryFaculty
// Description: 
// ------------------------------------------------------------------------
SensoryFaculty::~SensoryFaculty()
{
	delete myDriveChemicals;
}




// ------------------------------------------------------------------------
// Function:    Update
// Class:       SensoryFaculty
// Description: Sets most of the inputs to brain lobes so that the creature's
//				brain knows its situation ("situ"), any details of the IT object
//				("detl"), its drive levels ("driv"), the smell levels of the 40
//				categories ("smel"), the displacement of any objects in visual
//				range ("visn").
//
//				This function is also responsible for updating the list of all
//				the agents the creature knows about and choosing which agent
//				"represents" each category.  There are several "category representative
//				algorithms" (nearest object, random one and so on) and which algorithm
//				is used for which category is decided based on an array in the catalogue
//				files.
// ------------------------------------------------------------------------
void SensoryFaculty::Update()
{
	int i;

	// clean up invalid agents to break links
	CleanUpInvalidFriendAgentHandles();


	Creature& creature = myCreature.GetCreatureReference();
	if (!creature.Life()->GetWhetherAlert())
		return;

	Brain* brain = creature.GetBrain();

	// SITUATION LOBE:
	//	brain->SetSituationInput(IP_NEARWALL, ((float)((127-iSig)*2)/255.0f);
	brain->SetInput("situ", IP_IN_VEHICLE, creature.GetCarrier().IsValid() ? 1.0f : 0.0f);


	brain->SetInput("situ", IP_AGE_LEVEL, ((float)creature.Life()->GetAge())/NUMAGES);
	brain->SetInput("situ", IP_CARRYING_SOMETHING, creature.GetCarried()!=NULLHANDLE ? 1.0f : 0.0f);
	brain->SetInput("situ", IP_BEING_CARRIED, creature.GetMovementStatus()==Agent::CARRIED ? 1.0f : 0.0f);
	brain->SetInput("situ", IP_FALLING, !creature.IsStopped() ? 1.0f : 0.0f);// if not stopped you're falling:
	// find distance to nearest creature of same genus & opposite sex
	float f = DistanceToNearestCreature(
		creature.Life()->GetSex() == 1 ? 2 : 1,
		myCreature.GetAgentReference().GetClassifier().Genus());
	float vr = myCreature.GetCreatureReference().GetVisualRange();
	if (f<vr)	
		brain->SetInput("situ", IP_NEAR_OPPOSITE_SEX, (vr-f)/vr);

	brain->SetInput("situ", IP_MUSIC_MOOD, creature.Music()->Mood());
	brain->SetInput("situ", IP_MUSIC_THREAT, creature.Music()->Threat());
	brain->SetInput("situ", IP_SELECTED_CREATURE, myCreature==theApp.GetWorld().GetSelectedCreature() ? 1.0f : 0.0f);
	



	AgentHandle it = creature.GetItAgent();
	// DETAIL LOBE:
	// the IT object in detail:
    if ((it.IsValid()))
	{
		Agent& a = it.GetAgentReference();
		f = fabsf( creature.GetPosition().x - a.GetPosition().x );
		if (f<128.0f)										// starts firing @ <127 pels dist
	        brain->SetInput("detl", IP_IT_NEARNESS, ((255.0f-f-f))/255.0f);

		// is a being carried:
		if (a.GetMovementStatus() == Agent::CARRIED)
		{
			if (it == creature.GetCarried())
				brain->SetInput("detl", IP_IT_IS_BEING_CARRIED_BY_ME, 1.0f);
			else
				brain->SetInput("detl", IP_IT_IS_BEING_CARRIED_BY_SOMEONE_ELSE, 1.0f);
		}

		float size = (a.GetWidth()+a.GetHeight()) / 500.0f;
		brain->SetInput("detl", IP_IT_IS_OF_THIS_SIZE, size);
		brain->SetInput("detl", IP_IT_IS_SMELLING_THIS_MUCH, a.GetCAIncrease());
		brain->SetInput("detl", IP_IT_IS_FALLING, !a.IsStopped() ? 1.0f : 0.0f);

		if (it.IsCreature())
		{
			Creature& c = it.GetCreatureReference();
			brain->SetInput("detl", IP_IT_IS_CREATURE,	1.0f);  // IT is a creature
			brain->SetInput("detl", IP_IT_IS_MYPARENT,	c.GetMoniker()==creature.GetMotherMoniker() || c.GetMoniker()==creature.GetFatherMoniker() ? 1.0f : 0.0f);
			brain->SetInput("detl", IP_IT_IS_MYCHILD,		c.GetMotherMoniker()==creature.GetMoniker() || c.GetFatherMoniker()==creature.GetMoniker() ? 1.0f : 0.0f);
			brain->SetInput("detl", IP_IT_IS_MYSIBLING,	c.GetMotherMoniker()==creature.GetMotherMoniker() || c.GetFatherMoniker()==creature.GetFatherMoniker() ? 1.0f : 0.0f);
			brain->SetInput("detl", IP_IT_IS_OPPOSITESEX,	c.GetFamily()==creature.GetFamily() && c.GetGenus()==creature.GetGenus() && c.Life()->GetSex()!=creature.Life()->GetSex() ? 1.0f : 0.0f);
		}
	}


	// DRIVE LOBE:
	// Copy current drive levels from receptors to DRIVE_LOBE neus
	{
		for (i=0; i<NUMDRIVES; i++)
		{
			brain->SetInput("driv", i, creature.GetDriveLevel(i));
		}
	}


	// SMELL LOBE:
	// update the smell lobe from the 16 Map CAs.
	int roomId;
	if (theApp.GetWorld().GetMap().GetRoomIDForPoint( creature.GetDownFootPosition(), roomId))
	{
		for (i=0; i<CA_PROPERTY_COUNT; i++) {
			// Get CA value
			float smellValue = 0.0f;
			theApp.GetWorld().GetMap().GetRoomProperty(roomId, i, smellValue);

			// Set biochemistry chemicals:
			creature.GetBiochemistry()->SetChemical(FIRST_SMELL_CHEMICAL+i, smellValue);

			int neuronId = theAgentManager.GetCategoryIdFromSmellId(i);

			// Also, if the category is the creature's own (e.g. 36=norn) then
			// get the smell minus its own contribution:
			if (neuronId==GetCategoryIdOfAgent(myCreature))
				smellValue = theApp.GetWorld().GetMap().GetRoomPropertyMinusMyContribution(myCreature, smellValue);

			// Set brain neuron:
			brain->SetInput("smel", neuronId, smellValue);
		}
	}



	// VISION LOBE:
	ClearSeenFriendsOrFoes();

	int genusId;
	for ( genusId=0; genusId<ourNumCategories; genusId++) 
	{
		AgentHandle oldKnownAgent = myKnownAgents[genusId];
		std::vector<AgentHandle> agentsNearest;
		std::vector<float> agentsDistance;


		// DEBUG command (highlight known agents):
		if (myKnownAgents[genusId].IsValid())
		{
			myKnownAgents[genusId].GetAgentReference().SetWhetherHighlighted(false);
		}

		// if you can still see last known agent and have been talking about it - keep it
		if(myKnownAgents[genusId].IsValid() && creature.CanSee(myKnownAgents[genusId]) && creature.GetBrain()->GetNeuronState("noun", genusId, STATE_VAR) > 0.20f)
		{
			SetSeenFriendOrFoe(myKnownAgents[genusId]);
			continue;
		}

		myKnownAgents[genusId] = NULLHANDLE;


		AgentList visibleAgentsInThisCategory;
		theAgentManager.FindBySightAndFGS(myCreature, visibleAgentsInThisCategory, ourCategoryClassifiers[genusId]);



		int whichAlgorithm= atoi(theCatalogue.Get("Category Representative Algorithms", genusId));
		if (whichAlgorithm<0 || whichAlgorithm>=NUM_CATEGORY_REPRESENTATIVE_ALGORITHMS)
			whichAlgorithm= PICK_NEAREST_IN_X_DIRECTION;



		int whichAgentToChoose = -1;
		// if its safe to change the rep (and we want to) do so:
		if (whichAlgorithm==PICK_A_RANDOM_ONE && !creature.GetVirtualMachine().IsRunning())
		{
			whichAgentToChoose = Rnd(0, visibleAgentsInThisCategory.size()-1);
		}

		AgentHandle oldAgent = myKnownAgents[genusId];

		int agentNo=0;
		AgentHandle winningAgent;
		for (AgentListIterator l=visibleAgentsInThisCategory.begin(); l!=visibleAgentsInThisCategory.end(); l++, agentNo++) 
		{
			Agent& thisAgent = (*l).GetAgentReference();
	
			// Chosen a random one?
			if (whichAlgorithm==PICK_A_RANDOM_ONE)
			{
				if (agentNo==whichAgentToChoose ||
					(creature.GetVirtualMachine().IsRunning() && (*l)==oldKnownAgent))
				{
					winningAgent = thisAgent;
					break;
				}
			}
			if(whichAlgorithm==PICK_RANDOM_NEAREST_IN_X_DIRECTION) 
			{	
				if (creature.GetVirtualMachine().IsRunning() && (*l)==oldKnownAgent)
				{
					myKnownAgents[genusId] = (*l).GetAgentReference();
					break;
				}
			}
		
			if (whichAlgorithm==PICK_NEAREST_IN_CURRENT_ROOM) 
			{
				int creatureRoomId;
				if (!creature.GetRoomID(creatureRoomId))
					continue;
	
				int agentRoomId;
				if (!thisAgent.GetRoomID(agentRoomId))
					continue;

				// ignore agents in other rooms:
				if (agentRoomId!=creatureRoomId)
					continue;
			}

			// if there's no known agent use this agent:
			if (winningAgent.IsInvalid()) 
			{
				winningAgent = thisAgent;
			} 
			else
			{
				if (whichAlgorithm==PICK_NEAREST_IN_X_DIRECTION || whichAlgorithm==PICK_NEAREST_IN_CURRENT_ROOM
					|| whichAlgorithm==PICK_RANDOM_NEAREST_IN_X_DIRECTION) 
				{
					// closeness in X direction to creature's centre:
					float thisAgentXDistance = fabsf(
						creature.GetCentre().x -
						thisAgent.WhereShouldCreaturesPressMe().x
					);

					if(whichAlgorithm==PICK_RANDOM_NEAREST_IN_X_DIRECTION) 
					{
						for(i = 0; i != agentsNearest.size(); i++)
						{
							if(thisAgentXDistance < agentsDistance[i])
								break;
						}

						if(i < NO_RANDOM_NEAR_AGENTS)
						{
							if(i == agentsNearest.size())
							{
								// add on end
								agentsNearest.push_back(thisAgent);
								agentsDistance.push_back(thisAgentXDistance);
							}
							else
							{
								// insert
								agentsNearest.insert(agentsNearest.begin()+i, thisAgent);
								agentsDistance.insert(agentsDistance.begin()+i, thisAgentXDistance);
								if(agentsNearest.size() > NO_RANDOM_NEAR_AGENTS)
								{
									agentsNearest.pop_back();
									agentsDistance.pop_back();
								}
							}
						}

					} 
					else 
					{
						float winningAgentXDistance = fabsf(
							creature.GetCentre().x -
							winningAgent.GetAgentReference().WhereShouldCreaturesPressMe().x
						);

						if (thisAgentXDistance<winningAgentXDistance) 
						{
							winningAgent = thisAgent;
						}
					}

				} else if (whichAlgorithm==PICK_NEAREST_TO_GROUND) 
				{
					// closeness in Y direction to creature's centre:
					float winningAgentYDistance = fabsf(
						creature.GetPosition().y+creature.GetHeight() -
						winningAgent.GetAgentReference().WhereShouldCreaturesPressMe().y
					);
					float thisAgentYDistance = fabsf(
						creature.GetPosition().y+creature.GetHeight() -
						thisAgent.WhereShouldCreaturesPressMe().y
					);

					if (thisAgentYDistance<winningAgentYDistance) 
					{
						winningAgent = thisAgent;
					}
				}
			}
		}

		if(whichAlgorithm==PICK_RANDOM_NEAREST_IN_X_DIRECTION && 
			myKnownAgents[genusId] == NULLHANDLE && agentsNearest.size() != 0)
		{	
			// if have myKnownAgent[] already you are using the old one (cos still running VM) 
			// and you have some to select from then select from nearest vector
			int whichAgentToChoose = Rnd(0,agentsNearest.size()-1);	
			while(whichAgentToChoose != 0 && agentsDistance[whichAgentToChoose] > (visualRange/NEAR_RAND_VISUAL_RANGE))
			{
				agentsDistance.erase(agentsDistance.begin()+whichAgentToChoose);
				agentsNearest.erase(agentsNearest.begin()+whichAgentToChoose);
				whichAgentToChoose = Rnd(0,agentsNearest.size()-1);
			} 
					
			myKnownAgents[genusId] = agentsNearest[whichAgentToChoose];
				
		}


		if (myKnownAgents[genusId].IsInvalid()) 
			myKnownAgents[genusId] = winningAgent;;


		SetSeenFriendOrFoe(myKnownAgents[genusId]);	// adds if not there
	
	}

	// Ensure that a carried object is always the representative for that category:
	if (creature.GetCarried().IsValid())
	{
		int categoryOfCarriedObject = GetCategoryIdOfAgent(creature.GetCarried());
		if (categoryOfCarriedObject>=0 && categoryOfCarriedObject<ourNumCategories  && !creature.GetCarried().GetAgentReference().TestAttributes(Agent::attrInvisible))
			myKnownAgents[categoryOfCarriedObject] = creature.GetCarried();
	}

	// give positions as input to vision lobe:
	{
		for ( i=0; i<ourNumCategories; i++)
		{
			if (myKnownAgents[i].IsInvalid())
			{
				brain->SetInput("visn", i, 0.0f);
				brain->SetInput("elvn", i, 0.0f);
			}
			else
			{
				SetVisualInput(i);
			}
		}
	}

	// n.b. NOUN, VERB are set on the fly by various things as they take
	// nudges as inputs.



	// reset for next update
	myAddedAFriendOnThisUpdate = false;
}




// ------------------------------------------------------------------------
// Function:    SetVisualInput
// Class:       SensoryFaculty
// Description: Helper function to setting the "visn" input to the brain:
// Arguments:   int i = 
// ------------------------------------------------------------------------
void SensoryFaculty::SetVisualInput(int i)
{
	Creature& creature = myCreature.GetCreatureReference();
	Brain* brain = creature.GetBrain();
					
	// DEBUG command to highlight all known agents:
	if (theApp.ShouldHighlightAgentsKnownToCreature())
	{
		myKnownAgents[i].GetAgentReference().SetWhetherHighlighted(true);
	}

	float xDisplacement = BoundIntoMinusOnePlusOne((myKnownAgents[i].GetAgentReference().GetCentre().x - creature.GetCentre().x) / visualRange);
	float yDisplacement = BoundIntoMinusOnePlusOne((myKnownAgents[i].GetAgentReference().GetCentre().y - creature.GetCentre().y) / visualRange);

	brain->SetInput("visn", i, xDisplacement);
	brain->SetInput("elvn", i, yDisplacement);
}



// Use this agent's Family and Genus to produce a single number in range 0-39, which represents
// the 'type' of an object. Two objects of the same family+genus have the same ID#, and are
// INDISTINGUISHABLE from each other as far as creatures are concerned (they have the same name,
// 'look' the same, have the same significance, etc.)
// ------------------------------------------------------------------------
// Function:    GetCategoryIdOfAgent
// Class:       SensoryFaculty
// Description: 
// Arguments:   AgentHandle& a = 
// Returns:     int = 
// ------------------------------------------------------------------------
int SensoryFaculty::GetCategoryIdOfAgent(const AgentHandle& a)
{
	AgentHandle a2(a);
	if( a2.IsInvalid())
		return -1;

	return GetCategoryIdOfClassifier(&(a2.GetAgentReference().GetClassifier()));
}



// ------------------------------------------------------------------------
// Function:    GetCategoryIdOfClassifier
// Class:       SensoryFaculty
// Description: Get which category a given classifier is.
// Arguments:   const Classifier* c = 
// Returns:     int = 
// ------------------------------------------------------------------------
int SensoryFaculty::GetCategoryIdOfClassifier(const Classifier* c)
{
	for (int i = 0; i < ourNumCategories; ++i)
	{
		if (c->GenericMatchForWildCard(ourCategoryClassifiers[i], TRUE, TRUE, TRUE, FALSE))
			return i;
	}

	return ourCatagoryIdError;
}



// ------------------------------------------------------------------------
// Function:    Stimulate
// Class:       SensoryFaculty
// Description: Directly stimulate creature by using a built-in stimulus from creature's library
//				(defined genetically though).
// Arguments:   AgentHandle& from = who stimulus was emitted by
//				int stim = stimulus library entry
//				int fromScriptEventNo = script they were running
//				float strengthMultiplier = increase strength by this multiple
//				bool forceNoLearning = allow learning to be disabled
// ------------------------------------------------------------------------
void SensoryFaculty::Stimulate(AgentHandle& from,
						 int stim,
						 int fromScriptEventNo,
						 float strengthMultiplier,
						 bool forceNoLearning)
{
	Stimulus* s = &myStimulusLib[stim];			// get MY personal version of this stimulus
    s->fromAgent = from;
	s->toCreature = myCreature;					// I am dest of stimulus
	s->fromScriptEventNo = fromScriptEventNo;
	s->strengthMultiplier = strengthMultiplier;
	s->forceNoLearning = forceNoLearning;
	Stimulate(*s);
}



// ------------------------------------------------------------------------
// Function:    Stimulate
// Class:       SensoryFaculty
// Description: Main stimulus-handler.
// Arguments:   Stimulus s = 
// ------------------------------------------------------------------------
void SensoryFaculty::Stimulate(Stimulus s)			// COPY of stimulus data
{
	Creature &c = myCreature.GetCreatureReference();
	if (c.Life()->GetWhetherDead())
		return;

    // If you're asleep, either stim won't get through or it will be
    // attenuated
    if (!c.Life()->GetWhetherAlert() &&
		!c.Life()->GetWhetherZombie())
	{
        if  (!(s.bitFlags & IFASLEEP))      // if stim not perceptible by
		{
			#ifdef STIM_TEST_TRACE
			OutputFormattedDebugString("asleep\n");
			#endif


			// saying the creature's name wakes it up:
			if (c.Life()->GetWhetherAsleep() &&
				s.incomingSentence==c.Linguistic()->GetPlatonicWord(LinguisticFaculty::PERSONAL, LinguisticFaculty::ME))
			{
				c.Life()->SetWhetherAsleep(false);
			}

            return;                         // sleeper, go home
		}
        s.verbStim /= 2.0f;                   // else attenuate signal
        s.nounStim /= 2.0f;
    }

	// stims from the ORDR macro:
	c.Linguistic()->HearSentence(s.fromAgent, s.incomingSentence, s.verbIdToStim, s.nounIdToStim);

	// stims from the URGE macro (or STIM #):
	if (s.nounStim>1.0f)
	{
		c.Motor()->SetAttentionOverride(s.nounIdToStim);
	}
	else
	{
		if (s.nounStim!=0.0f)
			c.GetBrain()->SetInput("noun", s.nounIdToStim, s.nounStim);
	}
	if (s.verbStim>1.0f)
	{
		c.Motor()->SetDecisionOverride(s.verbIdToStim);
	}
	else
	{
		if (s.verbStim!=0.0f)
			c.GetBrain()->SetInput("verb", GetNeuronIdFromScriptOffset(s.verbIdToStim), s.verbStim);
	}

	// stims from the SWAY macro (or STIM #):
    // Emit given chemicals into the bloodstream - eg. drive changes or nutrition
	// n.b. stims only get through if the creature is looking at IT
#ifdef STIM_TEST_TRACE
	bool sent = false;
#endif
	for	(int i=0; i<4; i++)
	{
		int chemicalId = s.chemicalsToAdjust[i];
		if (chemicalId!=0)
		{
			#ifdef STIM_TEST_TRACE
				sent = true;
			#endif

			float adjustment = s.adjustments[i];
			adjustment = BoundIntoMinusOnePlusOne(s.strengthMultiplier * adjustment);
				
			// the brain will decide what sort of reward to give based on whether
			// the stimulus came from the IT agent and whether the norn was touching
			// that agent or not.
			if ((s.bitFlags & stimTrainingOffFlags[i]) || s.forceNoLearning)
			{
				#ifdef STIM_TEST_TRACE
					OutputFormattedDebugString("non training\n");
				#endif
				AdjustChemicalLevel(// training is off
					s.chemicalsToAdjust[i],
					adjustment
				);
			}
			else
			{
				AdjustChemicalLevelWithTraining(				// training is on
					s.chemicalsToAdjust[i],
					adjustment,
					s.fromScriptEventNo,
					s.fromAgent
				);
			}
		}
	}
	#ifdef STIM_TEST_TRACE
	if (!sent)
		OutputFormattedDebugString("empty stim\n");
	#endif
}



// ------------------------------------------------------------------------
// Function:    ReadFromGenome
// Class:       SensoryFaculty
// Description: Read in my personal stimulus list from the genome.
// Arguments:   Genome& g = 
// ------------------------------------------------------------------------
void SensoryFaculty::ReadFromGenome(Genome& g)
{
	// Read the STIMULUS genes to determine the creature-specific stimuli
	// Late-switching genes overwrite equivalent predecessors
	g.Reset();
	while ((g.GetGeneType(CREATUREGENE,G_STIMULUS,NUMCREATURESUBTYPES))!=false)
	{
		int i = g.GetCodon(0,NUMSTIMULI-1);			// stimulus# to define
		myStimulusLib[i].InitFromGenome(g);
	};
}


// ------------------------------------------------------------------------
// Function:    AdjustChemicalLevel
// Class:       SensoryFaculty
// Description: 
// Arguments:   int whichChemical = 
//              float adjustment = 
// ------------------------------------------------------------------------
void SensoryFaculty::AdjustChemicalLevel(int whichChemical, float adjustment)
{
	myCreature.GetCreatureReference().GetBiochemistry()->AddChemical(
		whichChemical, adjustment
	);
}

// ------------------------------------------------------------------------
// Function:    AdjustChemicalLevelWithTraining
// Class:       SensoryFaculty
// Description: 
// Arguments:   int whichChemical = 
//              float adjustment = 
//              int fromScriptEventNo = 
//              AgentHandle& fromAgent = 
// ------------------------------------------------------------------------
void SensoryFaculty::AdjustChemicalLevelWithTraining(int whichChemical, float adjustment, int fromScriptEventNo, AgentHandle const& fromAgent)
{
	AgentHandle from2(fromAgent);
	AdjustChemicalLevel(whichChemical, adjustment);

	int drive = GetDriveNumberOfChemical(whichChemical);

	if(drive != -1)
	{
		// is a drive so can train

		// need this line else it object may be NULL 
		// and hence will match NULL fromAgent and train brain
		if(from2.IsInvalid()) 
		{
#ifdef STIM_TEST_TRACE
			OutputFormattedDebugString("invalid\n");
#endif
			return;
		}

		if (myCreature.GetCreatureReference().Life()->GetWhetherAlert())
		{
			// check event script corresponds to current creature action:

			bool learn = true;
			if (theApp.GetWorld().GetGameVar("engine_synchronous_learning").GetInteger() == 1)
			{		
				if (fromAgent != myCreature && fromAgent != thePointer)
				{
					// check decision creature has made corresponds to script agent is running
					int decisionOffset = myCreature.GetCreatureReference().Motor()->GetCurrentDecisionId();
					int expectedAgentScript = GetExpectedAgentScriptFromDecisionOffset(decisionOffset);
#ifdef STIM_TEST_TRACE
					OutputFormattedDebugString("Agent script: %d Decision script crossref:%d\n", fromScriptEventNo, expectedAgentScript);
#endif
					if (expectedAgentScript == -1 || fromScriptEventNo == -1)
						learn = false;
					else if (fromScriptEventNo != expectedAgentScript)
						learn = false;

					// check the creature still has attention on the object
					const AgentHandle& itAgent = myCreature.GetCreatureReference().GetItAgent();
					if (itAgent != fromAgent)
						learn = false;

					// Warning: GENERICIVEBEEN script numbers won't work with this code
					// We don't use them in C3, so it doesn't matter
					if (learn)
						ASSERT(IsThisAnIveBeenScript(fromScriptEventNo));
				}
			}

			if (learn)
			{
#ifdef STIM_TEST_TRACE
	OutputFormattedDebugString("learning\n");
#endif
				myCreature.GetCreatureReference().GetBrain()->SetInput("resp", drive, adjustment);
			}
#ifdef STIM_TEST_TRACE
			else
	OutputFormattedDebugString("not learning\n");
#endif
		}
		else
		{
#ifdef STIM_TEST_TRACE
	OutputFormattedDebugString("prox\n");
#endif
			myCreature.GetCreatureReference().GetBrain()->SetInput("prox", drive, adjustment);
		}
	}
#ifdef STIM_TEST_TRACE
	else
		OutputFormattedDebugString("is not drive\n");
#endif	
}



// ------------------------------------------------------------------------
// Function:    Write
// Class:       SensoryFaculty
// Description: Serialisation.
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool SensoryFaculty::Write(CreaturesArchive &archive) const
{
	base::Write( archive );
	for (int i=0; i<ourNumCategories; i++)
	{
		archive << myKnownAgents[i];
	}
	for (int o=0; o<NUMSTIMULI; o++)
	{
		myStimulusLib[o].Write(archive);
	}
	archive << myFriendsAndFoeHandles;
	archive << myFriendsAndFoeMonikers;
	archive << myFriendsAndFoeLastEncounters;
	archive << myAddedAFriendOnThisUpdate;
	return true;
}

// ------------------------------------------------------------------------
// Function:    Read
// Class:       SensoryFaculty
// Description: Serialisation.
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool SensoryFaculty::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		if(!base::Read( archive ))
			return false;

		for (int i=0; i<ourNumCategories; i++)
		{
			archive >> myKnownAgents[i];
		}
		for (int o=0; o<NUMSTIMULI; o++) 
		{
			if(!myStimulusLib[o].Read(archive))
				return false;
		}
		archive >> myFriendsAndFoeHandles;
		archive >> myFriendsAndFoeMonikers;
		archive >> myFriendsAndFoeLastEncounters;
		archive >> myAddedAFriendOnThisUpdate;
	}
	else
	{
		_ASSERT(false);
		return false;
	}


	return true;
}


// ------------------------------------------------------------------------
// Function:    SetupStaticVariablesFromCatalogue
// Class:       SensoryFaculty
// Description: Called once, this sets up the category/classifier map.
// Returns:     bool = 
// ------------------------------------------------------------------------
bool SensoryFaculty::SetupStaticVariablesFromCatalogue()
{
	ourNumCategories = theCatalogue.GetArrayCountForTag("Agent Categories");
	int check = theCatalogue.GetArrayCountForTag("Agent Classifiers");
	if (ourNumCategories != check)
	{
		ErrorMessageHandler::Show("creature_error", 1, "SensoryFaculty::SetupStaticVariablesFromCatalogue");
		return false;
	}
	
	ourCategoryClassifiers.clear();
	ourCategoryNames.clear();

	for (int i = 0; i < ourNumCategories; ++i)
	{
		std::string classifier = theCatalogue.Get("Agent Classifiers", i);
		int f = 0, g = 0, s = 0;
#ifndef C2E_OLD_CPP_LIB
		std::istringstream in(classifier);
		in >> f >> g >> s;
#else
		// UGH!
		sscanf( classifier.c_str(), "%d %d %d",f,g,s );
#endif

		ourCategoryClassifiers.push_back(Classifier(f, g, s, 0));
		ourCategoryNames.push_back(theCatalogue.Get("Agent Categories", i));
	}

	return true;
}

std::string SensoryFaculty::GetCategoryName(int i)
{
	if (i < 0 || i >= ourNumCategories)
		return std::string("");
	return ourCategoryNames[i];
}

// ------------------------------------------------------------------------
// Function:    GetNumCategories
// Class:       SensoryFaculty
// Description: Get the no of categories (40 probably - this number is defined
//				in the catalogue).
// Returns:     int = 
// ------------------------------------------------------------------------
int SensoryFaculty::GetNumCategories()
{
	return ourNumCategories;
}


// ------------------------------------------------------------------------
// Function:    DistanceToNearestCreature
// Class:       SensoryFaculty
// Description: This function used to fill in the situation neuron which tracks 
//				norns of opposite sex, to use for "pheramones" etc.
// Arguments:   int sex = 
//              int genus = 
// Returns:     float = 
// ------------------------------------------------------------------------
float SensoryFaculty::DistanceToNearestCreature(int sex, int genus)
{
	CreatureCollection &creatureCollection = theAgentManager.GetCreatureCollection();
	int noCreatures = creatureCollection.size();
	Creature &meCreature = myCreature.GetCreatureReference();
	AgentHandle me = myCreature.GetAgentReference();
	float nearest = meCreature.GetVisualRange()+1.0f; // detect @ <= visual range

	for(int c = 0; c != noCreatures; c++)
	{
		Creature &nearCreature = creatureCollection[c].GetCreatureReference();
			
		if(meCreature.CanSee(AgentHandle(nearCreature)))
		{
			if((sex == 0 || nearCreature.Life()->GetSex() == sex) && 
				(genus == 0 || creatureCollection[c].GetAgentReference().GetClassifier().Genus() == genus))
			{
				float f = fabsf( meCreature.GetPosition().x - nearCreature.GetPosition().x );	
			
				if	(f<nearest)	
				{
					// detect at @ <=visual range
					nearest = f;
				}
			}
		}
	}
	return nearest;
}



// ------------------------------------------------------------------------
// Function:    PayAttentionToCreature
// Class:       SensoryFaculty
// Description: Set a category representative based on an ID in the game's
//				creature collection.
// Arguments:   int creatureNo = 
// Returns:     int = 
// ------------------------------------------------------------------------
int SensoryFaculty::PayAttentionToCreature(int creatureNo)
{

	CreatureCollection &creatures = theAgentManager.GetCreatureCollection();
	return PayAttentionToCreature(creatures[creatureNo]);
	
}


// ------------------------------------------------------------------------
// Function:    PayAttentionToCreature
// Class:       SensoryFaculty
// Description: Set a particular creature to be the category representative
//				for the norn/grendel/ettin category.
// Arguments:   AgentHandle& lookAtCreature = 
// Returns:     int = 
// ------------------------------------------------------------------------
int SensoryFaculty::PayAttentionToCreature(AgentHandle& lookAtCreature)
{
	int id = -1;

	if(lookAtCreature.IsInvalid())
		return id;

	Creature& creature = myCreature.GetCreatureReference();

	

	if(!lookAtCreature.IsCreature()) return id;

	if(creature.CanSee(lookAtCreature) && creature.Life()->GetWhetherAlert())
	{
		id = SensoryFaculty::GetCategoryIdOfAgent(lookAtCreature);

		if(creature.Sensory()->GetKnownAgent(id) != lookAtCreature)
		{
			// differant to current creature known about
			creature.Sensory()->SetKnownAgent(id, lookAtCreature);
			creature.Sensory()->SetVisualInput(id);
								
			// clear noun in question
			creature.GetBrain()->ClearNeuronActivity("noun", id);

		}
	}
	return id;
}




// ------------------------------------------------------------------------
// Function:    AddFriendOrFoe
// Class:       SensoryFaculty
// Description: Set up a creature (or pointer) you see for the first time in
//				your friend or foe list, initialising its friendliness towards
//				you based on your blood relation etc.
// Arguments:   AgentHandle& creatureOrPointer = 
// Returns:     int = 
// ------------------------------------------------------------------------
int SensoryFaculty::AddFriendOrFoe(AgentHandle& creatureOrPointer)
{
	// only allow one friend to be added per update
	// else forf dendrites will not know which to migrate too					
	if(myAddedAFriendOnThisUpdate)
		return -1;

	if(creatureOrPointer.IsInvalid())
		return -1;

	if(creatureOrPointer.IsCreature())
	{
		// don't add dead creatures
		if(creatureOrPointer.GetCreatureReference().Life()->GetWhetherDead())
			return -1;
	}	

	// only allow creatures or pointers to be friends
	if(!creatureOrPointer.IsCreature() && !creatureOrPointer.IsPointerAgent())
		return -1;

	Creature& creature = myCreature.GetCreatureReference();

	FriendOrFoeIterator ffi = std::find(myFriendsAndFoeHandles.begin(), myFriendsAndFoeHandles.end(), creatureOrPointer);

	if(ffi != myFriendsAndFoeHandles.end())
		return -1;


	// find oldest free slot
	int oldestEncounter = -1;
	int oldestSlot = -1;
	for(int i = 0; i != myFriendsAndFoeHandles.size(); ++i)
	{
		if(myFriendsAndFoeHandles[i].IsInvalid() && myFriendsAndFoeMonikers[i].empty())
		{
			// never used slot
			oldestSlot = i;
			break;
		}		
		else if(myFriendsAndFoeHandles[i].IsInvalid() && 
			(myFriendsAndFoeLastEncounters[i] < oldestEncounter || oldestSlot == -1))
			oldestSlot = i;
	}

	// no free slots
	if(oldestSlot == -1)
		return -1;



	// register new aquaintance
	myFriendsAndFoeHandles[oldestSlot] = creatureOrPointer;
	
	// get unique world id for pointer or moniker of creature
	// so can remember mates on export
	if(creatureOrPointer.IsCreature())
	{
		Creature &c = myFriendsAndFoeHandles[oldestSlot].GetCreatureReference();
		
		if(c.Life()->GetWhetherDead())
			return -1;

		myFriendsAndFoeMonikers[oldestSlot] = 
			creatureOrPointer.GetAgentReference().GetGenomeStore().MonikerAsString(0);

		// initialise neurons in brain
		if(c.GetMoniker()==creature.GetMotherMoniker() || 
			c.GetMoniker()==creature.GetFatherMoniker() ||
			c.GetMotherMoniker()==creature.GetMoniker() || 
			c.GetFatherMoniker()==creature.GetMoniker())
		{
			// parent child - love immediatly
			myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", oldestSlot, STATE_VAR, 0.8f);
		}
		else if(c.GetMotherMoniker()==creature.GetMotherMoniker() || 
			c.GetFatherMoniker()==creature.GetFatherMoniker())
		{
			// bothers and sisters
			myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", oldestSlot, STATE_VAR, 0.225f);

		}
		else if((c.GetFamily() == creature.GetFamily()) && (c.GetGenus() == creature.GetGenus()) &&	(c.GetGenus() != 2))
		{
			// creatures of smae genus (but not grendels)
			myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", oldestSlot, STATE_VAR, 0.175f);

		}
		
		else
		{
			// someone else set up for clearing and relearning
			myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", oldestSlot, FOURTH_VAR, -1.0f);
		}


	}
	else
	{
		myFriendsAndFoeMonikers[oldestSlot] = theApp.GetWorld().GetUniqueIdentifier();

		// hand friend/foe set up for clearing and relearning
		myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", oldestSlot, FOURTH_VAR, -1.0f);

	}

	// store last seen time stamp (will be over written at every load up)
	myFriendsAndFoeLastEncounters[oldestSlot] = creature.Life()->GetTickAge();



	// set up for dendrite to link to combo lobe

	// flag friend or foe neuron for dendrite migration
	myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", oldestSlot, NGF_VAR, 1.0f);
	
	// flag bad action concept neurons for this agent catagory to have dendrites migrate to them
	for(int b = 0; b != theCatalogue.GetArrayCountForTag("Bad Action Script"); b++)
	{
		int badDecisionNeuron = GetNeuronIdFromScriptOffset(atoi(theCatalogue.Get("Bad Action Script",b)));
		int badConceptNeuron = (GetNumCategories()*badDecisionNeuron)+
									GetCategoryIdOfAgent(creatureOrPointer);
		myCreature.GetCreatureReference().GetBrain()->SetNeuronState("comb", badConceptNeuron, NGF_VAR, 1.0f);
	}

	// flag good action concept neurons for this agent catagory to have dendrites migrate to them
	for(int g = 0; g != theCatalogue.GetArrayCountForTag("Good Action Script"); g++)
	{
		int goodDecisionNeuron = GetNeuronIdFromScriptOffset(atoi(theCatalogue.Get("Good Action Script",g)));
		int goodConceptNeuron = (GetNumCategories()*goodDecisionNeuron)+
									GetCategoryIdOfAgent(creatureOrPointer);
		myCreature.GetCreatureReference().GetBrain()->SetNeuronState("comb", goodConceptNeuron, NGF_VAR, 1.0f);
	}

	myAddedAFriendOnThisUpdate = true;
	return oldestSlot;
	

}


// ------------------------------------------------------------------------
// Function:    SetCreatureActingUponMe
// Class:       SensoryFaculty
// Description: Sets a brain input so the brain knows who is acting on it.
// Arguments:   AgentHandle& creatureOrPointer = 
// ------------------------------------------------------------------------
void SensoryFaculty::SetCreatureActingUponMe(AgentHandle& creatureOrPointer)
{
	if(creatureOrPointer.IsInvalid())
		return;

	for(int i = 0; i != myFriendsAndFoeHandles.size(); i++)
	{
		if(myFriendsAndFoeHandles[i] == creatureOrPointer)
		{
			myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", i, THIRD_VAR, 1.0f);
			break;
		}
	}
}



// ------------------------------------------------------------------------
// Function:    GetOpinionOfCreature
// Class:       SensoryFaculty
// Description: Used by the LinguisticFaculty for this norn say how he much he likes/
//				dislikes another creature.
// Arguments:   AgentHandle& creatureOrPointer = 
//              float &opinion = 
//              float &moodOpinion = 
// Returns:     int = 
// ------------------------------------------------------------------------
int SensoryFaculty::GetOpinionOfCreature(AgentHandle& creatureOrPointer, float &opinion, float &moodOpinion)
{
	opinion = 0.0f;
	moodOpinion = 0.0f;

	if(creatureOrPointer.IsValid())
	{

		for(int i = 0; i != myFriendsAndFoeHandles.size(); i++)
		{
			if(myFriendsAndFoeHandles[i] == creatureOrPointer)
			{
				opinion = myCreature.GetCreatureReference().GetBrain()->GetNeuronState("forf", i, STATE_VAR);
				moodOpinion = myCreature.GetCreatureReference().GetBrain()->GetNeuronState("forf", i, OUTPUT_VAR);
				return i;
			}
		}
	}

	return -1;
}




// ------------------------------------------------------------------------
// Function:    GetNearestCreatureOrPointer
// Class:       SensoryFaculty
// Description: Helper function for friend or foe stuff.
// Returns:     AgentHandle = 
// ------------------------------------------------------------------------
AgentHandle SensoryFaculty::GetNearestCreatureOrPointer()
{
	CreatureCollection &creatureCollection = theAgentManager.GetCreatureCollection();
	int noCreatures = creatureCollection.size();
	Creature &meCreature = myCreature.GetCreatureReference();
	AgentHandle me = myCreature.GetAgentReference();
	float nearest = meCreature.GetVisualRange()+1.0f; // detect @ <= visual range
	AgentHandle nearestAgent;

	for(int c = 0; c != noCreatures; c++)
	{
		Creature &nearCreature = creatureCollection[c].GetCreatureReference();
			
		if(meCreature.CanSee(AgentHandle(nearCreature)))
		{
			float f = fabsf( meCreature.GetPosition().x - nearCreature.GetPosition().x );	
			
			if	(f<nearest)	
			{
				// detect at @ <=visual range
				nearest = f;
				nearestAgent = nearCreature;
			}
		}
	}

	float f = fabsf( meCreature.GetPosition().x - thePointer.GetAgentReference().GetPosition().x );	
	if(f<nearest)	
		nearestAgent = thePointer;
	
	return nearestAgent;
}


// ------------------------------------------------------------------------
// Function:    PayAttentionToAgent
// Class:       SensoryFaculty
// Description: Set the category representative for this agent category.
// Arguments:   AgentHandle& agent = 
// Returns:     int = 
// ------------------------------------------------------------------------
int SensoryFaculty::PayAttentionToAgent(AgentHandle& agent)
{
	int id = -1;
	
	if(agent.IsInvalid())
		return id;

	Creature& creature = myCreature.GetCreatureReference();




	if(creature.CanSee(agent) && creature.Life()->GetWhetherAlert())
	{
		id = SensoryFaculty::GetCategoryIdOfAgent(agent);

		if(id >= 0 && id <= GetNumCategories())
		{
		
			if(GetKnownAgent(id) != agent)
			{
				// differant to current creature known about
				SetKnownAgent(id, agent);
				SetSeenFriendOrFoe(agent);
				SetVisualInput(id);
			}

		}
	}
	return id;
}



// ------------------------------------------------------------------------
// Function:    SetSeenFriendOrFoe
// Class:       SensoryFaculty
// Description: Helper function for friend or foe stuff.
// Arguments:   AgentHandle& creatureOrPointer = 
// ------------------------------------------------------------------------
void SensoryFaculty::SetSeenFriendOrFoe(AgentHandle& creatureOrPointer)
{
	// flag forf neuron to allow input to combination lobe to
	// sway actions based on likes or dislikes

	if(creatureOrPointer.IsInvalid())
		return;

	// only allow creatures or pointers to be friends
	if(!creatureOrPointer.IsCreature() && !creatureOrPointer.IsPointerAgent())
		return;
	
	for(int i = 0; i != myFriendsAndFoeHandles.size(); i++)
	{
		if(myFriendsAndFoeHandles[i] == creatureOrPointer)
		{
			myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", i, FIFTH_VAR, 1.0f);
			return;
		}
	}	

	// not found try adding
	int forfId = AddFriendOrFoe(creatureOrPointer);
	if(forfId != -1)
		myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", forfId, FIFTH_VAR, 1.0f);

	return;

}



// ------------------------------------------------------------------------
// Function:    ClearSeenFriendsOrFoes
// Class:       SensoryFaculty
// Description: Helper function for friend or foe stuff.
// ------------------------------------------------------------------------
void SensoryFaculty::ClearSeenFriendsOrFoes()
{
	// clears agents in the vacinity flag and reset migration flag

	for(int i = 0; i != myFriendsAndFoeHandles.size(); i++)
	{
		myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", i, FIFTH_VAR, 0.0f);
		myCreature.GetCreatureReference().GetBrain()->SetNeuronState("forf", i, NGF_VAR, 0.0f);
	}

	return;

}



// ------------------------------------------------------------------------
// Function:    ResolveFriendAndFoe
// Class:       SensoryFaculty
// Description: Helper function for friend or foe stuff.
// ------------------------------------------------------------------------
void SensoryFaculty::ResolveFriendAndFoe()
{
	CreatureCollection &creatureCollection = theAgentManager.GetCreatureCollection();
	Creature& creature = myCreature.GetCreatureReference();
	int noCreatures = creatureCollection.size();
	int thisCreatureIsImported = true;	// presume posibility as default


	// store time stamp for every creature you know that is 
	// in this world
	for(int i = 0; i != myFriendsAndFoeHandles.size(); i++)
	{
		if(myFriendsAndFoeHandles[i].IsValid())
		{
			myFriendsAndFoeLastEncounters[i] = creature.Life()->GetTickAge();
		
			// if agent handles or valid must be loading a world
			thisCreatureIsImported = false;
		}
		else if(!myFriendsAndFoeMonikers[i].empty() && thisCreatureIsImported)
		{
			// perhaps imported
			if(myFriendsAndFoeMonikers[i] == theApp.GetWorld().GetUniqueIdentifier())
			{
				// matches world id, know this pointer already
				// record tick age that you met this pointer before
				myFriendsAndFoeHandles[i] = thePointer;
				myFriendsAndFoeLastEncounters[i] = creature.Life()->GetTickAge();

				// must be inported so set name of pointer
				// reset the name cos may have changed in creatures absence
				int pointerCat = SensoryFaculty::GetCategoryIdOfClassifier(
					&thePointer.GetPointerAgentReference().GetClassifier());

				
				creature.Linguistic()->ClearWordStrength(LinguisticFaculty::NOUN, pointerCat);
				creature.Linguistic()->SetWord(LinguisticFaculty::NOUN, pointerCat, 
					thePointer.GetPointerAgentReference().GetName(), true);
			}
			else
			{
				// try all creatures against this moniker
				for(int c = 0; c != noCreatures; c++)
				{
					if(myFriendsAndFoeMonikers[i] == 
						creatureCollection[c].GetAgentReference().GetGenomeStore().MonikerAsString(0))
					{
						// met this creature before, record tick age that you met them again
						myFriendsAndFoeHandles[i] = creatureCollection[c];
						myFriendsAndFoeLastEncounters[i] = creature.Life()->GetTickAge();
					}
				}
			}

		}
	}

	if(thisCreatureIsImported)	
	{

		// may not be imported may just have no mates
		// so we waste a little time on world loading - who cares
		
		// tell other creatures your in town
		for(int c = 0; c != noCreatures; c++)
		{
			//  dont check imported creature against itself
			if(myCreature != creatureCollection[c])
			{
				creatureCollection[c].GetCreatureReference().Sensory()->
					AssignFriendOrFoeByMoniker(
					creature.GetGenomeStore().MonikerAsString(0),
					myCreature);	
			}
		}
	}

}



// ------------------------------------------------------------------------
// Function:    AssignFriendOrFoeByMoniker
// Class:       SensoryFaculty
// Description: Helper function for friend or foe stuff.
// Arguments:   std::string moniker = 
//              AgentHandle& creatureOrPointer = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool SensoryFaculty::AssignFriendOrFoeByMoniker(std::string moniker, AgentHandle& creatureOrPointer)
{
	for(int i = 0; i != myFriendsAndFoeHandles.size(); i++)
	{
		if(myFriendsAndFoeMonikers[i] == moniker)
		{
			Creature& creature = myCreature.GetCreatureReference();
			myFriendsAndFoeHandles[i] = creatureOrPointer;
			myFriendsAndFoeLastEncounters[i] = creature.Life()->GetTickAge();
			return true;
		}
	}

	return false;
}



// ------------------------------------------------------------------------
// Function:    RemoveFromAllFriendAndFoe
// Class:       SensoryFaculty
// Description: Helper function for friend or foe stuff.
// Arguments:   AgentHandle& creatureOrPointer = 
// ------------------------------------------------------------------------
void SensoryFaculty::RemoveFromAllFriendAndFoe(AgentHandle& creatureOrPointer)
{
	// recycle slots so keep as may friends from past worlds as posible
	
	CreatureCollection &creatureCollection = theAgentManager.GetCreatureCollection();
	int noCreatures = creatureCollection.size();

	// try all creatures against this moniker
	for(int c = 0; c != noCreatures; c++)
		creatureCollection[c].GetCreatureReference().Sensory()->RemoveFriendAndFoe(creatureOrPointer);
}



// ------------------------------------------------------------------------
// Function:    RemoveFriendAndFoe
// Class:       SensoryFaculty
// Description: Helper function for friend or foe stuff.
// Arguments:   AgentHandle& creatureOrPointer = 
// ------------------------------------------------------------------------
void SensoryFaculty::RemoveFriendAndFoe(AgentHandle& creatureOrPointer)
{
	for(int i = 0; i != myFriendsAndFoeHandles.size(); ++i)
	{
		if(myFriendsAndFoeHandles[i] == creatureOrPointer)
		{
			// recycle slots so keep as may friends from past worlds as posible
			myFriendsAndFoeLastEncounters[i] = 0;
			myFriendsAndFoeMonikers[i] = "";
			myFriendsAndFoeHandles[i] = NULLHANDLE;
			break;
		}
	}
}




// ------------------------------------------------------------------------
// Function:    CleanUpInvalidFriendAgentHandles
// Class:       SensoryFaculty
// Description: Helper function for friend or foe stuff.
// ------------------------------------------------------------------------
void SensoryFaculty::CleanUpInvalidFriendAgentHandles()
{
	// clean up invalid agents to break links
	for(int i = 0; i != myFriendsAndFoeHandles.size(); ++i)
		myFriendsAndFoeHandles[i].IsValid();
}
