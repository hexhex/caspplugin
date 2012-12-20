/*
 * CaspConcerter.h
 *
 *  Created on: Dec 17, 2012
 *      Author: faeton
 */

#ifndef CASPCONVERTER_H_
#define CASPCONVERTER_H_

#include "dlvhex2/PluginInterface.h"
#include <utility>

class CaspConverter : public dlvhex::PluginConverter {
public:
	CaspConverter();
	virtual ~CaspConverter();

	virtual void convert(std::istream& i, std::ostream& o);
	std::pair<int, int> getDomain();
};


#endif /* CASPCONCERTER_H_ */
