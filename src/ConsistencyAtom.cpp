#include "casp/ConsistencyAtom.h"
#include <dlvhex2/OrdinaryAtomTable.h>
using namespace casp;

ConsistencyAtom::ConsistencyAtom(boost::shared_ptr<LearningProcessor> learningProcessor,
		boost::shared_ptr<SimpleParser> simpleParser) :
		PluginAtom( "casp", 0),
		_learningProcessor(learningProcessor),
		_simpleParser(simpleParser),_idSaved(false)
{
	// This add predicates for all input parameters
	for (int i = 0; i < 6; i++)
		addInputPredicate();

	setOutputArity(0);
}


void ConsistencyAtom::retrieve(const Query& query, Answer& answer, NogoodContainerPtr nogoods) throw (PluginError)
{
	Registry &registry = *getRegistry();
	std::vector<std::string> expressions;
	std::vector<OrdinaryAtom> sumData;
	int domainMaxValue;
	int domainMinValue;
	bool definedDomain=false;
	std::string globalConstraintName = "";
	std::string globalConstraintValue = "";

	std::pair<Interpretation::TrueBitIterator, Interpretation::TrueBitIterator>
	trueAtoms = query.assigned->trueBits();

	vector<ID> atomIds;
	if(!_idSaved)
		storeID(registry);

	// Iterate over all input interpretation
	for (Interpretation::TrueBitIterator it = trueAtoms.first; it != trueAtoms.second; it++) {
		const OrdinaryAtom &atom = query.assigned->getAtomToBit(it);
		Term name = registry.terms.getByID(atom.tuple[0]);
		if(!query.interpretation->getFact(*it)){
			continue;
		}
		string expr="";

		if (atom.tuple[0]==_exprID) {
			expressions.push_back(getExpressionFromID(getRegistry(),atom,false));
			atomIds.push_back(registry.ogatoms.getIDByTuple(atom.tuple));
		}
		else if (atom.tuple[0]==_notExprID) {
			expressions.push_back(getExpressionFromID(getRegistry(),atom,true));
			atomIds.push_back(registry.ogatoms.getIDByTuple(atom.tuple));
		}
		else if (atom.tuple[0]==_domID) {
			definedDomain=true;
			domainMinValue=atom.tuple[1].address;
			domainMaxValue=atom.tuple[2].address;;
		}
		else if (atom.tuple[0]==_maxID ||atom.tuple[0]==_minID) {
			globalConstraintName = name.symbol;
			globalConstraintValue = removeQuotes(registry.terms.getByID(atom.tuple[1]).symbol);
		}
		else { // this predicate received as input to sum aggregate function
			sumData.push_back(atom);
		}
	}

	if(!definedDomain)
		throw dlvhex::PluginError("No domain specified");

	// Call gecode solver
	GecodeSolver* solver = new GecodeSolver(getRegistry(),sumData,domainMinValue, domainMaxValue, globalConstraintName, globalConstraintValue, _simpleParser);
	solver->propagate(expressions);

	Gecode::Search::Options opt;
	Gecode::BAB<GecodeSolver> solutions(solver,opt);

	// If there's at least one solution, then data is consistent
	if (solutions.next()) {
		Tuple out;
		answer.get().push_back(out);

		OrdinaryAtomTable::AddressIterator it, it_end;
		boost::tie(it, it_end) = registry.ogatoms.getAllByAddress();

		//iterate over all Ordinary ground atom
		while(it!=it_end)
		{
			if(query.assigned->getFact(it_end-it))
			{
				it++;
				continue;
			}
			//if the atom is not assigned, add true atom to lists in order to learn nogoods
			const OrdinaryAtom& atom=*it;
			if(atom.tuple[0]==_exprID)
			{
				expressions.push_back(getExpressionFromID(getRegistry(),atom,false));
				atomIds.push_back(registry.ogatoms.getIDByTuple(atom.tuple));
			}
			it++;
		}
		GecodeSolver* otherSolver = new GecodeSolver(getRegistry(),sumData, domainMinValue,domainMaxValue, globalConstraintName, globalConstraintValue, _simpleParser);
		//try to learn no goods
		_learningProcessor->learnNogoods(nogoods,expressions,atomIds,otherSolver);
	}
	else if (nogoods != 0){ // otherwise we need to learn IIS from it
		GecodeSolver* otherSolver = new GecodeSolver(getRegistry(),sumData, domainMinValue,domainMaxValue, globalConstraintName, globalConstraintValue, _simpleParser);
		_learningProcessor->learnNogoods(nogoods, expressions, atomIds, otherSolver);
		delete otherSolver;
	}
	delete solver;
}

void ConsistencyAtom::storeID( dlvhex::Registry& registry)
{
		_exprID=registry.storeConstantTerm("expr");
		_notExprID=registry.storeConstantTerm("not_expr");
		_domID=registry.storeConstantTerm("domain");
		_maxID=registry.storeConstantTerm("maximize");
		_minID=registry.storeConstantTerm("minimize");
		_sumElementID=registry.storeConstantTerm("sumElement");
		_idSaved=true;
}


string ConsistencyAtom::getExpressionFromID(RegistryPtr reg, const OrdinaryAtom& atom,bool replaceReversibleOperator )
{
	ostringstream os;
	for(int i=1;i<atom.tuple.size();i++)
	{
		if(atom.tuple[i].isConstantTerm())
		{
			string str=reg->getTermStringByID(atom.tuple[i]);
			os<<removeQuotes(str);
		}
		else
		{
			os<<printToString<RawPrinter>(atom.tuple[i],reg);
		}
	}
	string toReturn;
	if(replaceReversibleOperator)
	{
		toReturn= replaceInvertibleOperator(os.str());
	}
	else
	{
		toReturn=os.str();
	}
	return toReturn;
}
