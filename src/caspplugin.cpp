#include "casp/caspplugin.h"
#include <dlvhex2/Printer.h>
#include <boost/lexical_cast.hpp>
DLVHEX_NAMESPACE_BEGIN

using namespace dlvhex::casp;
namespace spirit = boost::spirit;
namespace qi = boost::spirit::qi;

CASPPlugin::CASPPlugin() :
_converter(new DefaultConverter()),
_rewriter(new DefaultRewriter()),
_simpleParser(new SimpleParser()),
_learningProcessor(new NoLearningProcessor(_simpleParser))
{
	setNameVersion(PACKAGE_TARNAME,CASPPLUGIN_VERSION_MAJOR,CASPPLUGIN_VERSION_MINOR,CASPPLUGIN_VERSION_MICRO);
}

std::vector<PluginAtomPtr> CASPPlugin::createAtoms(ProgramCtx&) const
{
std::vector<PluginAtomPtr> ret;

ret.push_back(PluginAtomPtr(new ConsistencyAtom(_learningProcessor, _simpleParser), PluginPtrDeleter<PluginAtom>()));

return ret;
}
void CASPPlugin::printUsage(std::ostream& o) const
{
	o << "     --cspenable - enabling casp plugin" << endl;
	o << "     --headguess - enabling guessing in constraint heads" << endl;
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
				boost::shared_ptr<PluginConverter> converter(new CaspConverter());
				boost::shared_ptr<PluginRewriter> rewriter(new CaspRewriter(false));
				_converter = converter;
				_rewriter = rewriter;

				it = pluginOptions.erase(it);
			}

			else if (option.find("--headguess") != std::string::npos) {
				boost::shared_ptr<PluginRewriter> rewriter(new CaspRewriter(true));
				_rewriter = rewriter;

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

//////////////////////////////////////////////////////////////////////////////////
class CASPParserModuleSemantics:
	public HexGrammarSemantics
{
public:
	CASPPlugin::CtxData& ctxdata;

public:
	CASPParserModuleSemantics(ProgramCtx& ctx):
		HexGrammarSemantics(ctx),
		ctxdata(ctx.getPluginData<CASPPlugin>())
	{
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

};

// create semantic handler for above semantic action
// (needs to be in globally specializable struct space)


template<>
struct sem<CASPParserModuleSemantics::caspRule>
{

  void operator()(
    CASPParserModuleSemantics& mgr,
    const boost::variant<dlvhex::ID, boost::fusion::vector2<dlvhex::ID, boost::optional<std::vector<dlvhex::ID> > > >& source,
    ID& target)
  {
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
			  set<ID> variables;
			  RegistryPtr reg = mgr.ctx.registry();
			  bool caspVariable=false;

			  ID variable;

			  caspVariable=getVariable(reg,boost::fusion::at_c<0>(source),variable);

			  if(!caspVariable)
			  {
				  reg->getVariablesInID(variable,variables,true);
			  }
			  caspVariable=false;
			  os<<printToString<RawPrinter>(variable, reg);
			  std::vector<boost::fusion::vector2<dlvhex::ID, boost::variant<dlvhex::ID, std::basic_string<char> > > >  v=boost::fusion::at_c<1>(source);
			  for(int i=0;i<v.size();i++)
			  {

				  boost::fusion::vector2<dlvhex::ID, boost::variant<dlvhex::ID, std::basic_string<char> > > v1=v[i];

				  ID operatorID= boost::fusion::at_c<0>(v1);
				  os<<printToString<RawPrinter>(operatorID, reg);
				  caspVariable=getVariable(reg,boost::fusion::at_c<0>(source),variable);
				  os<<printToString<RawPrinter>(variable, reg);
				  if(!caspVariable)
				  {
					  reg->getVariablesInID(variable,variables,true);
				  }
				  caspVariable=false;
			  }

			  //result atom of caspElement
			  OrdinaryAtom atom(ID::MAINKIND_ATOM);
			  string name="expr";
			  name= name + boost::lexical_cast< std::string >(variables.size());
			  ID atomName=reg->storeConstantTerm(name);
			  atom.tuple.push_back(atomName);
			  ID expression=reg->storeConstantTerm("\""+os.str()+"\"");
			  atom.tuple.push_back(expression);
			  for(set<ID>::iterator i=variables.begin();i!=variables.end();i++)
			  {
				  atom.tuple.push_back(*i);
			  }
			  target=reg->storeOrdinaryAtom(atom);
			  cout<<printToString<RawPrinter>(target, reg)<<endl;
	}

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
			const boost::fusion::vector2<char, std::vector<char, std::allocator<char> > > &  source,
			string& target)
	{
		target=boost::fusion::at_c<0>(source);
		std::vector<char, std::allocator<char> > v=boost::fusion::at_c<1>(source);
		for(int i=0;i<v.size();i++)
			target+=v[i];
	}
};
template<>
struct sem<CASPParserModuleSemantics::caspDirective>
{

  void operator()(
    CASPParserModuleSemantics& mgr,
    const boost::fusion::vector3<std::basic_string<char>, dlvhex::ID, dlvhex::ID>& source,
    ID& target)
  {
	  RegistryPtr reg = mgr.ctx.registry();
	  string directive= boost::fusion::at_c<0>(source);
	  if(directive!="domain" && directive!="maximize" && directive!="minimize")
	  {
		  assert(false);
		  return ;
	  }
	  ID directiveID=reg->storeConstantTerm(directive);
	  ID lowerID =boost::fusion::at_c<1>(source);
	  ID upperID =boost::fusion::at_c<2>(source);
	  OrdinaryAtom atom(ID::MAINKIND_ATOM);
	  atom.tuple.push_back(directiveID);
	  atom.tuple.push_back(lowerID);
	  atom.tuple.push_back(upperID);
	  target=reg->storeOrdinaryAtom(atom);

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
		target=dlvhex::ID::termFromBuiltinString(source);
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
  		= (caspDirective | (Base::headAtom >> -(qi::lit(":-") > (( caspElement | Base::bodyLiteral ) % qi::char_(',')) ) >> qi::lit('.') )) [ Sem::caspRule(sem) ];
  			caspElement
  		= (
  				(Base::term | caspVariable) >> +( caspOperator >> (Base::term | caspVariable)  )>> qi::eps
  		) [ Sem::caspElement(sem) ];

  			caspVariable
  		= (
  				 qi::lit("$") >> qi::lexeme[ ascii::upper >> *(ascii::alnum | qi::char_('_')) ]
		)[Sem::caspVariable(sem)];

  			caspDirective
  		= (
//  				$domain(0,100)
  				qi::lit("$")>>Base::cident>>qi::lit("(")>Base::term>qi::lit("..")>Base::term>>qi::lit(").")
  		)[ Sem::caspDirective(sem) ];
  			caspOperator
  		=(
  				qi::lit("$")>>(qi::string("==")|qi::string("+")| qi::string("-")|qi::string("*")|qi::string("/")|qi::string("%")| qi::string("<")|qi::string(">")|qi::string("<=")|qi::string("=>"))
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

	CASPParserModule(ProgramCtx& ctx):
		HexParserModule(moduletype),
		sem(ctx)
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<HexParserModulePtr>	CASPPlugin::createParserModules(ProgramCtx& ctx)
{
	DBGLOG(DBG,"CASPPlugin::createParserModules()");
	std::vector<HexParserModulePtr> ret;

	CASPPlugin::CtxData& ctxdata = ctx.getPluginData<CASPPlugin>();
	if( ctxdata.enabled )
	{
		ret.push_back(HexParserModulePtr(
				new CASPParserModule<HexParserModule::TOPLEVEL>(ctx)));
	}
	return ret;
}

DLVHEX_NAMESPACE_END
