/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2007 by Dmitry Tsarkov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "dlTBox.h"

#include <sstream>
#include "dltree.h"
#include "comerror.h"
#include "cppi.h"

#include "globaldef.h"
#include "Reasoner.h"
#include "DLConceptTaxonomy.h"
#include "procTimer.h"
#include "dumpLisp.h"
#include "dumpDIG.h"
#include "logging.h"

TBox :: ~TBox ( void )
{
	// remove all RELATED structures
	for ( RelatedCollection::iterator p = Related.begin(), p_end = Related.end(); p < p_end; ++p )
		delete *p;

	// remove all concepts
	delete pTop;
	delete pBottom;

	// remove aux structures
	delete stdReasoner;
	delete nomReasoner;
	delete pMonitor;
	delete pTax;
}

		/// get unique aux concept
TConcept* TBox :: getAuxConcept ( void )
{
	static unsigned int count;

	std::stringstream name;
	name << " aux" << ++count;
	TConcept* ret = getConcept(name.str());
	ret->setSystem();
	return ret;
}

void TBox :: initTopBottom ( void )
{
	// create BOTTOM concept
	TConcept* p = new TConcept ("BOTTOM");
	p->setId(-1);
	p->pName = p->pBody = bpBOTTOM;
	p->setSatisfiable(false);
	pBottom = p;

	// create TOP concept
	p = new TConcept ("TOP");
	p->setId(-1);
	p->pName = p->pBody = bpTOP;
	p->tsDepth=1;
	p->setSatisfiable(true);
	p->classTag = cttTrueCompletelyDefined;
	pTop = p;
}

void TBox :: prepareReasoning ( void )
{
	// do the preprocessing
	Preprocess();

	// init reasoner (if not exist)
	initReasoner();

	// disallow to add new names into DL
	setForbidUndefinedNames(true);

	// check if it is necessary to dump relevant part TBox
	if ( dumpQuery )
	{
		// set up relevance info
		markAllRelevant();
		std::ofstream of ( "tbox" );
		assert ( of.good() );
		dumpLisp lDump(of);
		dump(&lDump);
		clearRelevanceInfo();
	}

	// init values for SAT tests -- either cache, or consistency check
	DLHeap.setSatOrder();
	setToDoPriorities(/*sat=*/true);
}

bool
TBox :: isSatisfiable ( const TConcept* pConcept )
{
	assert ( pConcept != NULL );

	// logging the startpoint
	if ( LLM.isWritable(llBegSat) )
		LL << "\n--------------------------------------------\n"
			  "Checking satisfiability of '" << pConcept->getName() << "':";
	if ( LLM.isWritable(llGTA) )
		LL << "\n";

	// perform reasoning with a proper logical features
	prepareFeatures ( pConcept, NULL );
	bool result = getReasoner()->runSat ( pConcept->resolveId(), bpTOP );
	clearFeatures();

	CHECK_LL_RETURN_VALUE(llSatResult,result);

#if 1
	LL << "\n";		// usual checking -- time is extra info
#else
	LL << " ";		// time tests -- time is necessary info
#endif

	LL << "The '" << pConcept->getName() << "' concept is ";
	if ( !result )
		LL << "un";
	LL << "satisfiable w.r.t. TBox";

	return result;
}

bool
TBox :: isSubHolds ( const TConcept* pConcept, const TConcept* qConcept )
{
	assert ( pConcept != NULL && qConcept != NULL );

	// logging the startpoint
	if ( LLM.isWritable(llBegSat) )
		LL << "\n--------------------------------------------\nChecking subsumption '"
		   << pConcept->getName() << " [= " << qConcept->getName() << "':";
	if ( LLM.isWritable(llGTA) )
		LL << "\n";

	// perform reasoning with a proper logical features
	prepareFeatures ( pConcept, qConcept );
	bool result = !getReasoner()->runSat ( pConcept->resolveId(), (qConcept ? inverse(qConcept->resolveId()) : bpTOP) );
	clearFeatures();

	CHECK_LL_RETURN_VALUE(llSatResult,result);

#if 1
	LL << "\n";		// usual checking -- time is extra info
#else
	LL << " ";		// time tests -- time is necessary info
#endif

	LL << "The '" << pConcept->getName() << " [= " << qConcept->getName() << "' subsumption";
	if (!result)
			LL << " NOT";
	LL << " holds w.r.t. TBox";

	return result;
}

	// build cache for all named concepts
void TBox :: buildNamedConceptCache ( void )
{
	TsProcTimer pt;
	if ( verboseOutput )
		std::cerr << "\nInit named concept cache: ";

	pt.Start();
	CPPI ind ( Concepts.size()-1 );

	for ( c_const_iterator pc = c_begin(); pc != c_end(); ++pc )
	{
		initCache(*pc);		// make cache for BiPointer[i]
		ind.incIndicator();
	}
	for ( i_const_iterator pi = i_begin(); pi != i_end(); ++pi )
	{
		initCache(*pi);		// make cache for BiPointer[i]
		ind.incIndicator();
	}
	pt.Stop();
	if ( verboseOutput )
		std::cerr << " done in " << pt << " seconds\n";
}

// load init values from config file
void TBox :: readConfig ( const ifOptionSet* Options )
{
	assert ( Options != NULL );	// safety check

// define a macro for registering boolean option
#	define addBoolOption(name)				\
	name = Options->getBool ( #name );		\
	if ( LLM.isWritable(llAlways) )			\
		LL << "Init " #name " = " << name << "\n"

	addBoolOption(useAllNames);
	addBoolOption(useCompletelyDefined);
	addBoolOption(useRelevantOnly);
	addBoolOption(dumpQuery);
	addBoolOption(useDagCache);
	addBoolOption(useRangeDomain);
	addBoolOption(alwaysPreferEquals);

	if ( Axioms.initAbsorptionFlags(Options->getText("absorptionFlags"))
		 || !Axioms.isAbsorptionFlagsCorrect(useRangeDomain) )
		error ( "Incorrect absorption flags given" );

	verboseOutput = false;
#undef addBoolOption
}

/// create (and DAG-ify) temporary concept via its definition
TConcept* TBox :: createTempConcept ( const DLTree* desc )
{
	static const char* const defConceptName = "FaCT++.default";

	// clear the default concept def=desc
	if ( defConcept != NULL )
		removeConcept (defConcept);
//	else
	{
		// we have to add this concept in any cases. So change undefined names mode
		bool old = setForbidUndefinedNames(false);
		defConcept = getConcept(defConceptName);
		setForbidUndefinedNames(old);
	}

//	std::cerr << "Create new temp concept with description =" << desc << "\n";
	assert ( defConcept != NULL );

	// set concept system
	defConcept->setSystem ();

	// create description
	deleteTree ( makeNonPrimitive ( defConcept, clone(desc) ) );

	// build DAG entries for the default concept
	DLHeap.setExpressionCache(false);
	addConceptToHeap ( defConcept );

	// DEBUG_ONLY: print the DAG info
//	std::ofstream debugPrint ( defConceptName );
//	Print (debugPrint);
//	debugPrint << std::endl;

	// check satisfiability of the concept
	initCache ( defConcept );

	return defConcept;
}

/// remove concept from TBox by given EXTERNAL id. @return true in case of failure. WARNING!! tested only for TempConcept!!!
bool TBox :: removeConcept ( TConcept* p )
{
	assert ( p == defConcept);

	// clear DAG and name indeces (if necessary)
	if ( isCorrect (p->pName) )
	{
		clearBPforConcept(p->pName);
		DLHeap.resize (getValue(p->pName));
	}

	if ( Concepts.Remove(p) )
		assert(0);	// can't remove non-last concept

	return false;
}

/// classify temporary concept
bool TBox :: classifyTempConcept ( void )
{
	// prepare told subsumers for it
	initToldSubsumers(defConcept);

	assert ( pTax != NULL );

	// setup taxonomy behaviour flags
	pTax->setCompletelyDefined ( false );	// non-primitive concept
	pTax->setInsertIntoTaxonomy ( false );	// just classify

	// classify the concept
	pTax->classifyEntry (defConcept);

	return false;
}

/// dump QUERY processing time, reasoning statistics and a (preprocessed) TBox
void
TBox :: writeReasoningResult ( std::ostream& o, float time, bool isConsistent ) const
{
	if ( nomReasoner )
	{
		o << "Query processing reasoning statistic: Nominals";
		nomReasoner->writeTotalStatistic(o);
		o << "\n";
	}
	o << "Query processing reasoning statistic: Standard";
	stdReasoner->writeTotalStatistic(o);
	o << "\n";
	if ( isConsistent )
		o << "Required";
	else
		o << "KB is inconsistent. Query is NOT processed. Consistency";

	float sum = preprocTime;
	o << " check done in " << time << " seconds\nof which:\nPreproc. takes "
	  << preprocTime << " seconds";
	if ( nomReasoner )
	{
		o << "\nReasoning NOM:";
		sum += nomReasoner->printReasoningTime(o);
	}
	o << "\nReasoning STD:";
	sum += stdReasoner->printReasoningTime(o);
	o << "\nThe rest takes ";
	float f = time - sum;
	f = ((unsigned long)(f*100))/100.f;
	o << f << " seconds\n";
	Print(o);
}

void TBox :: PrintDagEntrySR ( std::ostream& o, const TRole* p ) const
{
	assert ( p != NULL );
	o << ' ' << p->getName();
}

void TBox :: PrintDagEntry ( std::ostream& o, BipolarPointer p ) const
{
	assert ( isValid (p) );

	// primitive ones -- check first
	if ( p == bpTOP )
	{
		o << " *TOP*";
		return;
	}
	else if ( p == bpBOTTOM )
	{
		o << " *BOTTOM*";
		return;
	}

	// checks inversion
	if ( isNegative(p) )
	{
		o << " (not";
		PrintDagEntry ( o, inverse(p) );
		o << ")";
		return;
	}

	if ( isNamedConcept (p) )
	{
		o << ' ' << getConceptByBP(p)->getName();
		return;
	}

	// not named -- check structure
	const DLVertex& v = DLHeap [getValue(p)];

	switch ( v.Type() )
	{
	case dtTop:
		o << " *TOP*";
		return;

	case dtDataType:
	case dtDataValue:
		o << ' ' << getDataEntryByBP(p)->getName();
		return;

	case dtDataExpr:
		o << ' ' << *getDataEntryByBP(p)->getFacet();
		return;

	case dtIrr:
		o << " (" << v.getTagName();
		PrintDagEntrySR ( o, v.getRole() );
		o << ")";
		return;

	case dtAnd:
		o << " (" << v.getTagName();
		for ( DLVertex::const_iterator q = v.begin(); q != v.end(); ++q )
			PrintDagEntry ( o, *q );
		o << ")";
		return;

	case dtForall:
	case dtLE:
		o << " (" << v.getTagName();
		if ( v.Type() == dtLE )
			o << ' ' << v.getNumberLE();
		PrintDagEntrySR ( o, v.getRole() );
		PrintDagEntry ( o, v.getC() );
		o << ")";
		return;

	default:
		std::cerr << "Error printing vertex of type " << v.getTagName() << "(" << v.Type () << ")";
		assert (0);
		return;	// invalid value
	}
}

void TBox :: PrintConcept ( std::ostream& o, const TConcept* p ) const
{
	// print only relevant concepts
	if ( isValid(p->pName) )
	{
		o << getCTTagName(p->getClassTag()) << '.';

		if ( p->isSingleton() )
			o << '!';

		o << p->getName() << " [" << p->tsDepth
		  << "] " << (p->isNonPrimitive() ? "=" : "[=");

		if ( isValid (p->pBody) )
			PrintDagEntry ( o, p->pBody );

		// if you want to check correctness of translation (print following info)
		// you should comment out RemoveExtraDescription() in TBox::Preprocess()
		// but check hasSynonym assignment
		if ( p->Description != NULL )
			o << (p->isNonPrimitive() ? "\n-=" : "\n-[=") << p->Description;

		o << "\n";
	}
}

void TBox :: PrintAxioms ( std::ostream& o ) const
{
	if ( T_G == bpTOP )
		return;

	o << "Axioms: \nT [=";
	PrintDagEntry ( o, T_G );
}

//-----------------------------------------------------------------------------
//--		DIG interface extension
//-----------------------------------------------------------------------------

	/// implement DIG-like roleFillers query; @return in Js all J st (I,J):R
void
TBox :: getRoleFillers ( TConcept* i, TRole* r, NamesVector& Js ) const
{
	Js.clear();
	TConcept* I = i->resolveSynonym();
	TRole* R = r->resolveSynonym();

	// for all related triples in which I participates,
	// check if triple is labelled by a sub-role of R
	TConcept::RelatedIndex::const_iterator p, p_end;
	for ( p = I->IndexFrom.begin(), p_end = I->IndexFrom.end(); p < p_end; ++p )
	{
		const TRelated& rel = **p;
		if ( *R >= *rel.getRole(/*from=*/true) )
			Js.push_back(rel.b);
	}
	for ( p = I->IndexTo.begin(), p_end = I->IndexTo.end(); p < p_end; ++p )
	{
		const TRelated& rel = **p;
		if ( *R >= *rel.getRole(/*from=*/false) )
			Js.push_back(rel.a);
	}
}
	/// implement DIG-like RelatedIndividuals query; @return Is and Js st (I,J):R
void
TBox :: getRelatedIndividuals ( TRole* r, NamesVector& Is, NamesVector& Js ) const
{
	Is.clear();
	Js.clear();
	TRole* R = r->resolveSynonym();

	// for all related triples
	// check if triple is labelled by a sub-role of R
	for ( RelatedCollection::const_iterator p = Related.begin(), p_end = Related.end(); p < p_end; ++p )
	{
		const TRelated& rel = **p;
		if ( *R >= *rel.getRole(/*from=*/true) )
		{
			Is.push_back(rel.a);
			Js.push_back(rel.b);
		}
		if ( *R >= *rel.getRole(/*from=*/false) )
		{
			Is.push_back(rel.b);
			Js.push_back(rel.a);
		}
	}
}

void TBox :: absorbedPrimitiveConceptDefinitions ( std::ostream& o ) const
{
	dumpDIG DIG(o);

	for ( c_const_iterator pc = c_begin(); pc != c_end(); ++pc )
		if ( (*pc)->isPrimitive() )
		{
			o << "\n<absorbedPrimitiveConceptDefinition>";
			DIG.dumpConcept(*pc);
			dumpExpression ( &DIG, (*pc)->pBody );
			o << "\n</absorbedPrimitiveConceptDefinition>";
		}
}

void TBox :: unabsorbed ( std::ostream& o ) const
{
	dumpDIG DIG(o);
	dumpExpression ( &DIG, getTG() );
}

