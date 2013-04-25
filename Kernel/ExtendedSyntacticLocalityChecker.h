/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2013 by Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EXTENDEDSYNLOCCHECKER_H
#define EXTENDEDSYNLOCCHECKER_H

#include "GeneralSyntacticLocalityChecker.h"

// forward declarations
class UpperBoundDirectEvaluator;
class LowerBoundDirectEvaluator;
class UpperBoundComplementEvaluator;
class LowerBoundComplementEvaluator;

class CardinalityEvaluatorBase: protected SigAccessor, public DLExpressionVisitorEmpty
{
protected:	// members
	UpperBoundDirectEvaluator* UBD;
	LowerBoundDirectEvaluator* LBD;
	UpperBoundComplementEvaluator* UBC;
	LowerBoundComplementEvaluator* LBC;

		/// keep the value here
	int value;

protected:	// methods to
		/// main method to use
	int getValue ( const TDLExpression& expr )
	{
		expr.accept(*this);
		return value;
	}

		/// implementation of evaluation
	int getUpperBoundDirect ( const TDLExpression& expr );
		/// implementation of evaluation
	int getUpperBoundComplement ( const TDLExpression& expr );
		/// implementation of evaluation
	int getLowerBoundDirect ( const TDLExpression& expr );
		/// implementation of evaluation
	int getLowerBoundComplement ( const TDLExpression& expr );

protected:	// visitor helpers
	int isBotEquivalent ( const TDLExpression* expr ) { return getUpperBoundDirect(*expr); }
	int isTopEquivalent ( const TDLExpression* expr ) { return getLowerBoundDirect(*expr); }

		/// helper for entities
	virtual int getEntityValue ( const TNamedEntity* entity ) = 0;
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C ) = 0;
		/// helper for things like >= m R.C
	virtual int getMinUpperBound ( int m, const TDLRoleExpression* R, const TDLExpression* C ) = 0;
		/// helper for things like <= m R.C
	virtual int getMaxUpperBound ( int m, const TDLRoleExpression* R, const TDLExpression* C ) = 0;
		/// helper for things like = m R.C
	virtual int getExactValue ( int m, const TDLRoleExpression* R, const TDLExpression* C ) = 0;

public:		// interface
		/// init c'tor
	CardinalityEvaluatorBase ( const TSignature* s ) : SigAccessor(s), value(0) {}
		/// empty d'tor
	virtual ~CardinalityEvaluatorBase ( void ) {}

		/// set all other evaluators
	void setEvaluators ( UpperBoundDirectEvaluator* pUD, LowerBoundDirectEvaluator* pLD, UpperBoundComplementEvaluator* pUC, LowerBoundComplementEvaluator* pLC )
	{
		UBD = pUD;
		LBD = pLD;
		UBC = pUC;
		LBC = pLC;
		fpp_assert ( (void*)UBD == this || (void*)LBD == this || (void*)UBC == this || (void*)LBC == this );
	}

		/// implementation of evaluation
	int getUpperBoundDirect ( const TDLExpression* expr ) { return getUpperBoundDirect(*expr); }
		/// implementation of evaluation
	int getUpperBoundComplement ( const TDLExpression* expr ) { return getUpperBoundComplement(*expr); }
		/// implementation of evaluation
	int getLowerBoundDirect ( const TDLExpression* expr ) { return getLowerBoundDirect(*expr); }
		/// implementation of evaluation
	int getLowerBoundComplement ( const TDLExpression* expr ) { return getLowerBoundComplement(*expr); }

public:		// visitor implementation: common cases
	// concept expressions
	virtual void visit ( const TDLConceptName& expr )
		{ value = getEntityValue(expr.getEntity()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ value = getMinUpperBound ( 1, expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectForall& expr )
		{ value = getForallValue ( expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ value = getMinUpperBound ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ value = getMaxUpperBound ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
		{ value = getExactValue ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptDataExists& expr )
		{ value = getMinUpperBound ( 1, expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataForall& expr )
		{ value = getForallValue ( expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ value = getMinUpperBound ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ value = getMaxUpperBound ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
		{ value = getExactValue ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleName& expr )
		{ value = getEntityValue(expr.getEntity()); }
		// equivalent to R(x,y) and C(x), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ value = getMinUpperBound ( 1, expr.getOR(), expr.getC() ); }
		// equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ value = getMinUpperBound ( 1, expr.getOR(), expr.getC() ); }

	// data role expressions
	virtual void visit ( const TDLDataRoleName& expr )
		{ value = getEntityValue(expr.getEntity()); }
}; // CardinalityEvaluatorBase

/// determine how many instances can an expression have
class UpperBoundDirectEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// define a special value for concepts that are not in C^{<= n}
	const int getNoneValue ( void ) const { return -1; }
		/// define a special value for concepts that are in C^{<= n} for all n
	const int getAllValue ( void ) const { return 0; }

		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return botCLocal() && nc(entity) ? getAllValue() : getNoneValue(); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
	{
		if ( isTopEquivalent(R) != getNoneValue() && getLowerBoundComplement(C) >= 1 )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like >= m R.C
	virtual int getMinUpperBound ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m > 0 and...
		if ( m <= 0 )
			return getNoneValue();
		// R = \bot or...
		if ( isBotEquivalent(R) != getNoneValue() )
			return getAllValue();
		// C \in C^{<= m-1}
		int ubC = getUpperBoundDirect(C);
		if ( ubC != getNoneValue() && ubC < m )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like <= m R.C
	virtual int getMaxUpperBound ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \top and...
		if ( isTopEquivalent(R) == getNoneValue() )
			return getNoneValue();
		// C\in C^{>= m+1}
		int lbC = getLowerBoundDirect(C);
		if ( lbC != getNoneValue() && lbC > m )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// here the maximal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		return std::max ( getMinUpperBound ( m, R, C ), getMaxUpperBound ( m, R, C ) );
	}

		/// helper for And
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		int min = getNoneValue(), n;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n != getNoneValue() )
				min = min == getNoneValue() ? n : std::min ( min, n );
		}
		return min;
	}
		/// helper for Or
	template<class C>
	int getOrValue ( const TDLNAryExpression<C>& expr )
	{
		int sum = 0, n;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n == getNoneValue() )
				return getNoneValue();
			sum += n;
		}
		return sum;
	}

public:		// interface
		/// init c'tor
	UpperBoundDirectEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~UpperBoundDirectEvaluator ( void ) {}

public:		// visitor implementation
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLConceptBottom& ) { value = getAllValue(); }
	virtual void visit ( const TDLConceptNot& expr ) { value = getUpperBoundComplement(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLConceptOneOf& expr ) { value = expr.size(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptDataValue& expr ) { value = isBotEquivalent(expr.getDR()); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = getAllValue(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getUpperBoundDirect(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) != getNoneValue() )
			{
				value = getAllValue();
				return;
			}
		value = getNoneValue();
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataRoleBottom& ) { value = getAllValue(); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataBottom& ) { value = getAllValue(); }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeName& ) { isBotEq = false; }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeRestriction& ) { isBotEq = false; }
	virtual void visit ( const TDLDataValue& ) { value = 1; }
	virtual void visit ( const TDLDataNot& expr ) { value = getUpperBoundComplement(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLDataOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLDataOneOf& expr ) { value = expr.size(); }
}; // UpperBoundDirectEvaluator

class UpperBoundComplementEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// define a special value for concepts that are not in C^{<= n}
	const int getNoneValue ( void ) const { return -1; }
		/// define a special value for concepts that are in C^{<= n} for all n
	const int getAllValue ( void ) const { return 0; }

		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return topCLocal() && nc(entity) ? getAllValue() : getNoneValue(); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
	{
		if ( isBotEquivalent(R) != getNoneValue() || getUpperBoundComplement(C) == 0 )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like >= m R.C
	int getMinUpperBound ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m == 0 or...
		if ( m == 0 )
			return getAllValue();
		// R = \top and...
		if ( isTopEquivalent(R) == getNoneValue() )
			return getNoneValue();
		// C \in C^{>= m}
		return getLowerBoundDirect(C) >= m ? getAllValue() : getNoneValue();
	}
		/// helper for things like <= m R.C
	int getMaxUpperBound ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \bot or...
		if ( isBotEquivalent(R) != getNoneValue() )
			return getAllValue();
		// C\in C^{<= m}
		int lbC = getUpperBoundDirect(C);
		if ( lbC != getNoneValue() && lbC <= m )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// here the minimal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		return std::min ( getMinUpperBound ( m, R, C ), getMaxUpperBound ( m, R, C ) );
	}

		/// helper for And
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		int sum = 0, n;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n == getNoneValue() )
				return getNoneValue();
			sum += n;
		}
		return sum;
	}
		/// helper for Or
	template<class C>
	int getOrValue ( const TDLNAryExpression<C>& expr )
	{
		int min = getNoneValue(), n;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n != getNoneValue() )
				min = min == getNoneValue() ? n : std::min ( min, n );
		}
		return min;
	}

public:		// interface
		/// init c'tor
	UpperBoundComplementEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~UpperBoundComplementEvaluator ( void ) {}

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLConceptBottom& ) { value = getNoneValue(); }
	virtual void visit ( const TDLConceptNot& expr ) { value = getUpperBoundDirect(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLConceptOneOf& ) { value = getNoneValue(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptDataValue& expr ) { value = isTopEquivalent(expr.getDR()); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = getNoneValue(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getUpperBoundComplement(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( getUpperBoundComplement(*p) == getNoneValue() )
			{
				value = getNoneValue();
				return;
			}
		value = getAllValue();
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLDataRoleBottom& ) { value = getNoneValue(); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLDataBottom& ) { value = getNoneValue(); }
	// FIXME: negated datatype is a union of all other DTs that are infinite
	virtual void visit ( const TDLDataTypeName& ) { value = getNoneValue(); }
	// FIXME: negeted restriction include negated DT
	virtual void visit ( const TDLDataTypeRestriction& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataValue& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataNot& expr ) { value = getUpperBoundDirect(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLDataOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLDataOneOf& ) { value = getNoneValue(); }
}; // UpperBoundComplementEvaluator

class LowerBoundDirectEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// define a special value for concepts that are not in C^{<= n}
	const int getNoneValue ( void ) const { return -1; }
		/// define a special value for concepts that are in C^{<= n} for all n
	const int getAllValue ( void ) const { return 0; }

		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return topCLocal() && nc(entity) ? getAllValue() : getNoneValue(); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
	{
		if ( isBotEquivalent(R) != getNoneValue() || getUpperBoundComplement(C) == 0 )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like >= m R.C
	int getMinUpperBound ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m == 0 or...
		if ( m == 0 )
			return getAllValue();
		// R = \top and...
		if ( isTopEquivalent(R) == getNoneValue() )
			return getNoneValue();
		// C \in C^{>= m}
		return getLowerBoundDirect(C) >= m ? getAllValue() : getNoneValue();
	}
		/// helper for things like <= m R.C
	int getMaxUpperBound ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \bot or...
		if ( isBotEquivalent(R) != getNoneValue() )
			return getAllValue();
		// C\in C^{<= m}
		int lbC = getUpperBoundDirect(C);
		if ( lbC != getNoneValue() && lbC <= m )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// here the minimal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		return std::min ( getMinUpperBound ( m, R, C ), getMaxUpperBound ( m, R, C ) );
	}

		/// helper for And
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		int sum = 0, n;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n == getNoneValue() )
				return getNoneValue();
			sum += n;
		}
		return sum;
	}
		/// helper for Or
	template<class C>
	int getOrValue ( const TDLNAryExpression<C>& expr )
	{
		int min = getNoneValue(), n;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n != getNoneValue() )
				min = min == getNoneValue() ? n : std::min ( min, n );
		}
		return min;
	}

public:		// interface
		/// init c'tor
	LowerBoundDirectEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~LowerBoundDirectEvaluator ( void ) {}
}; // LowerBoundDirectEvaluator

class LowerBoundComplementEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// helper for things like >= m R.C
	int getMinUpperBound ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		return 0;
	}
		/// helper for things like <= m R.C
	int getMaxUpperBound ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		return 0;
	}


public:		// interface
		/// init c'tor
	LowerBoundComplementEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~LowerBoundComplementEvaluator ( void ) {}
}; // LowerBoundComplementEvaluator

/// implementation of evaluation
inline int
CardinalityEvaluatorBase :: getUpperBoundDirect ( const TDLExpression& expr ) { return UBD->getValue(expr); }
/// implementation of evaluation
inline int
CardinalityEvaluatorBase :: getUpperBoundComplement ( const TDLExpression& expr ) { return UBD->getValue(expr); }
/// implementation of evaluation
inline int
CardinalityEvaluatorBase :: getLowerBoundDirect ( const TDLExpression& expr ) { return LBD->getValue(expr); }
/// implementation of evaluation
inline int
CardinalityEvaluatorBase :: getLowerBoundComplement ( const TDLExpression& expr ) { return LBC->getValue(expr); }

#if 0
/// determine how many instances can an expression have
class UpperBoundDirectEvaluator: public CardinalityEvaluatorBase
{
protected:	// members
		/// corresponding complement evaluator
	UpperBoundComplementEvaluator* UpperCEval;

protected:	// methods
		/// check whether the expression is top-equivalent
	bool isTopEquivalent ( const TDLExpression& expr );
		/// convenience helper
	bool isTopEquivalent ( const TDLExpression* expr ) { return isTopEquivalent(*expr); }

	// non-empty Concept/Data expression

		/// @return true iff C^I is non-empty
	bool isBotDistinct ( const TDLExpression* C )
	{
		// TOP is non-empty
		if ( isTopEquivalent(C) )
			return true;
		// built-in DT are non-empty
		if ( dynamic_cast<const TDLDataTypeName*>(C) )
			return true;
		// FIXME!! that's it for now
		return false;
	}

	// cardinality of a concept/data expression interpretation

		/// @return true if #C^I > n
	bool isCardLargerThan ( const TDLExpression* C, unsigned int n )
	{
		if ( n == 0 )	// non-empty is enough
			return isBotDistinct(C);
		if ( const TDLDataTypeName* namedDT = dynamic_cast<const TDLDataTypeName*>(C) )
		{	// string/time are infinite DT
			std::string name = namedDT->getName();
			if ( name == TDataTypeManager::getStrTypeName() || name == TDataTypeManager::getTimeTypeName() )
				return true;
		}
		// FIXME!! try to be more precise
		return false;
	}

	// QCRs

		/// @return true iff (>= n R.C) is botEq
	bool isMinBotEquivalent ( unsigned int n, const TDLRoleExpression* R, const TDLExpression* C )
		{ return (n > 0) && (isBotEquivalent(R) || isBotEquivalent(C)); }
		/// @return true iff (<= n R.C) is botEq
	bool isMaxBotEquivalent ( unsigned int n, const TDLRoleExpression* R, const TDLExpression* C )
		{ return isTopEquivalent(R) && isCardLargerThan ( C, n ); }

public:		// interface
		/// init c'tor
	UpperBoundComplementEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase {}
		/// empty d'tor
	virtual ~UpperBoundComplementEvaluator ( void ) {}

	// set fields

		/// set the corresponding top evaluator
	void setTopEval ( TopEquivalenceEvaluator* eval ) { TopEval = eval; }
		/// @return true iff an EXPRession is equivalent to bottom wrt defined policy
	bool isBotEquivalent ( const TDLExpression& expr )
	{
		expr.accept(*this);
		return isBotEq;
	}
		/// @return true iff an EXPRession is equivalent to bottom wrt defined policy
	bool isBotEquivalent ( const TDLExpression* expr ) { return isBotEquivalent(*expr); }

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { isBotEq = false; }
	virtual void visit ( const TDLConceptBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLConceptName& expr ) { isBotEq = !topCLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLConceptNot& expr ) { isBotEq = isTopEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr )
	{
		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )	// here isBotEq is true, so just return
				return;
		isBotEq = false;
	}
	virtual void visit ( const TDLConceptOr& expr )
	{
		for ( TDLConceptOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )	// here isBotEq is false, so just return
				return;
		isBotEq = true;
	}
	virtual void visit ( const TDLConceptOneOf& expr ) { isBotEq = expr.empty(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectForall& expr )
		{ isBotEq = isTopEquivalent(expr.getOR()) && isBotEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ isBotEq = isMinBotEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ isBotEq = isMaxBotEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLObjectRoleExpression* R = expr.getOR();
		const TDLConceptExpression* C = expr.getC();
		isBotEq = isMinBotEquivalent ( n, R, C ) || isMaxBotEquivalent ( n, R, C );
	}
	virtual void visit ( const TDLConceptDataValue& expr )
		{ isBotEq = isBotEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataForall& expr )
		{ isBotEq = isTopEquivalent(expr.getDR()) && !isTopEquivalent(expr.getExpr()); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ isBotEq = isMinBotEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ isBotEq = isMaxBotEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLDataRoleExpression* R = expr.getDR();
		const TDLDataExpression* D = expr.getExpr();
		isBotEq = isMinBotEquivalent ( n, R, D ) || isMaxBotEquivalent ( n, R, D );
	}

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { isBotEq = false; }
	virtual void visit ( const TDLObjectRoleBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLObjectRoleName& expr ) { isBotEq = !topRLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		isBotEq = true;
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )	// isBotEq is true here
				return;
		isBotEq = false;
	}
		// equivalent to R(x,y) and C(x), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getOR(), expr.getC() ); }
		// equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getOR(), expr.getC() ); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { isBotEq = false; }
	virtual void visit ( const TDLDataRoleBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLDataRoleName& expr ) { isBotEq = !topRLocal() && nc(expr.getEntity()); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { isBotEq = false; }
	virtual void visit ( const TDLDataBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLDataTypeName& ) { isBotEq = false; }
	virtual void visit ( const TDLDataTypeRestriction& ) { isBotEq = false; }
	virtual void visit ( const TDLDataValue& ) { isBotEq = false; }
	virtual void visit ( const TDLDataNot& expr ) { isBotEq = isTopEquivalent(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr )
	{
		for ( TDLDataAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )	// here isBotEq is true, so just return
				return;
		isBotEq = false;
	}
	virtual void visit ( const TDLDataOr& expr )
	{
		for ( TDLDataOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )	// here isBotEq is false, so just return
				return;
		isBotEq = true;
	}
	virtual void visit ( const TDLDataOneOf& expr ) { isBotEq = expr.empty(); }
}; // UpperBoundComplementEvaluator

/// check whether class expressions are equivalent to top wrt given locality class
class TopEquivalenceEvaluator: protected SigAccessor, public DLExpressionVisitorEmpty
{
protected:	// members
		/// corresponding bottom evaluator
	BotEquivalenceEvaluator* BotEval;
		/// keep the value here
	bool isTopEq;

protected:	// methods
		/// check whether the expression is top-equivalent
	bool isBotEquivalent ( const TDLExpression& expr ) { return BotEval->isBotEquivalent(expr); }
		/// convenience helper
	bool isBotEquivalent ( const TDLExpression* expr ) { return isBotEquivalent(*expr); }

	// non-empty Concept/Data expression

		/// @return true iff C^I is non-empty
	bool isBotDistinct ( const TDLExpression* C )
	{
		// TOP is non-empty
		if ( isTopEquivalent(C) )
			return true;
		// built-in DT are non-empty
		if ( dynamic_cast<const TDLDataTypeName*>(C) )
			return true;
		// FIXME!! that's it for now
		return false;
	}

	// cardinality of a concept/data expression interpretation

		/// @return true if #C^I > n
	bool isCardLargerThan ( const TDLExpression* C, unsigned int n )
	{
		if ( n == 0 )	// non-empty is enough
			return isBotDistinct(C);
		if ( dynamic_cast<const TDLDataExpression*>(C) && isTopEquivalent(C) )
			return true;
		if ( const TDLDataTypeName* namedDT = dynamic_cast<const TDLDataTypeName*>(C) )
		{	// string/time are infinite DT
			std::string name = namedDT->getName();
			if ( name == TDataTypeManager::getStrTypeName() || name == TDataTypeManager::getTimeTypeName() )
				return true;
		}
		// FIXME!! try to be more precise
		return false;
	}

	// QCRs

		/// @return true iff (>= n R.C) is topEq
	bool isMinTopEquivalent ( unsigned int n, const TDLRoleExpression* R, const TDLExpression* C )
		{ return (n == 0) || ( isTopEquivalent(R) && isCardLargerThan ( C, n-1 ) ); }
		/// @return true iff (<= n R.C) is topEq
	bool isMaxTopEquivalent ( unsigned int n ATTR_UNUSED, const TDLRoleExpression* R, const TDLExpression* C )
		{ return isBotEquivalent(R) || isBotEquivalent(C); }

public:		// interface
		/// init c'tor
	TopEquivalenceEvaluator ( const TSignature* s ) : SigAccessor(s), isTopEq(false) {}
		/// empty d'tor
	virtual ~TopEquivalenceEvaluator ( void ) {}

	// set fields

		/// set the corresponding bottom evaluator
	void setBotEval ( BotEquivalenceEvaluator* eval ) { BotEval = eval; }
		/// @return true iff an EXPRession is equivalent to top wrt defined policy
	bool isTopEquivalent ( const TDLExpression& expr )
	{
		expr.accept(*this);
		return isTopEq;
	}
		/// @return true iff an EXPRession is equivalent to top wrt defined policy
	bool isTopEquivalent ( const TDLExpression* expr ) { return isTopEquivalent(*expr); }

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { isTopEq = true; }
	virtual void visit ( const TDLConceptBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLConceptName& expr ) { isTopEq = topCLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLConceptNot& expr ) { isTopEq = isBotEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr )
	{
		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )	// here isTopEq is false, so just return
				return;
		isTopEq = true;
	}
	virtual void visit ( const TDLConceptOr& expr )
	{
		for ( TDLConceptOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isTopEquivalent(*p) )	// here isTopEq is true, so just return
				return;
		isTopEq = false;
	}
	virtual void visit ( const TDLConceptOneOf& ) { isTopEq = false; }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectForall& expr )
		{ isTopEq = isTopEquivalent(expr.getC()) || isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ isTopEq = isMinTopEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ isTopEq = isMaxTopEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLObjectRoleExpression* R = expr.getOR();
		const TDLConceptExpression* C = expr.getC();
		isTopEq = isMinTopEquivalent ( n, R, C ) && isMaxTopEquivalent ( n, R, C );
	}
	virtual void visit ( const TDLConceptDataValue& expr )
		{ isTopEq = isTopEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataForall& expr ) { isTopEq = isTopEquivalent(expr.getExpr()) || isBotEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ isTopEq = isMinTopEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ isTopEq = isMaxTopEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLDataRoleExpression* R = expr.getDR();
		const TDLDataExpression* D = expr.getExpr();
		isTopEq = isMinTopEquivalent ( n, R, D ) && isMaxTopEquivalent ( n, R, D );
	}

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { isTopEq = true; }
	virtual void visit ( const TDLObjectRoleBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLObjectRoleName& expr ) { isTopEq = topRLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		isTopEq = false;
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )	// isTopEq is false here
				return;
		isTopEq = true;
	}
		// equivalent to R(x,y) and C(x), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getOR(), expr.getC() ); }
		// equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getOR(), expr.getC() ); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { isTopEq = true; }
	virtual void visit ( const TDLDataRoleBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLDataRoleName& expr ) { isTopEq = topRLocal() && nc(expr.getEntity()); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { isTopEq = true; }
	virtual void visit ( const TDLDataBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLDataTypeName& ) { isTopEq = false; }
	virtual void visit ( const TDLDataTypeRestriction& ) { isTopEq = false; }
	virtual void visit ( const TDLDataValue& ) { isTopEq = false; }
	virtual void visit ( const TDLDataNot& expr ) { isTopEq = isBotEquivalent(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr )
	{
		for ( TDLDataAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )	// here isTopEq is false, so just return
				return;
		isTopEq = true;
	}
	virtual void visit ( const TDLDataOr& expr )
	{
		for ( TDLDataOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isTopEquivalent(*p) )	// here isTopEq is true, so just return
				return;
		isTopEq = false;
	}
	virtual void visit ( const TDLDataOneOf& ) { isTopEq = false; }
}; // TopEquivalenceEvaluator

inline bool
BotEquivalenceEvaluator :: isTopEquivalent ( const TDLExpression& expr )
{
	return TopEval->isTopEquivalent(expr);
}

#endif

/// syntactic locality checker for DL axioms
class ExtendedSyntacticLocalityChecker: public GeneralSyntacticLocalityChecker
{
protected:	// members
	UpperBoundDirectEvaluator UBD;
	LowerBoundDirectEvaluator LBD;
	UpperBoundComplementEvaluator UBC;
	LowerBoundComplementEvaluator LBC;

protected:	// methods
		/// @return true iff EXPR is top equivalent
	virtual bool isTopEquivalent ( const TDLExpression* expr ) { return UBC.getUpperBoundComplement(expr) == 0; }
		/// @return true iff EXPR is bottom equivalent
	virtual bool isBotEquivalent ( const TDLExpression* expr ) { return UBD.getUpperBoundDirect(expr) == 0; }
		/// @return true iff role expression in equivalent to const wrt locality
	bool isREquivalent ( const TDLExpression* expr ) { return topRLocal() ? isTopEquivalent(expr) : isBotEquivalent(expr); }

public:		// interface
		/// init c'tor
	ExtendedSyntacticLocalityChecker ( const TSignature* s )
		: GeneralSyntacticLocalityChecker(s)
		, UBD(s)
		, LBD(s)
		, UBC(s)
		, LBC(s)
	{
		UBD.setEvaluators ( &UBD, &LBD, &UBC, &LBC );
		LBD.setEvaluators ( &UBD, &LBD, &UBC, &LBC );
		UBC.setEvaluators ( &UBD, &LBD, &UBC, &LBC );
		LBC.setEvaluators ( &UBD, &LBD, &UBC, &LBC );
	}
		/// empty d'tor
	virtual ~ExtendedSyntacticLocalityChecker ( void ) {}
}; // ExtendedSyntacticLocalityChecker

#endif
