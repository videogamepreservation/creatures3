#ifndef AUTO_DOCUMENTATION_TABLE_H
#define AUTO_DOCUMENTATION_TABLE_H

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#include "TableSpec.h"

class AutoDocumentationTable
{
	typedef std::vector< std::pair<TableSpec *, int> > TableSpecTable;

public:
	static int RegisterTable(TableSpec *table, int sizeInBytes);

	static void SortTablesByTitle();

	static void StreamTitleLinksAsHTML(std::ostream& out);
	static void StreamAllTablesAsHTML(std::ostream& out);
	static void StreamTableAsHTML(std::ostream& out, TableSpec *table, int size);

	static TableSpecTable& GetOurTables();

protected:
	static bool SortComparison(std::pair<TableSpec *, int> a, std::pair<TableSpec *, int> b);
};


#endif // AUTO_DOCUMENTATION_TABLE_H