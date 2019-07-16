#include "cp211x.h"


CP2110::CP2110(std::string const& _id, Object* _parent)
:	HIDUART(_id, SILABS_VID, CP2110_PID, "", _parent)
{
}

CP2110::CP2110(std::string const& _id, std::string const& _serial, Object* _parent)
:	HIDUART(_id, SILABS_VID, CP2110_PID, _serial, _parent)
{
}

CP2110::~CP2110()
{
}

uint32_t	CP2110::GetDeviceCount()
{
	return	HIDUART::GetDeviceCount(CP2110_PID, SILABS_VID);
}

bool	CP2110::GetSerial(uint32_t _index, std::string& _serial)
{
	return	HIDUART::GetSerial(SILABS_VID, CP2110_PID, _index, _serial);
}
