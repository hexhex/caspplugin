#include "casp/CaspPlugin.h"
#include <dlvhex2/Printer.h>

DLVHEX_NAMESPACE_BEGIN

using namespace dlvhex::casp;
namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;

CASPPlugin::CASPPlugin() :
				_simpleParser(new SimpleParser()),
				_learningProcessor(new NoLearningProcessor(_simpleParser)),
				_numberSumPredicate(0)
{
	setNameVersion(PACKAGE_TARNAME,CASPPLUGIN_VERSION_MAJOR,CASPPLUGIN_VERSION_MINOR,CASPPLUGIN_VERSION_MICRO);
}

void CASPPlugin::incrementNumberSumPredicate()
{
	_numberSumPredicate++;
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
		else
			it++;
	}
}


class CASPParserModuleSemantics:
public HexGrammarSemantics
{
public:
	CASPPlugin caspPlugin;
	CASPPlugin::CtxData& ctxdata;
	bool defineDomain;
	ID expr;
	ID not_expr;
	ID extAtom;
	ID ruleExt;

public:
	CASPParserModuleSemantics(ProgramCtx& ctx,CASPPlugin& _caspPlugin):
		HexGrammarSemantics(ctx),
		ctxdata(ctx.getPluginData<CASPPlugin>()),
		defineDomain(false),
		caspPlugin(_caspPlugin)
	{
		expr=ctx.registry()->storeConstantTerm("expr");
		not_expr=ctx.registry()->storeConstantTerm("not_expr");
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
		ext.inputs.push_back(reg->storeConstantTerm("domain"));
		ext.inputs.push_back(reg->storeConstantTerm("maximize"));
		ext.inputs.push_back(reg->storeConstantTerm("minimize"));
		ext.inputs.push_back(expr);
		ext.inputs.push_back(not_expr);
		extAtom=reg->eatoms.storeAndGetID(ext);
		externalConstraint.body.push_back( ID::nafLiteralFromAtom(extAtom));
		ID extRuleID=reg->storeRule(externalConstraint);
		ruleExt=extRuleID;
		ctx.idb.push_back(extRuleID);
		cout<<printToString<RawPrinter>(extRuleID,reg)<<endl<<endl<<endl;
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
			else if(!!boost::fusion::at_c<1>(idAtomRule))
			{
				rule.kind |= ID::SUBKIND_RULE_CONSTRAINT;
			}
			if(!!boost::fusion::at_c<1>(idAtomRule))
			{
				std::vector<dlvhex::ID> idBody=boost::fusion::at_c<1>(idAtomRule).get();
				BOOST_FOREACH(const ID& id, idBody)
				{
					rule.body.push_back(id);
					if(id.isOrdinaryAtom())
					{
						OrdinaryAtom atom=reg->lookupOrdinaryAtom(id);
						if(atom.tuple[0]==mgr.expr)
						{
							bodyCasp.push_back(id);
						}
						else
						{
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
		ID ruleID=reg->storeRule(rule);
		mgr.ctx.idb.push_back(ruleID);
		cout<<printToString<RawPrinter>(ruleID,reg)<<endl;

		//create guessing rule
		BOOST_FOREACH(const ID& id, bodyCasp)
		{
			Rule guessingRule(ID::MAINKIND_RULE | ID::SUBKIND_RULE_REGULAR | ID::PROPERTY_RULE_DISJ);
			guessingRule.body.insert(guessingRule.body.end(),bodyWithoutCasp.begin(),bodyWithoutCasp.end());

			if (id.isLiteral()) {
				guessingRule.head.push_back(ID::posLiteralFromAtom(ID::atomFromLiteral(id)));
			}
			else {
				guessingRule.head.push_back(ID::posLiteralFromAtom(id));
			}

			OrdinaryAtom notCaspAtom = reg->lookupOrdinaryAtom(id);
			Term t = reg->terms.getByID(notCaspAtom.tuple[0]);
			//set name not_expr
			notCaspAtom.tuple[0]=mgr.not_expr;


			ID hid = reg->storeOrdinaryAtom(notCaspAtom);

			guessingRule.head.push_back(hid);

			ID guessingRuleID = reg->storeRule(guessingRule);
			mgr.ctx.idb.push_back(guessingRuleID);
			cout<<printToString<RawPrinter>(guessingRuleID,reg)<<endl;
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
		ostringstream os;
		//variables in expr
		vector<ID> variables;
		RegistryPtr reg = mgr.ctx.registry();
		bool caspVariable=false;

		ID variable;

		caspVariable=getVariable(reg,boost::fusion::at_c<0>(source),variable);

		//check if not is $z
		if(!caspVariable)
		{
			set<ID> var=reg->getVariablesInID(variable,true);
			variables.insert(variables.end(),var.begin(),var.end());
		}
		caspVariable=false;
		os<<printToString<RawPrinter>(variable, reg);
		std::vector<boost::fusion::vector2<dlvhex::ID, boost::variant<dlvhex::ID, std::basic_string<char> > > >  v=boost::fusion::at_c<1>(source);
		for(int i=0;i<v.size();i++)
		{

			boost::fusion::vector2<dlvhex::ID, boost::variant<dlvhex::ID, std::basic_string<char> > > v1=v[i];

			ID operatorID= boost::fusion::at_c<0>(v1);
			os<<printToString<RawPrinter>(operatorID, reg);
			caspVariable=getVariable(reg,boost::fusion::at_c<1>(v1),variable);
			os<<printToString<RawPrinter>(variable, reg);
			if(!caspVariable)
			{
				set<ID> var=reg->getVariablesInID(variable,true);
				variables.insert(variables.end(),var.begin(),var.end());
			}
			caspVariable=false;
		}

		//result atom of caspElement
		OrdinaryAtom atom(ID::MAINKIND_ATOM);
		atom.tuple.push_back(mgr.expr);
		ID expression=reg->storeConstantTerm("\""+os.str()+"\"");
		atom.tuple.push_back(expression);
		atom.tuple.insert(atom.tuple.end(),variables.begin(),variables.end());
		target=reg->storeOrdinaryAtom(atom);
	}

	//generate constant term from string or return ID of term
	bool getVariable(RegistryPtr& reg,const boost::variant<dlvhex::ID, std::basic_string<char> >& variant,ID& toReturn)
	{
		if(const std::basic_string<char>* variable = boost::get< std::basic_string<char> >( &variant ))
		{
			toReturn=reg->storeConstantTerm(*variable);
			return true;
		}
		else
		{
			toReturn=*( boost::get< ID >( &variant ));
			return false;
		}
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
		string predicateName;
		predicateName+=boost::fusion::at_c<0>(v);
		std::vector<char, std::allocator<char> > chars=boost::fusion::at_c<1>(v);
		for(int i=0;i<chars.size();i++)
			predicateName+=chars[i];
		ID predicateID=reg->storeConstantTerm(predicateName);

		//read i-th(1-based) argument
		ID integerID=boost::fusion::at_c<1>(source);
		assert(integerID.isIntegerTerm());

		ostringstream os;
		os<<printToString<RawPrinter>(integerID,reg);
		ExternalAtom ext=reg->eatoms.getByID(mgr.extAtom);
		ext.inputs.push_back(predicateID);

		mgr.caspPlugin.incrementNumberSumPredicate();
		target="sum_"+predicateName+"_"+os.str();
		//FIXME
		cout<<"regola  "<<printToString<RawPrinter>(mgr.ruleExt,reg)<<endl;
		cout<<"esterno "<<printToString<RawPrinter>(mgr.extAtom,reg)<<endl;
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
			string directive= "domain";
			OrdinaryAtom atom(ID::MAINKIND_ATOM);
			ID directiveID=reg->storeConstantTerm(directive);
			ID domainLowerBoudID =boost::fusion::at_c<0>(*upLowContainer);
			ID domainUpperBoudID =boost::fusion::at_c<1>(*upLowContainer);

			//domain must to have lower and upper bound as Integer
			assert(domainLowerBoudID.isIntegerTerm() && domainUpperBoudID.isIntegerTerm());

			atom.tuple.push_back(directiveID);
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
		target=reg->storeConstantTerm(source);
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
		string predicateName;
		predicateName+=boost::fusion::at_c<0>(v);
		std::vector<char, std::allocator<char> > chars=boost::fusion::at_c<1>(v);
		for(int i=0;i<chars.size();i++)
			predicateName+=chars[i];

		ID predicateID=reg->storeConstantTerm("\""+predicateName+"\"");

		//create atom $directive("namePredicate")
		OrdinaryAtom atom(ID::MAINKIND_ATOM);
		atom.tuple.push_back(reg->storeConstantTerm(directive));
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
		= (caspDirective | ((Base::headAtom % qi::lit("|") ) || (qi::lit(":-") > (( caspElement | Base::bodyLiteral ) % qi::char_(',')) )) >> qi::lit('.') ) [ Sem::caspRule(sem) ];
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
				qi::lit("$")>>(qi::string("==")|qi::string("+")| qi::string("-")|qi::string("*")|qi::string("/")|qi::string("%")|qi::string("<=")|qi::string(">=")| qi::string("<")|qi::string(">"))
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


	CASPParserModule(ProgramCtx& ctx,CASPPlugin& caspPlugin):
		HexParserModule(moduletype),
		sem(ctx,caspPlugin)
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
		ret.push_back(HexParserModulePtr(new CASPParserModule<HexParserModule::TOPLEVEL>(ctx,*this)));
	}
	return ret;
}

std::vector<PluginAtomPtr> CASPPlugin::createAtoms(ProgramCtx&) const
{
	std::vector<PluginAtomPtr> ret;
	ret.push_back(PluginAtomPtr(new ConsistencyAtom(_numberSumPredicate,_learningProcessor, _simpleParser), PluginPtrDeleter<PluginAtom>()));
	return ret;
}
DLVHEX_NAMESPACE_END
