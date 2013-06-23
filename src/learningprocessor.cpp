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

	vector<bool> processedFlags(expressions.size());
	for (int i = 0; i < expressions.size(); i++)
		processedFlags[i] = 0;

	vector<ID> iis;

	while (true) {
		GecodeSolver *innerSolver = static_cast<GecodeSolver*>(solver->clone());
		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i]) {
				innerSolver->propagate(expressions[i]);
			}
		}

		int propagateIndex = -1;
		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i] == 1)
				continue;

			innerSolver->propagate(expressions[i]);

			Gecode::BAB<GecodeSolver> solutions(innerSolver);
			// If it is inconsistent, IIS found, break
			if (!solutions.next()) {
				propagateIndex = i;
				break;
			}
		}

		delete innerSolver;

		if (propagateIndex == -1)
			break;

		processedFlags[propagateIndex] = 1;
		otherSolver->propagate(expressions[propagateIndex]);
		iis.push_back(atomIds[propagateIndex]);

		Gecode::BAB<GecodeSolver> solutions(otherSolver);
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

void JumpForwardLearningProcessor::learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions, vector<ID> atomIds,
		GecodeSolver* solver) {

	int jumpSize = 2;

	GecodeSolver *otherSolver = static_cast<GecodeSolver*>(solver->clone());

	vector<bool> processedFlags(expressions.size());
	for (int i = 0; i < expressions.size(); i++)
		processedFlags[i] = 0;

	vector<ID> iis;

	while (true) {

		GecodeSolver *innerSolver = static_cast<GecodeSolver*>(solver->clone());
		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i]) {
				innerSolver->propagate(expressions[i]);
			}
		}
		int propagateIndex = -1;
		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i])
				continue;

			innerSolver->propagate(expressions[i]);

			if (i % jumpSize == 0 || i == expressions.size() - 1) {
				Gecode::BAB<GecodeSolver> solutions(innerSolver);
				// If it is inconsistent, IIS found, break
				if (!solutions.next()) {
					processedFlags[i] = 1;
					propagateIndex = i;
					break;
				}
			}
		}
		delete innerSolver;

		if (propagateIndex == -1)
			break;

		otherSolver->propagate(expressions[propagateIndex]);
		iis.push_back(atomIds[propagateIndex]);

		Gecode::BAB<GecodeSolver> solutions(otherSolver);
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
	reverse(atomIds.begin(), atomIds.end());

	GecodeSolver *otherSolver = static_cast<GecodeSolver*>(solver->clone());

	vector<bool> processedFlags(expressions.size());
	for (int i = 0; i < expressions.size(); i++)
		processedFlags[i] = 0;

	vector<ID> iis;

	while (true) {
		GecodeSolver *innerSolver = static_cast<GecodeSolver*>(solver->clone());

		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i]) {
				innerSolver->propagate(expressions[i]);
			}
		}

		int propagateIndex = -1;
		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i])
				continue;

			innerSolver->propagate(expressions[i]);

			Gecode::BAB<GecodeSolver> solutions(innerSolver);
			// If it is inconsistent, IIS found, break
			if (!solutions.next()) {
				processedFlags[i] = 1;
				propagateIndex = i;
				break;
			}
		}
		delete innerSolver;

		if (propagateIndex == -1)
			break;

		otherSolver->propagate(expressions[propagateIndex]);
		iis.push_back(atomIds[propagateIndex]);

		Gecode::BAB<GecodeSolver> solutions(otherSolver);
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

void RangeLearningProcessor::learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions, vector<ID> atomIds,
		GecodeSolver* solver) {

	reverse(expressions.begin(), expressions.end());
	reverse(atomIds.begin(), atomIds.end());

	GecodeSolver *otherSolver = static_cast<GecodeSolver*>(solver->clone());

	// Note, that here actually not iis is computer, but approximation
	vector<ID> iis;

	for (int i = 0; i < expressions.size(); i++) {
		otherSolver->propagate(expressions[i]);
		iis.push_back(atomIds[i]);

		Gecode::BAB<GecodeSolver> solutions(otherSolver);
		// If it is inconsistent, select current subset as candidate
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

/**
 * @brief This helper method compares two possible constraints in CC learning.
 * The constraint is represent by pair <i,j> where i - the weight of constraint, j - index
 */
bool compareWeightedConstraints(pair<int, int> x, pair<int, int> y) {
	return x.first > y.first;
}

void CCLearningProcessor::learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions, vector<ID> atomIds, GecodeSolver* solver) {
	GecodeSolver *otherSolver = static_cast<GecodeSolver*>(solver->clone());

	vector<set<string> > expressionVariables(expressions.size());
	for (int i = 0; i < expressions.size(); i++) {
		ParseTree *root;
		_simpleParser->makeTree(expressions[i], &root);
		expressionVariables[i] = _simpleParser->getConstraintVariables(root);
		_simpleParser->deleteTree(root);
	}

	vector<bool> processedFlags(expressions.size());
	for (int i = 0; i < expressions.size(); i++) {
		processedFlags[i] = 0;
	}

	vector<ID> iis;
	set<string> currentVariables;

	while (true) {
		GecodeSolver *innerSolver = static_cast<GecodeSolver*>(solver->clone());
		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i]) {
				innerSolver->propagate(expressions[i]);
			}
		}

		int propagateIndex = -1;

		vector<pair<int, int> > weightedConstraints;

		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i])
				continue;
			vector<string> intersectionResult(currentVariables.size() + expressionVariables[i].size());

			vector<string>::iterator it = set_intersection(currentVariables.begin(), currentVariables.end(), expressionVariables[i].begin(),
					expressionVariables[i].end(), intersectionResult.begin());

			int size = int(it - intersectionResult.begin()) > 0 ? 1 : 0;

			pair<int, int> weightedConstraint(size, i);
			weightedConstraints.push_back(weightedConstraint);
		}

		sort(weightedConstraints.begin(), weightedConstraints.end(), compareWeightedConstraints);

		for (int i = 0; i < weightedConstraints.size(); i++) {
			int index = weightedConstraints[i].second;

			innerSolver->propagate(expressions[index]);
			Gecode::BAB<GecodeSolver> solutions(innerSolver);
			// If it is inconsistent, IIS found, break
			if (!solutions.next()) {
				processedFlags[i] = 1;
				propagateIndex = index;
				break;
			}
		}
		delete innerSolver;

		if (propagateIndex == -1)
			break;

		otherSolver->propagate(expressions[propagateIndex]);
		iis.push_back(atomIds[propagateIndex]);
		currentVariables.insert(expressionVariables[propagateIndex].begin(), expressionVariables[propagateIndex].end());

		Gecode::BAB<GecodeSolver> solutions(otherSolver);
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

void WeightedCCLearningProcessor::learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions, vector<ID> atomIds,
		GecodeSolver* solver) {

	GecodeSolver *otherSolver = static_cast<GecodeSolver*>(solver->clone());

	vector<set<string> > expressionVariables(expressions.size());
	for (int i = 0; i < expressions.size(); i++) {
		ParseTree *root;
		_simpleParser->makeTree(expressions[i], &root);
		expressionVariables[i] = _simpleParser->getConstraintVariables(root);
		_simpleParser->deleteTree(root);
	}

	vector<bool> processedFlags(expressions.size());
	for (int i = 0; i < expressions.size(); i++) {
		processedFlags[i] = 0;
	}

	vector<ID> iis;
	set<string> currentVariables;

	while (true) {

		GecodeSolver *innerSolver = static_cast<GecodeSolver*>(solver->clone());
		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i]) {
				innerSolver->propagate(expressions[i]);
			}
		}

		int propagateIndex = -1;

		vector<pair<int, int> > weightedConstraints;

		for (int i = 0; i < expressions.size(); i++) {
			if (processedFlags[i])
				continue;
			vector<string> intersectionResult(currentVariables.size() + expressionVariables[i].size());

			vector<string>::iterator it = set_intersection(currentVariables.begin(), currentVariables.end(), expressionVariables[i].begin(),
					expressionVariables[i].end(), intersectionResult.begin());

			int size = int(it - intersectionResult.begin());

			pair<int, int> weightedConstraint(size, i);
			weightedConstraints.push_back(weightedConstraint);
		}
		sort(weightedConstraints.begin(), weightedConstraints.end(), compareWeightedConstraints);

		for (int i = 0; i < weightedConstraints.size(); i++) {
			int index = weightedConstraints[i].second;
			innerSolver->propagate(expressions[index]);

			Gecode::BAB<GecodeSolver> solutions(innerSolver);
			// If it is inconsistent, IIS found, break
			if (!solutions.next()) {
				processedFlags[i] = 1;
				propagateIndex = index;
				break;
			}
		}
		delete innerSolver;

		if (propagateIndex == -1)
			break;

		otherSolver->propagate(expressions[propagateIndex]);
		iis.push_back(atomIds[propagateIndex]);
		currentVariables.insert(expressionVariables[propagateIndex].begin(), expressionVariables[propagateIndex].end());

		Gecode::BAB<GecodeSolver> solutions(otherSolver);
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
