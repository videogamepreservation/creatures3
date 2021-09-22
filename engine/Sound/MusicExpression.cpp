// --------------------------------------------------------------------------
// Filename:	Music Expression.cpp
// Class:		MusicExpression
// Purpose:		Generic expression structure used throughout music manager
//
// Description:
//
// Mathematical expression involving loci (values within the music system)
// arithmetic operators and the  random function

// Initially, expressions will be limited to:
//
//    R-Variable
//    Add(R-Variable, R-Variable)
//    Subtract(R-Variable, R-Variable)
//    Multiply(R-Variable, R-Variable)
//    Divide(R-Variable, R-Variable)
//    Mod(R-Variable, R-Variable)		 (Floating point MOD)
//    Div(R-Variable, R-Variable)		 (Floating point DIV)
//    Random(R-Variable, R-Variable)	 = ( minimum,  maximum )
//	  SineWave(R-Variable, R-Variable)   = ( position, wavelength )
//	  CosineWave(R-Variable, R-Variable) = ( position, wavelength )
//	  LessThan(R-Variable, R-Variable)		= ( value, threshold)
//	  LessEquals(R-Variable, R-Variable)    = ( value, threshold)
//	  GreaterThan(R-Variable, R-Variable)	= ( value, threshold)
//	  GreaterEquals(R-Variable, R-Variable) = ( value, threshold)
//	  Equals(R-Variable, R-Variable)		 = ( value, threshold)
// 
// The last five comparison operators return 1.0 if the condition is
// satisfied, 0.0 otherwise
// 
// RVars include any floating point value accessible in the music system,
// and floating point constants (all stored in an array within the music
// manager class)
//
// Operators include +,-,/,*
//
// A form of floating point MOD and DIV are also provided, to enable
// calculation of positions within bars and cycles
//
// History:
//  02Apr98	PeterC	Created
//  08May98	PeterC	Fixed bug with parsing scoped variables
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicExpression.h"
#include "MusicLayer.h"
#include "MusicTrack.h"
#include "MusicManager.h"
#include "MusicScript.h"

#include <math.h>

#define PI 3.14159265359

// ----------------------------------------------------------------------
// constants
// static array of constants used within expressions
// **** could have a much more efficient memory system here
// ----------------------------------------------------------------------
std::vector <MusicValue *> MusicExpression::constants;

// ----------------------------------------------------------------------
// totalExpressions
// static reference count of existing expressions, maintained by the
// constructor and destructor.  When this returns to zero, the list of
// constants will be deleted
// ----------------------------------------------------------------------
int MusicExpression::totalExpressions = 0;


// ----------------------------------------------------------------------
// Method:		MusicExpression
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicExpression::MusicExpression() : operatorType(None)
	{
	operands[0] = operands[1] = NULL;
	// Increase the reference count
	totalExpressions++;
	}

// ----------------------------------------------------------------------
// Method:		~MusicExpression
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicExpression::~MusicExpression()
	{
	// Decrease the reference count
	totalExpressions--;

	// Shouldn't be able to destroy more than we added
	ASSERT( totalExpressions >=0 );

	// Have we disposed of all the expressions ?
	if (totalExpressions==0)
		{
		// Yes - clear out the store of constants
		constantsIterator it;
		for (it = constants.begin(); it != constants.end(); it++ )
			{
			// First delete the allocated space
			delete (*it);
			}

		// Now clear out the entries
		constants.clear();
		}

	}

// ----------------------------------------------------------------------
// Method:		ParseWithScope
// Arguments:	script  - script, pointing to the start of an expression
//				manager - current music manager owning soundtracks
//				track   - current soundtrack (or NULL if this is being
//						  called from the manager)
//				layer	- current layer (or NULL if this is being called
//						  from a track)
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to Parse the expression from the script
//				This will take the current scope into account
//				(If neither layer nor track are defined, it will be
//				considered to be Parsed from a manger, if track only
//				is defined, it will be Parsed from a track etc.)
// ----------------------------------------------------------------------
MusicError MusicExpression::ParseWithScope(MusicScript &script,
										   MusicManager *manager,
										   MusicTrack *track,
										   MusicLayer *layer)
	{
	// Let's see if we have a function first
	// (This does not move the script on to the next token)
	operatorType = ParseOperator ( script );

	if (operatorType == None )
		{
		// Should be a straight assignment.  Find the address of the value
		// it refers to, and store it as the first of the arguments
		operands[0] = ParseRValue ( script, manager, track, layer );
		if (!operands[0])
			{
			// No defined variable could be found
			return MUSIC_SYNTAX_ERROR;
			}

		}
	else
		{
		// Must have been a function of some sort.  Parse the
		// remaining arguments
		script.Advance();

		// Should have a "("
		if (script.GetCurrentType() != MusicScript::StartArgument)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		script.Advance();

		// Now the first operand
		operands[0] = ParseRValue ( script, manager, track, layer );

		// Was a matching value found?
		if (!operands[0])
			{
			return MUSIC_SYNTAX_ERROR;
			}

		// Operands (always two) are separated by a comma
		if (script.GetCurrentType() != MusicScript::Separator)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		script.Advance();

		// Now the Second operand
		operands[1] = ParseRValue ( script, manager, track, layer );

		// Was a matching value found?
		if (!operands[1])
			{
			return MUSIC_SYNTAX_ERROR;
			}

		// Finally there should be a closing bracket
		if (script.GetCurrentType() != MusicScript::EndArgument)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		// Leave the script pointing to the next thing
		script.Advance();



		}

	return MUSIC_OK;

	}



// ----------------------------------------------------------------------
// Method:		Evaluate
// Arguments:	None
// Returns:		value of the expression
// Description:	Evaluates the expression
// ----------------------------------------------------------------------
MusicValue MusicExpression::Evaluate() const
	{
	// Was any operation defined
	if (operatorType == None)
		{
		// Just a straight assignment - return the
		// contents of the first operand
		ASSERT(operands[0]);
		return *operands[0];
		}

	// Should always have both operands defined if we get here
	ASSERT(operands[0]);
	ASSERT(operands[1]);


	switch(operatorType)
		{
		case (Add):
			{
			return (*operands[0] + *operands[1]);
			}
		case (Subtract):
			{
			return (*operands[0] - *operands[1]);
			}
		case (Multiply):
			{
			return (*operands[0] * (*operands[1]));
			}
		case (Divide):
			{
			// Prevent divide by zero errors crashing the maching
			if (*operands[1] != 0.0)
				{
				return (*operands[0] / (*operands[1]));
				}
			else
				{
				// Default to zero
				return 0.0;
				}
			}
		case (Mod):
			{
			// Wierd floating point equivalent to MOD

			// First catch any divide by zero errors
			if (*operands[1] == 0.0)
				{
				return (0.0);
				}

			// This is a bit tricky to explain verbally...
			MusicValue division = *operands[0] / *operands[1];
			MusicValue integerComponent = (MusicValue) floor ( (double) division );

			return ( ( division - integerComponent) * (*operands[1]) );
		
			}
		case (Div):
			{
			// Just return the integer component of a division

			// First catch any divide by zero errors
			if (*operands[1] == 0.0)
				{
				return (0.0);
				}

			// This is a bit tricky to explain verbally...
			return  (MusicValue) floor ( (double) (*operands[0] / *operands[1] ) );

			}
		case (Random):
			{
			// Want to get a random number between the two operands.
			// Pick a random integer, and use this to generate a value
			const MusicValue &min=*operands[0];
			const MusicValue &max=*operands[1];
			return (min+((MusicValue) rand() * (max-min))/( (MusicValue) (RAND_MAX)) );
			}
		case (SineWave):
			{
			// Returns the result of the expression
			// sin ( ( op1 / op2) * 2 PI )
			// 
			// i.e.
			//	op1 = position along wave
			//  op2 = wavelength

			// First catch any divide by zero errors
			if (*operands[1] == 0.0)
				{
				return (0.0);
				}

			return (MusicValue) sin( ( (MusicValue) ( *operands[0] / *operands[1] ) * 2 * PI ) );
			}

		case (CosineWave):
			{
			// Returns the result of the expression
			// cos ( ( op1 / op2) * 2 PI )
			// 
			// i.e.
			//	op1 = position along wave
			//  op2 = wavelength

			// First catch any divide by zero errors
			if (*operands[1] == 0.0)
				{
				return (0.0);
				}

			return (MusicValue) cos( ( (MusicValue) ( *operands[0] / *operands[1] ) * 2 * PI ) );
			}
		case (Less):
			{
			if ( *operands[0] < *operands[1] )
				{
				return (1.0);
				}	
			else
				{
				return (0.0);
				}
			}
		case (LessEquals):
			{
			if ( *operands[0] <= *operands[1] )
				{
				return (1.0);
				}	
			else
				{
				return (0.0);
				}
			}
		case (Greater):
			{
			if ( *operands[0] > *operands[1] )
				{
				return (1.0);
				}	
			else
				{
				return (0.0);
				}
			}
		case (GreaterEquals):
			{
			if ( *operands[0] >= *operands[1] )
				{
				return (1.0);
				}	
			else
				{
				return (0.0);
				}
			}
		case (Equals):
			{
			if ( *operands[0] == *operands[1] )
				{
				return (1.0);
				}	
			else
				{
				return (0.0);
				}
			}
		}

	// There has been a serious bug with the expression parse if
	// we get here
	ASSERT(TRUE);

	// Default behaviour
	return (0.0);

	}

// ----------------------------------------------------------------------
// Method:		ParseOperator
// Arguments:	script - script, pointing to start of expression
// Returns:		Operator found (or None)
// Description:	Identifies the operator at the current script position
//				Note that this does not move the script forward
// ----------------------------------------------------------------------
MusicExpression::MusicOperator MusicExpression::ParseOperator(MusicScript &script) const
	{
	// Define a lookup table, giving each operator and
	// its name
	struct OpLookup
		{
		MusicOperator op;
		char name[32];
		};

	const OpLookup table[]=
		{ 
			{Add, "Add"},
			{Subtract,"Subtract"},
			{Multiply,"Multiply"},
			{Divide,"Divide"},
			{Mod,"Mod"},
			{Div,"Div"},
			{Random,"Random"},
			{SineWave,"SineWave"},
			{CosineWave,"CosineWave"},
			{Less,"Less"},
			{LessEquals,"LessEquals"},
			{Greater,"Greater"},
			{GreaterEquals,"GreaterEquals"},
			{Equals,"Equals"},
		};

	std::string name(script.GetCurrentToken());

	// Iterate through each member of the lookup table
	int i;
	for ( i=0; i< 14; i++)
		{
		// Is this a match?
		if (strcmp(name.data(),table[i].name)==0)
			{
			return table[i].op;
			}
		}

	// Nothing found - must be a variable assignment
	return None;
	}

// ----------------------------------------------------------------------
// Method:		ParseRValue
// Arguments:	script - script, pointing to start of expression
//				manager - current music manager owning soundtracks
//				track   - current soundtrack (or NULL if this is being
//						  called from the manager)
//				layer	- current layer (or NULL if this is being called
//						  from a track)
// Returns:		Address of the contents of the value (or NULL if this 
//				could not be found)
// Description:	Locates a named variable, or allocates space for a
//				constant (this will be deleted automatically)
//				Note that this does not move the script forward
//				This will take the current scope into account
//				(If neither layer nor track are defined, it will be
//				considered to be Parsed from a manger, if track only
//				is defined, it will be Parsed from a track etc.)
// ----------------------------------------------------------------------
MusicValue *MusicExpression::ParseRValue(MusicScript &script,
										 MusicManager *manager,
										 MusicTrack *track,
										 MusicLayer *layer)
	{
	// Shouldn't be able to get this far without a manager
	ASSERT(manager);

	// First of all, check if its a scoped variable (i.e. one that
	// refers to a particular layer)

	if (script.GetCurrentType() == MusicScript::ScopedString)
		{
		// It was - split the string into its two components
		std::string layerName;
		std::string varName;

		LPCTSTR pos = script.GetCurrentToken();

		bool foundDivide = false;

		// Start by adding letters to the layer name
		std::string* currentName = &layerName;

		// Iterate through the string, copying the two halves
		while (*pos!=0)
			{
			// Is this the divide ?
			if (*pos=='_')
				{
				// just skip past it
				pos++;

				// Future letters should be added to the variable name
				currentName = &varName;
				}
			else
				{
				// Must be a letter - add it to whatever name we're
				// maintaining
				*currentName += *pos++;
				}
			}

		// Make sure we're pointing to the next thing
		// when we leave
		script.Advance();

		// Should now have split the name into a layer and
		// a variable

		// See if the named layer existed
		MusicLayer *namedLayer = track -> GetLayer(layerName.data());

		if (namedLayer)
			{
			// Try and find the named variable within this layer
			return namedLayer -> GetVariable ( varName.data() );
			}
		else
			{
			// No layer, thus no variable
			return (NULL);
			}
		}

	// Was this a simple variable name ?
	if (script.GetCurrentType() == MusicScript::String)
		{
		// Search for the variable in the following order:
		//   Look in the layer (if present)
		//   Then the track (ditto)
		//   Finally the manager

		std::string varName(script.GetCurrentToken());

		// Make sure we're pointing to the next thing
		// when we leave
		script.Advance();

		// Does the layer have a variable with this name ?
		if (layer)
			{
			MusicValue *variable = layer -> GetVariable ( varName.data() );
			if (variable)
				{
				return variable;
				}
			}
		// Now try the track
		if (track)
			{
			MusicValue *variable = track -> GetVariable ( varName.data() );
			if (variable)
				{
				return variable;
				}
			}
		
		// Failing that, look in the manager itself
		return manager->GetVariable( varName.data() );
		}

	// Must have been a constant

	ASSERT(script.GetCurrentType() == MusicScript::Constant);

	// Store the value before skipping over it
	MusicValue constant = script.GetCurrentValue();
	script.Advance();

	// Now create space matching this constant
	// (This will automatically be deleted)
	return CreateConstant(constant);

	}

// ----------------------------------------------------------------------
// Method:		CreateConstant
// Arguments:	contents - contents of new constant
// Returns:		pointer to declared constant
// Description:	Adds a constant to a static list of constants
//				This simplifies expression evaluation, as there is no
//				need to test if the operand is a variable or constant
// ----------------------------------------------------------------------
MusicValue *MusicExpression::CreateConstant(MusicValue constant)
	{
	// Allocate space for a constant...
	MusicValue *newConstant = new MusicValue;

	// ...Fill it with the given value...
	*newConstant = constant;

	// ...and add it to the array
	constants.push_back( newConstant );

	// ... then let the parser know what its pointing to
	return (newConstant);
	}

