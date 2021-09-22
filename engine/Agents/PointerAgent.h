#ifndef POINTERAGENT_H
#define POINTERAGENT_H

#include "SimpleAgent.h"

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

// PointerAgent class - the (single) default object attached to the mouse
// This agent can activate other objects by clicking on them


class PointerAgent : public SimpleAgent
{
	typedef SimpleAgent base;

	CREATURES_DECLARE_SERIAL( PointerAgent )

public:

	PointerAgent();							// serialisation constructor				
	PointerAgent(	int family, int genus, int species,uint32 id,
		FilePath const& gallery, 
		int numimages,
		int baseimage);

	virtual ~PointerAgent();					// destructor

	virtual void Tint(const uint16* tintTable, int part = 0);
    virtual void Trash();


	
	Vector2D GetHotSpot()
	{
		return (myPositionVector+myHotSpot);
	}
	void SetHotSpot(const Vector2D& hotspot)
	{
		myHotSpot = hotspot;
	}


	AgentHandle Find();				// Find the object
	AgentHandle FindCreature();				// Find the object

	virtual void Update();			       		// called every tick

	void CameraPositionNotify();


	


	AgentHandle GetHandHeldCreature()
	{
		return myHandHeldCreature;
	}
	void StartHoldingHandsWithCreature(AgentHandle& creature);
	void HoldHandsWithCreature();
	void StopHoldingHandsWithCreature();

	bool TreatBothButtonsAsLeftClick();
	bool TreatBothButtonsAsRightClick();

	void DefineMouseClicks(int definition);

	void SetScanInputEvents(bool bValue);
	bool GetScanInputEvents();


	void DecideIfCarriedAgentShouldBeDropped();

	void CarryAgentsFromMetaRoomToMetaRoom(bool flag){
				myCanCarryAgentsFromMetaRoomToMetaRoom = flag;}

	// serialisation
	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);



// capitalize these?
	enum
	{ 
		DoMouseClicksAsNormal =0, 
		TreatBothButtonsAsALeftClick,
		TreatBothButtonsAsARightClick
	};

	virtual int32 GetPlaneForCarriedAgent();

	std::string GetName() {return myName;};
	void SetName(std::string name);

protected:
	Vector2D myHotSpot;
	bool myConnecting;
	int8 myMouseButtonAction; 
	InputPort *myInputPort;
	OutputPort *myOutputPort;
	Port* myPort;
	AgentHandle myConnectingAgent;

	// Holding hands stuff
	bool myIsHoldingHandsWithCreature;
	AgentHandle myHandHeldCreature;


	bool myScanInputEvents;
	bool myCanCarryAgentsFromMetaRoomToMetaRoom;
	uint16 myLastPointerActionDispatched;
	

private:
	virtual void HandlePickup(Message* Msg);// special actions in response to
	virtual void HandleDrop(Message* Msg);	// pickup & drop messages

	void ScanInputEvents();
	std::string myName;
};


inline bool PointerAgent::TreatBothButtonsAsLeftClick()
{
	_ASSERT(!myGarbaged);
	return (myMouseButtonAction & TreatBothButtonsAsALeftClick) ? true: false;
}


inline bool PointerAgent::TreatBothButtonsAsRightClick()
{
	_ASSERT(!myGarbaged);
	return (myMouseButtonAction & TreatBothButtonsAsARightClick) ? true: false;
}




#endif // POINTERAGENT_H

