#ifndef HISTORY_STORE_H
#define HISTORY_STORE_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include "CreatureHistory.h"

#include <map>
#include <string>

class CreaturesArchive;

class HistoryStore
{
public:
	CreatureHistory& GetCreatureHistory(const std::string& moniker);
	bool IsThereCreatureHistory(const std::string& moniker) const;
	void WipeCreatureHistory(const std::string& moniker);
	bool ReconcileImportedHistory(const std::string& moniker, CreatureHistory& importedHistory, bool performReconciliation);
	int GetOutOfWorldState(const std::string& moniker);

	std::string Next(const std::string& current);
	std::string Previous(const std::string& current);

	friend CreaturesArchive &operator<<( CreaturesArchive &archive, HistoryStore const &historyStore );
	friend CreaturesArchive &operator>>( CreaturesArchive &archive, HistoryStore &historyStore );

private:
	// map from moniker to creature history
	std::map<std::string, CreatureHistory> myCreatureHistories;

	// keep the null history separate - events sometimes end up here
	// (for example, when born to unknown parents), and we don't want
	// to ever find them again
	CreatureHistory myNullHistory;
};


#endif // HISTORY_STORE_H
;