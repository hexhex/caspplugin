#include "casp/ConsistencyAtom.h"
#include <dlvhex2/OrdinaryAtomTable.h>
using namespace casp;

ConsistencyAtom::ConsistencyAtom(boost::shared_ptr<LearningProcessor> learningProcessor_,
		boost::shared_ptr<SimpleParser> simpleParser_,const CPVariableAndConnection& cpVariableAndConnection_,bool cspGraphLearning_,bool cspAnticipateLearning_) :
		PluginAtom( "casp", 0),
		learningProcessor(learningProcessor_),
		backwardlearningProcessor(new BackwardLearningProcessor(simpleParser_)),
		simpleParser(simpleParser_),
		idSaved(false),
		cpVariables(cpVariableAndConnection_.cpVariable),
		possibleConflictCpVariable(cpVariableAndConnection_.possibleConflictCpVariable),
		cspGraphLearning(cspGraphLearning_),
		cspAnticipateLearning(cspAnticipateLearning_)
{
	// This add predicates for all input parameters
	for (int i = 0; i < 6; i++)
		addInputPredicate();

	setOutputArity(0);
}


void ConsistencyAtom::retrieve(const Query& query, Answer& answer, NogoodContainerPtr nogoods) throw (PluginError)
{
	Interpretation toCheck;

	RegistryPtr registry = getRegistry();
	std::vector<std::string> expressions;
	std::vector<OrdinaryAtom> sumData;
	int domainMaxValue;
	int domainMinValue;
	bool definedDomain=false;
	std::string globalConstraintName = "";
	std::string globalConstraintValue = "";

	std::pair<Interpretation::TrueBitIterator, Interpretation::TrueBitIterator>
		trueAtoms ;
	InterpretationConstPtr toUse;
	if(query.assigned!=NULL)
	{
		trueAtoms= query.assigned->trueBits();
		toUse=query.assigned;
	}
	else
	{
		toUse=query.interpretation;
		trueAtoms= query.interpretation->trueBits();
	}


	vector<ID> atomIds;
	if(!idSaved)
		storeID(registry);

	// Iterate over all input interpretation
	for (Interpretation::TrueBitIterator it = trueAtoms.first; it != trueAtoms.second; it++) {
		const OrdinaryAtom &atom = toUse->getAtomToBit(it);
		Term name = registry->terms.getByID(atom.tuple[0]);
		if(!query.interpretation->getFact(*it)){
			continue;
		}
		string expr="";

		if (atom.tuple[0]==exprAuxID) {
			expressions.push_back(getExpressionFromID(registry,atom,false));
			ID atomID=registry->ogatoms.getIDByTuple(atom.tuple);
			atomIds.push_back(atomID);
			if(cspGraphLearning && possibleConflictCpVariable.find(atomID)!=possibleConflictCpVariable.end())
			{
				set< Interpretation* > s=possibleConflictCpVariable.at(atomID);
				for( set<Interpretation*>::iterator it=s.begin();it!=s.end();++it)
				{
					toCheck.add(**it);
				}
			}
		}
		else if (atom.tuple[0]==not_exprAuxID) {
			expressions.push_back(getExpressionFromID(registry,atom,true));
			ID atomID=registry->ogatoms.getIDByTuple(atom.tuple);
			atomIds.push_back(atomID);
			// if the atom doesn't contain ASP variables insert all atom that are possible conflict
			if(cspGraphLearning && possibleConflictCpVariable.find(atomID)!=possibleConflictCpVariable.end())
			{
				set< Interpretation* > s=possibleConflictCpVariable.at(atomID);
				for( set<Interpretation*>::iterator it=s.begin();it!=s.end();++it)
				{
					toCheck.add(**it);
				}
			}
		}
		else if (atom.tuple[0]==domainAuxID) {
			definedDomain=true;
			domainMinValue=atom.tuple[1].address;
			domainMaxValue=atom.tuple[2].address;;
		}
		else if (atom.tuple[0]==maximizeAuxID ||atom.tuple[0]==minimizeAuxID) {
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
	GecodeSolver* solver = new GecodeSolver(registry,sumData,domainMinValue, domainMaxValue, globalConstraintName, globalConstraintValue, simpleParser);
	solver->propagate(expressions);

	Gecode::Search::Options opt;
	Gecode::BAB<GecodeSolver> solutions(solver,opt);

	// If there's at least one solution, then data is consistent
	if (solutions.next()) {
		Tuple out;
		answer.get().push_back(out);
		if(cspAnticipateLearning && query.assigned!=NULL)
			anticipateLearning(registry,query.assigned,nogoods,expressions,atomIds,sumData,domainMinValue,domainMaxValue,globalConstraintName,globalConstraintValue,toCheck);
	}
	else if (nogoods != 0){ // otherwise we need to learn IIS from it
		GecodeSolver* otherSolver = new GecodeSolver(registry,sumData, domainMinValue,domainMaxValue, globalConstraintName, globalConstraintValue, simpleParser);
		learningProcessor->learnNogoods(nogoods, expressions, atomIds, otherSolver);
		delete otherSolver;
	}
	delete solver;
}


void ConsistencyAtom::anticipateLearning(RegistryPtr& registry,const InterpretationConstPtr& assigned,NogoodContainerPtr& nogoods,
		vector<string>& expressions,vector<ID>& atomIds,vector<OrdinaryAtom> &sumData,int domainMinValue,int domainMaxValue,string& globalConstraintName,string& globalConstraintValue,Interpretation& toCheck)
{
	pm.updateMask();
	Interpretation::TrueBitIterator it, it_end;
	boost::tie(it, it_end) = pm.mask()->trueBits();
	//iterate over all Ordinary ground atom (expr and not_expr)
	for(;it!=it_end;it++)
	{
		//if the atom is not assigned, add atom to lists in order to check consistency
		if(assigned->getFact(*it))
		{
			continue;
		}
		const OrdinaryAtom& atom=pm.mask()->getAtomToBit(it);

		if (atom.tuple[0]==exprAuxID && (!cspGraphLearning || !cpVariables.getFact(*it) || toCheck.getFact(*it))) {
			expressions.push_back(getExpressionFromID(registry,atom,false));
			atomIds.push_back(registry->ogatoms.getIDByTuple(atom.tuple));
		}
		else if (atom.tuple[0]==not_exprAuxID && (!cspGraphLearning ||!cpVariables.getFact(*it) || toCheck.getFact(*it))) {
			expressions.push_back(getExpressionFromID(registry,atom,true));
			atomIds.push_back(registry->ogatoms.getIDByTuple(atom.tuple));
		}


		GecodeSolver* solver = new GecodeSolver(registry,sumData,domainMinValue, domainMaxValue, globalConstraintName, globalConstraintValue, simpleParser);
		solver->propagate(expressions);

		Gecode::Search::Options opt;
		Gecode::BAB<GecodeSolver> solutions(solver,opt);

		//if the solution is not consistent  try to learn nogoods
		if (solutions.next()==NULL)
		{
			GecodeSolver* otherSolver = new GecodeSolver(registry,sumData, domainMinValue,domainMaxValue, globalConstraintName, globalConstraintValue, simpleParser);
			//try to learn no goods
			backwardlearningProcessor->learnNogoods(nogoods,expressions,atomIds,otherSolver);
			delete otherSolver;
		}
		delete solver;
		expressions.pop_back();
		atomIds.pop_back();
	}


}

void ConsistencyAtom::storeID( RegistryPtr& registry)
{
	domainAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(0));
	maximizeAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(1));
	minimizeAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(2));
	exprAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(3));
	not_exprAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(4));
	sumElementAuxID=registry->getAuxiliaryConstantSymbol('t',ID::termFromInteger(5));
	pm.setRegistry(registry);
	pm.addPredicate(exprAuxID);
	pm.addPredicate(not_exprAuxID);
	idSaved=true;
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
