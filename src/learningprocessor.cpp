#include <dlvhex2/Nogood.h>
#include <dlvhex2/PluginInterface.h>

#include "casp/learningprocessor.h"
#include "casp/gecodesolver.h"

#include <vector>
#include <string>

using namespace dlvhex;
using namespace std;

void NoLearningProcessor::learnNogoods(NogoodContainerPtr nogoods,
		vector<string> expressions, vector<ID> atomIds, GecodeSolver* solver) {
}

void BackwardLearningProcessor::learnNogoods(NogoodContainerPtr nogoods,
		vector<string> expressions, vector<ID> atomIds, GecodeSolver* solver) {

	vector<ID> iis;
	for (int i = 0; i < expressions.size(); i++) {
		string oldExpression = expressions[i];
		expressions[i] = "";

		GecodeSolver *otherSolver = static_cast<GecodeSolver*>(solver->clone());
		otherSolver->propagate(expressions);

		Gecode::BAB<GecodeSolver> solutions(otherSolver);

		// If it is consistent, restore original constraint
		if (solutions.next()) {
			expressions[i] = oldExpression;
			iis.push_back(atomIds[i]);
		} // Otherwise we can safely remove it

		delete otherSolver;
	}

	Nogood nogood;
	BOOST_FOREACH(ID atomId, iis) {
		nogood.insert(NogoodContainer::createLiteral(atomId));
	}
	nogoods->addNogood(nogood);
}
