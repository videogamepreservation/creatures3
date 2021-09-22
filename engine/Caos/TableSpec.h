#ifndef TABLE_SPEC_H
#define TABLE_SPEC_H

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#include <vector>
#include <string>

class TableSpec
{
	public:
		TableSpec()
		{
		}
		TableSpec(const std::string& entry)
		{
			entries.push_back(entry);
		}
		TableSpec(const std::string& entry1, const std::string& entry2)
		{
			entries.push_back(entry1); entries.push_back(entry2);
		}
		TableSpec(const std::string& entry1, const std::string& entry2, const std::string& entry3)
		{
			entries.push_back(entry1); entries.push_back(entry2); entries.push_back(entry3);
		}
		TableSpec(const std::string& entry1, const std::string& entry2, const std::string& entry3, const std::string& entry4)
		{
			entries.push_back(entry1); entries.push_back(entry2); entries.push_back(entry3); entries.push_back(entry4);
		}
		TableSpec(const std::string& entry1, const std::string& entry2, const std::string& entry3, const std::string& entry4, const std::string& entry5)
		{
			entries.push_back(entry1); entries.push_back(entry2); entries.push_back(entry3); entries.push_back(entry4); entries.push_back(entry5);
		}
		TableSpec(const std::string& entry1, const std::string& entry2, const std::string& entry3, const std::string& entry4, const std::string& entry5, const std::string& entry6)
		{
			entries.push_back(entry1); entries.push_back(entry2); entries.push_back(entry3); entries.push_back(entry4); entries.push_back(entry5); entries.push_back(entry6);
		}
	
		std::vector<std::string> entries;
};

#endif // TABLE_SPEC_H
