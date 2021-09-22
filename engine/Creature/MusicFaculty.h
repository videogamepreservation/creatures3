// MusicFaculty.h: interface for the MusicFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MusicFaculty_H
#define MusicFaculty_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Faculty.h"

class MusicFaculty : public Faculty {
	CREATURES_DECLARE_SERIAL(MusicFaculty)
public:
	typedef Faculty base;
	MusicFaculty();
	virtual ~MusicFaculty();

	void Update();

    bool SelectableByUser() const;
    bool Hatching() const;
    float Mood() const;
    float Threat();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);
};

#endif//MusicFaculty_H
