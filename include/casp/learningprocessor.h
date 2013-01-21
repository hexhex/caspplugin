#ifndef LEARNINGPROCESSOR_H_
#define LEARNINGPROCESSOR_H_

#include <dlvhex2/Nogood.h>
#include <vector>
#include <string>

#include "casp/gecodesolver.h"

using namespace dlvhex;
using namespace std;

class LearningProcessor {
public:
	LearningProcessor() {}
	virtual ~LearningProcessor() {}
	virtual void learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions,
			vector<ID> atomIds, GecodeSolver *solver) = 0;
};

class BackwardLearningProcessor : public LearningProcessor {
	virtual void learnNogoods(NogoodContainerPtr nogoods, vector<string> expressions,
				vector<ID> atomIds, GecodeSolver *solver);
};


#endif
