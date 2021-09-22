// -------------------------------------------------------------------------
// Filename:    AgentManager.cpp
// Class:       AgentManager
// Purpose:     Looks after the Agent set
// Description: Initially just moving static information from the Agent Class
//
//				I have made the constructor private so that there can be
//				only one agent manager
// Usage:		All Agent creation requests should go through the agent
//				manager
//				
//
// History:
// 20Jan99 Alima	Initial version
//					Moved the main view from the world class to here.
//					agents add themselves to the display engine when they
//					construct and remove themselves during destruction.
//					This is all done through the Agent manager.
//
// 27Jan99	Alima	Added update plane so that objects can tell the display
//					engine that they have switched planes
// 2Feb99	Alima	Moved Main view functions out of here now they are
//					completely stand alone. see MainCamera.cpp/h
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "AgentManager.h"
#include "App.h"
#include "Agents/Agent.h"
#include "Creature/Creature.h"
#include "Agents/SimpleAgent.h"
#include "Agents/CompoundAgent.h"
#include "Agents/PointerAgent.h"
#include "Agents/Vehicle.h"
#include "Display/MainCamera.h"
#include "Display/EntityImage.h"
#include "Classifier.h"
#include "General.h"
#include "Display/ErrorMessageHandler.h"
#include "Display/DisplayEngine.h"
#include "C2eServices.h"
#include "Creature/LifeFaculty.h"
#include "Creature/SensoryFaculty.h"
#include "Creature/MusicFaculty.h"
#include "CreaturesArchive.h"
#include "World.h"

#ifdef C2E_OLD_CPP_LIB
#include <strstream>
#else
#include <sstream>
#endif

#include "Map/Map.h" // FastFloatToInteger

////////////////////////////////////////////////////////////////////////////
// static member variables
////////////////////////////////////////////////////////////////////////////
AgentManager AgentManager::ourAgentManager;
uint32 AgentManager::ourBaseUniqueID = 1;
AgentMapIteratorList AgentManager::ourKillList;
AgentMap AgentManager::ourAgentMap;
AgentList AgentManager::ourAgentList;
TaxonomicalAgentMap AgentManager::ourFGSMap;
TaxonomicalHelperMap AgentManager::ourFGSHelperMap;
CreatureCollection AgentManager::ourCreatureCollection;
int AgentManager::ourCategoryIdsForSmellIds[CA_PROPERTY_COUNT];

////////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////////
AgentManager::AgentManager()
{
	for (int i=0; i<CA_PROPERTY_COUNT; i++) {
		ourCategoryIdsForSmellIds[i] = -1;
	}
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
AgentManager::~AgentManager()
{
}

////////////////////////////////////////////////////////////////////////////
// The only way of accessing the agent manager
////////////////////////////////////////////////////////////////////////////
AgentManager& AgentManager::GetAgentManager()
{
	return ourAgentManager;
}

////////////////////////////////////////////////////////////////////////////
// Agent Creation methods
////////////////////////////////////////////////////////////////////////////


AgentHandle AgentManager::CreateCreature
	(int family, 
	AgentHandle& gene, int gene_index, int8 sex, int8 variant)
{
	std::string monikerForDebugMessage = gene.GetAgentReference().GetGenomeStore().MonikerAsString(gene_index);

	uint32 id = CreateUniqueAgentID();
	AgentHandle creature;
	try
	{
		// Choose my sex randomly if required
		if (sex==0)	
		{
			sex = Rnd(1,2);
		}
		if (variant==0) 
		{
			variant = Rnd(1,NUM_BEHAVIOUR_VARIANTS);
		}
		AgentHandle creature(new Creature(family, id, gene, gene_index, sex, variant));
		if(creature.IsCreature() && creature.GetAgentReference().myFailedConstructionException == "")
		{
		}
		else
		{
#ifdef C2E_OLD_CPP_LIB
			char hackbuf[256];
			std::ostrstream ost( hackbuf, sizeof( hackbuf ) );
#else
			std::ostringstream ost;
#endif

			ost << "Creature:" << std::endl << "Family: " << family << " Moniker: " << monikerForDebugMessage;
			ost << " Gender: " << (int)sex << " Variant: " << (int)variant << std::endl;
			ost << creature.GetAgentReference().myFailedConstructionException;
			ErrorMessageHandler::NonLocalisable(ost.str(),"AgentManager::CreateCreature");
		}
		return creature;
	}

	catch(BasicException& e)
	{
		ErrorMessageHandler::Show(e, std::string("AgentManager::CreateCreature"));
		return NULLHANDLE;
	}
}

AgentHandle AgentManager::CreateSimpleAgent
	(int family, int genus, int species,
	FilePath const& gallery,				
	int32 imagecount,
	int32 firstimage,
	int32 plane)
{
	uint32 id = CreateUniqueAgentID();
	try
	{
		AgentHandle agent(new SimpleAgent(family, genus, species,id,
			gallery,
											imagecount,
											firstimage,
											plane, false));

	
		if (agent.GetAgentReference().myFailedConstructionException.size() == 0) 
		{
			ourAgentList.push_front(agent);
			ourAgentMap[id] = ourAgentList.begin();
			ourFGSHelperMap[id] = ourFGSMap.insert(std::make_pair(agent.GetAgentReference().GetClassifier(),agent));
		}
		else
		{
#ifdef C2E_OLD_CPP_LIB
			char hackbuf[256];
			std::ostrstream ost( hackbuf, sizeof( hackbuf ) );
#else
			std::ostringstream ost;
#endif
			ost << "Simple Agent:" << std::endl << "Family: " << family << " Genus: " << genus << " Species: " << species << std::endl;
			ost << "Gallery: " << gallery.GetFileName() << " First Image: " << firstimage << " Image Count: " << imagecount << std::endl;
			ost << "Created in plane: " << plane << std::endl;
			ost << agent.GetAgentReference().myFailedConstructionException;
			agent.GetAgentReference().Trash();
			ErrorMessageHandler::NonLocalisable(ost.str(),"AgentManager::CreateSimpleAgent");			
		}
		return agent;
		}
	catch(EntityImage::EntityImageException& e)
		{
			ErrorMessageHandler::Show(e, std::string("AgentManager::CreateSimpleAgent"));
			return NULLHANDLE;
		}

	}



AgentHandle AgentManager::CreateCompoundAgent
(int family, int genus, int species,
 FilePath const& gallery,int32 numimages,
							   int32 baseimage,
							   int32 plane)
{
	uint32 id = CreateUniqueAgentID();
	try
	{
		AgentHandle agent(new CompoundAgent(family, genus, species,id,
			gallery,
									 numimages,
								   baseimage,
								   plane));
			
		if (agent.GetAgentReference().myFailedConstructionException.size() == 0)
		{
			ourAgentList.push_front(agent);
			ourAgentMap[id] = ourAgentList.begin();
			ourFGSHelperMap[id] = ourFGSMap.insert(std::make_pair(agent.GetAgentReference().GetClassifier(),agent));
		}
		else
		{
			
#ifdef C2E_OLD_CPP_LIB
			char hackbuf[256];
			std::ostrstream ost( hackbuf, sizeof( hackbuf ) );
#else
			std::ostringstream ost;
#endif
			ost << "Compound Agent:" << std::endl << "Family: " << family << " Genus: " << genus << " Species: " << species << std::endl;
			ost << "Gallery: " << gallery.GetFileName() << " First Image: " << baseimage << " Image Count: " << numimages << std::endl;
			ost << "Created in plane: " << plane << std::endl;
			ost << agent.GetAgentReference().myFailedConstructionException;
			agent.GetAgentReference().Trash();
			ErrorMessageHandler::NonLocalisable(ost.str(),"AgentManager::CreateCompoundAgent");
		}
		return agent;
		}
	catch(EntityImage::EntityImageException& e)
		{
			ErrorMessageHandler::Show(e, std::string("AgentManager::CreateCompoundAgent"));
			return NULLHANDLE;
		}

}



AgentHandle AgentManager::CreateVehicle
	(int family, int genus, int species,
	FilePath const& gallery,
	int32 numimages,
	int32 baseimage,
	int32 plane)
{
	uint32 id = CreateUniqueAgentID();
	try
		{
		AgentHandle agent(new Vehicle(family, genus, species, id, gallery,
								numimages,
							   baseimage,
							   plane));
		
		if (agent.GetAgentReference().myFailedConstructionException.size() == 0)
		{
			ourAgentList.push_front(agent);
			ourAgentMap[id] = ourAgentList.begin();
			ourFGSHelperMap[id] = ourFGSMap.insert(std::make_pair(agent.GetAgentReference().GetClassifier(),agent));
		}
		else
		{	
#ifdef C2E_OLD_CPP_LIB
			char hackbuf[256];
			std::ostrstream ost( hackbuf, sizeof( hackbuf ) );
#else
			std::ostringstream ost;
#endif
			ost << "Vehicle:" << std::endl << "Family: " << family << " Genus: " << genus << " Species: " << species << std::endl;
			ost << "Gallery: " << gallery.GetFileName() << " First Image: " << baseimage << " Image Count: " << numimages << std::endl;
			ost << "Created in plane: " << plane << std::endl;
			ost << agent.GetAgentReference().myFailedConstructionException;
			agent.GetAgentReference().Trash();
			ErrorMessageHandler::NonLocalisable(ost.str(),"AgentManager::CreateVehicle");
		}
		return agent;
		}
	catch(EntityImage::EntityImageException& e)
		{
		ErrorMessageHandler::Show(e, std::string("AgentManager::CreateVehicle"));
		return NULLHANDLE;
		}
}


AgentHandle AgentManager::CreatePointer()
{
	int family, genus, species;
	
	int hotx, hoty;

	// Read the family, genus and species
	std::string classifier = theCatalogue.Get("Pointer Information", 0);
#ifdef C2E_OLD_CPP_LIB
	std::istrstream classifierin( classifier.c_str() );
#else
	std::istringstream classifierin(classifier);
#endif
	classifierin >> family >> genus >> species;

	std::string hotspot = theCatalogue.Get("Pointer Information", 1);

#ifdef C2E_OLD_CPP_LIB
	std::istrstream hotspotin(hotspot.c_str());
#else
	std::istringstream hotspotin(hotspot);
#endif
	hotspotin >> hotx >> hoty;
	Vector2D hotspotvector(hotx, hoty);

	std::string sprite = theCatalogue.Get("Pointer Information", 2);
	FilePath gallery(sprite + ".s16", IMAGES_DIR);
	uint32 id = CreateUniqueAgentID();
	try
	{
		AgentHandle agent( new PointerAgent(family, genus, species, id,
			gallery, 0, 0));

		if (agent.GetPointerAgentReference().myFailedConstructionException.size() == 0)
		{
			agent.GetPointerAgentReference().SetHotSpot(hotspotvector);
			ourAgentList.push_front(agent);
			ourAgentMap[id] = ourAgentList.begin();
			ourFGSHelperMap[id] = ourFGSMap.insert(std::make_pair(agent.GetAgentReference().GetClassifier(),agent));
			theMainView.KeepUpWithMouse(agent.GetAgentReference().GetEntityImage());
		}
		else
		{
			
#ifdef C2E_OLD_CPP_LIB
			char hackbuf[256];
			std::ostrstream ost( hackbuf, sizeof( hackbuf ) );
#else
			std::ostringstream ost;
#endif
			ost << "Pointer Agent:" << std::endl << "Family: " << family << " Genus: " << genus << " Species: " << species << std::endl;
			ost << "Gallery: " << gallery.GetFileName() << std::endl;
			ost << agent.GetAgentReference().myFailedConstructionException;
			agent.GetAgentReference().Trash();
			ErrorMessageHandler::NonLocalisable(ost.str(),"AgentManager::CreatePointer");
		}
	return agent;
	}
	catch(EntityImage::EntityImageException& e)
	{
		ErrorMessageHandler::Show(e, std::string("AgentManager::CreatePointer"));
		return NULLHANDLE;
	}

}



////////////////////////////////////////////////////////////////////////////
// Agent Update methods
////////////////////////////////////////////////////////////////////////////

void AgentManager::UpdateAllAgents()
{
	ExecuteDeferredScripts();
	
	AgentListIterator it;
	AgentMapIteratorListIterator kit;
	// By declaring the handle here, we ensure that there is
	// at least one reference to the agent memory during the Update()
	// call.
	AgentHandle agent;

	for (it=ourAgentList.begin(); it!=ourAgentList.end(); ++it)
	{
		agent = (*it);
		if (agent.IsValid())
		{
			Agent& agentref = agent.GetAgentReference();

			
			if (agentref.IsRunning())
			{
				#ifdef AGENT_PROFILER
					int64 stamp1 = GetHighPerformanceTimeStamp();
				#endif
				if (!agentref.AreYouDoomed())
					agentref.Update();

				#ifdef AGENT_PROFILER
					if (!agentref.IsGarbage())
					{
						int64 stamp2 = GetHighPerformanceTimeStamp();
						agentref.myAgentProfilerCumulativeTime -= stamp1;
						agentref.myAgentProfilerCumulativeTime += stamp2;
						++agentref.myAgentProfilerCumulativeTicks;
					}
				#endif

				if (agentref.AreYouDoomed())
					KillAgent(agent);
			}
			
		}
	}
#ifdef AGENT_PROFILER
	++Agent::ourAgentProfilerTicks;
	Agent::ourAgentProfilerPaceTotal += (double)theApp.GetTickRateFactor();
#endif

	// Now we clean the map.
	for (kit = ourKillList.begin(); kit != ourKillList.end(); ++kit)
	{
		ourAgentList.erase((*(*kit)).second);
		ourAgentMap.erase(*kit);
	}
	ourKillList.clear();
}


void AgentManager::ExecuteScriptOnAllAgents
	(int event, AgentHandle& from, 
	 const CAOSVar& p1, const CAOSVar& p2)
{
	AgentListIterator it;
	AgentMapIteratorListIterator kit;
	// By declaring the handle here, we ensure that there is
	// at least one reference to the agent memory during the Update()
	// call.
	AgentHandle agent;

	for (it=ourAgentList.begin(); it!=ourAgentList.end(); ++it)
	{
		agent = (*it);
		if (agent.IsValid())
		{
			Agent& agentref = agent.GetAgentReference();
			
				if (agentref.IsRunning() && !agentref.AreYouDoomed())
					agentref.ExecuteScriptForEvent(event, from, 
						p1, p2);
				if (agentref.AreYouDoomed())
					KillAgent(agent);

		
		}
	}
	// Now we clean the map.
	for (kit = ourKillList.begin(); kit != ourKillList.end(); ++kit)
	{
		ourAgentList.erase((*(*kit)).second);
		ourAgentMap.erase(*kit);
	}
	ourKillList.clear();
}

void AgentManager::ExecuteScriptOnAllAgentsDeferred
	(int event, AgentHandle& from, 
	 const CAOSVar& p1, const CAOSVar& p2)
{
	myDeferredScripts.push_back( DeferredScript( event, from, p1, p2 ) );
}

void AgentManager::ExecuteDeferredScripts()
{
	DeferredScriptList tempList;
	tempList.swap( myDeferredScripts );

	for( DeferredScriptList::iterator iter = tempList.begin();
		iter != tempList.end(); ++iter )
		ExecuteScriptOnAllAgents( iter->event, iter->from, iter->p1, iter->p2 );
}

CreaturesArchive& operator<<( CreaturesArchive &ar, AgentManager::DeferredScript const &script )
{
	ar << script.event << script.from << script.p1 << script.p2;
	return ar;
}

CreaturesArchive& operator>>( CreaturesArchive &ar, AgentManager::DeferredScript &script )
{
	ar >> script.event >> script.from >> script.p1 >> script.p2;
	return ar;
}

// Plot planes for different objects
const int MINCREATUREPLANE = 1000;	// all creatures will be plotted (uniquely) 
const int MAXCREATUREPLANE = 3000;	// on a plane within this range

// ----------------------------------------------------------------------
// Method:      UniqueCreaturePlane 
// Arguments:   none	
//
// Returns:     a unique plane
//
// Description: cycle through agents and find an available plane
//				Scan all existing creatures and pick a plot plane 
//				that doesn't clash with any of them.
//				This plot plane will be yours for the rest of your life 
//				OR UNTIL YOU ARE IMPORTED ELSEWHERE!				
// ----------------------------------------------------------------------
int32 AgentManager::UniqueCreaturePlane(AgentHandle& me)
{
	int32 plane, unique, i;

	do {
		// pick a plane at random
		plane = Rnd(MINCREATUREPLANE,MAXCREATUREPLANE);	
	
		// assume it's mine
		unique=1;										
		
		// scan other creatures fr clash
		for	(i=0; i<ourCreatureCollection.size(); i++)				
				
			if	((ourCreatureCollection[i]!=me)&&
				 (ourCreatureCollection[i].GetCreatureReference().GetPlane(0) >= plane)&&
				 (ourCreatureCollection[i].GetCreatureReference().GetPlane(0) <= plane+8)) {
				unique = 0;
				break;
		}
	} while (!unique);
	return plane;
}

// ----------------------------------------------------------------------
// Method:      WhoAmITouching
// Arguments:   me - agent doing the touching		
//
// Returns:     the first creature you can hear
//
// Description: cycle through creatures and find the first one that
//				 the given creature is touching
//						
// ----------------------------------------------------------------------
AgentHandle AgentManager::WhoAmITouching(AgentHandle& me)
{
	int32 i;
	AgentHandle o;
	Box rs,rd,rtemp;


	me.GetAgentReference().GetAgentExtent(rs);                                 // get bound of sender

	for (i=0; i<ourCreatureCollection.size(); i++) 
	{           // scan creature library
		if  (ourCreatureCollection[i]==me)                      // (ignoring self)
			continue;
		o = ourCreatureCollection[i];            					// get ptr to it

		// if sender = vehicle
		// & carrying creature
		// vehicle cast
		if  (me.IsVehicle() &&					
			 (o.GetAgentReference().GetCarrier()==me))
		{		
			return o;
		}

		else 
		{											
			// if not a vehicle
			// get sender's loc
			o.GetAgentReference().GetAgentExtent(rd);							

			// if creature is touching
			if  (rtemp.IntersectRect(rs,rd))			
					return o;	

		}
	}
return o;
}


AgentHandle AgentManager::WhatVehicleCouldIDropInto
	(const AgentHandle& me, const AgentHandle& ignore)
{
	AgentListIterator it;
	AgentHandle v, a;
	uint32 plane, foundPlane = 0;
	Box agentbox, cabin;

	me.GetAgentReference().GetAgentExtent(agentbox);
	float agentWidth = agentbox.Width();
	float agentHeight = agentbox.Height();
	Vector2D agentCentre = me.GetAgentReference().GetCentre();

	for (it=ourAgentList.begin(); it!=ourAgentList.end(); ++it)
	{
		a = *it;
		if (!a.IsVehicle())
			continue;
		
		if (a == ignore)
			continue;
	
		Vehicle& vehicleref = a.GetVehicleReference();

		plane = vehicleref.GetPlaneForCarriedAgent();

		if (plane < foundPlane)
			continue;
			
		if (vehicleref.TestAttributes(Agent::attrGreedyCabin) != Agent::attrGreedyCabin)
			continue;

		vehicleref.GetCabinExtent(cabin);

		if ((agentWidth > (cabin.Width()-2.0f)) ||
			(agentHeight > (cabin.Height()-2.0f)))
			// Agent can't fit into this cabin
			continue;

		if ((agentCentre.x <= cabin.left) ||
			(agentCentre.x >= cabin.right) ||
			(agentCentre.y <= cabin.top) ||
			(agentCentre.y >= cabin.bottom))
			// Centre of agent does not lie inside cabin
			continue;

		foundPlane = plane;
		v = a;
	}
	return v;
}



// ----------------------------------------------------------------------
// Method:      HowManyCreaturesCanIHear
// Arguments:   me - agent doing the hearing		
//				score - total for how much noise neighbouring creatures
//						are making
//
// Returns:     the first creature you can hear
//
// Description: cycle through creatures and accumulate a score for each
//				one I can hear
//						
// ----------------------------------------------------------------------
void AgentManager::HowManyCreaturesCanIHear(AgentHandle& me,
											int32& score)
{
	
	CreatureCollectionIterator creature;
	for( creature = ourCreatureCollection.begin(); 
					creature != ourCreatureCollection.end(); creature++ )
	{     
        if (me.GetCreatureReference().CanHear((*creature)))
        {
            int32 tmp = (int32)me.GetCreatureReference().GetAudibleRange() - 
				abs( (int32)(me.GetCreatureReference().GetPosition().x - (*creature).GetCreatureReference().GetPosition().x) );
            score += (tmp * tmp);
        }
    }
}

// ----------------------------------------------------------------------
// Method:      WhatAmITouching
// Arguments:   me - creature doing the looking		
//
// Returns:     the first creature you can hear
//
// Description: cycle through agents and find the best one that
//				 the given agent is touching
//						
// ----------------------------------------------------------------------
AgentHandle AgentManager::WhatAmITouching(Vector2D& ptThis,
									 AgentHandle& me,
									 uint32 AttrFields,
									 uint32 AttrState)
{
	int32 BestPlane = -1;
	AgentHandle BestObj;
    AgentHandle Obj;
	AgentListIterator it;
	for( it = ourAgentList.begin(); it != ourAgentList.end(); ++it )
	{
		
		Obj = (*it);
		if (Obj.IsInvalid())
			continue;

        if (Obj == me)
            continue;

	    // If agent nearer to the front and valid?
        if (((Obj.GetAgentReference().GetAttributes() & AttrFields) == AttrState)
							&& (BestPlane < Obj.GetAgentReference().GetPlane()))
		{
			if (Obj.GetAgentReference().HitTest(ptThis))
			{
				BestPlane = Obj.GetAgentReference().GetPlane();		// remember its details
				BestObj = Obj;
			}          
        }
    }                                               // look at others

    return BestObj;        
}

AgentHandle AgentManager::WhatCreatureAmIOver(const AgentHandle& me, const Vector2D& xy)
{
	Box r;
	int i;
	AgentHandle agent;
	AgentHandle bestObj;
	int32 bestPlane = -1;
	for( i = 0; i < ourCreatureCollection.size(); i++ )
	{
		agent = ourCreatureCollection[i];

		if (agent.IsInvalid())
			continue;

        if (agent==me) 
			continue;					
        
		agent.GetAgentReference().GetAgentExtent(r);						// get its boundary
        
		if( r.PointInBox(xy) && bestPlane < agent.GetAgentReference().GetPlane())
		{
			if (agent.GetAgentReference().HitTest(xy))
			{
				bestPlane  = agent.GetAgentReference().GetPlane();
				bestObj = agent;
			}
		}
    }                                               

    
	return bestObj;
}

AgentHandle AgentManager::WhatAmIOver(const AgentHandle& me, const Vector2D& xy)
{
	Box r;
	AgentListIterator it;
	AgentHandle agent;
	AgentHandle bestObj;
	int32 bestPlane = -1;
	for( it=ourAgentList.begin(); it != ourAgentList.end(); ++it )
	{
		agent = (*it);

		if (agent.IsInvalid())
			continue;

		if(agent.IsPointerAgent())
			continue;

        if (agent==me) 
			continue;					
        
		agent.GetAgentReference().GetAgentExtent(r);						// get its boundary
        
		if(bestPlane < agent.GetAgentReference().GetPlane())
		{
//			return agent;			// if I'm over it, return
			//Let's do a hit test on the agent now...
			if (agent.GetAgentReference().HitTest(xy))
			{
				bestPlane  = agent.GetAgentReference().GetPlane();
				bestObj = agent;
			}
		}
    }                                               

    
	return bestObj;
}


// ----------------------------------------------------------------------
// Method:      CalculateMoodAndThreat
// Arguments:   iSelectableCount - how many creatures were selectable and on screen
//				iLowestHealth - the lowest health value encountered
//				fMood		- average creature's mood
//				fThreat   - average creature's threat		
//
// Returns:     the first creature you can hear
//
// Description: get a sum of selectable creature's mood and threat.
//						
// ----------------------------------------------------------------------
void AgentManager::CalculateMoodAndThreat(uint32& iSelectableCount,
											uint32& iLowestHealth,
											float& fMood,
											float& fThreat)
{
	iSelectableCount = 0;
	fMood = 0.0f;
	fThreat = 0.0f;
	iLowestHealth = 255;
    // Calculate mood & threat.
    CreatureCollectionIterator subject;
	for (subject=ourCreatureCollection.begin(); subject!=ourCreatureCollection.end(); subject++)
	{
		if ((*subject).IsInvalid())
			continue;

		Creature& c = (*subject).GetCreatureReference();
		// Sum mood of all selectable creatures visible in view port,
		// and which aren't still hatching
		if (c.Visibility(0) &&
			c.Music()->SelectableByUser() && !c.Music()->Hatching())
		{
			fMood += c.Music()->Mood();
			fThreat += c.Music()->Threat();
			iSelectableCount++;

			uint32 health = Map::FastFloatToInteger(c.Life()->Health() * 255.0f);
            if (health < iLowestHealth)
			{
                iLowestHealth = Map::FastFloatToInteger(health);
            }
        }
	}
	if (iSelectableCount > 0)
	{
		fMood /= iSelectableCount;
		fThreat /= iSelectableCount;
	}
}


void AgentManager::KillAllAgents()
{
	AgentListIterator it;
	AgentMapIteratorListIterator kit;
	for( it=ourAgentList.begin(); it!=ourAgentList.end(); ++it )
	{
		if ((*it).IsValid() && !(*it).IsPointerAgent())
			KillAgent((*it));
	}
	for( it=ourAgentList.begin(); it!=ourAgentList.end(); ++it )
	{
		if ((*it).IsValid())
			KillAgent((*it));
	}

	for (kit = ourKillList.begin(); kit != ourKillList.end(); ++kit)
	{
		ourAgentList.erase((*(*kit)).second);
		ourAgentMap.erase(*kit);
	}
	ourKillList.clear();

}




void AgentManager::KillAgent(const AgentHandle& agent)
{
	AgentHandle agent2(agent);
	_ASSERT( agent2.IsValid() );

	if (agent2.IsInvalid())
		return;				// We are not interested in removing agents which don't exist.
	if (agent2.IsCreature())
	{
		CreatureCollectionIterator creature;
		creature = std::find(ourCreatureCollection.begin(), ourCreatureCollection.end(),
			agent);
		ourCreatureCollection.erase(creature);
		if (theApp.GetWorld().GetSelectedCreature() == agent)
			theApp.GetWorld().SetSelectedCreature(NULLHANDLE);
	}
	// We assume that agent is a valid agent on entry
	// Remove the agent from the agent list
	AgentMapIterator it;
	it = ourAgentMap.find(agent.GetAgentReference().GetUniqueID());
	
	_ASSERT( it != ourAgentMap.end() );

	TaxonomicalHelperIterator thit = ourFGSHelperMap.find(agent.GetAgentReference().GetUniqueID());
	
	ASSERT( thit != ourFGSHelperMap.end() );
	
	TaxonomicalAgentIterator tait = (*thit).second;

	ASSERT( tait != ourFGSMap.end() );

	ourFGSHelperMap.erase(thit);
	ourFGSMap.erase(tait);

	ourKillList.push_back(it);
	
	agent.GetAgentReference().Trash();

	(*((*it).second)) = NULLHANDLE;
	
}



// ----------------------------------------------------------------------
// Method:      GetAgentFromID 
// Arguments:   id - id of agent to find			
//
// Returns:     the agent with the requested id or none
//
// Description: go figure
//						
// ----------------------------------------------------------------------
AgentHandle AgentManager::GetAgentFromID(uint32 id)
{
	AgentHandle agent;
	AgentMapIterator it;

	it = ourAgentMap.find( id );
	if( it == ourAgentMap.end() )
		agent = NULLHANDLE;
	else
	{
		agent = *((*it).second);	// return the Agent ptr
		if( agent.IsInvalid() )
			agent = NULLHANDLE;
	}
	return agent;
}









bool AgentManager::UpdateSmellClassifierList(int caIndex, Classifier* classifier)
{
	if (caIndex<0 || 
		(classifier->Family()==0 && classifier->Genus()==0 &&
		classifier->Species()==0 && classifier->Event()==0))
		return false;

	ASSERT(caIndex<CA_PROPERTY_COUNT);

	int categoryId = SensoryFaculty::GetCategoryIdOfClassifier(classifier);

	// nav ca's can not be given a catagory if they already have one
	for (int i=0; i<CA_PROPERTY_COUNT; i++) {
		if (ourCategoryIdsForSmellIds[i]==categoryId && caIndex != i 
			&& theApp.GetWorld().GetMap().IsCANavigable(i))
			return false;
	}

	ourCategoryIdsForSmellIds[caIndex] = categoryId;
	return true;
}

int AgentManager::GetCategoryIdFromSmellId(int smellId) {
	return ourCategoryIdsForSmellIds[smellId];
}

int AgentManager::GetSmellIdFromCategoryId(int categoryId) {
	for (int i=0; i<CA_PROPERTY_COUNT; i++) {
		if (ourCategoryIdsForSmellIds[i]==categoryId)
			return i;
	}
	return -1;
}



// NOTE - THESE FUNCTIONS CURRENTLY DO BRUTE-FORCE SEARCHES.
// THIS IS A BAD THING(tm).
//
// SHOULD BE REIMPLEMENTED USING A CLASSIFICATION TREE AND/OR SPATIAL
// PARTITIONING.


void AgentManager::FindByFGS( AgentList& agents, const Classifier& c )
{
	if (((c.myFamily == 0) && ((c.myGenus != 0) || (c.mySpecies != 0))) || ((c.myGenus == 0) && (c.mySpecies != 0)))
	{
		AgentListIterator it;
		for( it=ourAgentList.begin(); it!=ourAgentList.end(); ++it )
		{
			if( (*it).IsValid() &&
				(*it).GetAgentReference().GetClassifier().GenericMatchForWildCard( c,
				TRUE, TRUE, TRUE, FALSE ) )
			{
				agents.push_back( (*it) );
			}
		}
		return;
		
	}
	TaxonomicalAgentIterator bit,uit;
	bit = ourFGSMap.lower_bound(c);
	if (bit == ourFGSMap.end())
	{
		return;
	}
	// Cunning Upper bounder :)
	Classifier d(c);
	if (d.Family() == 0)
		uit = ourFGSMap.end();
	else if (d.Genus() == 0)
	{
		d.myFamily += 1;
		uit = ourFGSMap.lower_bound(d);
	}
	else if (d.Species() == 0)
	{
		d.myGenus += 1;
		uit = ourFGSMap.lower_bound(d);
	}
	else
		uit = ourFGSMap.upper_bound(c);
	for ( ; bit != uit; bit++)
	{
		if ((*bit).second.IsValid())
			agents.push_back( (*bit).second );
	}
	
}

void AgentManager::FindByArea( AgentList& agents, const Box& r )
{
	FindByArea( ourAgentList, agents, r );
}
void AgentManager::FindByArea( AgentList& input, AgentList& agents, const Box& r )
{
	AgentListIterator it;
	for( it=input.begin(); it!=input.end(); ++it )
	{
		if( (*it).IsValid() && (*it).GetAgentReference().DoBoundsIntersect(r))
		{
				agents.push_back( (*it) );
		}
	}
}




void AgentManager::FindByAreaAndFGS( AgentList& agents,
	const Classifier& c, const Box& r )
{
	AgentListIterator it;
	AgentListIterator itnext;
	AgentList temp;
	// EITHER:
	// 1) FindByArea( agents, r );
	// 2) filter out all agents with non-matching classifiers
	//
	// OR:
	// 1) FindByFGS( agents, c );
	// 2) filter out all agents outside rect
	//
	// WHICHEVER IS FASTER!

	FindByFGS( temp, c );
	FindByArea( temp,agents, r );

	/*
	// cull out ones with wrong fgs
	it = agents.begin();
	while(  it != agents.end() )
	{
		itnext = it;
		++itnext;

		if( !(*it).GetAgentReference().GetClassifier().GenericMatchForWildCard( c, TRUE, TRUE,
			TRUE, FALSE ) )
		{
			agents.erase( it );
		}
		it = itnext;
	}*/

}


// find agents, of particular fgs, which can be seen by a viewer agent
void AgentManager::FindBySightAndFGS( AgentHandle const& viewer,
	AgentList& agents,
	const Classifier& c )
{
	AgentListIterator it;
	AgentListIterator itnext;

	FindByFGS( agents, c );

	// cull out ones which can't be seen
	it = agents.begin();
	while(  it != agents.end() )
	{
		itnext = it;
		++itnext;

		// note: CanSee() will ignore viewer agent
		if( !viewer.GetAgentReference().CanSee(*it) )
			agents.erase( it );

		it = itnext;
	}
}

AgentHandle AgentManager::FindNextAgent(AgentHandle& was, const Classifier& c)
{
	AgentList agents;
	FindByFGS(agents, c);

	if (agents.empty())
		return NULLHANDLE;

	AgentListIterator it;
	AgentListIterator itprev = --agents.end();
	for (it = agents.begin(); it != agents.end(); ++it)
	{
		if (*itprev == was)
			return *it;

		itprev = it;
	}

	return *agents.begin();
}

AgentHandle AgentManager::FindPreviousAgent(AgentHandle& was, const Classifier& c)
{
	AgentList agents;
	FindByFGS(agents, c);

	if (agents.empty())
		return NULLHANDLE;

	AgentListIterator it;
	AgentListIterator itprev = --agents.end();
	for (it = agents.begin(); it != agents.end(); ++it)
	{
		if (*it == was)
			return *itprev;

		itprev = it;
	}

	return *agents.begin();
}

// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool AgentManager::Write(CreaturesArchive &ar) const
{
	AgentListIterator it;
	int size = ourAgentList.size();
	ar << size;
	theApp.SpecifyProgressIntervals(size);
	for( it=ourAgentList.begin(); it!=ourAgentList.end(); ++it )
	{
		if ((*it).IsValid())
			ar << (*it).GetAgentReference().GetUniqueID() << (*it);
		else
			ar << ((int)-1);
		theApp.UpdateProgressBar();
	}

	for (int i=0; i<CA_PROPERTY_COUNT; i++) {
		ar << ourCategoryIdsForSmellIds[i];
	}

	ar << ourCreatureCollection;
	ar << ourBaseUniqueID;
	ar << myDeferredScripts;
	
	return true;
}

bool AgentManager::Read(CreaturesArchive &ar)
{

	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{
		int count;
		ar >> count;
		theApp.SpecifyProgressIntervals(count);
		while( count-- )
		{
			int id;
			AgentHandle agent;
			ar >> id;
			if( id != -1 )
			{
				ar >> agent;
				if( agent.IsValid() )
				{
					agent.GetAgentReference().SetUniqueID(id);
					ourAgentList.push_front(agent);
					ourAgentMap[id] = ourAgentList.begin();
					ourFGSHelperMap[id] = ourFGSMap.insert(std::make_pair(agent.GetAgentReference().GetClassifier(),agent));
				}
				if (agent.IsPointerAgent())
					thePointer = agent;
			}
			theApp.UpdateProgressBar();
		}

		for (int i=0; i<CA_PROPERTY_COUNT; i++) {
			ar >> ourCategoryIdsForSmellIds[i];
		}

		ar >> ourCreatureCollection;
		ar >> ourBaseUniqueID;
		ar >> myDeferredScripts;
	
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// Right, all the blasted agents are loaded in and stabilised - let's have some fun :)
	AgentListIterator it;
	for(it = ourAgentList.begin(); it != ourAgentList.end(); it++)
		if ((*it).GetAgentReference().AreYouDoomed())
			KillAgent(*it);

	return true;
}


void AgentManager::RegisterClone(AgentHandle& clone)
{
	uint32 id = CreateUniqueAgentID();
	ourAgentList.push_front(clone);
	ourAgentMap[id] = ourAgentList.begin();
	Agent& cloneref = clone.GetAgentReference();
	cloneref.SetUniqueID(id);
	ourFGSHelperMap[id] = ourFGSMap.insert(std::make_pair(clone.GetAgentReference().GetClassifier(),clone));
	if (clone.IsCreature())
		ourCreatureCollection.push_back(clone);
}


// returns invalid handle if no creature agent for moniker
AgentHandle AgentManager::FindCreatureAgentForMoniker(std::string moniker)
{	
	if (moniker.empty())
		return NULLHANDLE;

	CreatureCollectionIterator creature;
	for( creature = ourCreatureCollection.begin(); 
					creature != ourCreatureCollection.end(); creature++ )
	{     
		if (creature->GetCreatureReference().GetMoniker() == moniker)
		{
			return *creature;
		}
    }

	return NULLHANDLE;
}

// returns invalid handle if no agent has a reference to the moniker
AgentHandle AgentManager::FindAgentForMoniker(std::string moniker)
{	
	if (moniker.empty())
		return NULLHANDLE;

	AgentListIterator it;
	for( it=ourAgentList.begin(); it!=ourAgentList.end(); ++it )
	{
		if (it->IsValid())
		{
			GenomeStore& genomeStore= it->GetAgentReference().GetGenomeStore();
			int n = genomeStore.GetSlotCount();
			for (int i = 0; i < n; ++i)
			{
				if (genomeStore.MonikerAsString(i) == moniker)
					return *it;
			}
		}
	}

	return NULLHANDLE;
}

void AgentManager::AddCreatureToUpdateList(AgentHandle const & creature)
{
	uint32 id = creature.GetCreatureReference().GetUniqueID();

	ourCreatureCollection.push_back(creature);
	ourAgentList.push_front(creature);
	ourAgentMap[id] = ourAgentList.begin();
	ourFGSHelperMap[id] = ourFGSMap.insert(std::make_pair(creature.GetAgentReference().GetClassifier(),creature));
}


