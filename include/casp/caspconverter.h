#ifndef CASPCONVERTER_H_
#define CASPCONVERTER_H_

#include "dlvhex2/PluginInterface.h"
#include <utility>

class CaspConverter : public dlvhex::PluginConverter {
public:
	CaspConverter();
	virtual ~CaspConverter();

	virtual void convert(std::istream& i, std::ostream& o);
};


#endif /* CASPCONCERTER_H_ */
