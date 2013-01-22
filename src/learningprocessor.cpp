#include <dlvhex2/Nogood.h>
#include <dlvhex2/PluginInterface.h>

#include "casp/learningprocessor.h"
#include "casp/gecodesolver.h"

#include <vector>
#include <string>

using namespace dlvhex;
using namespace std;

// TODO: add special case check for last constraint

void NoLearningProcessor::learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions, vector<ID> atomIds, GecodeSolver* solver) {
}

void DeletionLearningProcessor::learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions, vector<ID> atomIds,
		GecodeSolver* solver) {

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

void ForwardLearningProcessor::learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions, vector<ID> atomIds,
		GecodeSolver* solver) {

	GecodeSolver *otherSolver = static_cast<GecodeSolver*>(solver->clone());

	vector<ID> iis;

	for (int i = 0; i < expressions.size(); i++) {
		otherSolver->propagate(expressions[i]);

		iis.push_back(atomIds[i]);

		Gecode::BAB<GecodeSolver> solutions(otherSolver);
		// If it is inconsistent, IIS found, break
		if (!solutions.next()) {
			break;
		}

	}
	delete otherSolver;

	Nogood nogood;
	BOOST_FOREACH(ID atomId, iis) {
		nogood.insert(NogoodContainer::createLiteral(atomId));
	}
	nogoods->addNogood(nogood);
}

void BackwardLearningProcessor::learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions, vector<ID> atomIds,
		GecodeSolver* solver) {

	reverse(expressions.begin(), expressions.end());

	GecodeSolver *otherSolver = static_cast<GecodeSolver*>(solver->clone());

	vector<ID> iis;

	for (int i = 0; i < expressions.size(); i++) {
		otherSolver->propagate(expressions[i]);

		iis.push_back(atomIds[i]);

		Gecode::BAB<GecodeSolver> solutions(otherSolver);
		// If it is inconsistent, IIS found, break
		if (!solutions.next()) {
			break;
		}

	}
	delete otherSolver;

	Nogood nogood;
	BOOST_FOREACH(ID atomId, iis) {
		nogood.insert(NogoodContainer::createLiteral(atomId));
	}
	nogoods->addNogood(nogood);
}

void CCLearningProcessor::learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions, vector<ID> atomIds, GecodeSolver* solver) {

	vector<bool> processedFlags(expressions.size());
	for (int i = 0; i < expressions.size(); i++)
		processedFlags[i] = 0;

	set<string> currentVariables;

	GecodeSolver *otherSolver = static_cast<GecodeSolver*>(solver->clone());

	vector<ID> iis;

	cout << "Processing: ";
	while (true) {
		int processIndex = -1;
		set<string> expressionVariables;

		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i]) continue;

			processIndex = i;

			ParseTree *root = new ParseTree;
			_simpleParser->makeTree(expressions[i], &root);
			expressionVariables = _simpleParser->getConstraintVariables(root);
			delete root;

			vector<string> intersectionResult(currentVariables.size() +  expressionVariables.size());
			vector<string>::iterator it = set_intersection(currentVariables.begin(), currentVariables.end(),
					expressionVariables.begin(), expressionVariables.end(), intersectionResult.begin());

			if ((int(it - intersectionResult.begin())) != 0) {
				break;
			}
		}
		if (processIndex == -1) break;

		currentVariables.insert(expressionVariables.begin(), expressionVariables.end());

		otherSolver->propagate(expressions[processIndex]);
		iis.push_back(atomIds[processIndex]);

		processedFlags[processIndex] = 1;

		Gecode::BAB<GecodeSolver> solutions(otherSolver);

		// If it is inconsistent, IIS found, break
		if (!solutions.next()) {
			break;
		}
	}
	cout << endl;
	delete otherSolver;

	Nogood nogood;
	BOOST_FOREACH(ID atomId, iis) {
		nogood.insert(NogoodContainer::createLiteral(atomId));
	}
	nogoods->addNogood(nogood);

}

void WeightedCCLearningProcessor::learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions, vector<ID> atomIds,
		GecodeSolver* solver) {
	vector<bool> processedFlags(expressions.size());
	for (int i = 0; i < expressions.size(); i++)
		processedFlags[i] = 0;

	set<string> currentVariables;

	GecodeSolver *otherSolver = static_cast<GecodeSolver*>(solver->clone());

	vector<ID> iis;

	while (true) {
		int processIndex = -1, bestCardinality = -1;
		set<string> expressionVariables;

		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i])
				continue;


			ParseTree *root = new ParseTree;
			_simpleParser->makeTree(expressions[i], &root);
			expressionVariables = _simpleParser->getConstraintVariables(root);
			delete root;

			vector<string> intersectionResult(currentVariables.size() + expressionVariables.size());
			vector<string>::iterator it = set_intersection(currentVariables.begin(), currentVariables.end(), expressionVariables.begin(),
					expressionVariables.end(), intersectionResult.begin());

			int cardinality = (int(it - intersectionResult.begin()));
			if (cardinality > bestCardinality ) {
				processIndex = i;
				bestCardinality = cardinality;
			}
		}
		if (processIndex == -1)
			break;

		currentVariables.insert(expressionVariables.begin(), expressionVariables.end());

		otherSolver->propagate(expressions[processIndex]);
		iis.push_back(atomIds[processIndex]);

		processedFlags[processIndex] = 1;

		Gecode::BAB<GecodeSolver> solutions(otherSolver);

		// If it is inconsistent, IIS found, break
		if (!solutions.next()) {
			break;
		}
	}
	delete otherSolver;

	Nogood nogood;
	BOOST_FOREACH(ID atomId, iis) {
		nogood.insert(NogoodContainer::createLiteral(atomId));
	}
	nogoods->addNogood(nogood);
}
