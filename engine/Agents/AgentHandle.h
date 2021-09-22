#ifndef AGENT_HANDLE_H
#define AGENT_HANDLE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../common/BasicException.h"

// Forward references
class Agent;
class SimpleAgent;
class PointerAgent;
class CompoundAgent;
class Vehicle;
class Skeleton;
class Creature;
class CreaturesArchive;

class AgentHandle {
public:	

	enum {
		agentNormal = 1,
		agentSimple = 2,
		agentPointer = 4,
		agentCompound = 8,
		agentVehicle = 16,
		agentSkeleton = 32,
		agentCreature = 64
	};
	// Default constructor
	AgentHandle();
	// Constructor
	AgentHandle(Agent* agent);

	// Constructor added by Gavin
	AgentHandle(Agent& agent);

	// Copy constructor
	AgentHandle(const AgentHandle& handle);
	
	// Destructor
	~AgentHandle();


	//
	// Overloaded operators
	// 

	// Assignment
	AgentHandle& operator=(const AgentHandle& handle);

	// Equality
	bool operator==(const AgentHandle& handle) const;

	// Inequality
	bool operator!=(const AgentHandle& handle) const; 

private:
	// These functions are declared but not defined so that the
	// compiler can catch comparisons and assignments to a raw
	// agent pointer
	bool operator!=(const Agent* agent) const;
	bool operator==(const Agent* agent) const;
	AgentHandle& operator=(const Agent* agent);
public:

	//
	// Check functions
	//

	bool IsValid();
	bool IsInvalid();
	bool IsSimpleAgent();
	bool IsPointerAgent();
	bool IsCompoundAgent();
	bool IsVehicle();
	bool IsSkeleton();
	bool IsCreature();
	

	// 
	// Accessor functions
	//
	 
	Agent& GetAgentReference() const;
	SimpleAgent& GetSimpleAgentReference() const;
	PointerAgent& GetPointerAgentReference() const;
	CompoundAgent& GetCompoundAgentReference() const;
	Vehicle& GetVehicleReference() const;
	Skeleton& GetSkeletonReference() const;
	Creature& GetCreatureReference() const;
	
private:
	Agent* myAgentPointer;
	friend CreaturesArchive& operator<<( CreaturesArchive &ar, AgentHandle const &agentHandle );
	friend CreaturesArchive& operator>>( CreaturesArchive &ar, AgentHandle &agentHandle );
};

CreaturesArchive& operator<<( CreaturesArchive &ar, AgentHandle const &agentHandle );
CreaturesArchive& operator>>( CreaturesArchive &ar, AgentHandle &agentHandle );


class AgentHandleException : public BasicException
{
public:
	AgentHandleException(char* message) : BasicException(message) { }
};

// Global
extern AgentHandle NULLHANDLE;



#endif // AGENT_HANDLE_H