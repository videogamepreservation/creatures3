#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#include "AutoDocumentationTable.h"
#include "../../common/C2eDebug.h"
#include "CAOSDescription.h"

#ifdef C2E_OLD_CPP_LIB
// not too sure what is ansi.
#include <iostream>
#else
#include <ostream>	// ok under visual c
#endif

#include <algorithm>

// std::vector< std::pair<TableSpec *, int> > AutoDocumentationTable::GetOurTables();

int AutoDocumentationTable::RegisterTable(TableSpec *table, int sizeInBytes)
{
	int size = sizeInBytes / sizeof(TableSpec);
//	ASSERT(size > 2); // first is heading, second titles for columns, others are rows
	if (size <= 2)
		return 0;

	GetOurTables().push_back(std::make_pair(table, size));
	int tableSize = GetOurTables().size(); // debugging

	return 0;
}

// lexicographical compare of table titles
bool AutoDocumentationTable::SortComparison(std::pair<TableSpec *, int> a, std::pair<TableSpec *, int> b)
{
	// first row is heading, and must have only one entry
	ASSERT(a.first->entries.size() == 1);
	ASSERT(b.first->entries.size() == 1);

	return a.first->entries[0] < b.first->entries[0];
}

void AutoDocumentationTable::SortTablesByTitle()
{
	std::sort(GetOurTables().begin(), GetOurTables().end(), SortComparison);
}

void AutoDocumentationTable::StreamTitleLinksAsHTML(std::ostream& out)
{
	SortTablesByTitle();

	for (int i = 0; i < GetOurTables().size(); ++i)
	{
		std::string title = GetOurTables()[i].first->entries[0];
		out << "<a href=\"#" << title << "\">" << title << "</a> " << std::endl;
	}
	out << "<p>";
}

void AutoDocumentationTable::StreamAllTablesAsHTML(std::ostream& out)
{
	SortTablesByTitle();

	for (int i = 0; i < GetOurTables().size(); ++i)
	{
		StreamTableAsHTML(out, GetOurTables()[i].first, GetOurTables()[i].second);
	}
}

void AutoDocumentationTable::StreamTableAsHTML(std::ostream& out, TableSpec *table, int size)
{
	out << "<p>";

	std::string title = table->entries[0];

	out << "<p><hr width=\"90%\" align=\"center\"><div align=\"center\"><h2><a name=\"" << title << "\">" << title << "</a></h2></div>" << std::endl;
	std::string tableStartString = "<table align=center border=1 cellpadding=3 cellspacing=0 width=\"99%\">";
	out << tableStartString << std::endl;
	for (int rowNumber = 1; rowNumber < size; ++rowNumber)
	{
		int rowNumberDo = rowNumber;
		// check for case of redoing the heading
		if ((table + rowNumberDo)->entries.size() == 0)
		{
			rowNumberDo = 1; 
			// out << "</table><p>" << tableStartString << std::endl;
		}

		out << "<tr>";
		TableSpec* tableSpec = table + rowNumberDo;
		int rowSize = tableSpec->entries.size();
		for (int entryNumber = 0; entryNumber < rowSize; ++entryNumber)
		{
			std::string styleClass = (rowNumberDo == 1) ? "command" : "description";
			out << "<td class=\"" + styleClass + "\"><span class=\"description\">";
			std::string description = tableSpec->entries[entryNumber];
			CAOSDescription::HyperlinkAndAnchorise(description);
			out << description;
			out << "</span></td>" << std::endl;
		}
		out << "</tr>" << std::endl;
	}
	out << "</table>";
}

AutoDocumentationTable::TableSpecTable& AutoDocumentationTable::GetOurTables()
{
	static TableSpecTable ourmyTables;
	return ourmyTables;
}
