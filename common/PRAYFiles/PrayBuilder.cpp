#include "PrayBuilder.h"

#include "../SimpleLexer.h"

#include "StringIntGroup.h"
#include "PrayManager.h"
#include "PrayChunk.h"

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#ifdef C2E_OLD_CPP_LIB
#include <strstream>
#endif



void PrayBuilder::munchComment(int& currentTokenType, std::string& currentToken, SimpleLexer& inputLexer)
{
	bool eat;
	while (currentTokenType == SimpleLexer::typeSymbol && currentToken == "(-")
	{
		// Let's eat comments :)
		eat = true;
		while (eat)
		{
			currentTokenType = inputLexer.GetToken(currentToken);
			if (currentTokenType == SimpleLexer::typeFinished)
				eat = false;
			if (currentTokenType == SimpleLexer::typeSymbol)
				if (currentToken == "-)")
					eat = false;
		}
		// Eaten comment - ready to carry on :)
		if (currentTokenType == SimpleLexer::typeFinished)
		{
			outputStream << "Unexpected end of file processing comment :(" << std::endl;
			// TODO: yell at Daniel ;-)
			exit(1);
		}
		currentTokenType = inputLexer.GetToken(currentToken);
	}
}

#define EXPECT(a) expect(currentTokenType,currentToken,SimpleLexer::a,inputLexer.GetLineNum())
bool PrayBuilder::expect(int thisType, std::string& thisToken, int expectedType, int linenumber)
{
	if (thisType == expectedType)
		return true;
	std::string wantType, gotType;

	decodeType(thisType,gotType);
	decodeType(expectedType,wantType);
	outputStream << "Error on line " << linenumber
		<< ". Expecting " << wantType << ", got " << gotType
		<< " (" << thisToken << ")" << std::endl;
	return false;
}


void PrayBuilder::decodeType(int thisType,std::string& into)
{
	switch(thisType)
	{
	case SimpleLexer::typeFinished:
		into = "End of File";
		break;
	case SimpleLexer::typeError:
		into = "***LEXER ERROR***";
		break;
	case SimpleLexer::typeString:
		into = "Quoted String";
		break;
	case SimpleLexer::typeNumber:
		into = "Numerical Value";
		break;
	case SimpleLexer::typeSymbol:
		into = "Symbol or keyword";
		break;
	default:
		into = "Unknown type";
	}
}


#ifdef C2E_OLD_CPP_LIB
PrayBuilder::PrayBuilder(std::string& sourceFile, std::string& destFile):
	outputStream( myHackBuf, sizeof( myHackBuf ) )
#else
PrayBuilder::PrayBuilder(std::string& sourceFile, std::string& destFile)
#endif
{
	SuccessfulBuild = false;

	outputStream.clear();

	outputStream << "Embedded PRAYBuilder now processing..." << std::endl;

	std::ifstream sourceFileStream;

	try
	{
		sourceFileStream.open(sourceFile.c_str());
	}
	catch (...)
	{
		outputStream << "Oh dear, couldn't open input stream" << std::endl;
		Output = outputStream.str();
		Output.resize(outputStream.tellp());
		return;
	}

	SimpleLexer inputLexer(sourceFileStream);

	std::string currentChunkName;
	std::string currentChunkType;

	std::string currentToken;
	StringIntGroup sg;

	enum
	{
		lookingChunk,
		lookingTag
	} myState;

	int currentTokenType = inputLexer.GetToken(currentToken);

	myState = lookingChunk;

	munchComment(currentTokenType,currentToken,inputLexer);

	if (!EXPECT(typeString))
	{ 
		Output = outputStream.str();
		Output.resize(outputStream.tellp());
		return;
	}
	PrayManager pm(currentToken);

	currentTokenType = inputLexer.GetToken(currentToken);

	std::string chunkName,chunkType;

	while (currentTokenType != SimpleLexer::typeFinished)
	{
		munchComment(currentTokenType,currentToken,inputLexer);
		if (myState == lookingChunk)
		{
			// We are looking for a chunk identifier (Symbol/Token)
			if (!EXPECT(typeSymbol))
			{ 
				Output = outputStream.str();
				Output.resize(outputStream.tellp());
				return;
			}
			// Right then, let's think... If "inline" do file chunk, if "chunk" do prep for stringintgroup.

			if (currentToken == "inline")
			{
				outputStream << "Parsing inline chunk..." << std::endl;
				// Deal with inline chunk...
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);
				// Expecting a nice Symbol...
				if (!EXPECT(typeSymbol))
				{ 
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				// The symbol should be 4 chars exactly
				if (currentToken.size() != 4)
				{
					outputStream << "Expecting chunk symbol type. It was not four chars. " <<
						"PRAYBuilder aborting." << std::endl << "Offending token was " << 
						currentToken << " on line " << inputLexer.GetLineNum() << std::endl;
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				// The symbol is indeed 4 chars.
				chunkType = currentToken;
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);
				if (!EXPECT(typeString))
				{ 
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				// The string is the name of the chunk.
				chunkName = currentToken;
				outputStream << "Chunk is of type: " << chunkType <<
					" and is called " << chunkName << std::endl;
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);
				if (!EXPECT(typeString))
				{ 
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				// The string is the name of the file.
				outputStream << "The data for the inline chunk comes from " << currentToken << std::endl;
				FILE* inlineFile;
				inlineFile = fopen(currentToken.c_str(),"rb");
				if (inlineFile == NULL)
				{
					outputStream << "Oh dear, couldn't open the file :(" << std::endl;
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				// File opened, let's munch in the data...
				fseek(inlineFile,0,SEEK_END);
				int bytes = ftell(inlineFile);
				fseek(inlineFile,0,SEEK_SET);
				uint8* filedata = new uint8[bytes];
				fread(filedata,bytes,1,inlineFile);
				fclose(inlineFile);
				// Let's dump that good old chunk into the file (nice and compressed too :)
				try
				{
					pm.AddChunkToFile(chunkName,chunkType,destFile,bytes,filedata,CompressionFlag);
				}
				catch (...)
				{
					outputStream << "Erkleroo, excepted during chunk add :( (Some day I'll put some better error routines in :)" << std::endl;
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				delete [] filedata;
				currentTokenType = inputLexer.GetToken(currentToken);
				continue;
			} // inline
			if (currentToken == "group")
			{
				outputStream << "Please wait, preparing StringIntGroup chunk..." << std::endl;
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);
				// Expecting a nice Symbol...
				if (!EXPECT(typeSymbol))
				{ 
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				// The symbol should be 4 chars exactly
				if (currentToken.size() != 4)
				{
					outputStream << "Expecting chunk symbol type. It was not four chars. "
						<< "PRAYBuilder aborting." << std::endl << "Offending token was " <<
						currentToken << " on line " << inputLexer.GetLineNum() << std::endl;
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				chunkType = currentToken;
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);
				if (!EXPECT(typeString))
				{ 
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				// The string is the name of the chunk.
				chunkName = currentToken;
				outputStream << "Chunk is of type: " << chunkType <<
					" and is called " << chunkName << std::endl;
				currentTokenType = inputLexer.GetToken(currentToken);
				myState = lookingTag;
				continue;
			}

		}
		else if (myState == lookingTag)
		{
			if (currentTokenType == SimpleLexer::typeSymbol)
			{
				// Hit end of chunk, so write it out, and reset state
#ifdef C2E_OLD_CPP_LIB
				char hackbuf[2048];
				std::ostrstream os(hackbuf, sizeof(hackbuf) );
#else
				std::ostringstream os;
#endif
				sg.SaveToStream(os);
				std::string tempStr = os.str();
				const char* str = tempStr.data();
				int leng = os.tellp();
				try
				{
					pm.AddChunkToFile(chunkName,chunkType,destFile,leng,(unsigned char *)str,CompressionFlag);
				}
				catch (...)
				{
					outputStream << "Erkleroo, excepted during group add :( (Some day I'll put some better error routines in :)" << std::endl;
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				sg.Clear();
				myState = lookingChunk;
				continue;
			}
			if (currentTokenType == SimpleLexer::typeString)
			{
				// Right then, we have a tag. Format is...
				// "tag" <value>
				// <value> is either a number, a string, or a symbol.
				std::string thisTag = currentToken;
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);

				std::string tempString;
				int bytes;
				char* filedata;

				switch(currentTokenType)
				{
				case SimpleLexer::typeString:
					// We have a string->string mapping
					sg.AddString(thisTag,currentToken);
					break;
				case SimpleLexer::typeNumber:
					// We have a string->int mapping
					sg.AddInt(thisTag,atoi(currentToken.c_str()));
					break;
				case SimpleLexer::typeSymbol:
					// We have a string->file (I.E. string) mapping
					if (currentToken != "@")
					{
						outputStream << "The only valid symbol in this context is \"@\". Error on line " << inputLexer.GetLineNum() << std::endl;
						Output = outputStream.str();
						Output.resize(outputStream.tellp());
						return;
					}
					currentTokenType = inputLexer.GetToken(currentToken);
					munchComment(currentTokenType,currentToken,inputLexer);
					if (!EXPECT(typeString))
					{ 
						Output = outputStream.str();
						Output.resize(outputStream.tellp());
						return;
					}
					outputStream << "Trying to add a file mapping to " << currentToken << std::endl;
					FILE* inlineFile;
					inlineFile = fopen(currentToken.c_str(),"rb");
					if (inlineFile == NULL)
					{
						outputStream << "Oh dear, couldn't open the file :(" << std::endl;
						Output = outputStream.str();
						Output.resize(outputStream.tellp());
						return;
					}
					// File opened, let's munch in the data...
					fseek(inlineFile,0,SEEK_END);
					bytes = ftell(inlineFile);
					fseek(inlineFile,0,SEEK_SET);
					filedata = new char[bytes];
					fread(filedata,bytes,1,inlineFile);
					fclose(inlineFile);

					tempString.assign(filedata,bytes);
					delete [] filedata;
					sg.AddString(thisTag,tempString);
					break;
				default:
					outputStream << "Expecting number,symbol or string. Error on line " << inputLexer.GetLineNum() << std::endl;
					Output = outputStream.str();
					Output.resize(outputStream.tellp());
					return;
				}
				currentTokenType = inputLexer.GetToken(currentToken);
				continue;
			}
		}
		else
		{
			outputStream << "Unknown lookahead state :(" << std::endl;
			Output = outputStream.str();
			Output.resize(outputStream.tellp());
			return;
		}
	}

	if (myState == lookingTag)
	{
		
#ifdef C2E_OLD_CPP_LIB
		char hackbuf[2048];
		std::ostrstream os( hackbuf, sizeof(hackbuf) );
#else
		std::ostringstream os;
#endif
		sg.SaveToStream(os);
		std::string tempStr = os.str();
		const char* str = tempStr.data();
		int leng = os.tellp();
		try
		{
			pm.AddChunkToFile(chunkName,chunkType,destFile,leng,(unsigned char *)str,CompressionFlag);
		}
		catch (...)
		{
			outputStream << "Erkleroo, excepted during group add :( (Some day I'll put some better error routines in :)" << std::endl;
			Output = outputStream.str();
			Output.resize(outputStream.tellp());
			return;
		}
		sg.Clear();
		os.clear();
		myState = lookingChunk;
	}
	SuccessfulBuild = true;
	outputStream << std::endl << std::endl << "***END***" << std::endl << "\0";
	
	Output = outputStream.str();
	Output.resize(outputStream.tellp());
	outputStream.clear();
	pm.GarbageCollect(true);

}
