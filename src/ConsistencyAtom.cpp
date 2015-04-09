#include "casp/ConsistencyAtom.h"
#include <dlvhex2/OrdinaryAtomTable.h>
using namespace casp;

ConsistencyAtom::ConsistencyAtom(boost::shared_ptr<LearningProcessor> learningProcessor,
		boost::shared_ptr<SimpleParser> simpleParser,const CPVariableAndConnection& cpVariableAndConnection,bool cspGraphLearning) :
		PluginAtom( "casp", 0),
		_learningProcessor(learningProcessor),
		_simpleParser(simpleParser),_idSaved(false),
		_cpVariables(cpVariableAndConnection.cpVariable),
		_possibleConflictCpVariable(cpVariableAndConnection.possibleConflictCpVariable),
		_cspGraphLearning(cspGraphLearning)
{
	// This add predicates for all input parameters
	for (int i = 0; i < 6; i++)
		addInputPredicate();

	setOutputArity(0);
}


void ConsistencyAtom::retrieve(const Query& query, Answer& answer, NogoodContainerPtr nogoods) throw (PluginError)
{
	SetID toCheck;

	RegistryPtr registry = getRegistry();
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
		Term name = registry->terms.getByID(atom.tuple[0]);
		if(!query.interpretation->getFact(*it)){
			continue;
		}
		string expr="";

		if (atom.tuple[0]==_exprID) {
			expressions.push_back(getExpressionFromID(registry,atom,false));
			ID atomID=registry->ogatoms.getIDByTuple(atom.tuple);
			atomIds.push_back(atomID);
			if(_cspGraphLearning && _possibleConflictCpVariable.find(atomID)!=_possibleConflictCpVariable.end())
			{
				set< SetID* > s=_possibleConflictCpVariable.at(atomID);
				for( set<SetID*>::iterator it=s.begin();it!=s.end();++it)
				{
					toCheck.insert((*it)->begin(),(*it)->end());
				}
			}
		}
		else if (atom.tuple[0]==_notExprID) {
			expressions.push_back(getExpressionFromID(registry,atom,true));
			ID atomID=registry->ogatoms.getIDByTuple(atom.tuple);
			atomIds.push_back(atomID);
			// if the atom doesn't contain ASP variables insert all atom that are possible conflict
			if(_cspGraphLearning && _possibleConflictCpVariable.find(atomID)!=_possibleConflictCpVariable.end())
			{
				set< SetID* > s=_possibleConflictCpVariable.at(atomID);
				for( set<SetID*>::iterator it=s.begin();it!=s.end();++it)
				{
					toCheck.insert((*it)->begin(),(*it)->end());
				}
			}
		}
		else if (atom.tuple[0]==_domID) {
			definedDomain=true;
			domainMinValue=atom.tuple[1].address;
			domainMaxValue=atom.tuple[2].address;;
		}
		else if (atom.tuple[0]==_maxID ||atom.tuple[0]==_minID) {
			globalConstraintName = name.symbol;
			globalConstraintValue = removeQuotes(registry->terms.getByID(atom.tuple[1]).symbol);
		}
		else { // this predicate received as input to sum aggregate function
			sumData.push_back(atom);
		}
	}

	if(!definedDomain)
		throw dlvhex::PluginError("No domain specified");

//	 Call gecode solver
	GecodeSolver* solver = new GecodeSolver(registry,sumData,domainMinValue, domainMaxValue, globalConstraintName, globalConstraintValue, _simpleParser);
	solver->propagate(expressions);

	Gecode::Search::Options opt;
	Gecode::BAB<GecodeSolver> solutions(solver,opt);

	// If there's at least one solution, then data is consistent
	if (solutions.next()) {
		Tuple out;
		answer.get().push_back(out);
		tryToLearnMore(registry,query.assigned,nogoods,expressions,atomIds,sumData,domainMinValue,domainMaxValue,globalConstraintName,globalConstraintValue,toCheck);
	}
	else if (nogoods != 0){ // otherwise we need to learn IIS from it
		GecodeSolver* otherSolver = new GecodeSolver(registry,sumData, domainMinValue,domainMaxValue, globalConstraintName, globalConstraintValue, _simpleParser);
		_learningProcessor->learnNogoods(nogoods, expressions, atomIds, otherSolver);
		delete otherSolver;
	}
	delete solver;
}


void ConsistencyAtom::tryToLearnMore(RegistryPtr& registry,const InterpretationConstPtr& assigned,NogoodContainerPtr& nogoods,
		vector<string>& expressions,vector<ID>& atomIds,vector<OrdinaryAtom> &sumData,int domainMinValue,int domainMaxValue,string& globalConstraintName,string& globalConstraintValue,SetID& toCheck)
{
	_pm.updateMask();
	Interpretation::TrueBitIterator it, it_end;
	boost::tie(it, it_end) = _pm.mask()->trueBits();
	//iterate over all Ordinary ground atom (expr and not_expr)
	for(;it!=it_end;it++)
	{
		//if the atom is not assigned, add atom to lists in order to check consistency
		if(assigned->getFact(*it))
		{
			continue;
		}
		const OrdinaryAtom& atom=_pm.mask()->getAtomToBit(it);
		ID atomID=registry->ogatoms.getIDByAddress(*it);

		if (atom.tuple[0]==_exprID && (!_cspGraphLearning || _cpVariables.find(atomID)==_cpVariables.end() || toCheck.find(atomID)!=toCheck.end())) {
			expressions.push_back(getExpressionFromID(registry,atom,false));
			atomIds.push_back(registry->ogatoms.getIDByTuple(atom.tuple));

		}
		else if (atom.tuple[0]==_notExprID && (!_cspGraphLearning || _cpVariables.find(atomID)==_cpVariables.end() || toCheck.find(atomID)!=toCheck.end())) {
			expressions.push_back(getExpressionFromID(registry,atom,true));
			atomIds.push_back(registry->ogatoms.getIDByTuple(atom.tuple));
		}


		GecodeSolver* solver = new GecodeSolver(registry,sumData,domainMinValue, domainMaxValue, globalConstraintName, globalConstraintValue, _simpleParser);
		solver->propagate(expressions);

		Gecode::Search::Options opt;
		Gecode::BAB<GecodeSolver> solutions(solver,opt);

		//if the solution is not consistent  try to learn nogoods
		if (solutions.next()==NULL)
		{
			GecodeSolver* otherSolver = new GecodeSolver(registry,sumData, domainMinValue,domainMaxValue, globalConstraintName, globalConstraintValue, _simpleParser);
			//try to learn no goods
			_learningProcessor->learnNogoods(nogoods,expressions,atomIds,otherSolver);
			delete otherSolver;
		}
		delete solver;
		expressions.pop_back();
		atomIds.pop_back();
	}


}

void ConsistencyAtom::storeID( RegistryPtr& registry)
{
		_exprID=registry->storeConstantTerm("expr");
		_notExprID=registry->storeConstantTerm("not_expr");
		_domID=registry->storeConstantTerm("domain");
		_maxID=registry->storeConstantTerm("maximize");
		_minID=registry->storeConstantTerm("minimize");
		_sumElementID=registry->storeConstantTerm("sumElement");
		_pm.setRegistry(registry);
		_pm.addPredicate(_exprID);
		_pm.addPredicate(_notExprID);
		_idSaved=true;
}


string ConsistencyAtom::getExpressionFromID(RegistryPtr& reg, const OrdinaryAtom& atom,bool replaceReversibleOperator )
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
