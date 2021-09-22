// -------------------------------------------------------------------------
// Filename:    Orderiser.cpp
// Class:       Orderiser
// Purpose:     Simple compiler for CAOS code.
// Description:
//
//
// Usage:
//
//
// History:
// 30Nov98  BenC	Initial version
// 22Mar99  Robert  Tidy up/Added floating-point support
// -------------------------------------------------------------------------


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "Orderiser.h"
#include "MacroScript.h"
#include "../C2eServices.h"
#include "../Map/Map.h"



// The one and only description of the CAOS langauge
// Maybe should be moved elsewhere...
CAOSDescription theCAOSDescription;


Orderiser::Orderiser()
{
	myDebugInfo = NULL;
	myOutputBufferSize = OUT_BUFFER_INITIAL_SIZE;
	myOutputBuffer = new unsigned char [ myOutputBufferSize ];
	myErrorPos = -1;
}


Orderiser::~Orderiser()
{
	if( myOutputBuffer )
		delete [] myOutputBuffer;
}


MacroScript* Orderiser::OrderFromCAOS( const char* srctext )
{
	int type;
	bool done = false;
	bool allok=true;
	const OpSpec* op;

	// setup - have to clear out all working data, because
	// the Orderiser object can be reused over and over.
	myLexer.Attach( srctext );
	myLabels.erase( myLabels.begin(), myLabels.end() );
	while( !myContextStack.empty() )
		myContextStack.pop();
	while( !myLabelRefs.empty() )
		myContextStack.pop();
	myIP = 0;
	myUniqueLabelID = 0;

	// this debuginfo object will be passed on to the output
	// macroscript if all goes well:
	myDebugInfo = new DebugInfo( srctext );

	// the main loop
	while( allok && !done )
	{
		type = myLexer.NextItem();
		if( type == Lexer::itemFinished )
		{
			// plonk a STOP at the end for all us stupid people who
			// keep forgetting it.
			EncodeOpID( CAOSDescription::cmdSTOP );
			done = true;
		}
		else if( type == Lexer::itemError )
		{
			ReportError( -1, sidSyntaxError );
			allok = false;
		}
		else
		{
			op = theCAOSDescription.FindCommand( myLexer.GetAsText() );
			if( !op )
			{
				ReportError( -1, sidInvalidCommand );
				allok = false;
			}
			else
			{
				myDebugInfo->AddAddressToPositionMapping( myIP, myLexer.GetPos() );
				allok = EncodeOp( op );
			}
		}
	}

	if( allok )
	{
		// do any required postprocessing:

		// still in a DOIF/ENUM/etc block?
		if( !myContextStack.empty() )
		{
			ReportError( -1, sidFailedToCloseBlock );
			allok = false;
		}
	}

	if( allok )
		allok = ResolveLabels();
	
	if( allok )
	{

		//int a;
		//for (a=0; a<myIP; a++) {
		//OutputFormattedDebugString("Byte %d  %d\n", a, myOutputBuffer[a]);
		//}

		// Put the compiled code into a MacroScript object
		// (ownership of debuginfo is passed to the macroscript)
		return new MacroScript( myOutputBuffer, myIP, myDebugInfo );
	}
	else
	{
		if( myDebugInfo )
		{
			delete myDebugInfo;
			myDebugInfo = NULL;
		}

		return NULL;
	}
}



//////////////////////////////////////////////////////////////////

bool Orderiser::EncodeOp( const OpSpec* op )
{
	int arg;
	bool allok = true;

	allok = SpecialPreProcessing( op );

	if( allok )
		EncodeOpID( op->GetOpcode() );

	// encode params
	for( arg=0; arg < op->GetParameterCount() && allok; ++arg )
	{
		switch( op->GetParameter(arg) )
		{
		case '*':
			allok = ExpectSubCommand( op );
			break;
		case 'i':
		case 'f':
		case 'd':
			allok = ExpectNumericRV();
			break;
		case 'm':
			allok = ExpectGenericRV();
			break;
		case 'v':
			allok = ExpectVariable();
			break;
		case 's':
			allok = ExpectStringRV();
			break;
		case 'a':
			allok = ExpectAgentRV();
			break;
		case 'b':
			allok = ExpectByteString();
			break;
		case 'c':
			allok = ExpectConditional();
			break;
		case '-':	// placeholder - output an int.
			EncodeInt( 0 );
			break;
		case '#':	// label
			if( FetchLabel() )
			{
				// record a label reference - it'll get resolved later on
				myLabelRefs.push_back( LabelRef( myLexer.GetAsText(),
					myIP, myLexer.GetPos() ) );

				// put placeholder for label
				EncodeInt( 0 );

			}
			else
				allok = false;
			break;
		}
	}

	if( allok )
		allok = SpecialPostProcessing( op );

	return allok;
}


// want to read in a string
bool Orderiser::ExpectStringRV()
{
	const OpSpec* op;
	int lextype;

	lextype = myLexer.NextItem();

	// string rval?
	if( lextype == Lexer::itemSymbol )
	{
		op = theCAOSDescription.FindStringRV( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argStringRV );
			return EncodeOp( op );
		}

		op = theCAOSDescription.FindVariable( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argVariable );
			return EncodeOp( op );
		}

	}

	// String constant?
	if( lextype == Lexer::itemString )
	{
		EncodeOpID( CAOSDescription::argStringConstant );
		EncodeString( myLexer.GetAsText() );
		return true;
	}


	if( lextype == Lexer::itemError )
		ReportError( -1, sidSyntaxError );
	else
		ReportError( -1, sidExpectedString );
	return false;
}





bool Orderiser::ExpectAgentRV()
{
	const OpSpec* op;
	int lextype;

	lextype = myLexer.NextItem();

	if( lextype == Lexer::itemSymbol )
	{
		op = theCAOSDescription.FindAgentRV( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argAgentRV );
			return EncodeOp( op );
		}

		// try for a variable instead
		op = theCAOSDescription.FindVariable( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argVariable );
			return EncodeOp( op );
		}

	
	}

	if( lextype == Lexer::itemError )
		ReportError( -1, sidSyntaxError );
	else
		ReportError( -1, sidExpectedAgent );
	return false;
}



bool Orderiser::ExpectAnyRV(int& argType)
{
	const OpSpec* op;
	int lextype;

	lextype = myLexer.NextItem();

	if( lextype == Lexer::itemSymbol )
	{
		// try for an rval
		op = theCAOSDescription.FindIntegerRV( myLexer.GetAsText() );
		if( op )
		{
			argType = CAOSDescription::argIntegerRV;
			EncodeOpID( argType );
			return EncodeOp( op );
		}

		// try for a float rval
		op = theCAOSDescription.FindFloatRV( myLexer.GetAsText() );
		if( op )
		{
			argType = CAOSDescription::argFloatRV;
			EncodeOpID( argType );
			return EncodeOp( op );
		}

		// try for a string rval
		op = theCAOSDescription.FindStringRV( myLexer.GetAsText() );
		if( op )
		{
			argType = CAOSDescription::argStringRV;
			EncodeOpID( argType );
			return EncodeOp( op );
		}

		// try for an agent rvalue
		op = theCAOSDescription.FindAgentRV( myLexer.GetAsText() );
		if( op )
		{
			argType = CAOSDescription::argAgentRV;
			EncodeOpID( argType );
			return EncodeOp( op );
		}

		// try for a variable instead
		op = theCAOSDescription.FindVariable( myLexer.GetAsText() );
		if( op )
		{
			argType = CAOSDescription::argVariable;
			EncodeOpID( argType );
			return EncodeOp( op );
		}	
	}

	// integer constant?
	if( lextype == Lexer::itemInteger )
	{
		// encode the integer
		argType = CAOSDescription::argIntegerConstant;
		EncodeOpID( argType );
		EncodeInt( myLexer.GetIntegerValue() );
		return true;
	}

	// float constant?
	if( lextype == Lexer::itemFloat )
	{
		// encode the float
		argType = CAOSDescription::argFloatConstant;
		EncodeOpID( argType );
		EncodeFloat( myLexer.GetFloatValue() );
		return true;
	}

	// String constant?
	if( lextype == Lexer::itemString )
	{
		argType = CAOSDescription::argStringConstant;
		EncodeOpID( argType );
		EncodeString( myLexer.GetAsText() );
		return true;
	}

	if( lextype == Lexer::itemError )
		ReportError( -1, sidSyntaxError );
	else
		ReportError( -1, sidExpectedAnyRValue );
	return false;
}



// want to read in a subcommand and args
bool Orderiser::ExpectSubCommand( const OpSpec* parentop )
{
	const OpSpec* subop;
	int lextype;

	lextype = myLexer.NextItem();
	if( lextype == Lexer::itemSymbol )
	{
		subop = theCAOSDescription.FindSubCommand( myLexer.GetAsText(), parentop );
		if( subop )
			return EncodeOp( subop );
	}

	// "Expected a subcommand"
	ReportError( -1, sidExpectedSubcommand );
	return false;
}




// want to read in a variable and args
bool Orderiser::ExpectVariable()
{
	int lextype;
	const OpSpec* op;

	lextype = myLexer.NextItem();
	if( lextype == Lexer::itemSymbol )
	{
		op = theCAOSDescription.FindVariable( myLexer.GetAsText() );
		if( op )
		{
			// encode the var
			// (don't need to code an arg type)
			return EncodeOp( op );
		}
	}
	ReportError( -1, sidExpectedVariable );
	return false;
}


bool Orderiser::ExpectNumericRV()
{
	const OpSpec* op;
	int lextype;

	lextype = myLexer.NextItem();

	if( lextype == Lexer::itemSymbol )
	{
		// try for a float rval
		op = theCAOSDescription.FindFloatRV( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argFloatRV );
			return EncodeOp( op );
		}

		// try for an rval
		op = theCAOSDescription.FindIntegerRV( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argIntegerRV );
			return EncodeOp( op );
		}

		// try for a var
		op = theCAOSDescription.FindVariable( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argVariable );
			return EncodeOp( op );
		}

	}

	if( lextype == Lexer::itemFloat )
	{
		// encode the float
		EncodeOpID( CAOSDescription::argFloatConstant );
		float f = myLexer.GetFloatValue();
		EncodeFloat( f );
		return true;
	}

	if( lextype == Lexer::itemInteger )
	{
		// encode the integer
		EncodeOpID( CAOSDescription::argIntegerConstant );
		int i = myLexer.GetIntegerValue();
		EncodeInt( i );
		return true;
	}

	ReportError( -1, sidExpectedNumericRValue );
	return false;
}


bool Orderiser::ExpectGenericRV()
{
	const OpSpec* op;
	int lextype;

	lextype = myLexer.NextItem();

	if( lextype == Lexer::itemSymbol )
	{
		// try for a float rval
		op = theCAOSDescription.FindFloatRV( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argFloatRV );
			return EncodeOp( op );
		}

		// try for an integer rval
		op = theCAOSDescription.FindIntegerRV( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argIntegerRV );
			return EncodeOp( op );
		}

		// Try for a string rvalue
		op = theCAOSDescription.FindStringRV( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argStringRV );
			return EncodeOp( op );
		}

		// Try for an agent rvalue
		op = theCAOSDescription.FindAgentRV( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argAgentRV );
			return EncodeOp( op );
		}

		// try for a var
		op = theCAOSDescription.FindVariable( myLexer.GetAsText() );
		if( op )
		{
			EncodeOpID( CAOSDescription::argVariable );
			return EncodeOp( op );
		}

	}

	if( lextype == Lexer::itemFloat )
	{
		// encode the float
		EncodeOpID( CAOSDescription::argFloatConstant );
		float f = myLexer.GetFloatValue();
		EncodeFloat( f );
		return true;
	}

	if( lextype == Lexer::itemInteger )
	{
		// encode the integer
		EncodeOpID( CAOSDescription::argIntegerConstant );
		int i = myLexer.GetIntegerValue();
		EncodeInt( i );
		return true;
	}

	// String constant?
	if( lextype == Lexer::itemString )
	{
		EncodeOpID( CAOSDescription::argStringConstant );
		EncodeString( myLexer.GetAsText() );
		return true;
	}

	ReportError( -1, sidExpectedNumericRValue );
	return false;
}


bool Orderiser::ExpectConditional()
{
	bool done = false;
	int lextype;
	int argType;
	int dummy;

	while( !done )
	{
		if( !ExpectAnyRV(argType) )
			return false;

		// check for comparison op
		lextype = myLexer.NextItem();
		if( lextype != Lexer::itemComparison )
		{
			ReportError( -1, sidExpectedComparisonOp );
			return false;
		}

		// encode the comparison operator.
		EncodeOpID( myLexer.GetIntegerValue() );

		switch (argType) {
		case CAOSDescription::argVariable:
			if( !ExpectAnyRV(dummy) )
				return false;
			break;
		case CAOSDescription::argAgentRV:
			if( !ExpectAgentRV() )
				return false;
			break;
		case CAOSDescription::argStringConstant:
		case CAOSDescription::argStringRV:
			if( !ExpectStringRV() )
				return false;
			break;
		default:
			if( !ExpectNumericRV() )
				return false;
			break;
		} // switch

		// check for more conditions (AND, OR etc)
		lextype = myLexer.NextItem();
		if( lextype == Lexer::itemLogical )
		{
			// encode the logical op...
			EncodeOpID( myLexer.GetIntegerValue() );
		}
		else
		{
			// oops - we've read too far, unget the item...
			myLexer.BackTrack();
			done = true;
		}
	}

	// put a null logical op here to terminate the conditional
	EncodeOpID( CAOSDescription::logicalNULL );

	return true;
}








// reads in a label from the lexer, returns true if a valid name
bool Orderiser::FetchLabel()
{
	int lextype;
	lextype = myLexer.NextItem();
	if( lextype != Lexer::itemSymbol )
	{
		// "Label expected"
		ReportError( -1, sidExpectedLabel );
		return false;
	}

	return true;
}



// suss out all the references to labels.
bool Orderiser::ResolveLabels()
{
	std::list<LabelRef>::iterator ref;
	std::map<std::string, int>::iterator l;

	for( ref = myLabelRefs.begin(); ref != myLabelRefs.end(); ++ref )
	{
		l = myLabels.find( (*ref).Label );
		if( l != myLabels.end() )
		{
			// backpatch the code to resolve the reference...
			PlonkInt( (*ref).RefAddr, (*l).second );

//			printf("resolved label ref at 0x%04x to %s (=%04x)\n",
//				(*ref).RefAddr,
//				(*l).first.c_str(),
//				(*l).second );
		}
		else
		{
			// "Unresolved label"
			ReportError( (*ref).SrcPos, sidUnresolvedLabel,
				(*ref).Label.c_str() );
			return false;
		}
	}

	return true;
}


void Orderiser::GrowOutputBuffer( int sizerequired )
{

	if( myOutputBufferSize < sizerequired )
	{
		int newsize;
		unsigned char* p;

		newsize = sizerequired + OUT_BUFFER_GROW_STEP;
		_ASSERT( newsize >= myOutputBufferSize ); 	// can't shrink
		p = new unsigned char [ newsize ];
		memcpy(p, myOutputBuffer, myOutputBufferSize );
		delete [] myOutputBuffer;
		myOutputBuffer = p;
		myOutputBufferSize = newsize;
	}
}




void Orderiser::PlonkInt( int addr, int i )
{
	GrowOutputBuffer( addr + sizeof(int) );
	*((int*)(&myOutputBuffer[addr])) = i;
}


void Orderiser::PlonkFloat( int addr, float f )
{
	GrowOutputBuffer( addr + sizeof(float) );
	*((float*)(&myOutputBuffer[addr])) = f;
}


void Orderiser::PlonkOp( int addr, OpType op )
{
	GrowOutputBuffer( addr + sizeof( OpType ) );
	*((OpType*)(&myOutputBuffer[addr])) = op;
}

void Orderiser::PlonkByte( int addr, unsigned char b )
{
	GrowOutputBuffer( addr + sizeof( unsigned char ) );
	*((unsigned char*)(&myOutputBuffer[addr])) = b;
}

int Orderiser::PeekInt( int addr )
{
	_ASSERT( addr <= myOutputBufferSize - sizeof(int) );
	return *((int*)(&myOutputBuffer[addr]));
}


void Orderiser::EncodeInt( int i )
{
	PlonkInt( myIP, i );
	myIP += sizeof(int);
}


void Orderiser::EncodeFloat( float f )
{
	PlonkFloat( myIP, f );
	myIP += sizeof(float);
}



void Orderiser::EncodeByte( unsigned char b )
{
	PlonkByte( myIP, b );
	myIP += sizeof( unsigned char );
}


void Orderiser::EncodeOpID( OpType op )
{
	PlonkOp( myIP, op );
	myIP += sizeof(OpType);
}


void Orderiser::EncodeString( const char* str )
{
	int count;

	count = strlen( str ) + 1;	// allow for null char
	if( count & 1 )	// pad if odd
		count++;

	GrowOutputBuffer( myIP + count );
	
	strncpy( (char*)(&myOutputBuffer[myIP]), str, count );

	myIP += count;
}



void Orderiser::EnterBlock( int type, bool labeled )
{
	char buf[32];

	if( labeled )
	{
		// generate a unique label name for the end of the block
		sprintf( buf, "_block_%d",myUniqueLabelID++ );
	}
	else
		buf[0] = '\0';

	myContextStack.push( BlockContext( type, myIP, buf ) );
}


// close a block
bool Orderiser::LeaveBlock( int type )
{
	if( myContextStack.empty() )
	{
		// "Already at top level"
		ReportError( -1, sidAlreadyTopLevel );
		return false;
	}

	if( myContextStack.top().Type != type )
	{
		// "Mismatched block type"
		ReportError( -1, sidMismatchedBlockType );
		return false;
	}

	// if the block specified an ending label, define it now.
	if( myContextStack.top().EndLabel != "" )
	{
		if( !DefineLabel( myContextStack.top().EndLabel.c_str() ) )
			return false;
	}

	myContextStack.pop();
	return true;
}




// called just before the op is encoded...
bool Orderiser::SpecialPreProcessing( const OpSpec* op )
{
	bool toplevel = myContextStack.empty();

	switch( op->GetSpecialCode() )
	{
	case specialDOIF:
		// enter a DOIF block
		EnterBlock( specialDOIF, true );
//		printf( "%s\n", myLexer.GetAsText() );
		break;
	case specialELIF:
	case specialELSE:
		// sanity check:
		if( myContextStack.empty() )
		{
			// "Already at top level"
			ReportError( -1, sidAlreadyTopLevel );
			return false;
		}

		// insert a goto at the end of the block to jump to
		//the next matching ENDI
		EncodeOpID( CAOSDescription::cmdGOTO );

		myLabelRefs.push_back( LabelRef(
			myContextStack.top().EndLabel.c_str(),
			myIP,
			myLexer.GetPos() ) );
		EncodeInt( 0 );		// placeholder for label

		// backpatch the previous doif/elif to jump to here if
		// condition failed:
		PlonkInt( myContextStack.top().Addr + sizeof(OpType), myIP );

		// fiddle the block to start at this statement instead. A bit nasty.
		// Don't want to start another block, because we need the endblock
		// label named by the original DOIF.
		myContextStack.top().Addr = myIP;
		break;
	case specialENDI:
		// backpatch the previous doif/elif/else to jump to here if
		// condition failed.
		// We don't really need to backpatch ELSE blocks, but we're
		// going to anyway. So there.
		PlonkInt( myContextStack.top().Addr + sizeof(OpType), myIP );
		break;
	case specialENUM:
	case specialESEE:
	case specialETCH:
	case specialEPAS:
	case specialECON:
		// enter an ENUM block
		EnterBlock( specialENUM, false );
		break;
	case specialREPS:
		// enter an REPS block
		EnterBlock( specialREPS, false );
		break;
	case specialLOOP:
		// enter an LOOP block
		EnterBlock( specialLOOP, false );
		break;
	case specialNEXT:
		// sanity check:
		if( myContextStack.empty() )
		{
			// "Already at top level"
			ReportError( -1, sidAlreadyTopLevel );
			return false;
		}
		if( myContextStack.top().Type != specialENUM )
		{
			// "NEXT without matching ENUM/ESEE/ETCH/EPAS"
			ReportError( -1, sidNEXTError );
			return false;
		}
		break;
	case specialUNTL:
	case specialEVER:
		// sanity check:
		if( myContextStack.empty() )
		{
			// "Already at top level"
			ReportError( -1, sidAlreadyTopLevel );
			return false;
		}
		if( myContextStack.top().Type != specialLOOP )
		{
			// "UNTL or EVER without matching LOOP"
			ReportError( -1, sidBrokenLOOP );
			return false;
		}
		break;
	case specialREPE:
		// sanity check:
		if( myContextStack.empty() )
		{
			// "Already at top level"
			ReportError( -1, sidAlreadyTopLevel );
			return false;
		}
		if( myContextStack.top().Type != specialREPS )
		{
			// "REPE without matching REPS"
			ReportError( -1, sidREPEWithoutREPS );
			return false;
		}
		break;
	}
	return true;
}


// called after the rest of the op is encoded:

bool Orderiser::SpecialPostProcessing( const OpSpec* op )
{
	switch( op->GetSpecialCode() )
	{
	case specialSUBR:
		// read in label name and define it
		if( FetchLabel() )
			return DefineLabel( myLexer.GetAsText() );
		else
			return false;
		break;
	case specialENDI:
		LeaveBlock( specialDOIF );
		break;
	case specialNEXT:
		// backpatch the previous ENUM/etc to point to the NEXT statement
		PlonkInt( myContextStack.top().Addr + sizeof( OpType ), myIP - sizeof( OpType) );
		LeaveBlock( specialENUM );
		break;
	case specialUNTL:
	case specialEVER:
		LeaveBlock( specialLOOP );
		break;
	case specialREPE:
		LeaveBlock( specialREPS );
		break;
	}
	return true;
}



// define a new label at the current address
bool Orderiser::DefineLabel( const char* name )
{
	if( myLabels.find( name ) != myLabels.end() )
	{
		// "Label already defined"
		ReportError( -1, sidLabelAlreadyDefined );
		return false;
	}
	myLabels[ name ] = myIP;
//		printf( "label %s\n", myLexer.GetAsText() );
	return true;
}




// want to read in a string of bytes eg "[0 1 255 42]"
bool Orderiser::ExpectByteString()
{
	int lextype;
	bool done;
	int itemcountaddr;
	int value;

	lextype = myLexer.NextItem();
	if( lextype != Lexer::itemOpenSquareBrackets )
	{
		// "Expected '['"
		ReportError( -1, sidExpectedByteString );
		return false;
	}

	// record where element count is stored
	itemcountaddr = myIP;
	EncodeInt( 0 );		// start with zero items

	done=false;
	while( !done )
	{
		lextype = myLexer.NextItem();
		switch( lextype )
		{
		case Lexer::itemCloseSquareBrackets:
			done = true;
			break;
		case Lexer::itemInteger:
			// boundscheck and encode the byte
			value = myLexer.GetIntegerValue();
			if( value < 0 || value > 255 )
			{
				// "Value out of valid range (0..255)"
				ReportError( -1, sidOutOfByteRange );
				return false;
			}
			EncodeByte( (unsigned char) value );
			// update the count
			PlonkInt( itemcountaddr, PeekInt( itemcountaddr )+1 );
			break;
		default:
			// "Expected byte or ']'"
			ReportError( -1, sidExpectedMidByteStream );
			return false;
			break;
		}
	}

	// pad to even length
	if( PeekInt( itemcountaddr ) & 1 )
		EncodeByte( 0 );

	return true;
}

void Orderiser::ReportError( int pos, int stringid, ... )
{
	char buf[ 512 ];
	const char* fmt;
	va_list args;

	if( pos == -1 )
		pos = myLexer.GetPos();

	myErrorPos = pos;

	try
	{
		fmt = theCatalogue.Get("orderiser", stringid );
		va_start( args, stringid );
		vsprintf( buf, fmt, args );
		va_end( args );

		std::string wrapper = theCatalogue.Get( "orderiser", sidAtToken );
		sprintf( myErrorBuffer, wrapper.c_str(), buf, myLexer.GetAsText());
	}
	catch( Catalogue::Err& e )
	{
		strcpy( myErrorBuffer, e.what() );
	}


}

