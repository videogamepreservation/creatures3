// -------------------------------------------------------------------------
// Filename:    CreatureHandlers.cpp
// Class:       CreatureHandlers
// Purpose:     Routines to implement creature-related commands/values in CAOS.
// Description:
//
// Usage:
//
// History:
// 21Dec98  BenC	Initial version
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CreatureHandlers.h"
#include "CAOSMachine.h"

#include "../Agents/Agent.h"
#include "../Agents/SimpleAgent.h"
#include "../World.h"
#include "../App.h"

#include "../Stimulus.h"
#include "../Creature/Creature.h"

#include "../Creature/Biochemistry/Biochemistry.h"
#include "../Creature/Biochemistry/Organ.h"
#include "../Creature/Brain/Brain.h"

#include "../Creature/LifeFaculty.h"
#include "../Creature/LinguisticFaculty.h"
#include "../Creature/SensoryFaculty.h"
#include "../Creature/MotorFaculty.h"
#include "../Creature/MusicFaculty.h"
#include "../Creature/ReproductiveFaculty.h"


float CreatureHandlers::FloatRV_MTHX( CAOSMachine& vm )
{
	Creature& c = vm.GetCreatureTarg();
	return c.GetMouthMapPosition().x;
}

float CreatureHandlers::FloatRV_MTHY( CAOSMachine& vm )
{
	Creature& c = vm.GetCreatureTarg();
	return c.GetMouthMapPosition().y;
}

void CreatureHandlers::Command_DONE( CAOSMachine& vm )
{
	Creature& c = vm.GetCreatureTarg();

	c.Motor()->StopCurrentInvoluntaryAction();
}

int CreatureHandlers::IntegerRV_DEAD( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().Life()->GetWhetherDead() ? 1 : 0;
}

void CreatureHandlers::Command_DEAD( CAOSMachine& vm )
{
	vm.GetCreatureTarg().Life()->SetWhetherDead(true);
}

void CreatureHandlers::Command_SPNL( CAOSMachine& vm )
{
	std::string lobeToken;
	vm.FetchStringRV(lobeToken);
	int neuronNo = vm.FetchIntegerRV();
	vm.GetCreatureTarg().GetBrain()->SetInput(lobeToken.c_str(), neuronNo, vm.FetchFloatRV());
}

void CreatureHandlers::Command_WALK( CAOSMachine& vm )
{
	vm.GetCreatureTarg().Walk();
}


void CreatureHandlers::Command_MVFT( CAOSMachine& vm )
{
	float x = vm.FetchFloatRV();
	float y = vm.FetchFloatRV();

	Creature& creature = vm.GetCreatureTarg();

	if(!creature.AreYouHoldingHandsWithThePointer())
	{
		bool ok = creature.TestMoveFootTo(x,y,false);
		if (!ok) 
		{
			if (creature.GetMovementStatus() == Agent::INVEHICLE)
				vm.ThrowRunError( CAOSMachine::sidInvalidVehiclePosition, x, y);
			else
				vm.ThrowRunError( CAOSMachine::sidInvalidMapPosition2, x, y );
		}
		
		creature.MoveFootTo( x,y );
		// it's now in a valid position (agents when they're
		// constructed aren't)
		creature.MarkValidPositionNow();
	}
}


void CreatureHandlers::Command_GAIT( CAOSMachine& vm )
{
	int gait = vm.FetchIntegerRV();
	if (gait<0 || gait>=MAX_GAITS) {
		vm.ThrowRunError( CAOSMachine::sidNoSuchGaitNo);
	}
	vm.GetCreatureTarg().SetGait(gait);
}


void CreatureHandlers::Command_DIRN( CAOSMachine& vm )
{
	int d = vm.FetchIntegerRV();
	if (d<0 || d>=4) {
		vm.ThrowRunError( CAOSMachine::sidNoSuchDirectionNo);
	}
	vm.GetCreatureTarg().SetDirection(d);
}


int CreatureHandlers::IntegerRV_DIRN( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().GetDirection();
}



void CreatureHandlers::Command_DRIV( CAOSMachine& vm ) 
{
	int driveId = vm.FetchIntegerRV();
	float value = vm.FetchFloatRV();

	// silently ignore if we're not a creature
	if (!vm.GetTarg().IsCreature())
		return;

	int chem = vm.GetCreatureTarg().Sensory()->GetChemicalNumberOfDrive(driveId);
	if(chem == -1) return;	// allow no mapping to a chemical
	
	Classifier scriptNo;
	vm.GetScript()->GetClassifier(scriptNo);
#ifdef STIM_TEST_TRACE
	OutputFormattedDebugString("DRIV classifier %d %d %d %d\n", scriptNo.Family(), scriptNo.Genus(), scriptNo.Species(), scriptNo.Event());
#endif
	vm.GetCreatureTarg().Sensory()->AdjustChemicalLevelWithTraining(
		chem,
		value,
		scriptNo.Event(),
		vm.GetOwner());


}

void CreatureHandlers::Command_CHEM( CAOSMachine& vm ) 
{
	int chem = vm.FetchIntegerRV();
	float conc = vm.FetchFloatRV();

	// silently ignore if we're not a creature
	if (!vm.GetTarg().IsCreature())
		return;

	if (chem >= 0 && chem <=255)
		vm.GetCreatureTarg().GetBiochemistry()->AddChemical(chem, conc);
	else
		vm.ThrowRunError( CAOSMachine::sidChemicalOutOfRange );
}

float CreatureHandlers::FloatRV_CHEM( CAOSMachine& vm ) 
{
	int chem = vm.FetchIntegerRV();
	if (chem >= 0 && chem <=255)
		return vm.GetCreatureTarg().GetBiochemistry()->GetChemical(chem);
	else
		vm.ThrowRunError( CAOSMachine::sidChemicalOutOfRange );

	return -999.0f;
}

void CreatureHandlers::Command_INJR( CAOSMachine& vm ) 
{
	Creature &c = vm.GetCreatureTarg();

	// Organ number. -1 = Randomly choose organ. 0 = Implied 'body' organ.
	int organNumber = vm.FetchIntegerRV();
	float amountOfInjury = vm.FetchFloatRV();

	int noOfOrgans = c.GetBiochemistry()->GetOrganCount();
	if (organNumber<0 || organNumber>=noOfOrgans)
	{
		organNumber = Rnd(0, noOfOrgans-1);
	}

	Organ* organ = c.GetBiochemistry()->GetOrgan(organNumber);
	organ->Injure(organ->LocToLf(amountOfInjury) / 10.0);
}

void CreatureHandlers::Command_APPR( CAOSMachine& vm )
{
	Creature& c = vm.GetCreatureTarg();

	// Approach:
	c.Walk();
	if (!vm.IsBlocking()) {					// we want to keep walking every tick
		vm.Block();
	}

	// Map:
	Map& map = theApp.GetWorld().GetMap();
	int creatureFootRoomId;
	if (!map.GetRoomIDForPoint(c.GetDownFootPosition(), creatureFootRoomId)) {
		c.ResetAnimationString();		// stop walking if not in a room
		if (vm.IsBlocking())
			vm.UnBlock();
		return;
	}
	int attnId = c.Motor()->GetCurrentAttentionId();
	int creatureTargCategory = SensoryFaculty::GetCategoryIdOfAgent(AgentHandle(c));
	int caIndex = theAgentManager.GetSmellIdFromCategoryId(attnId);
	int creatureRealRoomId = -1;
	// set it so you can't smell yourself if you're navigating to other creatures of your category:
	if (attnId==creatureTargCategory)
	{	
		if (c.GetRoomID(creatureRealRoomId))
			map.AlterCAEmission(creatureRealRoomId, caIndex, -c.GetCAIncrease());
		else
			creatureRealRoomId = -1;
	}
	int mapDirection = GO_NOWHERE;
	map.WhichDirectionToFollowCA(creatureFootRoomId, caIndex, mapDirection, true);		// true means approach
	// set it so you can smell yourself again:
	if (attnId==creatureTargCategory && creatureRealRoomId>=0)
		map.AlterCAEmission(creatureRealRoomId, caIndex, +c.GetCAIncrease());

	
	// Send peak of smell signal:
	if (mapDirection==GO_NOWHERE)
	{ 
		AgentHandle targ = vm.GetTarg();
		c.Sensory()->Stimulate(targ, STIM_REACHED_PEAK_OF_SMELL0 + caIndex, -1);
	}

	// Regular walking:
	if (c.GetItAgent().IsValid()) {
		c.SetIntrospective(false);
		if (!c.CannotBeMuchCloser())		// keep trying to get to it:
			return;

		c.ResetAnimationString();			// got as close as we can get so stop walking!
		if (vm.IsBlocking())
			vm.UnBlock();
		return;
	}
	else
	{
		// CA stuff:
		c.SetIntrospective(true);
		NavigateDirection(c, mapDirection, caIndex);
	}
}






void CreatureHandlers::Command_FLEE( CAOSMachine& vm )
{
	Creature& c = vm.GetCreatureTarg();

	c.Walk();
	if (!vm.IsBlocking()) {					// we want to keep walking every tick
		vm.Block();
	}

	// Regular walking:
	if (c.GetItAgent().IsValid()) {
//		if (!c.ByWall())		// keep trying to get to it:
			return;

//		c.ResetAnimationString();			// got as close as we can get so stop walking!
//		if (vm.IsBlocking())
//			vm.UnBlock();
//		return;
	}


	// default is ignoring the IT agent:
	c.SetIntrospective(true);

	// Get Current Room:
	Map& map = theApp.GetWorld().GetMap();
	int creatureFootRoomId;
	if (!theApp.GetWorld().GetMap().GetRoomIDForPoint(c.GetDownFootPosition(), creatureFootRoomId)) {
		c.ResetAnimationString();		// stop walking!
		if (vm.IsBlocking())
			vm.UnBlock();
		return;							// not in a room?
	}

	// check which smell to approach given the winning attention id
	int caIndex = theAgentManager.
		GetSmellIdFromCategoryId(c.Motor()->GetCurrentAttentionId());

	int mapDirection = GO_NOWHERE;
	map.WhichDirectionToFollowCA(creatureFootRoomId, caIndex, mapDirection, false);		// false means retreat

	NavigateDirection(c, mapDirection, caIndex);
}


void CreatureHandlers::NavigateDirection(Creature& c, int mapDirection, int caIndex)
{
	if (mapDirection==GO_WALK_LEFT || mapDirection==GO_WALK_RIGHT) 
	{
		// Set to walk left or right:
		Vector2D pos = c.GetDownFootPosition();
		pos.x += (mapDirection==GO_WALK_LEFT) ? -500.0f : +500.0f;
		c.SetItPosition(pos);
		c.SetIntrospective(false);
	} 
	else 
	{
		//Otherwise if we're there already keep walking anyway (introspectively)
		//in the hope of getting to the linked door.
		if (c.GetDirection()==NORTH || c.GetDirection()==SOUTH)
		{
			// (but if we're walking into the screen try to walk west or east)
			c.SetDirection(RndFloat()>0.5f ? WEST : EAST);
		}

		// Stimulate the creature that we want to go a particular direction:
		AgentHandle onC(c);
		c.Sensory()->Stimulate(onC, STIM_GO_NOWHERE + mapDirection, -1);
	}
}





void CreatureHandlers::Command_LTCY( CAOSMachine& vm )
{
	int x = vm.FetchIntegerRV();// action# 0-7
	int y = vm.FetchIntegerRV();// min
	int l = vm.FetchIntegerRV();	// max

	if (x<0 || x>=NUMINVOL) {
		vm.ThrowRunError( CAOSMachine::sidNoSuchInvoluntaryActionNo);
	}
	vm.GetCreatureTarg().Motor()->SetLatencyOfInvoluntaryAction(x, y<l ? Rnd(y,l) : Rnd(l,y));
}

void CreatureHandlers::Command_DREA( CAOSMachine& vm )
{
	int i = vm.FetchIntegerRV();
	vm.GetCreatureTarg().Life()->SetWhetherDreaming(i==0?false:true);
}
int CreatureHandlers::IntegerRV_DREA( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().Life()->GetWhetherDreaming();
}


void CreatureHandlers::Command_MATE( CAOSMachine& vm )
{
	vm.GetCreatureTarg().Reproductive()->DonateSperm();
}

void CreatureHandlers::Command_SAYN( CAOSMachine& vm )
{
	vm.GetCreatureTarg().Linguistic()->ExpressNeed();
}

int CreatureHandlers::IntegerRV_BVAR( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().Life()->GetVariant();
}

int CreatureHandlers::IntegerRV_EXPR( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().GetFacialExpression();
}

int CreatureHandlers::IntegerRV_INS(  CAOSMachine& vm )
{
	return vm.GetCreatureTarg().GetBrain()->GetNoOfInstinctsLeftToProcess();
}



void CreatureHandlers::Command_ZOMB( CAOSMachine& vm ) {
	int i = vm.FetchIntegerRV();
	vm.GetCreatureTarg().Life()->SetWhetherZombie(i>0);
}
int CreatureHandlers::IntegerRV_ZOMB( CAOSMachine& vm ) {
	return vm.GetCreatureTarg().Life()->GetWhetherZombie() ? 1 : 0;
}


void CreatureHandlers::Command_ASLP( CAOSMachine& vm ) {
	int i = vm.FetchIntegerRV();
	vm.GetCreatureTarg().Life()->SetWhetherAsleep(i>0);
}
int CreatureHandlers::IntegerRV_ASLP( CAOSMachine& vm ) {
	return vm.GetCreatureTarg().Life()->GetWhetherAsleep() ? 1 : 0;
}


void CreatureHandlers::Command_UNCS( CAOSMachine& vm ) {
	int i = vm.FetchIntegerRV();
	vm.GetCreatureTarg().Life()->SetWhetherUnconscious(i>0);
}

int CreatureHandlers::IntegerRV_UNCS( CAOSMachine& vm ) {
	return vm.GetCreatureTarg().Life()->GetWhetherUnconscious() ? 1 : 0;
}



int CreatureHandlers::IntegerRV_ATTN( CAOSMachine& vm ) {
	return vm.GetCreatureTarg().Motor()->GetCurrentAttentionId();
}

int CreatureHandlers::IntegerRV_DECN( CAOSMachine& vm ) {
	return vm.GetCreatureTarg().Motor()->GetCurrentDecisionId();
}



void CreatureHandlers::Command_STIM( CAOSMachine& vm )
{
	Stimulus s;

	Classifier scriptClassifier;
	vm.GetScript()->GetClassifier(scriptClassifier);
	s.fromScriptEventNo = scriptClassifier.Event();
#ifdef STIM_TEST_TRACE
	OutputFormattedDebugString("STIM classifier %d %d %d %d\n", scriptClassifier.Family(), scriptClassifier.Genus(), scriptClassifier.Species(), scriptClassifier.Event());
#endif

	s.stimulusType = (Stimulus::StimulusType)vm.FetchOp();

	if (s.stimulusType==WRIT) {
		s.toCreature = vm.FetchAgentRV();	// returns NULL if not a creature
	} else {
		vm.ValidateOwner();
	}

	s.predefinedStimulusNo = vm.FetchIntegerRV();

	float strength = vm.FetchFloatRV();
	s.strengthMultiplier = strength==0.0f ? 1.0f : strength;
	s.forceNoLearning = (strength == 0.0f);

	s.fromAgent = vm.GetOwner();
	s.Process();
}

void CreatureHandlers::Command_URGE( CAOSMachine& vm )
{
	Stimulus s;

	Classifier scriptClassifier;
	vm.GetScript()->GetClassifier(scriptClassifier);
	s.fromScriptEventNo = scriptClassifier.Event();

	s.stimulusType = (Stimulus::StimulusType)vm.FetchOp();

	if (s.stimulusType==WRIT) {
		s.toCreature = vm.FetchAgentRV();	// returns NULL if not a creature
		s.nounIdToStim = vm.FetchIntegerRV();
	} else {
		vm.ValidateOwner();
	}

	s.nounStim = vm.FetchFloatRV();
	s.verbIdToStim = vm.FetchIntegerRV();
	s.verbStim = vm.FetchFloatRV();

	s.fromAgent = vm.GetTarg();
	s.Process();
}


void CreatureHandlers::Command_SWAY( CAOSMachine& vm )
{
	Stimulus s;

	Classifier scriptClassifier;
	vm.GetScript()->GetClassifier(scriptClassifier);
	s.fromScriptEventNo = scriptClassifier.Event();

	s.stimulusType = (Stimulus::StimulusType)vm.FetchOp();

	if (s.stimulusType==WRIT) {
		s.toCreature = vm.FetchAgentRV();	// returns NULL if not a creature
	} else {
		vm.ValidateOwner();
	}

	for (int i=0; i<4; i++) {
		s.chemicalsToAdjust[i] = s.StimChemToBioChem(vm.FetchIntegerRV());
		s.adjustments[i]	= vm.FetchFloatRV();
	}

	s.fromAgent = vm.GetOwner();
	s.Process();
}


void CreatureHandlers::Command_ORDR( CAOSMachine& vm )
{
	Stimulus s;

	Classifier scriptClassifier;
	vm.GetScript()->GetClassifier(scriptClassifier);
	s.fromScriptEventNo = scriptClassifier.Event();

	s.stimulusType = (Stimulus::StimulusType)vm.FetchOp();

	if (s.stimulusType==WRIT) {
		s.toCreature = vm.FetchAgentRV();	// returns NULL if not a creature
	}

	vm.FetchStringRV(s.incomingSentence);

	s.fromAgent = vm.GetTarg();
	s.Process();
}


void CreatureHandlers::Command_NORN( CAOSMachine& vm )
{
	AgentHandle creature = vm.FetchAgentRV();
	if (creature.IsValid()) 
	{
		if (creature.IsCreature())
			theApp.GetWorld().SetSelectedCreature(creature);
		else
			vm.ThrowRunError( CAOSMachine::sidNotACreature );
	}
	else 
	{
		theApp.GetWorld().SetSelectedCreature(NULLHANDLE);
	}
}

void CreatureHandlers::Command_VOCB( CAOSMachine& vm )
{
	vm.GetCreatureTarg().Linguistic()->LearnVocab();
}

void CreatureHandlers::Command_TOUC( CAOSMachine& vm )
{
	if (!vm.IsBlocking()) {		// we want to keep reaching every tick
		vm.Block();
		// make the attempt to reach out to IT:
		int reachReturnValue = vm.GetCreatureTarg().ReachOut();
		if (reachReturnValue!=0) {	// not still moving (either failed or finished)
			vm.UnBlock();
			return;
		}
	}
	if (vm.GetCreatureTarg().HasTargetPoseStringBeenReached())
		vm.UnBlock();
}


void CreatureHandlers::Command_FACE(CAOSMachine& vm )
{
	int setNumber = vm.FetchIntegerRV();

	vm.GetCreatureTarg().SetFacialExpression(setNumber);

}

void CreatureHandlers::Command_WEAR( CAOSMachine& vm )
{
	int bodyid = vm.FetchIntegerRV();
	int setNumber = vm.FetchIntegerRV();
	int layer = vm.FetchIntegerRV();
	
	vm.GetCreatureTarg().WearItem(bodyid,setNumber,layer);

}

void CreatureHandlers::Command_BODY( CAOSMachine& vm )
{
	int setNumber = vm.FetchIntegerRV();
	int layer = vm.FetchIntegerRV();
	vm.GetCreatureTarg().WearOutfit(setNumber,layer);
}

void CreatureHandlers::Command_NUDE( CAOSMachine& vm )
{
	vm.GetCreatureTarg().WearOutfit(-1,-1);
}

int CreatureHandlers::IntegerRV_BYIT( CAOSMachine& vm )
{
	return (vm.GetCreatureTarg().CanReachIt()) ? 1 : 0;
}

int CreatureHandlers::IntegerRV_FACE( CAOSMachine& vm )
{
	int index = vm.GetCreatureTarg().GetOverlayIndex(REGION_HEAD);

	// for the head work out which set it is and
	// then return the facing forward sprite
		index = index / 16;

	return (index * 16) + 9;
}

float CreatureHandlers::FloatRV_DRIV( CAOSMachine& vm )
{
	float driveLevel = vm.GetCreatureTarg().GetDriveLevel(vm.FetchIntegerRV());
	if (driveLevel<=-1.0f) {
		vm.ThrowRunError(CAOSMachine::sidNoSuchDriveNo);
	}
	return driveLevel;
}

int CreatureHandlers::IntegerRV_DRV( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().GetHighestDrive();
}

int CreatureHandlers::IntegerRV_BODY( CAOSMachine& vm )
{
	int bodypart = vm.FetchIntegerRV();
	int index = vm.GetCreatureTarg().GetClothingSet(bodypart,-1);

	return index;
}

void CreatureHandlers::StringRV_FACE( CAOSMachine& vm, std::string &str)
{
//	int ageset = vm.FetchIntegerRV();
	// TO DO: use the ageset when SKELETON.CPP is free
	vm.GetCreatureTarg().GetBodyPartFileName(0,str);
}

void CreatureHandlers::Command_LOCI( CAOSMachine& vm )
{
	int lociType = vm.FetchIntegerRV();
	int lociOrgan = vm.FetchIntegerRV();
	int lociTissue = vm.FetchIntegerRV();
	int lociId = vm.FetchIntegerRV();

	float* locus = vm.GetCreatureTarg().GetLocusAddress(
		lociType, lociOrgan, lociTissue, lociId
	);
	float l = vm.FetchFloatRV();
	if (locus==NULL) {
		vm.ThrowRunError(CAOSMachine::sidLocusNotValid);
	}
	*locus = l;
}



float CreatureHandlers::FloatRV_LOCI( CAOSMachine& vm )
{
	int lociType = vm.FetchIntegerRV();
	int lociOrgan = vm.FetchIntegerRV();
	int lociTissue = vm.FetchIntegerRV();
	int lociId = vm.FetchIntegerRV();

	float* locus = vm.GetCreatureTarg().GetLocusAddress(
		lociType, lociOrgan, lociTissue, lociId
	);
	if (locus==NULL) {
		vm.ThrowRunError(CAOSMachine::sidLocusNotValid);
	}
	return *locus;
}

void CreatureHandlers::Command_BRN( CAOSMachine& vm )
{
	const int subcount=9;
	static CommandHandler HandlerTable[ subcount ] =
	{
		SubCommand_BRN_SETN,
		SubCommand_BRN_SETD,
		SubCommand_BRN_SETL,
		SubCommand_BRN_SETT,
		SubCommand_BRN_DMPB,
		SubCommand_BRN_DMPL,
		SubCommand_BRN_DMPT,
		SubCommand_BRN_DMPN,
		SubCommand_BRN_DMPD
	};
	int subcmd;

	subcmd = vm.FetchOp();
	(HandlerTable[ subcmd ])( vm );
}


void CreatureHandlers::SubCommand_BRN_DMPB( CAOSMachine& vm )
{
	vm.GetCreatureTarg().GetBrain()->DumpSpec(*vm.GetOutStream());
}


void CreatureHandlers::SubCommand_BRN_SETN( CAOSMachine& vm )
{
	int lobe = vm.FetchIntegerRV();
	int neuron = vm.FetchIntegerRV();
	int state = vm.FetchIntegerRV();
	float value = vm.FetchFloatRV();
	if(!vm.GetCreatureTarg().GetBrain()->SetNeuronState(lobe, neuron, state, value))
		vm.ThrowRunError(CAOSMachine::sidCouldNotSetNeuron);
}


void CreatureHandlers::SubCommand_BRN_SETD( CAOSMachine& vm )
{
	int tract = vm.FetchIntegerRV();
	int dendrite = vm.FetchIntegerRV();
	int weight = vm.FetchIntegerRV();
	float value = vm.FetchFloatRV();
	if(!vm.GetCreatureTarg().GetBrain()->SetDendriteWeight(tract, dendrite, weight, value))
		vm.ThrowRunError(CAOSMachine::sidCouldNotSetDendrite);
}


void CreatureHandlers::SubCommand_BRN_SETL( CAOSMachine& vm )
{
	int lobe = vm.FetchIntegerRV();
	int entryNo = vm.FetchIntegerRV();
	float value = vm.FetchFloatRV();
	if(!vm.GetCreatureTarg().GetBrain()->SetLobeSVFloat(lobe, entryNo, value))
		vm.ThrowRunError(CAOSMachine::sidCouldNotSetLobe);
}


void CreatureHandlers::SubCommand_BRN_SETT( CAOSMachine& vm )
{
	int tract = vm.FetchIntegerRV();
	int entryNo = vm.FetchIntegerRV();
	float value = vm.FetchFloatRV();
	if(!vm.GetCreatureTarg().GetBrain()->SetTractSVFloat(tract, entryNo, value))
		vm.ThrowRunError(CAOSMachine::sidCouldNotSetTract);
}


void CreatureHandlers::SubCommand_BRN_DMPL( CAOSMachine& vm )
{
	if(!vm.GetCreatureTarg().GetBrain()->DumpLobe(vm.FetchIntegerRV(), *vm.GetOutStream()))
		vm.ThrowRunError(CAOSMachine::sidCouldNotDumpLobe);
}


void CreatureHandlers::SubCommand_BRN_DMPT( CAOSMachine& vm )
{
	if(!vm.GetCreatureTarg().GetBrain()->DumpTract(vm.FetchIntegerRV(), *vm.GetOutStream()))
		vm.ThrowRunError(CAOSMachine::sidCouldNotDumpTract);
}


void CreatureHandlers::SubCommand_BRN_DMPN( CAOSMachine& vm )
{
	int lobe = vm.FetchIntegerRV();
	int neuron = vm.FetchIntegerRV();

	if(!vm.GetCreatureTarg().GetBrain()->DumpNeuron(lobe, neuron, *vm.GetOutStream()))
		vm.ThrowRunError(CAOSMachine::sidCouldNotDumpNeurone);
}


void CreatureHandlers::SubCommand_BRN_DMPD( CAOSMachine& vm )
{
	int tract = vm.FetchIntegerRV();
	int dendrite = vm.FetchIntegerRV();
	if(!vm.GetCreatureTarg().GetBrain()->DumpDendrite(tract, dendrite, *vm.GetOutStream()))
		vm.ThrowRunError(CAOSMachine::sidCouldNotDumpDendrite);
}



void CreatureHandlers::Command_AGES( CAOSMachine& vm)
{
	int times = vm.FetchIntegerRV();
	for(int i=0; i < times; i++)
		vm.GetCreatureTarg().Life()->ForceAgeing();
}



int CreatureHandlers::IntegerRV_CREA( CAOSMachine& vm ) 
{
	return vm.FetchAgentRV().IsCreature() ? 1 : 0;
}




int CreatureHandlers::IntegerRV_CAGE( CAOSMachine& vm ) 
{
	return vm.GetCreatureTarg().Life()->GetAge();
}
   

void CreatureHandlers::StringRV_GTOS( CAOSMachine& vm, std::string &str)
{
	vm.ValidateTarg();
	int i = vm.FetchIntegerRV();
	str = vm.GetTarg().GetAgentReference().GetGenomeStore().MonikerAsString(i);
}



float CreatureHandlers::FloatRV_DFTX( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().GetDownFootPosition().x;
}


float CreatureHandlers::FloatRV_DFTY( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().GetDownFootPosition().y;
}


float CreatureHandlers::FloatRV_UFTX( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().GetUpFootPosition().x;
}


float CreatureHandlers::FloatRV_UFTY( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().GetUpFootPosition().y;
}



void CreatureHandlers::Command_HAIR( CAOSMachine& vm)
{
	vm.GetCreatureTarg().ChangeHairStateInSomeWay(vm.FetchIntegerRV());
}


int CreatureHandlers::IntegerRV_CATI(CAOSMachine& vm)
{
	int f = vm.FetchIntegerRV();
	int g = vm.FetchIntegerRV();
	int s = vm.FetchIntegerRV();
	Classifier c(f, g, s);

	return SensoryFaculty::GetCategoryIdOfClassifier(&c);
}

void CreatureHandlers::StringRV_CATX(CAOSMachine& vm, std::string& str )
{
	int id = vm.FetchIntegerRV();
	str = SensoryFaculty::GetCategoryName(id);
}

AgentHandle CreatureHandlers::AgentRV_IITT( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().GetItAgent();
}

void CreatureHandlers::Command_GENE( CAOSMachine& vm )
{
	const int subcount=5;
	static CommandHandler HandlerTable[ subcount ] =
	{
		SubCommand_GENE_CROS,
		SubCommand_GENE_MOVE,
		SubCommand_GENE_KILL,
		SubCommand_GENE_LOAD,
		SubCommand_GENE_CLON,
	};
	int subcmd;

	subcmd = vm.FetchOp();
	(HandlerTable[ subcmd ])( vm );
}

void CreatureHandlers::SubCommand_GENE_CROS( CAOSMachine& vm )
{
	AgentHandle child = vm.FetchAgentRV();
	int child_index = vm.FetchIntegerRV();

	AgentHandle mum = vm.FetchAgentRV();
	int mum_index = vm.FetchIntegerRV();

	AgentHandle dad = vm.FetchAgentRV();
	int dad_index = vm.FetchIntegerRV();

	int mumChance = vm.FetchIntegerRV();
	int mumDegree = vm.FetchIntegerRV();
	int dadChance = vm.FetchIntegerRV();
	int dadDegree = vm.FetchIntegerRV();
	
	if (child_index < 1)
		vm.ThrowRunError( CAOSMachine::sidInvalidGeneVariable );	

	if(mumChance < 0 || mumChance > 255 || mumDegree < 0 || mumDegree > 255 ||
		dadChance < 0 || dadChance > 255 || dadDegree < 0 || dadDegree > 255)
		vm.ThrowRunError( CAOSMachine::sidMutationParamaterOutOfRange );
	else
	{
		if (!child.GetAgentReference().GetGenomeStore().CrossoverFrom(child_index,
			mum.GetAgentReference().GetGenomeStore(), mum_index,
			dad.GetAgentReference().GetGenomeStore(), dad_index,
			mumChance, mumDegree,
			dadChance, dadDegree,
			false))
		{
			vm.ThrowRunError( CAOSMachine::sidInvalidGeneVariable );	
		}

	}

	// note: requires exception handling if concieve fails cos cant create mum or dad
	//		genomes
}

void CreatureHandlers::SubCommand_GENE_MOVE( CAOSMachine& vm )
{
	AgentHandle dest = vm.FetchAgentRV();
	int dest_index = vm.FetchIntegerRV();

	AgentHandle source = vm.FetchAgentRV();
	int source_index = vm.FetchIntegerRV();

	if (dest_index < 1 || source_index < 1)
		vm.ThrowRunError( CAOSMachine::sidInvalidGeneVariable );	

	if (!dest.GetAgentReference().GetGenomeStore().MoveSlotFrom(dest_index,
			source.GetAgentReference().GetGenomeStore(), source_index))
	{
		vm.ThrowRunError( CAOSMachine::sidInvalidGeneVariable );	
	}
}

void CreatureHandlers::SubCommand_GENE_KILL( CAOSMachine& vm )
{
	AgentHandle kill = vm.FetchAgentRV();
	int kill_index = vm.FetchIntegerRV();

	if (kill_index < 1)
		vm.ThrowRunError( CAOSMachine::sidInvalidGeneVariable );	

	kill.GetAgentReference().GetGenomeStore().ClearSlot(kill_index);
}

void CreatureHandlers::SubCommand_GENE_LOAD( CAOSMachine& vm )
{
	AgentHandle load = vm.FetchAgentRV();
	int load_index = vm.FetchIntegerRV();
	std::string gene_file;
	vm.FetchStringRV(gene_file);

	if (load_index < 1)
		vm.ThrowRunError( CAOSMachine::sidInvalidGeneVariable );	

	if (!load.GetAgentReference().GetGenomeStore().LoadEngineeredFile(load_index, gene_file))
		vm.ThrowRunError( CAOSMachine::sidGeneLoadEngineeredFileError );	
}

void CreatureHandlers::SubCommand_GENE_CLON( CAOSMachine& vm )
{
	AgentHandle dest = vm.FetchAgentRV();
	int dest_index = vm.FetchIntegerRV();

	AgentHandle source = vm.FetchAgentRV();
	int source_index = vm.FetchIntegerRV();

	if (dest_index < 1 || source_index < 0)
		vm.ThrowRunError( CAOSMachine::sidInvalidGeneVariable );	

	if (!dest.GetAgentReference().GetGenomeStore().CloneSlot(dest_index,
			source.GetAgentReference().GetGenomeStore().MonikerAsString(source_index)))
	{
		vm.ThrowRunError( CAOSMachine::sidInvalidGeneVariable );	
	}
}

void CreatureHandlers::Command_BORN( CAOSMachine& vm )
{
	if (vm.GetCreatureTarg().Life()->GetProperlyBorn())
		vm.ThrowRunError( CAOSMachine::sidBornAgainNorn );	

	vm.GetCreatureTarg().Life()->SetProperlyBorn();
}

int CreatureHandlers::IntegerRV_TAGE( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().Life()->GetTickAge();
}

// command to stop holding hands (no holding hands)

AgentHandle CreatureHandlers::AgentRV_HHLD( CAOSMachine& vm )
{
	return thePointer.GetPointerAgentReference().GetHandHeldCreature();
}

void CreatureHandlers::Command_NOHH( CAOSMachine& vm)
{
	if (vm.GetTarg().IsInvalid())
		return;						// Silently ignore bad targs on nohh
	vm.ValidateTarg();
	vm.GetCreatureTarg().StopHoldingHandsWithThePointer
		(vm.GetOwner(), INTEGERZERO, INTEGERZERO);
}

AgentHandle CreatureHandlers::AgentRV_MTOC( CAOSMachine& vm )
{
	std::string moniker;
	vm.FetchStringRV(moniker);
	
	return theAgentManager.FindCreatureAgentForMoniker(moniker);
}


AgentHandle CreatureHandlers::AgentRV_MTOA( CAOSMachine& vm )
{
	std::string moniker;
	vm.FetchStringRV(moniker);
	
	return theAgentManager.FindAgentForMoniker(moniker);
}

int CreatureHandlers::IntegerRV_ORGN( CAOSMachine& vm )
{
	return vm.GetCreatureTarg().GetBiochemistry()->GetOrganCount();
}

int CreatureHandlers::IntegerRV_ORGI( CAOSMachine& vm )
{
	int organNumber = vm.FetchIntegerRV();
	int countNumber = vm.FetchIntegerRV();

	Organ* organ = vm.GetCreatureTarg().GetBiochemistry()->GetOrgan(organNumber);
	if (!organ)
		return -1;

	if (countNumber == 0)
		return organ->ReceptorCount();
	else if (countNumber == 1)
		return organ->EmitterCount();
	else if (countNumber == 2)
		return organ->ReactionCount();

	return -1;
}


float CreatureHandlers::FloatRV_ORGF( CAOSMachine& vm )
{
	int organNumber = vm.FetchIntegerRV();
	int countNumber = vm.FetchIntegerRV();

	Organ* organ = vm.GetCreatureTarg().GetBiochemistry()->GetOrgan(organNumber);
	if (!organ)
		return -1;

	if (countNumber == 0)
		return organ->LocusClockRate();
	else if (countNumber == 1)
		return organ->LocusLifeForce();
	else if (countNumber == 2)
		return organ->LocusLongTermRateOfRepair();
	else if (countNumber == 3)
		return organ->LocusInjuryToApply();		
	else if (countNumber == 4)
		return organ->InitialLifeForce();
	else if (countNumber == 5)
		return organ->ShortTermLifeForce();
	else if (countNumber == 6)
		return organ->LongTermLifeForce();
	else if (countNumber == 7)
		return organ->LongTermRateOfRepair();
	else if (countNumber == 8)
		return organ->EnergyCost();
	else if (countNumber == 9)
		return organ->DamageDueToZeroEnergy();

	return -1;
}


void CreatureHandlers::Command_FORF( CAOSMachine& vm )
{
	Creature& c = vm.GetCreatureTarg();
	AgentHandle from = vm.FetchAgentRV();
	if(from.IsCreature() || from.IsPointerAgent()) 
		c.Sensory()->SetCreatureActingUponMe(from);
}



void CreatureHandlers::Command_LIKE( CAOSMachine& vm )
{
	Creature& c = vm.GetCreatureTarg();
	AgentHandle from = vm.FetchAgentRV();
	if(from.IsCreature() || from.IsPointerAgent()) 
		c.Linguistic()->ExpressOpinion(from);
}
