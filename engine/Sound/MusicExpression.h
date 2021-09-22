// --------------------------------------------------------------------------
// Filename:	Music Expression.h
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
// --------------------------------------------------------------------------


#ifndef _MUSIC_EXPRESSION_H

#define _MUSIC_EXPRESSION_H


#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER


#include "MusicTypes.h"
#include "MusicErrors.h"
#include <vector>

class MusicManager;
class MusicTrack;
class MusicLayer;
class MusicScript;

class MusicExpression
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicExpression
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicExpression();

		// ----------------------------------------------------------------------
		// Method:		~MusicExpression
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicExpression();

		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script  - script, pointing to the start of an expression
		//				manager - current music manager owning soundtracks
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to Parse the expression from the script
		//				This should be called for an expression Parsed from the
		//				MusicManager itself
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script,
						   MusicManager &manager)
			{return ParseWithScope(script,&manager);}

		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script  - script, pointing to the start of an expression
		//				manager - current music manager owning soundtracks
		//				track   - current soundtrack
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to Parse the expression from the script
		//				This should be called for an expression Parsed from a 
		//				MusicTrack
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script,
						   MusicManager &manager,
						   MusicTrack &track)
			{return ParseWithScope(script,&manager, &track);}

		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script   - script, pointing to the start of an expression
		//				manager* - current music manager owning soundtracks
		//				track*   - current soundtrack
		//				layer*   - current layer within soundtrack
		//						   * can be reference or pointer
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to Parse the expression from the script
		//				This should be called for an expression Parsed from a 
		//				MusicLayer
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script,
						   MusicManager &manager,
						   MusicTrack &track,
						   MusicLayer &layer)
   			{return ParseWithScope(script,&manager, &track, &layer);}

		MusicError Parse(MusicScript &script,
						   MusicManager *manager,
						   MusicTrack *track=NULL,
						   MusicLayer *layer=NULL)
   			{return ParseWithScope(script,manager, track, layer);}

		// ----------------------------------------------------------------------
		// Method:		Evaluate
		// Arguments:	None
		// Returns:		value of the expression
		// Description:	Evaluates the expression
		// ----------------------------------------------------------------------
		MusicValue Evaluate() const;
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
		MusicValue *ParseRValue(MusicScript &script,
								MusicManager *manager,
								MusicTrack *track,
								MusicLayer *layer);

		// ----------------------------------------------------------------------
		// Method:		ParseRValue
		// Arguments:	script - script, pointing to start of expression
		//				manager - current music manager owning soundtracks
		//				track   - current soundtrack
		//				layer   - current layer
		// Returns:		Address of the contents of the value (or NULL if this 
		//				could not be found)
		// Description:	Locates a named variable, or allocates space for a
		//				constant (this will be deleted automatically)
		//				Note that this does not move the script forward
		// ----------------------------------------------------------------------
		MusicValue *ParseRValue(MusicScript &script,
						   MusicManager &manager,
						   MusicTrack &track,
						   MusicLayer &layer);

	private:
		// ----------------------------------------------------------------------
		// list of available operators
		// ----------------------------------------------------------------------
		enum MusicOperator
			{
			// No operator - just return the first operand
			None,

			// Standard mathematical operators
			Add,
			Subtract,
			Multiply,
			Divide,

			// "Modulo" operator in floating point (???)
			Mod,

			// Returns the result of the division, without
			// its fraction part
			Div,

			// Returns a random number in between the two
			// operands
			Random,

			// Returns the result of the expression
			// sin ( ( op1 / op2) * 2 PI )
			// 
			// i.e.
			//	op1 = position along wave
			//  op2 = wavelength
			SineWave,

			// Returns the result of the expression
			// cos ( ( op1 / op2) * 2 PI )
			// 
			// i.e.
			//	op1 = position along wave
			//  op2 = wavelength
			CosineWave,

			// Comparison operators
			Less,
			LessEquals,
			Greater,
			GreaterEquals,
			Equals,

			};

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
		// ----------------------------------------------------------------------
		MusicError ParseWithScope(MusicScript &script,
								  MusicManager *manager,
								  MusicTrack *track=NULL,
								  MusicLayer *layer=NULL);

		// ----------------------------------------------------------------------
		// Method:		ParseOperator
		// Arguments:	script - script, pointing to start of expression
		// Returns:		Operator found (or None)
		// Description:	Identifies the operator at the current script position
		//				Note that this does not move the script forward
		// ----------------------------------------------------------------------
		MusicOperator ParseOperator(MusicScript &script) const;
		


		// ----------------------------------------------------------------------
		// Method:		CreateConstant
		// Arguments:	contents - contents of new constant
		// Returns:		pointer to declared constant
		// Description:	Adds a constant to a static list of constants
		//				This simplifies expression evaluation, as there is no
		//				need to test if the operand is a variable or constant
		// ----------------------------------------------------------------------
		static MusicValue *CreateConstant(MusicValue constant);


		MusicOperator operatorType;

		// ----------------------------------------------------------------------
		// Point to each operand within the soundtrack (these will need to have 
		// been allocated before expressions are Parsed)
		// ----------------------------------------------------------------------
		const MusicValue *operands[2];

		// ----------------------------------------------------------------------
		// constants
		// static array of constants used within expressions
		// **** could have a much more efficient memory system here
		// ----------------------------------------------------------------------
		static std::vector<MusicValue *> constants;
		typedef std::vector<MusicValue *>::iterator constantsIterator;

		// ----------------------------------------------------------------------
		// totalExpressions
		// static reference count of existing expressions, maintained by the
		// constructor and destructor.  When this returns to zero, the list of
		// constants will be deleted
		// ----------------------------------------------------------------------
		static int totalExpressions;

	};

#endif
