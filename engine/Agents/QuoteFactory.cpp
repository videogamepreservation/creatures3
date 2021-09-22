#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "QuoteFactory.h"
#include "../Maths.h"

#include "../Display/DisplayEngine.h"

const char* dictionary = "acbdefghi jklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!\"£$%^&*()-=_+,./<>?;'#:@~[]{}`¬\\|\n";
const char*    encdict = "JKLMacb?;'#:@~[defpqrstuvwxyzABC3456=_+,.]{}`¬\\|DEFG HIghijklmnoNOPQR/<>STUVW789!\"£$%^&*()-XYZ012¦";

const std::string unc = dictionary;
const std::string enc = encdict;

inline void encode(std::string& s)
{
	for(int i=0;i<s.length();i++)
		s[i] = enc[ unc.find( s[i] ) ];
}

inline void decode(std::string& s)
{
	for(int i=0;i<s.length();i++)
		s[i] = unc[ enc.find( s[i] ) ];
}

#include <map>

std::multimap<std::string,std::string> quoteMap;

#include "QuoteFactoryHelper.h"

int tempDummy = fillMap();

void ConvertDullText(std::string& thisString)
{
	if (thisString.length() == 0)
		return;
	// First off, is it a "Quoted" string?
	if (thisString.at(0) != '"' || thisString.at(thisString.length()-1) != '"')
		return;

	// Woohoo, extract the desired username from it..

	std::string thisUser = thisString.substr(1,thisString.length()-2);

	std::multimap<std::string,std::string>::iterator first,last;

	if (thisUser == "\"")
	{
		// We need to select a random user...
		first = quoteMap.begin();
		last = quoteMap.end();
	}
	else
	{
		for (int i = 0; i < thisUser.size(); ++i)
			thisUser[i] = tolower(thisUser[i]);
		encode(thisUser);

		first = quoteMap.lower_bound(thisUser);
		last  = quoteMap.upper_bound(thisUser);
	}
	if (first == last)
		return;
	int num = 0;
	while (first != last)
	{
		first++;
		num++;
	}
	first = quoteMap.lower_bound(thisUser);

	num = Rnd(0,num-1);
	for(int i=0;i<num;i++)
		first++;

	if (thisUser != "\"")
		thisString = (*first).second;
	else
		thisString = (*first).first;
	decode(thisString);

	if (thisString == "***")
	{
		// Make the world flat once more...
		DisplayEngine::theRenderer().SetDesiredRoundness(false);
		thisString = "It isn't!";
	}
	if (thisString == "###")
	{
		// Make the world round instead...
		DisplayEngine::theRenderer().SetDesiredRoundness(true);
		thisString = "It isn't!";
	}
}