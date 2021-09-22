// Faculty.h: interface for the Faculty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef Faculty_H
#define Faculty_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Biochemistry/ChemicallyActive.h"
#include "Biochemistry/BiochemistryConstants.h"
#include "../PersistentObject.h"

#include "../Agents/AgentHandle.h"

class Genome;

//class Faculty : public ChemicallyActive, public PersistentObject {
class Faculty : public PersistentObject, public ChemicallyActive  {
	CREATURES_DECLARE_SERIAL(Faculty)
public:
	Faculty();
	virtual ~Faculty();

	virtual void Init(AgentHandle c);
	virtual void Update();
	virtual void ReadFromGenome(Genome& g);
	virtual float* GetLocusAddress(int type, int organ, int tissue, int locus);

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	AgentHandle GetCreatureOwner() {
		return myCreature;
	}

	virtual void Trash()
	{
		myCreature = NULLHANDLE;
	}
protected:
	AgentHandle myCreature;
};
 
#endif//Faculty_H
