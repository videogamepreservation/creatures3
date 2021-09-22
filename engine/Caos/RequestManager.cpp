#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "RequestManager.h"

#include "Orderiser.h"
#include "CAOSMachine.h"

#include "../App.h"
#include "../World.h"
#include "../Display/ErrorMessageHandler.h"
#include <strstream>

void RequestManager::HandleIncoming( ServerSide& server )
{
	static int noOfTimesEntered = 0;

	// Note: this function should not be reentrant
	if (noOfTimesEntered>0) {
		ErrorMessageHandler::Show("script_error", 6,
 							std::string("RequestManager::HandleIncoming"));
		return;
	}
	noOfTimesEntered++;

	try
	{
		std::vector< std::string > args;
		char* p;
		int errorcode=0;

		p = (char*)server.GetBuffer();

		// chop first line up into arguments. Args are space-delimited.

		// CR/LF allowed, LF only prefered.
		// Ugh. DOS has _so_ much to answer for.
		while( *p && *p != '\n' && *p != '\r' )
		{
			// new arg
			args.push_back("");
			while( *p && *p != '\n' && *p != '\r' && *p != ' ' && *p != '\t' )
				args.back() += *p++;

			// skip separating spaces (and/or tabs)
			while( *p == ' ' || *p == '\t' )
				++p;
		}

		// skip end-of-line crap
		if( *p == '\r' )
			++p;
		if( *p == '\n' )
			++p;

#ifdef C2E_OLD_CPP_LIB
		std::ostrstream out( (char*)server.GetBuffer(),
			server.GetMaxBufferSize()-1, std::ios::out | std::ios::binary );
#else
		std::ostrstream out( (char*)server.GetBuffer(),
			server.GetMaxBufferSize()-1, std::ios_base::out | std::ios_base::binary );
#endif
		ASSERT(args.size() > 0);

		if( args[0] == "execute" || args[0] == "iscr" )
		{
			Orderiser o;
			MacroScript* m;
			CAOSMachine vm;

			m = o.OrderFromCAOS( p );
			if( m )
			{
				try {
					vm.StartScriptExecuting(m, NULLHANDLE, NULLHANDLE, INTEGERZERO, INTEGERZERO);
					vm.SetOutputStream(&out);
					vm.UpdateVM(-1);
				}
				catch( CAOSMachine::RunError& e )
				{
					// return vm error message
					out.seekp( 0 );
					out << e.what();
					vm.StreamIPLocationInSource(out);

					// clean up after the error
					vm.StopScriptExecuting();

					errorcode = 1;
				}

				// finished with this script now.
				delete m;
			}
			else
			{
				// return orderiser error message
				std::string source(p); // copy p into sourceas out overwrites it
				out.seekp( 0 );
				out << o.GetLastError() << std::endl;
				CAOSMachine::FormatErrorPos(out, o.GetLastErrorPos(), source);
				errorcode = 1;
			}
		}


		if( args[0] == "scrp" )
		{
			if( args.size() >= 5 )
			{
				int f,g,s,e;

				f = atoi( args[1].c_str() );
				g = atoi( args[2].c_str() );
				s = atoi( args[3].c_str() );
				e = atoi( args[4].c_str() );

				Orderiser o;
				MacroScript* m;

				m = o.OrderFromCAOS( p );
				if( m )
				{
					m->SetClassifier( Classifier( f, g, s, e ) );
					if( theApp.GetWorld().GetScriptorium().InstallScript( m ) )
						out << theCatalogue.Get("script_error", 0); // OK
					else
					{
						// script probably in use
						out << theCatalogue.Get("script_error", 1);
						Classifier( f, g, s, e ).StreamClassifier(out);
						out << theCatalogue.Get("script_error", 2);
						errorcode = 1;
					}

					// Don't delete this as it is referenced by the scriptorium
					// delete m;
				}
				else
				{
					// return orderiser error message
					std::string source(p); // copy p into sourceas out overwrites it
					out.seekp( 0 );
					out << o.GetLastError() << std::endl;
					CAOSMachine::FormatErrorPos(out, o.GetLastErrorPos(), source);
					errorcode = 1;
				}
			}
			else
			{
				out.seekp( 0 );
				// "Error: Correct format is \"scrp <family> <genus> <species> <event>\""
				out << theCatalogue.Get("script_error", 5);
				errorcode = 1;
			}
		}

		out.put( '\0' );	// terminate the string

/*		OutputFormattedDebugString("strlen: %ld, outblah: %ld\n", 
			strlen( (char*)server.GetBuffer() ) + 1,
			out.pcount()
		);
*/

//		server.Respond( strlen( (char*)server.GetBuffer() ) + 1, errorcode );
		server.Respond( out.pcount(), errorcode );
		noOfTimesEntered--;

#ifdef _WIN32
		// why is this here?  -BenC
		Sleep(5);
#endif
	}
	// We catch all exceptions in release mode for robustness.
	// In debug mode it is more useful to see what line of code
	// the error was caused by.
	catch(BasicException& e) {
		server.GetBuffer()[0] = 0;
		server.Respond( 0, 1 );
		noOfTimesEntered--;
		ErrorMessageHandler::Show(e, std::string("RequestManager::HandleIncoming"));
	}
	catch( ... ) {
		server.GetBuffer()[0] = 0;
		server.Respond( 0, 1 );
		noOfTimesEntered--;
		// We don't want to localise this message, to avoid getting into
		// infinite exception traps when the catalogues are bad.
		ErrorMessageHandler::NonLocalisable("NLE0005: Unknown exception caught in request manager",
			std::string("RequestManager::HandleIncoming"));
	}
}

