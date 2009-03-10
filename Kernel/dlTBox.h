/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2009 by Dmitry Tsarkov

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

#ifndef _DLTBOX_H
#define _DLTBOX_H

#include <string>
#include <vector>

#include "tConcept.h"
#include "tIndividual.h"
#include "RoleMaster.h"
#include "LogicFeature.h"
#include "dlDag.h"
#include "ifOptions.h"
#include "tRelated.h"
#include "tNECollection.h"
#include "tAxiomSet.h"
#include "DataTypeCenter.h"
#include "tProgressMonitor.h"
#include "tKBFlags.h"

class DlSatTester;
class DLConceptTaxonomy;
class dumpInterface;
class modelCacheSingleton;

/// enumeration for the reasoner status
enum KBStatus
{
	kbEmpty,		// no axioms loaded yet; not used in TBox
	kbLoading,		// axioms are added to the KB, no preprocessing done
	kbCChecked,		// KB is preprocessed and consistency checked
	kbClassified,	// KB is classified
	kbRealised,		// KB is realised
};

class TBox
{
	friend class Precompletor;
	friend class DlSatTester;
	friend class ReasoningKernel;
	friend class TAxiom;	// FIXME!! while TConcept can't get rid of told cycles

public:	// interface
		/// type for DISJOINT-like statements
	typedef std::vector<DLTree*> ConceptSet;
		/// set of concepts together with creation methods
	typedef TNECollection<TConcept> ConceptCollection;
		/// vector of CONCEPT-like elements
	typedef std::vector<TConcept*> ConceptVector;
		/// vector of SINGLETON-like elements
	typedef std::vector<TIndividual*> SingletonVector;
		/// type for the array of Related elements
	typedef std::vector<TRelated*> RelatedCollection;
		/// type for a collection of DIFFERENT individuals
	typedef std::vector<SingletonVector> DifferentIndividuals;
		/// return type for a set of names
	typedef std::vector<TNamedEntry*> NamesVector;

protected:	// types
		/// collection of individuals
	class IndividualCollection: public TNECollection<TIndividual>
	{
	protected:	// methods
			/// virtual method for additional tuning of newly created element
		virtual void registerNew ( TIndividual* p ATTR_UNUSED ) {}

	public:		// interface
			/// c'tor: clear 0-th element
		IndividualCollection ( void ) : TNECollection<TIndividual>("individual") {}
			/// empty d'tor: all elements will be deleted in other place
		virtual ~IndividualCollection ( void ) {}
	}; // IndividualCollection

		/// class for simple rules like Ch :- Cb1, Cbi, CbN; all C are primitive named concepts
	class TSimpleRule
	{
	public:		// type interface
			/// type for the rule body
		typedef TBox::ConceptVector TRuleBody;
			/// RW iterator over body
		typedef TRuleBody::iterator iterator;
			/// RO iterator over body
		typedef TRuleBody::const_iterator const_iterator;

	public:		// members
			/// body of the rule
		TRuleBody Body;
			/// head of the rule as a DLTree
		DLTree* tHead;
			/// head of the rule as a BP
		BipolarPointer bpHead;

	private:	// no copy
			/// no copy c'tor
		TSimpleRule ( const TSimpleRule& );
			/// no assignment
		TSimpleRule& operator= ( const TSimpleRule& );

	public:		// interface
			/// init c'tor
		TSimpleRule ( const TRuleBody& body, DLTree* head )
			: Body(body)
			, tHead(head)
			, bpHead(bpINVALID)
			{}
			/// empty d'tor
		virtual ~TSimpleRule ( void ) { deleteTree(tHead); }

		// apply rule -- implementation in Reasoner.h

			/// allow reasoner to check the applicability according to the type of the rule
		virtual bool applicable ( DlSatTester& Reasoner ) const;
	}; // TSimpleRule

		/// all simple rules in KB
	typedef std::vector<TSimpleRule*> TSimpleRules;

protected:	// typedefs
		/// RW concept iterator
	typedef ConceptCollection::iterator c_iterator;
		/// RO concept iterator
	typedef ConceptCollection::const_iterator c_const_iterator;
		/// RW individual iterator
	typedef IndividualCollection::iterator i_iterator;
		/// RO individual iterator
	typedef IndividualCollection::const_iterator i_const_iterator;

protected:	// members
	TLabeller relevance;

	DLDag DLHeap;

		/// reasoner for TBox-related queries w/o nominals
	DlSatTester* stdReasoner;
		/// reasoner for TBox-related queries with nominals
	DlSatTester* nomReasoner;
		/// progress monitor
	TProgressMonitor* pMonitor;

	/// taxonomy structure of a TBox
	DLConceptTaxonomy* pTax;
		/// DataType center
	DataTypeCenter DTCenter;
	/// set of reasoning options
	const ifOptionSet* pOptions;
		/// status of the KB
	KBStatus Status;

		/// global KB features
	LogicFeatures KBFeatures;
		/// GCI features
	LogicFeatures GCIFeatures;
		/// nominal cloud features
	LogicFeatures NCFeatures;
		/// aux features
	LogicFeatures auxFeatures;
		/// pointer to current feature (in case of local ones)
	LogicFeatures* curFeature;

	// auxiliary concepts for Taxonomy
	TConcept* pTop;
	TConcept* pBottom;

	/// temporary concept
	TConcept* defConcept;

	/** all named concepts */
	ConceptCollection Concepts;
		/// all named individuals/nominals
	IndividualCollection Individuals;
	RoleMaster RM;
	TAxiomSet Axioms;
		/// given individual-individual relations
	RelatedCollection RelatedI;
		/// known disjoint sets of individuals
	DifferentIndividuals Different;
		/// all simple rules in KB
	TSimpleRules SimpleRules;

		/// internalisation of a general axioms
	BipolarPointer T_G;
		/// KB flags about GCIs
	TKBFlags GCIs;

		/// current axiom's ID
	unsigned int axiomId;

		/// fairness constraints
	ConceptVector Fairness;

	/////////////////////////////////////////////////////
	// Flags section
	/////////////////////////////////////////////////////
		/// flag for full/short KB
	bool useRelevantOnly;
		/// flag for using native range and domain support
	bool useRangeDomain;
		/// flag for creating taxonomy
	bool useCompletelyDefined;
		/// flag for dumping TBox relevant to query
	bool dumpQuery;
		/// whether or not we need classification. Set up in checkQueryNames()
	bool needClassification;
		/// shall we prefer C=D axioms to C[=E in definition of concepts
	bool alwaysPreferEquals;
		/// shall verbose output be used
	bool verboseOutput;
		/// whether we use sorted reasoning; depends on some simplifications
	bool useSortedReasoning;
		/// flag whether TBox is GALEN-like
	bool isLikeGALEN;
		/// flag whether TBox is WINE-like
	bool isLikeWINE;
		/// flag whether precompletion should be used
	bool usePrecompletion;

		/// whether KB is consistent
	bool Consistent;
		/// whether KB(ABox) is precompleted
	bool Precompleted;

		/// time spend for preprocessing
	float preprocTime;

private:	// no copy
		/// no copy c'tor
	TBox ( const TBox& );
		/// no assignment
	TBox& operator = ( const TBox& );

protected:	// methods
		/// init all flags using given set of options
	void readConfig ( const ifOptionSet* Options );
		/// initialise Top and Bottom internal concepts
	void initTopBottom ( void );


//-----------------------------------------------------------------------------
//--		internal iterators
//-----------------------------------------------------------------------------

		/// RW begin() for concepts
	c_iterator c_begin ( void ) { return Concepts.begin(); }
		/// RW end() for concepts
	c_iterator c_end ( void ) { return Concepts.end(); }
		/// RO begin() for concepts
	c_const_iterator c_begin ( void ) const { return Concepts.begin(); }
		/// RO end() for concepts
	c_const_iterator c_end ( void ) const { return Concepts.end(); }

		/// RW begin() for individuals
	i_iterator i_begin ( void ) { return Individuals.begin(); }
		/// RW end() for individuals
	i_iterator i_end ( void ) { return Individuals.end(); }
		/// RO begin() for individuals
	i_const_iterator i_begin ( void ) const { return Individuals.begin(); }
		/// RO end() for individuals
	i_const_iterator i_end ( void ) const { return Individuals.end(); }

//-----------------------------------------------------------------------------
//--		internal ensure*-like interface
//-----------------------------------------------------------------------------

		/// @return true if given name is registered as a concept-like structure in given TBox
	bool isRegisteredConcept ( const TNamedEntry* name ) const { return Concepts.isRegistered(name); }

		/// @return concept by given Named Entry ID
	TConcept* toConcept ( TNamedEntry* id ) { return static_cast<TConcept*>(id); }
		/// @return concept by given Named Entry ID
	const TConcept* toConcept ( const TNamedEntry* id ) const { return static_cast<const TConcept*>(id); }
		/// @return individual by given Named Entry ID
	TIndividual* toIndividual ( TNamedEntry* id ) { return static_cast<TIndividual*>(id); }
		/// @return individual by given Named Entry ID
	const TIndividual* toIndividual ( const TNamedEntry* id ) const { return static_cast<const TIndividual*>(id); }
		/// get TOP/BOTTOM/CN by the DLTree entry
	TConcept* getCI ( const DLTree* name )
	{
		if ( name->Element() == TOP )
			return pTop;
		if ( name->Element() == BOTTOM )
			return pBottom;

		if ( !isName(name) )
			return NULL;

		if ( name->Element().getToken() == CNAME )
			return toConcept(name->Element().getName());
		else
			return toIndividual(name->Element().getName());
	}

//-----------------------------------------------------------------------------
//--		internal BP-to-concept interface
//-----------------------------------------------------------------------------

		/// set P as a concept corresponding BP
	void setBPforConcept ( BipolarPointer bp, TConcept* p )
	{
		DLHeap[bp].setConcept(p);
		p->pName = bp;
	}

		/// get concept by it's BP (non-const version)
	TDataEntry* getDataEntryByBP ( BipolarPointer bp )
	{
		TDataEntry* p = static_cast<TDataEntry*>(DLHeap[bp].getConcept());
		assert ( p != NULL );
		return p;
	}
		/// get concept by it's BP (const version)
	const TDataEntry* getDataEntryByBP ( BipolarPointer bp ) const
	{
		const TDataEntry* p = static_cast<const TDataEntry*>(DLHeap[bp].getConcept());
		assert ( p != NULL );
		return p;
	}


//-----------------------------------------------------------------------------
//--		internal concept building interface
//-----------------------------------------------------------------------------

		/// add description to a concept; @return true in case of error
	bool initNonPrimitive ( TConcept* p, DLTree* desc )
	{
		if ( !p->canInitNonPrim(desc) )
			return true;
		// delete return value in case of duplicated desc
		deleteTree(makeNonPrimitive(p,desc));
		return false;
	}
		/// make concept non-primitive; @return it's old description
	DLTree* makeNonPrimitive ( TConcept* p, DLTree* desc )
	{
		DLTree* ret = p->makeNonPrimitive(desc);
		checkEarlySynonym(p);
		return ret;
	}
	/// checks if C is defined as C=D and set Synonyms accordingly
	void checkEarlySynonym ( TConcept* p )
	{
		if ( p->isSynonym() )
			return;	// nothing to do
		if ( p->isPrimitive() )
			return;	// couldn't be a synonym
		if ( !isCN (p->Description) )
			return;	// complex expression -- not a synonym(imm.)

		p->setSynonym(getCI(p->Description));
		p->initToldSubsumers();
	}

	/// remove concept from TBox by given EXTERNAL id. @return true in case of failure. WARNING!! tested only for TempConcept!!!
	bool removeConcept ( TConcept* p );

//-----------------------------------------------------------------------------
//--		support for n-ary predicates
//-----------------------------------------------------------------------------

		/// build a construction in the form AND (\neg q_i)
	template<class Iterator>
	DLTree* buildDisjAux ( Iterator beg, Iterator end )
	{
		DLTree* t = new DLTree(TOP);
		for ( Iterator i = beg; i < end; ++i )
			t = createSNFAnd ( t, createSNFNot(clone(*i)) );
		return t;
	}
		/// process a disjoint set [beg,end) in a usual manner
	template<class Iterator>
	void processDisjoint ( Iterator beg, Iterator end )
	{
		for ( Iterator i = beg; i < end; ++i )
			addSubsumeAxiom ( *i, buildDisjAux ( i+1, end ) );
	}
//-----------------------------------------------------------------------------
//--		internal DAG building methods
//-----------------------------------------------------------------------------

		/// build a DAG-structure for concepts and axioms
	void buildDAG ( void );

		/// translate concept P (together with definition) to DAG representation
	void addConceptToHeap ( TConcept* p );
		/// register data-related expression in the DAG; @return it's DAG index
	BipolarPointer addDataExprToHeap ( TDataEntry* p );
		/// builds DAG entry by general concept expression
	BipolarPointer tree2dag ( const DLTree* );
		/// create forall node (together with transitive sub-roles entries)
	BipolarPointer forall2dag ( const TRole* R, BipolarPointer C );
		/// create atmost node (together with NN-rule entries)
	BipolarPointer atmost2dag ( unsigned int n, const TRole* R, BipolarPointer C );
		/// create REFLEXIVE node
	BipolarPointer reflexive2dag ( const TRole* R )
	{
		// input check: only simple roles are allowed in the reflexivity construction
		if ( !R->isSimple() )
			throw EFPPNonSimpleRole(R->getName());
		return inverse ( DLHeap.add ( new DLVertex ( dtIrr, R ) ) );
	}
		/// fills AND-like vertex V with an AND-like expression T; process result
	BipolarPointer and2dag ( DLVertex* v, const DLTree* t );
		/// process AND-like expression T
	BipolarPointer and2dag ( const DLTree* t ) { return and2dag ( new DLVertex(dtAnd), t ); }
		/// add elements of T to and-like vertex V; @return true if clash occures
	bool fillANDVertex ( DLVertex* v, const DLTree* t );
		/// create forall node for data role
	BipolarPointer dataForall2dag ( const TRole* R, BipolarPointer C )
		{ return DLHeap.add ( new DLVertex ( dtForall, 0, R, C ) ); }
		/// create atmost node for data role
	BipolarPointer dataAtMost2dag ( unsigned int n, const TRole* R, BipolarPointer C )
		{ return DLHeap.add ( new DLVertex ( dtLE, n, R, C ) ); }
		/// @return a pointer to concept representation
	BipolarPointer concept2dag ( TConcept* p )
	{
		if ( p == NULL )
			return bpINVALID;
		if ( !isValid(p->pName) )
			addConceptToHeap(p);
		return p->resolveId();
	}

//-----------------------------------------------------------------------------
//--		internal parser (input) interface
//-----------------------------------------------------------------------------

		/// tries to apply axiom D [= CN; @return true if applicable
	bool applyAxiomCToCN ( DLTree* D, DLTree*& CN );
		/// tries to apply axiom CN [= D; @return true if applicable
	bool applyAxiomCNToC ( DLTree*& CN, DLTree* D );
		/// tries to add LEFT = RIGHT for the concept LEFT; @return true if OK
	bool addNonprimitiveDefinition ( DLTree* left, DLTree* right );
		/// tries to add LEFT = RIGHT for the concept LEFT [= X; @return true if OK
	bool switchToNonprimitive ( DLTree* left, DLTree* right );

	// for complex Concept operations
		/// try to absorb GCI C[=D; if not possible, just record this GCI
	void processGCI ( DLTree* C, DLTree* D ) { Axioms.addAxiom ( C, D ); }

	// recognize Range/Domain restriction in an axiom and transform it into role R&D.
	// return true if transformation was performed
	bool axiomToRangeDomain ( DLTree* l, DLTree* r );

		/// @return true if BP represents a named entry in a DAG
	bool isNamedConcept ( BipolarPointer bp ) const
	{
		DagTag tag = DLHeap[bp].Type();
		return isCNameTag(tag) || tag == dtDataType || tag == dtDataValue;
	}

		/// check if TBox contains too many GCIs to switch strategy
	bool isGalenLikeTBox ( void ) const { return isLikeGALEN; }
		/// check if TBox contains too many nominals and GCIs to switch strategy
	bool isWineLikeTBox ( void ) const { return isLikeWINE; }

//-----------------------------------------------------------------------------
//--		internal preprocessing methods
//-----------------------------------------------------------------------------

		/// build a roles taxonomy and a DAG
	void Preprocess ( void );
		/// absorb all axioms and set hasGCI
	void AbsorbAxioms ( void )
	{
		unsigned int nSynonyms = countSynonyms();
		GCIs.setGCI(Axioms.absorb());
		if ( countSynonyms() > nSynonyms )
			replaceAllSynonyms();
		if ( Axioms.wasRoleAbsorptionApplied() )
			initToldSubsumers();
	}

		/// pre-process RELATED axioms: resolve synonyms, mark individuals as related
	void preprocessRelated ( void );
		/// perform precompletion;
	void performPrecompletion ( void );
		/// determine all sorts in KB (make job only for SORTED_REASONING)
	void determineSorts ( void );

		/// calculate statistic for DAG and Roles
	void CalculateStatistic ( void );
		/// Remove DLTree* from TConcept after DAG is constructed
	void RemoveExtraDescriptions ( void );

		/// init Range and Domain for all roles; sets hasGCI if R&D was found
	void initRangeDomain ( void );
		/// init functional roles with functional entries
	void initFunctionalRoles ( void );

		/// build related roles cache for given individual
	void buildRelatedRoles ( TIndividual* i ) const
		{ i->RelatedRoleMap = RM.buildRelatedRoles ( i->RelatedIndex.begin(), i->RelatedIndex.end() ); }
		/// build related roles cache for every individual
	void buildRelatedRoles ( void ) const
	{
		for ( i_const_iterator pi = i_begin(); pi != i_end(); ++pi )
			if ( !(*pi)->isSynonym() )
				buildRelatedRoles(*pi);
	}

		/// set told TOP concept whether necessary
	void initToldSubsumers ( void )
	{
		for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
			if ( !(*pc)->isSynonym() )
				(*pc)->initToldSubsumers();
		for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
			if ( !(*pi)->isSynonym() )
				(*pi)->initToldSubsumers();
	}
		/// set told TOP concept whether necessary
	void setToldTop ( void )
	{
		TConcept* top = const_cast<TConcept*>(pTop);
		for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
			(*pc)->setToldTop(top);
		for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
			(*pi)->setToldTop(top);
	}
		/// calculate TS depth for all concepts
	void calculateTSDepth ( void )
	{
		for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
			(*pc)->calculateTSDepth();
		for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
			(*pi)->calculateTSDepth();
	}

		/// find referential cycles (like A [= (and B C), B [= A) and transform them to synonyms (B=A, A[=C)
	void transformToldCycles ( void );
		/// check if P appears in referential cycle;
		/// @return concept which creates cycle, NULL if no such concept exists.
	TConcept* checkToldCycle ( TConcept* p );
		/// transform i [= C [= j into i=C=j for i,j nominals
	void transformSingletonHierarchy ( void );
		/// make P and all its non-singleton parents synonyms to its singleton parent; @return that singleton
	TIndividual* transformSingletonWithSP ( TConcept* p );
		/// helper to the previous function
	TIndividual* getSPForConcept ( TConcept* p );

		/// @return number of synonyms in the KB
	unsigned int countSynonyms ( void ) const
	{
		unsigned int nSynonyms = 0;
		for ( c_const_iterator pc = c_begin(); pc != c_end(); ++pc )
			if ( (*pc)->isSynonym() )
				++nSynonyms;

		for ( i_const_iterator pi = i_begin(); pi != i_end(); ++pi )
			if ( (*pi)->isSynonym() )
				++nSynonyms;
		return nSynonyms;
	}
		/// replace all synonyms in concept descriptions with their definitions
	void replaceAllSynonyms ( void );

		/// init Extra Rule field in concepts given by a vector V with a given INDEX
	inline void
	initRuleFields ( const ConceptVector& v, unsigned int index ) const
	{
		for ( ConceptVector::const_iterator q = v.begin(), q_end = v.end(); q < q_end; ++q )
			(*q)->addExtraRule(index);
	}
		/// mark all concepts wrt their classification tag
	void fillsClassificationTag ( void )
	{
		for ( c_const_iterator pc = c_begin(); pc != c_end(); ++pc )
			(*pc)->getClassTag();
		for ( i_const_iterator pi = i_begin(); pi != i_end(); ++pi )
			(*pi)->getClassTag();
	}

//-----------------------------------------------------------------------------
//--		internal reasoner-related interface
//-----------------------------------------------------------------------------

		/// get RW reasoner wrt nominal case
	DlSatTester* getReasoner ( void )
	{
		assert ( curFeature != NULL );
		if ( curFeature->hasSingletons() )
			return nomReasoner;
		else
			return stdReasoner;
	}
		/// set ToDo priorities using OPTIONS
	void setToDoPriorities ( void );		// implemented in Reasoner.h
		/// check whether KB is consistent; @return true if it is
	bool performConsistencyCheck ( void );	// implemented in Reasoner.h

//-----------------------------------------------------------------------------
//--		internal reasoning interface
//-----------------------------------------------------------------------------

		/// init reasoning service: create reasoner(s)
	void initReasoner ( void );				// implemented in Reasoner.h
		/// init taxonomy (for the first time)
	void initTaxonomy ( void );				// implemented in DLConceptTaxonomy.h
		/// creating taxonomy for given TBox; include individuals if necessary
	void createTaxonomy ( bool needIndividuals );
		/// classify all concepts from given COLLECTION with given CD value
	void classifyConcepts ( const ConceptVector& collection, bool curCompletelyDefined, const char* type );

//-----------------------------------------------------------------------------
//--		internal cache-related methods
//-----------------------------------------------------------------------------

		/// init [singleton] cache for given concept implementation
	void initSingletonCache ( BipolarPointer p );	// implemented in Reasoner.h
		/// create cache for ~C where C is a primitive concept (as it is simple)
	void buildSimpleCache ( void );

//-----------------------------------------------------------------------------
//--		internal output helper methods
//-----------------------------------------------------------------------------

	void PrintDagEntry ( std::ostream& o, BipolarPointer p ) const;
	void PrintRoles ( std::ostream& o ) const;
		/// print one concept-like entry
	void PrintConcept ( std::ostream& o, const TConcept* p ) const;
		/// print all registered concepts
	void PrintConcepts ( std::ostream& o ) const
	{
		if ( Concepts.size() == 0 )
			return;
		o << "Concepts (" << Concepts.size() << "):\n";
		for ( c_const_iterator pc = c_begin(); pc != c_end(); ++pc )
			PrintConcept(o,*pc);
	}
		/// print all registered individuals
	void PrintIndividuals ( std::ostream& o ) const
	{
		if ( Individuals.size() == 0 )
			return;
		o << "Individuals (" << Individuals.size() << "):\n";
		for ( i_const_iterator pi = i_begin(); pi != i_end(); ++pi )
			PrintConcept(o,*pi);
	}
	void PrintSimpleRules ( std::ostream& o ) const
	{
		if ( SimpleRules.empty() )
			return;
		o << "Simple rules (" << SimpleRules.size() << "):\n";
		for ( TSimpleRules::const_iterator p = SimpleRules.begin(); p < SimpleRules.end(); ++p )
		{
			ConceptVector::const_iterator q = (*p)->Body.begin(), q_end = (*p)->Body.end();
			o << "(" << (*q)->getName();
			while ( ++q < q_end )
				o << ", " << (*q)->getName();
			o << ") => " << (*p)->tHead << "\n";
		}
	}
	void PrintAxioms ( std::ostream& o ) const
	{
		if ( T_G == bpTOP )
			return;
		o << "Axioms:\nT [=";
		PrintDagEntry ( o, T_G );
	}

//-----------------------------------------------------------------------------
//--		 save/load support; implementation in SaveLoad.cpp
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//--		internal relevance helper methods
//-----------------------------------------------------------------------------
		/// is given concept relevant wrt current TBox
	bool isRelevant ( const TConcept* p ) const { return p->isRelevant(relevance); }
		/// set given concept relevant wrt current TBox
	void setRelevant1 ( TConcept* p );
		/// set given concept relevant wrt current TBox if not checked yet
	void setRelevant ( TConcept* p ) { if ( !isRelevant(p) ) setRelevant1(p); }

		/// is given role relevant wrt current TBox
	bool isRelevant ( const TRole* p ) const { return p->isRelevant(relevance); }
		/// set given role relevant wrt current TBox
	void setRelevant1 ( TRole* p );
		/// set given role relevant wrt current TBox if not checked yet
	void setRelevant ( TRole* p ) { if ( !isRelevant(p) ) setRelevant1(p); }

		/// set given DAG entry relevant wrt current TBox
	void setRelevant ( BipolarPointer p );

		/// gather information about logical features of relevant concept
	void collectLogicFeature ( const TConcept* p ) const
	{
		if ( curFeature )
			curFeature->fillConceptData(p);
	}
		/// gather information about logical features of relevant role
	void collectLogicFeature ( const TRole* p ) const
	{
		if ( curFeature )	// update features w.r.t. current concept
			curFeature->fillRoleData ( p, isRelevant(p->inverse()) );
	}
		/// gather information about logical features of relevant DAG entry
	void collectLogicFeature ( const DLVertex& v, bool pos ) const
	{
		if ( curFeature )
			curFeature->fillDAGData ( v, pos );
	}
		/// mark all active GCIs relevant
	void markGCIsRelevant ( void ) { setRelevant(T_G); }

//-----------------------------------------------------------------------------
//--		internal relevance interface
//-----------------------------------------------------------------------------
		/// set all TBox content (namely, concepts and GCIs) relevant
	void markAllRelevant ( void )
	{
		for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
			setRelevant(*pc);
		for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
			setRelevant(*pi);

		markGCIsRelevant();
	}
		/// mark chosen part of TBox (P, Q and GCIs) relevant
	void calculateRelevant ( TConcept* p, TConcept* q )
	{
		setRelevant(p);
		if ( q != NULL )
			setRelevant(q);
		markGCIsRelevant();
	}
		/// clear all relevance info
	void clearRelevanceInfo ( void ) { relevance.newLab(); }
		/// gather relevance statistic for the whole KB
	void gatherRelevanceInfo ( void );
		/// put relevance information to a concept's data
	void setConceptRelevant ( TConcept* p )
	{
		curFeature = &p->posFeatures;
		setRelevant(p->pBody);
		KBFeatures |= p->posFeatures;
		collectLogicFeature(p);
		clearRelevanceInfo();

		// nothing to do for neg-prim concepts
		if ( p->isPrimitive() )
			return;

		curFeature = &p->negFeatures;
		setRelevant(inverse(p->pBody));
		KBFeatures |= p->negFeatures;
		clearRelevanceInfo();
	}
		/// update AUX features with the given one; update roles if necessary
	void updateAuxFeatures ( const LogicFeatures& lf )
	{
		if ( !lf.empty() )
		{
			auxFeatures |= lf;
			auxFeatures.mergeRoles();
		}
	}
		/// prepare features for SAT(P), or SUB(P,Q) test
	void prepareFeatures ( const TConcept* pConcept, const TConcept* qConcept );
		/// clear current features
	void clearFeatures ( void ) { curFeature = NULL; }

//-----------------------------------------------------------------------------
//--		internal dump output interface
//-----------------------------------------------------------------------------
		/// dump concept-like essence using given dump method
	void dumpConcept ( dumpInterface* dump, const TConcept* p ) const;
		/// dump role-like essence using given dump method
	void dumpRole ( dumpInterface* dump, const TRole* p ) const;
		/// dump general concept expression using given dump method
	void dumpExpression ( dumpInterface* dump, BipolarPointer p ) const;
		/// dump all (relevant) roles
	void dumpAllRoles ( dumpInterface* dump ) const;

public:
	/// C'tor
	TBox ( const ifOptionSet* Options );
	/// D'tor
	~TBox ( void );

		/// get RW access to used Role Master
	RoleMaster* getRM ( void ) { return &RM; }
		/// get RO access to used Role Master
	const RoleMaster* getRM ( void ) const { return &RM; }

//-----------------------------------------------------------------------------
//--		public parser ensure* interface
//-----------------------------------------------------------------------------

		/// return registered concept by given NAME; @return NULL if can't register
	TConcept* getConcept ( const std::string& name ) { return Concepts.get(name); }
		/// return registered individual by given NAME; @return NULL if can't register
	TIndividual* getIndividual ( const std::string& name ) { return Individuals.get(name); }

		/// @return true iff given name is a registered individual
	bool isIndividual ( const TNamedEntry* name ) const { return Individuals.isRegistered(name); }
		/// @return true iff given DLTree represents a registered individual
	bool isIndividual ( const DLTree* entry ) const
		{ return (entry->Element().getToken() == INAME && isIndividual(entry->Element().getName())); }
		/// @return true iff given DLTree represents a data value
	static bool isDataValue ( const DLTree* entry )
	{
		return entry->Element().getToken() == DATAEXPR &&
			static_cast<const TDataEntry*>(entry->Element().getName())->isDataValue();
	}

		/// get a DL tree by a given concept-like C
	DLTree* getTree ( TConcept* C ) const
	{
		if ( C == NULL )
			return NULL;
		if ( C == pTop )
			return new DLTree(TOP);
		if ( C == pBottom )
			return new DLTree(BOTTOM);
		return new DLTree ( TLexeme ( isIndividual(C) ? INAME : CNAME, C ) );
	}

	// n-ary absorption support

		/// get unique aux concept
	TConcept* getAuxConcept ( void );
		/// replace (AR:C) with X such that C [= AR^-:X for fresh X. @return X
	TConcept* replaceForall ( DLTree* R, DLTree* C );

//-----------------------------------------------------------------------------
//--		public parser (input) interface
//-----------------------------------------------------------------------------

		/// set the ID of the axiom to be processed next
	void setAxiomId ( unsigned int id ) { axiomId = id; }

		/// set the flag that forbid usage of undefined names for concepts/roles; @return old value
	bool setForbidUndefinedNames ( bool val )
	{
		RM.setUndefinedNames(!val);
		DTCenter.setLocked(val);
		Individuals.setLocked(val);
		return Concepts.setLocked(val);
	}

	bool RegisterInstance ( TNamedEntry* name, DLTree* Desc )
	{
		if ( !isIndividual(name) )
			return true;
		addSubsumeAxiom ( toConcept(name), Desc );
		return false;
	}
	bool RegisterIndividualRelation ( TNamedEntry* a, TNamedEntry* R, TNamedEntry* b )
	{
		if ( !isIndividual(a) || !isIndividual(b) )
			return true;
		RelatedI.push_back ( new
			TRelated ( toIndividual(a),
					   toIndividual(b),
					   static_cast<TRole*>(R) ) );
		RelatedI.push_back ( new
			TRelated ( toIndividual(b),
					   toIndividual(a),
					   static_cast<TRole*>(R)->inverse() ) );
		return false;
	}

		/// add general subsumption axiom C [= D
	void addSubsumeAxiom ( DLTree* C, DLTree* D );
		/// add axiom CN [= D for concept CN
	void addSubsumeAxiom ( TConcept* C, DLTree* D ) { addSubsumeAxiom ( getTree(C), D ); }
		/// add an axiom CN [= D for defined CN (CN=E already in base)
	void addSubsumeForDefined ( TConcept* C, DLTree* D );
		/// add an axiom C = D
	void addEqualityAxiom ( DLTree* left, DLTree* right );

		/// add simple rule RULE to the TBox' rules
	inline
	void addSimpleRule ( TSimpleRule* Rule )
	{
		initRuleFields ( Rule->Body, SimpleRules.size() );
		SimpleRules.push_back(Rule);
	}
		/// add binary simple rule (C0,C1)=>H
	void addBSimpleRule ( TConcept* C0, TConcept* C1, DLTree* H )
	{
		ConceptVector v;
		v.push_back(C0);
		v.push_back(C1);
		addSimpleRule ( new TSimpleRule ( v, H ) );
	}

	// external-set methods for set-of-concept-expressions
	void processEquivalent ( const ConceptSet& v );
	void processDisjoint ( const ConceptSet& v );
	void processEquivalentR ( const ConceptSet& v );
	void processDisjointR ( const ConceptSet& v );
	void processSame ( const ConceptSet& v );
	void processDifferent ( const ConceptSet& v );
	DLTree* processAnd ( const ConceptSet& v );
	DLTree* processOr ( const ConceptSet& v );
	DLTree* processOneOf ( const ConceptSet& v, bool data );
	DLTree* processRComposition ( const ConceptSet& v );

		/// @return true if KB contains fairness constraints
	bool hasFC ( void ) const { return !Fairness.empty(); }
		/// add concept expression C as a fairness constraint
	void setFairnessConstraint ( const ConceptSet& v )
	{
		for ( ConceptSet::const_iterator p = v.begin(), p_end = v.end(); p < p_end; ++p )
		{
			// build a flag for a FC
			TConcept* fc = getAuxConcept();
			Fairness.push_back(fc);
			// make an axiom: C [= FC
			addSubsumeAxiom ( *p, getTree(fc) );
		}
	}

//-----------------------------------------------------------------------------
//--		public access interface
//-----------------------------------------------------------------------------

		/// GCI Axioms access
	BipolarPointer getTG ( void ) const { return T_G; }
		/// get simple rule by its INDEX
	const TSimpleRule* getSimpleRule ( unsigned int index ) const { return SimpleRules[index]; }

		/// check if the relevant part of KB contains inverse roles.
	bool isIRinQuery ( void ) const
	{
		if ( curFeature != NULL )
			return curFeature->hasInverseRole();
		else
			return KBFeatures.hasInverseRole();
	}
		/// check if the relevant part of KB contains number restrictions.
	bool isNRinQuery ( void ) const
	{
		const LogicFeatures* p = curFeature ? curFeature : &KBFeatures;
		return p->hasFunctionalRestriction() || p->hasNumberRestriction() || p->hasQNumberRestriction();
	}
		/// check if the relevant part of KB contains singletons
	bool testHasNominals ( void ) const
	{
		if ( curFeature != NULL )
			return curFeature->hasSingletons();
		else
			return KBFeatures.hasSingletons();
	}
		/// check if Sorted Reasoning is applicable
	bool canUseSortedReasoning ( void ) const
		{ return useSortedReasoning && !GCIs.isGCI() && !GCIs.isReflexive(); }

//-----------------------------------------------------------------------------
//--		public reasoning interface
//-----------------------------------------------------------------------------
		/// prepare to reasoning
	void prepareReasoning ( void );
		/// perform classification (assuming KB is consistent)
	void performClassification ( void ) { createTaxonomy ( /*needIndividuals=*/false ); }
		/// perform realisation (assuming KB is consistent)
	void performRealisation ( void ) { createTaxonomy ( /*needIndividuals=*/true ); }

	/// get (READ-WRITE) access to internal Taxonomy of concepts
	DLConceptTaxonomy* getTaxonomy ( void ) { return pTax; }
	/// get (READ-ONLY) access to internal Taxonomy of concepts
	const DLConceptTaxonomy* getTaxonomy ( void ) const { return pTax; }

		/// get (RW) access to the DataType center
	DataTypeCenter& getDataTypeCenter ( void ) { return DTCenter; }
		/// get (RO) access to the DataType center
	const DataTypeCenter& getDataTypeCenter ( void ) const { return DTCenter; }

		/// set given structure as a progress monitor
	void setProgressMonitor ( TProgressMonitor* pMon ) { delete pMonitor; pMonitor = pMon; }
		/// check that reasoning progress was cancelled by external application
	bool isCancelled ( void ) const { return pMonitor != NULL && pMonitor->isCancelled(); }
		/// set verbose output (ie, default progress monitor, concept and role taxonomies
	void useVerboseOutput ( void ) { verboseOutput = true; }

	/// create (and DAG-ify) temporary concept via its definition
	TConcept* createTempConcept ( const DLTree* desc );
	/// classify temporary concept
	bool classifyTempConcept ( void );

//-----------------------------------------------------------------------------
//--		public reasoning interface
//-----------------------------------------------------------------------------

		/// inform that KB is precompleted
	void setPrecompleted ( void ) { Precompleted = true; }
		/// if KB is precompleted
	bool isPrecompleted ( void ) const { return Precompleted; }
		/// get status flag
	KBStatus getStatus ( void ) const { return Status; }
		/// set consistency flag
	void setConsistency ( bool val )
	{
		Status = kbCChecked;
		Consistent = val;
	}
		/// check if the ontology is consistent
	bool isConsistent ( void )
	{
		if ( Status < kbCChecked )
		{
			prepareReasoning();
			if ( Status < kbCChecked && Consistent )	// we can detect inconsistency during preprocessing
				setConsistency(performConsistencyCheck());
		}
		return Consistent;
	}
		/// check if a subsumption C [= D holds
	bool isSubHolds ( const TConcept* C, const TConcept* D );
		/// check if a concept C is satisfiable
	bool isSatisfiable ( const TConcept* C );
		/// check that 2 individuals are the same
	bool isSameIndividuals ( const TIndividual* a, const TIndividual* b )
	{
		if ( !isIndividual(a) || !isIndividual(b) )
			throw EFaCTPlusPlus("Individuals are expected in the isSameIndividuals() query");
		if ( a->node == NULL || b->node == NULL )
			throw EFaCTPlusPlus("isSameIndividuals() query with non-realised ontology");
		return a->node->resolvePBlocker() == b->node->resolvePBlocker();
	}

		/// fills cache entry for given concept; set up SAT flag to a concept
	const modelCacheInterface* initCache ( TConcept* pConcept );

		/// test if 2 concept non-subsumption can be determined by cache merging
	enum modelCacheState testCachedNonSubsumption ( const TConcept* p, const TConcept* q );
		/// test if 2 concept non-subsumption can be determined by sorts checking
	bool testSortedNonSubsumption ( const TConcept* p, const TConcept* q )
	{
		// sorted reasoning doesn't work in presence of GCIs
		if ( !canUseSortedReasoning() )
			return false;
		// doesn't work for the SAT tests
		if ( q == NULL )
			return false;
		return !DLHeap.haveSameSort ( p->pName, q->pName );
	}


//-----------------------------------------------------------------------------
//--		public output interface
//-----------------------------------------------------------------------------

		/// dump query processing TIME, reasoning statistics and a (preprocessed) TBox
	void writeReasoningResult ( std::ostream& o, float time ) const;
		/// print TBox as a whole
	void Print ( std::ostream& o ) const
	{
		DLHeap.PrintStat(o);
		RM.Print(o);
		PrintConcepts(o);
		PrintIndividuals(o);
		PrintSimpleRules(o);
		PrintAxioms(o);
		DLHeap.Print(o);
	}

		/// create dump of relevant part of query using given method
	void dump ( dumpInterface* dump ) const;

//-----------------------------------------------------------------------------
//--		 save/load interface; implementation in SaveLoad.cpp
//-----------------------------------------------------------------------------

		/// save the KB into the given stream
	void Save ( std::ostream& o ) const;
		/// load the KB from given stream wrt STATUS
	void Load ( std::istream& o, KBStatus status );

		/// implement related-based query; @return in Rs all R s.t. (I,x):R
	void getRelatedRoles ( TIndividual* I, NamesVector& Rs, bool data, bool needI ) const
	{
		TIndividual* i = resolveSynonym(I);
		RM.getRelatedRoles ( i->RelatedRoleMap, Rs, data, needI );
	}
		/// implement DIG-like roleFillers query; @return in Js all J st (I,J):R
	void getRoleFillers ( TIndividual* I, TRole* R, NamesVector& Js ) const;
		/// implement DIG-like RelatedIndividuals query; @return Is and Js st (I,J):R
	void getRelatedIndividuals ( TRole* R, NamesVector& Is, NamesVector& Js ) const;
		/// implement related-based query; @return true iff (I,J):R
	bool isRelated ( TIndividual* I, TRole* R, TIndividual* J ) const;
		/// implement absorbedPrimitiveConceptDefinitions DIG extension
	void absorbedPrimitiveConceptDefinitions ( std::ostream& o ) const;
		/// implement unabsorbed DIG extension
	void unabsorbed ( std::ostream& o ) const;
}; // TBox

#endif
