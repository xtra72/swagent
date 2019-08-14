#ifndef	UTILS_H__
#define	UTILS_H__

#include <stdint.h>
#include <vector>
#include "libjson/libjson.h"

bool	GetMemberValue(JSONNode const& _object, std::string const& _title, std::string& _value);
bool	GetMemberValue(JSONNode const& _object, std::string const& _title, JSONNode& _value);
bool	GetMemberValue(JSONNode const& _object, std::string const& _title, bool& _value);
bool	GetMemberValue(JSONNode const& _object, std::string const& _title, uint32_t& _value);
bool	GetMemberValue(JSONNode const& _object, std::string const& _title, uint64_t& _value);
bool	SetMemberValue(JSONNode& _object, std::string const& _title, std::string const& _value);
bool	SetMemberValue(JSONNode& _object, std::string const& _title, uint16_t _value);
bool	SetMemberValue(JSONNode& _object, std::string const& _title, uint32_t _value);
bool	SetMemberValue(JSONNode& _object, std::string const& _title, uint64_t _value);
bool	SetMemberValue(JSONNode& _object, std::string const& _title, JSONNode& _value);

std::string ToHexString(uint64_t _value);
std::string ToHexString(uint16_t _value);

bool	HexStringToUint16(std::string const& _string, uint16_t& _value);
bool	HexStringToUint64(std::string const& _string, uint64_t& _value);
bool	StringToUint64(std::string const& _string ,uint64_t& _number);
bool	StringToUint8(std::string const& _string ,uint8_t* _array, uint32_t _max_length, uint32_t& _length);

bool	StringToUint32(std::string const& _string, uint32_t& value);
bool	StringToUint32(char* _string, uint32_t& value);
bool	StringToInt32(char* _string, int32_t& value);

std::vector<std::string> split(const std::string& s, char delimiter);

#endif
