#ifndef	CP211X_H_
#define	CP211X_H_

#include "hiduart.h"

class	CP2110 : public HIDUART
{
public:
	CP2110(std::string const& _serial = "", Object* _parent = NULL);
	~CP2110();

	static	uint32_t	GetDeviceCount();
	static	bool		GetSerial(uint32_t _index, std::string& _serial);
};
#endif

