#include "casp/ConsistencyAtom.h"
using namespace casp;

ConsistencyAtom::ConsistencyAtom(int numberSumPredicate,boost::shared_ptr<LearningProcessor> learningProcessor,
		boost::shared_ptr<SimpleParser> simpleParser) :
		PluginAtom( "casp", 0),
		_learningProcessor(learningProcessor),
		_simpleParser(simpleParser),_idSaved(false)
{
	// This add predicates for all input parameters
	cout<<"Input num: "<<5+numberSumPredicate<<endl;
	for (int i = 0; i < 5+numberSumPredicate; i++)
		addInputPredicate();

	setOutputArity(0);
}


void ConsistencyAtom::retrieve(const Query& query, Answer& answer, NogoodContainerPtr nogoods) throw (PluginError)
{
	Registry &registry = *getRegistry();

	std::vector<std::string> expressions;
	std::vector<std::string> sumData;
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
			Term value = registry.terms.getByID(atom.tuple[1]);
			expr = value.symbol;
		}
		else if (atom.tuple[0]==_notExprID) {
			Term value = registry.terms.getByID(atom.tuple[1]);
			expr = replaceInvertibleOperator(value.symbol);
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
			sumData.push_back(atom.text);
		}
		if (expr != "") { // handle expr and not_expr
			// Replace all ASP variables with their actual values.
			int variableIndex = 2;

			boost::char_separator<char> sep(" ", ",v.:-$%<>=+-/*\"<>=()", boost::drop_empty_tokens);

			std::string result = "";
			boost::tokenizer<boost::char_separator<char> > tokens(expr, sep);
			for ( boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin(); it != tokens.end(); ++it) {
				std::string val = *it;
				if (isVariable(val)) {
					string res;
					ID id = atom.tuple[variableIndex++];
					if (id.isIntegerTerm()) {
						ostringstream os(res);
						os << id.address;
						res = os.str();
					}
					else if (id.isTerm()) {
						res = registry.terms.getByID(id).symbol;
					}
					else if (id.isOrdinaryAtom()) {
						res = registry.lookupOrdinaryAtom(id).text;
					}
					result += res;
				}
				else result += val;
			}

			// Store the replaced expression without quotes
			expressions.push_back(removeQuotes(result));
			atomIds.push_back(registry.ogatoms.getIDByTuple(atom.tuple));
		}
	}

	if(!definedDomain)
		throw dlvhex::PluginError("No domain specified");

	// Call gecode solver
	GecodeSolver* solver = new GecodeSolver(sumData,domainMinValue, domainMaxValue, globalConstraintName, globalConstraintValue, _simpleParser);
	solver->propagate(expressions);

	Gecode::Search::Options opt;
	Gecode::BAB<GecodeSolver> solutions(solver,opt);

	// If there's at least one solution, then data is consistent
	if (solutions.next()) {
		Tuple out;
		answer.get().push_back(out);
	}
	else if (nogoods != 0){ // otherwise we need to learn IIS from it
		GecodeSolver* otherSolver = new GecodeSolver(sumData, domainMinValue,domainMaxValue, globalConstraintName, globalConstraintValue, _simpleParser);
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
		_idSaved=true;
}

