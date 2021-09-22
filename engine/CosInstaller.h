// --------------------------------------------------------------------------
// Filename:	CosInstaller.h
// Class:		CosInstaller
// Purpose:		To install scripts into the scriptorium
//				To create new objects from a cos file
//			
//				
//
// Description: The cosinstaller can still load cos scripts for the bootstrap
//				directory but Note that the new world switcher style requires
//				one folder per product or update to appear in the bootstrap 
//				directory.  The products must be named alphabetically 
//				corresponding to their release dates.  Eg 0100 C3, 0200 C3 Life Kit
//			
//
// History:
// -------  
// 16Mar98	Alima		Created
// --------------------------------------------------------------------------
#ifndef COS_INSTALLER_H
#define COS_INSTALLER_H
#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

// --------------------------------------------------------------------------
// Filename:	CosInstaller.h
// Class:		CosInstaller
// Purpose:		This class loads installation and event scripts.
//			
//				
//
//			
//
// History:
// -------  Alima		created
// 26Jun99	Alima		Altered to work with the world switcher directory structure.
//						  The cos installer now needs to load all scripts
//						for each new product detected.
// --------------------------------------------------------------------------

#include <string>
#include <vector>
#include <iostream>
#include "Classifier.h"

class CosInstaller
{
public:

	CosInstaller();
	CosInstaller(std::string& script);
	CosInstaller(std::vector<std::string>& bootStrapFoldersToLoad);

	~CosInstaller();

	void LoadCosFiles(const std::string& bootstrap_dir,bool showProgress= false,
		bool updateScriptoriumOnly =false );


	void SetUpProgressBar(std::vector<std::string>& bootstrapFoldersToLoad);

	bool AddScript(Classifier classifier);
	bool ReadScriptFile(std::string const& filename, bool updateScriptoriumOnly =false);
	bool ReadScriptStream(std::istream& in, bool updateScriptoriumOnly = false);
	bool ExecuteScripts();

	std::string myTextBuffer;
	std::vector<std::string> myInstallScripts;
	std::vector<std::string> myExecuteScripts;

	std::string myCurrentFileForErrorMessages;
};
#endif //COS_INSTALLER_H

