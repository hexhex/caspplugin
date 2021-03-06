/* CASPPlugin -- Pluging for dlvhex - Answer-Set Programming with constraint programming
 * Copyright (C) 2015 Alessandro Francesco De Rosis
 *
 * This file is part of caspplugin.
 *
 * CASPPlugin is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * CASPPlugin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dlvhex; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
#include "casp/CaspPlugin.h"
#include <boost/unordered_map.hpp>

DLVHEX_NAMESPACE_BEGIN

using namespace dlvhex::casp;
namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;

CASPPlugin::CASPPlugin() :
				_simpleParser(new SimpleParser()),
				_learningProcessor(new NoLearningProcessor(_simpleParser)),
				cspGraphLearning(false),
				cspAnticipateLearning(false)
{
	setNameVersion(PACKAGE_TARNAME,CASPPLUGIN_VERSION_MAJOR,CASPPLUGIN_VERSION_MINOR,CASPPLUGIN_VERSION_MICRO);
}

void CASPPlugin::printUsage(std::ostream& o) const
{
	o << "     --cspenable - enabling casp plugin" << endl;
	o << "     --csplearning=[none,deletion,forward,backward,cc,wcc] " << endl;
	o << "                   Enable csp learning(none by default)." << endl;
	o << "                   none        - No learning." << endl;
	o << "                   deletion    - Deletion filtering learning." << endl;
	o << "                   forward     - Forward filtering learning." << endl;
	o << "                   jumpforward - Jump forward filtering learning." << endl;
	o << "                   backward    - Backward filtering learning." << endl;
	o << "                   range       - Range filtering learning." << endl;
	o << "                   cc          - Connected component filtering learning." << endl;
	o << "                   wcc         - Weighted connected component filtering learning." << endl;
	o << "     --cspgraphlearning=[false,true]"<<endl;
	o << "                   Enable csp graph learning(disabled by default)." << endl;
	o << "                   Enable only if --eaevalheuristics=always" << endl;
	o << "     --cspanticipatelearning=[false,true]"<<endl;
	o << "                   Enable anticipate learning(disabled by default)." << endl;
	o << "                   Enable only if --eaevalheuristics=always" << endl;
}
void CASPPlugin::
processOptions(std::list<const char*>& pluginOptions, ProgramCtx& ctx)
{
	CASPPlugin::CtxData& ctxdata = ctx.getPluginData<CASPPlugin>();
	ctxdata.enabled = false;
	typedef std::list<const char*>::iterator listIterator;
	listIterator it = pluginOptions.begin();
	while (it != pluginOptions.end()) {
		std::string option(*it);

		if (option.find("--cspenable") != std::string::npos) {
			ctxdata.enabled = true;
			it = pluginOptions.erase(it);
		}

		else if (option.find("--csplearning=") != std::string::npos) {
			string processorName = option.substr(std::string("--csplearning=").length());
			if (processorName == "none") {
				_learningProcessor = boost::shared_ptr<LearningProcessor>(new NoLearningProcessor(_simpleParser));
			}
			else if (processorName == "deletion") {
				_learningProcessor = boost::shared_ptr<LearningProcessor>(new DeletionLearningProcessor(_simpleParser));
			}
			else if (processorName == "forward") {
				_learningProcessor = boost::shared_ptr<LearningProcessor>(new ForwardLearningProcessor(_simpleParser));
			}
			else if (processorName == "jumpforward") {
				_learningProcessor = boost::shared_ptr<LearningProcessor>(new JumpForwardLearningProcessor(_simpleParser));
			}
			else if (processorName == "backward") {
				_learningProcessor = boost::shared_ptr<LearningProcessor>(new BackwardLearningProcessor(_simpleParser));
			}
			else if (processorName == "range") {
				_learningProcessor = boost::shared_ptr<LearningProcessor>(new RangeLearningProcessor(_simpleParser));
			}
			else if (processorName == "cc") {
				_learningProcessor = boost::shared_ptr<LearningProcessor>(new CCLearningProcessor(_simpleParser));
			}
			else if (processorName == "wcc") {
				_learningProcessor = boost::shared_ptr<LearningProcessor>(new WeightedCCLearningProcessor(_simpleParser));
			}
			else
				throw PluginError("Unrecognized option for --csplearning: " + processorName);
			it = pluginOptions.erase(it);
		}

		else if(option.find("--cspgraphlearning=")!= std::string::npos)
		{
			string flag = option.substr(std::string("--cspgraphlearning=").length());
			if(flag=="true")
				cspGraphLearning=true;
			else if(flag=="false")
				cspGraphLearning=false;
			else
				throw PluginError("Unrecognized option for --cspgraphlearning: "+flag);
			it= pluginOptions.erase(it);
		}
		else if(option.find("--cspanticipatelearning=")!= std::string::npos)
		{
			string flag = option.substr(std::string("--cspanticipatelearning=").length());
			if(flag=="true")
				cspAnticipateLearning=true;
			else if(flag=="false")
				cspAnticipateLearning=false;
			else
				throw PluginError("Unrecognized option for --cspanticipatelearning: "+ flag);
			it= pluginOptions.erase(it);
		}
		else
			it++;
	}
}



struct SumElement
{
	string predicateName;
	int predicateArity;
	ID index;
	SumElement(int _predicateArity=-1):predicateArity(_predicateArity)
	{
	}
};

class CASPParserModuleSemantics:
public HexGrammarSemantics
{
public:
	CASPPlugin::CtxData& ctxdata;
	bool defineDomain;
	ID domainAuxID;
	ID maximizeAuxID;
	ID minimizeAuxID;
	ID sumElementAuxID;
	ID exprAuxID;
	ID not_exprAuxID;
	vector<SumElement> sumPredicates;
	int  previousSizeOfSumPredicates;

	bool cspGraphLearning;
	boost::unordered_map <string,Interpretation > possibleConflictVariable;
	MapPossibleConflict& possibleConflictID;
	Interpretation& cpVariable;

public:
	CASPParserModuleSemantics(ProgramCtx& ctx,CPVariableAndConnection& cpVariableAndConnection,bool _cspGraphLearning):
		HexGrammarSemantics(ctx),
		ctxdata(ctx.getPluginData<CASPPlugin>()),
		defineDomain(false),
		cpVariable(cpVariableAndConnection.cpVariable),
		possibleConflictID(cpVariableAndConnection.possibleConflictCpVariable),
		cspGraphLearning(_cspGraphLearning)
	{
		RegistryPtr registry=ctx.registry();
		domainAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(0));
		maximizeAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(1));
		minimizeAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(2));
		exprAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(3));
		not_exprAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(4));
		sumElementAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(5));
		previousSizeOfSumPredicates=0;
		createCaspExternalAtom();
	}

	// use SemanticActionBase to redirect semantic action call into globally
	// specializable sem<T> struct space
	struct caspRule:
		SemanticActionBase<CASPParserModuleSemantics, ID, caspRule>
	{
		caspRule(CASPParserModuleSemantics& mgr):
			caspRule::base_type(mgr)
		{
		}
	};
	struct caspElement:
		SemanticActionBase<CASPParserModuleSemantics, ID, caspElement>
	{
		caspElement(CASPParserModuleSemantics& mgr):
			caspElement::base_type(mgr)
		{
		}
	};
	struct caspDirective:
		SemanticActionBase<CASPParserModuleSemantics, ID, caspDirective>
	{
		caspDirective(CASPParserModuleSemantics& mgr):
			caspDirective::base_type(mgr)
		{
		}
	};
	struct caspOperator:
		SemanticActionBase<CASPParserModuleSemantics, ID, caspOperator>
	{
		caspOperator(CASPParserModuleSemantics& mgr):
			caspOperator::base_type(mgr)
		{
		}
	};

	struct caspVariable:
		SemanticActionBase<CASPParserModuleSemantics, string, caspVariable>
	{
		caspVariable(CASPParserModuleSemantics& mgr):
			caspVariable::base_type(mgr)
		{
		}
	};

	struct caspSum:
		SemanticActionBase<CASPParserModuleSemantics, string, caspSum>
	{
		caspSum(CASPParserModuleSemantics& mgr):
			caspSum::base_type(mgr)
		{
		}
	};

	struct caspGlobalConstraint:
		SemanticActionBase<CASPParserModuleSemantics, ID, caspGlobalConstraint>
	{
		caspGlobalConstraint(CASPParserModuleSemantics& mgr):
			caspGlobalConstraint::base_type(mgr)
		{
		}
	};


	void createCaspExternalAtom()
	{
		RegistryPtr reg=ctx.registry();
		Rule externalConstraint(ID::MAINKIND_RULE |ID::SUBKIND_RULE_CONSTRAINT |ID::PROPERTY_RULE_EXTATOMS);
		ExternalAtom ext(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_EXTERNAL | ID::NAF_MASK);
		ext.predicate=reg->storeConstantTerm("casp");
		ext.inputs.push_back(domainAuxID);
		ext.inputs.push_back(maximizeAuxID);
		ext.inputs.push_back(minimizeAuxID);
		ext.inputs.push_back(exprAuxID);
		ext.inputs.push_back(not_exprAuxID);
		ext.inputs.push_back(sumElementAuxID);
		externalConstraint.body.push_back( ID::nafLiteralFromAtom(reg->eatoms.storeAndGetID(ext)));
		ID extRuleID=reg->storeRule(externalConstraint);
		ctx.idb.push_back(extRuleID);
	}
};


// create semantic handler for above semantic action
// (needs to be in globally specializable struct space)

template<>
struct sem<CASPParserModuleSemantics::caspRule>
{
	void operator()(
			CASPParserModuleSemantics& mgr,
			const boost::variant<dlvhex::ID, boost::fusion::vector2<boost::optional<std::vector<dlvhex::ID> >,
			boost::optional<std::vector<dlvhex::ID> > > >&  source,
			ID& target)
	{

		bool insertSumRule=false;
		//check if there is some new sum predicate in the rule
		if(mgr.previousSizeOfSumPredicates!=mgr.sumPredicates.size())
		{
			insertSumRule=true;
		}

		Tuple  bodyWithoutCasp;
		Tuple bodyCasp;

		RegistryPtr reg=mgr.ctx.registry();
		Rule rule(ID::MAINKIND_RULE );
		if(const ID* idDirective = boost::get< ID >( &source ))
		{
			rule.kind |= ID::SUBKIND_RULE_REGULAR;
			rule.head.push_back(*idDirective);
		}
		else
		{
			const boost::fusion::vector2<boost::optional<std::vector<dlvhex::ID> >, boost::optional<std::vector<dlvhex::ID> > > idAtomRule=*(boost::get< boost::fusion::vector2<boost::optional< std::vector<dlvhex::ID> >, boost::optional<std::vector<dlvhex::ID> > > > ( &source ));
			//if the head exist
			if(!!boost::fusion::at_c<0>(idAtomRule))
			{
				vector<ID> head =boost::fusion::at_c<0>(idAtomRule).get();
				rule.kind |= ID::SUBKIND_RULE_REGULAR ;
				if(head.size()>1)
				{
					//guessing rule
					rule.kind |= ID::PROPERTY_RULE_DISJ;
				}

				rule.head.insert(rule.head.end(),head.begin(),head.end());
			}
			//if not exist the head but there is the body
			else if(!!boost::fusion::at_c<1>(idAtomRule))
			{
				rule.kind |= ID::SUBKIND_RULE_CONSTRAINT;
			}

			//if exist the body
			if(!!boost::fusion::at_c<1>(idAtomRule))
			{
				std::vector<dlvhex::ID> idBody=boost::fusion::at_c<1>(idAtomRule).get();
				BOOST_FOREACH(const ID& id, idBody)
				{
					rule.body.push_back(id);
					if(id.isOrdinaryAtom())
					{
						OrdinaryAtom atom=reg->lookupOrdinaryAtom(id);
						if(atom.tuple[0]==mgr.exprAuxID)
						{
							bodyCasp.push_back(id);
						}
						else
						{
							if(insertSumRule)
							{
								for(int i=mgr.previousSizeOfSumPredicates;i<mgr.sumPredicates.size();i++)
								{
									Term term=reg->terms.getByID(atom.tuple[0]);
									if(mgr.sumPredicates[i].predicateName==term.symbol)
									{
										//save the arity of the predicate involved in the sum
										mgr.sumPredicates[i].predicateArity=atom.tuple.size()-1;
									}
								}
							}
							bodyWithoutCasp.push_back(id);
						}
					}
					else
					{
						bodyWithoutCasp.push_back(id);
					}
				}
			}
		}
		createSumRule(mgr);
		mgr.previousSizeOfSumPredicates=mgr.sumPredicates.size();

		ID ruleID=reg->storeRule(rule);
		mgr.ctx.idb.push_back(ruleID);

		//create guessing rule
		BOOST_FOREACH(const ID& id, bodyCasp)
		{



			Rule guessingRule(ID::MAINKIND_RULE | ID::SUBKIND_RULE_REGULAR | ID::PROPERTY_RULE_DISJ);
			guessingRule.body.insert(guessingRule.body.end(),bodyWithoutCasp.begin(),bodyWithoutCasp.end());

			guessingRule.head.push_back(id);

			OrdinaryAtom notCaspAtom = reg->lookupOrdinaryAtom(id);
			Term t = reg->terms.getByID(notCaspAtom.tuple[0]);
			//set name not_expr
			notCaspAtom.tuple[0]=mgr.not_exprAuxID;


			ID hid = reg->storeOrdinaryAtom(notCaspAtom);

			// if the atom haven't ASP variables map also the not_expr atom
			if(mgr.cspGraphLearning && mgr.possibleConflictID.find(id)!=mgr.possibleConflictID.end())
			{
				set<Interpretation*>& toFill=mgr.possibleConflictID[hid];
				for(set<Interpretation*>::iterator it=mgr.possibleConflictID[id].begin();it!=mgr.possibleConflictID[id].end();++it)
				{
					(*it)->setFact(hid.address);
					toFill.insert(*it);
				}
			}

			guessingRule.head.push_back(hid);

			ID guessingRuleID = reg->storeRule(guessingRule);
			mgr.ctx.idb.push_back(guessingRuleID);
		}
	}

	void createSumRule(CASPParserModuleSemantics& mgr)
	{
		RegistryPtr reg=mgr.ctx.registry();

		for(int i=mgr.previousSizeOfSumPredicates;i<mgr.sumPredicates.size();i++)
		{
			Rule rule(ID::MAINKIND_RULE | ID::SUBKIND_RULE_REGULAR);
			SumElement sumElement=mgr.sumPredicates[i];

			//check the arity of argument of $sum
			assert(sumElement.predicateArity!=-1 && sumElement.index.address<=sumElement.predicateArity);

			OrdinaryAtom headAtom(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYN);
			headAtom.tuple.push_back(mgr.sumElementAuxID);
			ID predicateID=reg->storeConstantTerm(sumElement.predicateName);
			headAtom.tuple.push_back(predicateID);
			headAtom.tuple.push_back(sumElement.index);
			headAtom.tuple.push_back(reg->storeVariableTerm("X"+boost::lexical_cast<string>(sumElement.index.address)));
			rule.head.push_back(reg->storeOrdinaryAtom(headAtom));

			OrdinaryAtom bodyAtom(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYN);
			bodyAtom.tuple.push_back(predicateID);
			for(int i=1;i<=sumElement.predicateArity;i++)
			{
				bodyAtom.tuple.push_back(reg->storeVariableTerm("X"+boost::lexical_cast<string>(i)));
			}
			rule.body.push_back(reg->storeOrdinaryAtom(bodyAtom));

			ID ruleID=reg->storeRule(rule);
			mgr.ctx.idb.push_back(ruleID);
		}
	}
};


template<>
struct sem<CASPParserModuleSemantics::caspElement>
{
	void operator()(
			CASPParserModuleSemantics& mgr,
			const boost::fusion::vector2 < boost::variant<dlvhex::ID, std::basic_string<char> >,
			std::vector<boost::fusion::vector2<dlvhex::ID, boost::variant<dlvhex::ID, std::basic_string<char> > >,
			std::allocator<boost::fusion::vector2<dlvhex::ID, boost::variant<dlvhex::ID, std::basic_string<char> > > > > > & source,
			ID& target)
	{
		boost::unordered_set<string> listCASPVariables;
		bool ASPVariable=!mgr.cspGraphLearning;
		bool alreadyComparisonOperatorDefined=false;
		RegistryPtr reg = mgr.ctx.registry();


		//result atom of caspElement
		OrdinaryAtom atom(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG);
		atom.tuple.push_back(mgr.exprAuxID);

		ID variable;

		variable=getVariable(reg,atom,boost::fusion::at_c<0>(source),ASPVariable,listCASPVariables);

		atom.tuple.push_back(variable);

		std::vector<boost::fusion::vector2<dlvhex::ID, boost::variant<dlvhex::ID, std::basic_string<char> > > >  v=boost::fusion::at_c<1>(source);
		for(int i=0;i<v.size();i++)
		{

			boost::fusion::vector2<dlvhex::ID, boost::variant<dlvhex::ID, std::basic_string<char> > > v1=v[i];

			ID operatorID= boost::fusion::at_c<0>(v1);
			if(isComparisonOperator(reg->getTermStringByID(operatorID)))
			{
				//only one comparisonOperator is allowed in expression
				assert(!alreadyComparisonOperatorDefined);
				alreadyComparisonOperatorDefined=true;
			}
			atom.tuple.push_back(operatorID);
			variable=getVariable(reg,atom,boost::fusion::at_c<1>(v1),ASPVariable,listCASPVariables);
			atom.tuple.push_back(variable);
		}
		target=reg->storeOrdinaryAtom(atom);

		//if there isn't ASPVariable, define possible conflict
		if(!ASPVariable)
		{
			//insert atom in the conflict set
			set< Interpretation* >& conflictSet=mgr.possibleConflictID[target];
			for(boost::unordered_set<string>::iterator i=listCASPVariables.begin();i!=listCASPVariables.end();i++)
			{
				Interpretation& s=mgr.possibleConflictVariable[*i];
				s.setFact(target.address);
				conflictSet.insert(&s);
			}
			mgr.cpVariable.setFact(target.address);
		}
	}

	//generate constant term from string or return ID of term
	ID getVariable(RegistryPtr& reg,OrdinaryAtom& atom,const boost::variant<dlvhex::ID, std::basic_string<char> >& variant,bool& ASPVariable,boost::unordered_set<string>& listCASPVariables)
	{
		ID toReturn;
		if(const std::basic_string<char>* variable = boost::get< std::basic_string<char> >( &variant ))
		{
			toReturn=reg->storeConstantTerm("\""+*variable+"\"");
			if(!ASPVariable)
			{
				listCASPVariables.insert(*variable);
			}
		}
		else
		{
			toReturn=*( boost::get< ID >( &variant ));
			if(toReturn.isVariableTerm())
			{
				atom.kind|=ID::SUBKIND_ATOM_ORDINARYN;
				ASPVariable=true;
			}
		}
		return toReturn;
	}
};

template<>
struct sem<CASPParserModuleSemantics::caspVariable>
{

	void operator()(
			CASPParserModuleSemantics& mgr,
			const boost::variant<std::basic_string<char>, boost::fusion::vector2<char, std::vector<char, std::allocator<char> > > > & source,
			string& target)
	{
		if(const string* variable = boost::get< string >( &source ))
		{
			target=*variable;
		}
		else
		{
			const boost::fusion::vector2<char, std::vector<char, std::allocator<char> > >& v=*(boost::get< boost::fusion::vector2<char, std::vector<char, std::allocator<char> > > >( &source ));
			target=boost::fusion::at_c<0>(v);
			std::vector<char, std::allocator<char> > chars=boost::fusion::at_c<1>(v);
			for(int i=0;i<chars.size();i++)
				target+=chars[i];
		}
	}
};

template<>
struct sem<CASPParserModuleSemantics::caspSum>
{

	void operator()(
			CASPParserModuleSemantics& mgr,
			const boost::fusion::vector2<boost::fusion::vector2<char, std::vector<char, std::allocator<char> > >, dlvhex::ID>& source, std::basic_string<char>& target)
	{
		// transform sum(p,2) to sum_p_2 for proper parsing
		RegistryPtr reg=mgr.ctx.registry();

		//read predicate name
		const boost::fusion::vector2<char, std::vector<char, std::allocator<char> > >& v=boost::fusion::at_c<0>(source);
		SumElement sumElement;

		sumElement.predicateName+=boost::fusion::at_c<0>(v);
		std::vector<char, std::allocator<char> > chars=boost::fusion::at_c<1>(v);
		for(int i=0;i<chars.size();i++)
			sumElement.predicateName+=chars[i];
		ID predicateID=reg->storeConstantTerm(sumElement.predicateName);

		//read i-th(1-based) argument
		ID integerID=boost::fusion::at_c<1>(source);
		sumElement.index=integerID;
		assert(integerID.isIntegerTerm());

		ostringstream os;
		os<<printToString<RawPrinter>(integerID,reg);
		target="sum_"+sumElement.predicateName+"_"+os.str();

		mgr.sumPredicates.push_back(sumElement);

	}
};
template<>
struct sem<CASPParserModuleSemantics::caspDirective>
{
	void operator()(
			CASPParserModuleSemantics& mgr,
			const boost::variant<boost::fusion::vector2<dlvhex::ID, dlvhex::ID>, dlvhex::ID>& source,
			ID& target)
	{
		RegistryPtr reg = mgr.ctx.registry();

		if(const boost::fusion::vector2<dlvhex::ID, dlvhex::ID>* upLowContainer= boost::get< boost::fusion::vector2<dlvhex::ID, dlvhex::ID> >( &source ))
		{
			//domain must be defined one time
			assert(!mgr.defineDomain);
			mgr.defineDomain=true;
			OrdinaryAtom atom(ID::MAINKIND_ATOM);
			ID domainLowerBoudID =boost::fusion::at_c<0>(*upLowContainer);
			ID domainUpperBoudID =boost::fusion::at_c<1>(*upLowContainer);

			//domain must to have lower and upper bound as Integer
			assert(domainLowerBoudID.isIntegerTerm() && domainUpperBoudID.isIntegerTerm());

			atom.tuple.push_back(mgr.domainAuxID);
			atom.tuple.push_back(domainLowerBoudID);
			atom.tuple.push_back(domainUpperBoudID);
			target=reg->storeOrdinaryAtom(atom);
		}
		else
		{
			target=*(boost::get< ID >( &source ));
		}

	}
};

template<>
struct sem<CASPParserModuleSemantics::caspOperator>
{
	void operator()(
			CASPParserModuleSemantics& mgr,
			const std::basic_string< char >& source, dlvhex::ID& target)
	{
		RegistryPtr reg = mgr.ctx.registry();
		target=reg->storeConstantTerm("\""+source+"\"");
	}

};

template<>
struct sem<CASPParserModuleSemantics::caspGlobalConstraint>
{
	void operator()(
			CASPParserModuleSemantics& mgr,
			const boost::fusion::vector2<std::basic_string<char>, boost::fusion::vector2<char, std::vector<char, std::allocator<char> > > >& source, dlvhex::ID& target)
	{
		RegistryPtr reg = mgr.ctx.registry();

		string directive=boost::fusion::at_c<0>(source);
		assert(directive=="maximize" || directive=="minimize");

		//read argument
		boost::fusion::vector2<char, std::vector<char, std::allocator<char> > > v=boost::fusion::at_c<1>(source);
		string variableName;
		variableName+=boost::fusion::at_c<0>(v);
		std::vector<char, std::allocator<char> > chars=boost::fusion::at_c<1>(v);
		for(int i=0;i<chars.size();i++)
			variableName+=chars[i];

		ID predicateID=reg->storeConstantTerm("\""+variableName+"\"");

		//create atom $directive("namePredicate")
		OrdinaryAtom atom(ID::MAINKIND_ATOM);
		if(directive=="maximize")
			atom.tuple.push_back(mgr.maximizeAuxID);
		else
			atom.tuple.push_back(mgr.minimizeAuxID);
		atom.tuple.push_back(predicateID);
		target=reg->storeOrdinaryAtom(atom);
	}

};

namespace {
namespace ascii = boost::spirit::ascii;
template<typename Iterator, typename Skipper>
struct CASPParserModuleGrammarBase:
	// we derive from the original hex grammar
	// -> we can reuse its rules
public HexGrammarBase<Iterator, Skipper>
{
	typedef HexGrammarBase<Iterator, Skipper> Base;
	CASPParserModuleSemantics& sem;
	CASPParserModuleGrammarBase(CASPParserModuleSemantics& sem):
		Base(sem),
		sem(sem)
	{
		typedef CASPParserModuleSemantics Sem;
		caspRule
		= (caspDirective | ((( caspElement | Base::headAtom )   % qi::lit("|") ) || (qi::lit(":-") > (( caspElement | Base::bodyLiteral ) % qi::char_(',')) )) >> qi::lit('.') ) [ Sem::caspRule(sem) ];
		caspElement
		= (
				(Base::term | caspVariable) >> +( caspOperator >> (Base::term | caspVariable)  )>> qi::eps
		) [ Sem::caspElement(sem) ];

		caspVariable
		= (
				qi::lit("$") >> (caspSum | qi::lexeme[ ascii::lower >> *(ascii::alnum | qi::char_('_')) ])

		)[Sem::caspVariable(sem)];

		caspSum
		= (
				qi::lit("sum(") >>  qi::lexeme[ ascii::lower >> *(ascii::alnum | qi::char_('_')) ] >> qi::lit(",") >> Base::term >> qi::lit(")")
		)[Sem::caspSum(sem)];

		caspDirective
		= (
				(qi::lit("$domain(")>>Base::term>qi::lit("..")>Base::term>>qi::lit(").")) | caspGlobalConstraint
		)[ Sem::caspDirective(sem) ];
		caspGlobalConstraint
		= (
				qi::lit("$")>>Base::cident>>qi::lit("(")>>qi::lexeme[ ascii::lower >> *(ascii::alnum | qi::char_('_')) ] >>qi::lit(")")>>qi::lit('.')
		)[ Sem::caspGlobalConstraint(sem)];
		caspOperator
		= (
				qi::lit("$")>>(qi::string("+")| qi::string("-")|qi::string("*")|qi::string("/")|qi::string("%")|qi::string("==")|qi::string("<=")|qi::string(">=")| qi::string("<")|qi::string(">"))
		)[Sem::caspOperator(sem)];
#ifdef BOOST_SPIRIT_DEBUG
		BOOST_SPIRIT_DEBUG_NODE(caspElement);
#endif
	}
	qi::rule<Iterator, ID(), Skipper> caspRule;
	qi::rule<Iterator, ID(), Skipper> caspElement;
	qi::rule<Iterator, ID(), Skipper> caspDirective;
	qi::rule<Iterator, ID(), Skipper> caspOperator;
	qi::rule<Iterator, string(), Skipper> caspVariable;
	qi::rule<Iterator, string(), Skipper> caspSum;
	qi::rule<Iterator, ID(), Skipper> caspGlobalConstraint;
};

struct CASPParserModuleGrammar:
	CASPParserModuleGrammarBase<HexParserIterator, HexParserSkipper>,
	// required for interface
	// note: HexParserModuleGrammar =
	//       boost::spirit::qi::grammar<HexParserIterator, HexParserSkipper>
	HexParserModuleGrammar
	{
		typedef CASPParserModuleGrammarBase<HexParserIterator, HexParserSkipper> GrammarBase;
		typedef HexParserModuleGrammar QiBase;
		CASPParserModuleGrammar(CASPParserModuleSemantics& sem):
			GrammarBase(sem),
			QiBase(GrammarBase::caspRule)
		{
		}
	};
	typedef boost::shared_ptr<CASPParserModuleGrammar>
	CASPParserModuleGrammarPtr;

	// moduletype = HexParserModule::TOPLEVEL
	template<enum HexParserModule::Type moduletype>
	class CASPParserModule:
public HexParserModule
{
public:
	// the semantics manager is stored/owned by this module!
	CASPParserModuleSemantics sem;
	// we also keep a shared ptr to the grammar module here
	CASPParserModuleGrammarPtr grammarModule;


	CASPParserModule(ProgramCtx& ctx,CPVariableAndConnection& cpVariableAndConnection,bool cspGraphLearning):
		HexParserModule(moduletype),
		sem(ctx,cpVariableAndConnection,cspGraphLearning)
	{
		LOG(INFO,"constructed CASPParserModule");
	}

	virtual HexParserModuleGrammarPtr createGrammarModule()
	{
		assert(!grammarModule && "for simplicity (storing only one grammarModule pointer) we currently assume this will be called only once .. should be no problem to extend");
		grammarModule.reset(new CASPParserModuleGrammar(sem));
		LOG(INFO,"created CASPParserModuleGrammar");
		return grammarModule;
	}
};
}

std::vector<HexParserModulePtr>	CASPPlugin::createParserModules(ProgramCtx& ctx)
{
	DBGLOG(DBG,"CASPPlugin::createParserModules()");
	std::vector<HexParserModulePtr> ret;

	CASPPlugin::CtxData& ctxdata = ctx.getPluginData<CASPPlugin>();
	if( ctxdata.enabled )
	{
		ret.push_back(HexParserModulePtr(new CASPParserModule<HexParserModule::TOPLEVEL>(ctx,cpVariableAndConnection,cspGraphLearning)));
	}
	return ret;
}

std::vector<PluginAtomPtr> CASPPlugin::createAtoms(ProgramCtx&) const
{
	std::vector<PluginAtomPtr> ret;
	ret.push_back(PluginAtomPtr(new ConsistencyAtom(_learningProcessor, _simpleParser,cpVariableAndConnection,cspGraphLearning,cspAnticipateLearning), PluginPtrDeleter<PluginAtom>()));
	return ret;
}
DLVHEX_NAMESPACE_END
