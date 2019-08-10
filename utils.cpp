#include <iostream>
#include <sstream>
#include <iomanip>
#include "utils.h"


bool	GetMemberValue(JSONNode const& _object, std::string const& _title, std::string& _value)
{
	JSONNode::const_iterator it = _object.find(_title);
	if ((it == _object.end()) || (it->type() != JSON_STRING))
	{
		return	false;
	}

	_value = it->as_string();

	return	true;
}

bool	GetMemberValue(JSONNode const& _object, std::string const& _title, JSONNode& _value)
{
	JSONNode::const_iterator it = _object.find(_title);
	if ((it == _object.end()) || ((it->type() != JSON_NODE) && (it->type() != JSON_ARRAY)))
	{
		return	false;
	}

	if (it->type() == JSON_NODE)
	{
		_value = it->as_node();
	}
	else
	{
		_value = it->as_array();
	}

	return	true;
}

bool	GetMemberValue(JSONNode const& _object, std::string const& _title, bool& _value)
{
	JSONNode::const_iterator it = _object.find(_title);
	if ((it == _object.end()) || (it->type() != JSON_BOOL))
	{
		return	false;
	}
	_value = it->as_bool();

	return	true;
}

bool	GetMemberValue(JSONNode const& _object, std::string const& _title, uint32_t& _value)
{
	JSONNode::const_iterator it = _object.find(_title);
	if ((it == _object.end()) || (it->type() != JSON_NUMBER))
	{
		return	false;
	}
	_value = it->as_int();

	return	true;
}

bool	GetMemberValue(JSONNode const& _object, std::string const& _title, uint64_t& value)
{
	JSONNode::const_iterator it = _object.find(_title);

	if (it == _object.end())
	{
		return	false;
	}

	if (it->type() == JSON_STRING)
	{
		std::istringstream iss(it->as_string());
		iss >> value;
	}
	else 
	{
		value = it->as_int();
	}

	return	true;
}

bool	SetMemberValue(JSONNode& _object, std::string const& _title, std::string const& _value)
{
	if (_object.type() != JSON_NODE)
	{
		return	false;
	}

	auto it = _object.find(_title);
	if (it == _object.end())
	{
		_object.push_back(JSONNode(_title, _value));
	}
	else
	{
		*it = _value;
	}

	return	true;
}

bool	SetMemberValue(JSONNode& _object, std::string const& _title, uint16_t _value)
{
	if (_object.type() != JSON_NODE)
	{
		return	false;
	}

	auto it = _object.find(_title);
	if (it == _object.end())
	{
		_object.push_back(JSONNode(_title, _value));
	}
	else
	{
		*it = _value;
	}

	return	true;
}

bool	SetMemberValue(JSONNode& _object, std::string const& _title, uint32_t _value)
{
	if (_object.type() != JSON_NODE)
	{
		return	false;
	}

	auto it = _object.find(_title);
	if (it == _object.end())
	{
		_object.push_back(JSONNode(_title, _value));
	}
	else
	{
		*it = _value;
	}

	return	true;
}

bool	SetMemberValue(JSONNode& _object, std::string const& _title, uint64_t _value)
{
	if (_object.type() != JSON_NODE)
	{
		return	false;
	}

	auto it = _object.find(_title);
	if (it == _object.end())
	{
		_object.push_back(JSONNode(_title, std::to_string(_value)));
	}
	else
	{
		*it = _value;
	}

	return	true;
}

bool	SetMemberValue(JSONNode& _object, std::string const& _title, JSONNode& _value)
{
	if (_object.type() != JSON_NODE)
	{
		return	false;
	}

	auto it = _object.find(_title);
	if (it == _object.end())
	{
		JSONNode	new_node = _value;

		new_node.set_name(_title);
		_object.push_back(new_node);
	}
	else
	{
		*it = _value;
	}

	return	true;
}

std::string ToHexString(uint64_t _value)
{
	std::ostringstream	oss;

	oss << std::hex;

	oss << std::setfill('0') << std::setw(2) << (_value >> 56);
	for(int i = 6 ; i >= 0 ; i--)
	{
		oss << " " << std::setfill('0') << std::setw(2) << ((_value >> (i * 8)) & 0xFF);
	}

	return	oss.str();
}

std::string ToHexString(uint16_t _value)
{
	std::ostringstream	oss;

	oss << std::hex;

	oss << std::setfill('0') << std::setw(2) << (_value >> 8);
	oss << " " << std::setfill('0') << std::setw(2) << (_value & 0xFF);

	return	oss.str();
}

template <class T>
bool	HexStringToUint(std::string const& _string, uint32_t max_len, T& _value)
{
	T	value = 0;

	if (_string.length() > max_len)
	{
		return	false;
	}

	for(uint32_t i = 0 ; i < _string.length() ; i++)
	{
		if (('0' <= _string[i]) && (_string[i] <= '9'))
		{
			value = (value << 4) + _string[i] - '0';
		}
		else if (('A' <= _string[i]) && (_string[i] <= 'F'))
		{
			value = (value << 4) + _string[i] - 'A' + 10;
		}
		else if (('a' <= _string[i]) && (_string[i] <= 'f'))
		{
			value = (value << 4) + _string[i] - 'a' + 10;
		}
		else
		{
			return	false;
		}
	}

	_value = value;

	return	true;
}

bool	HexStringToUint16(std::string const& _string, uint16_t& _value)
{
	return	HexStringToUint<uint16_t>(_string, 4, _value);
}

bool	HexStringToUint64(std::string const& _string, uint64_t& _value)
{
	return	HexStringToUint<uint64_t>(_string, 16, _value);
}

bool	StringToUint64(std::string const& _string, uint64_t& _value)
{
	std::istringstream iss(_string);
	iss >> _value;

	return	true;
}

bool	StringToUint8(std::string const& _string ,uint8_t* _array, uint32_t _max_length, uint32_t& _length)
{
	if ((_string.length() % 2 != 0) || (_string.length() > _max_length * 2))
	{
		return	false;
	}

	for(uint32_t i = 0 ; i < _string.length() ; i+=2)
	{
		_array[i/2] = 0;

		if (('0' <= _string[i]) && (_string[i] <= '9'))
		{
			_array[i/2] = (_string[i] - '0') << 4;
		}
		else if (('A' <= _string[i]) && (_string[i] <= 'F'))
		{
			_array[i/2] = (_string[i] - 'A' + 10) << 4;
		}
		else if (('a' <= _string[i]) && (_string[i] <= 'f'))
		{
			_array[i/2] = (_string[i] - 'a' + 10) << 4;
		}
		else
		{
			return	false;
		}

		if (('0' <= _string[i + 1]) && (_string[i + 1] <= '9'))
		{
			_array[i/2] += (_string[i + 1] - '0');
		}
		else if (('A' <= _string[i + 1]) && (_string[i + 1] <= 'F'))
		{
			_array[i/2] += (_string[i + 1] - 'A' + 10);
		}
		else if (('a' <= _string[i + 1]) && (_string[i + 1] <= 'f'))
		{
			_array[i/2] += (_string[i + 1] - 'a' + 10);
		}
		else
		{
			return	false;
		}
	}

	_length = _string.length() / 2;

	return	true;
}

bool	StringToUint32(std::string const& _string, uint32_t& _value)
{
	if (10 < _string.length())
	{
		return	false;
	}

	for(uint32_t i = 0 ; i < _string.length() ; i++)
	{
		if ((_string[i] < '0') || ('9' < _string[i]))
		{
			return	false;
		}
	}

	_value = strtoul(_string.c_str(), 0, 10);

	return	true;
}

bool	StringToUint32(char* _string, uint32_t& _value)
{
	if (_string == NULL)
	{
		return	false;
	}

	while(*_string != 0)
	{
		if (!isspace(*_string))
		{
			break;
		}
		_string++;
	}

	char*	ptr = _string;
	while(*ptr != 0)
	{
		if (isspace(*_string))
		{
			*ptr = 0;
			break;
		}
		ptr++;
	}


	if (10 < strlen(_string))
	{
		return	false;
	}

	for(uint32_t i = 0 ; i < strlen(_string) ; i++)
	{
		if ((_string[i] < '0') || ('9' < _string[i]))
		{
			return	false;
		}
	}

	_value = strtoul(_string, 0, 10);

	return	true;
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}


