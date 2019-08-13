#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include "libjson/libjson.h"
#include "config.h"
#include "exception.h"
#include "trace.h"

Config::Config(std::string const& _file_name)
:	file_name_(_file_name)
{
}

Config::~Config()
{
}


bool	Config::Load(int _argc, char* _argv[])
{
	int	opt;

	while ((opt = getopt(_argc, _argv, "c:")) != -1) 
	{
		switch (opt) 
		{
		case 'c':
			file_name_ = optarg;
			break;
		default: /* '?' */
			return	false;
		}
	}

	try
	{
		std::ifstream	fs(file_name_.c_str());
	
		fs.seekg (0, fs.end);
    	int size = fs.tellg();
    	fs.seekg (0, fs.beg);

		std::string		stream;
		stream.resize(size);

		fs.read(&stream[0], size);
		fs.close();
		
		settings_ = libjson::parse(stream);
	}
	catch(std::exception e)
	{
		return	false;		
	}

	return	true;
}

bool	Config::Find(std::string const& _key, Config& _config)
{
	auto item =	settings_.find(_key);
	if (item == settings_.end())
	{
		TRACE_DEBUG("Not found : " << _key);
		return	false;
	}

	TRACE_DEBUG("Found : " << _key << " - " << item->write_formatted());
	_config = *item;

	return	true;
}

bool	Config::Find(std::string const& _key, std::string& _value)
{
	auto item =	settings_.find(_key);
	if (item == settings_.end())
	{
		return	false;
	}

	_value = item->as_string();

	return	true;
}

bool	Config::Find(std::string const& _key, uint32_t& _value)
{
	auto item =	settings_.find(_key);
	if (item == settings_.end())
	{
		return	false;
	}

	_value = item->as_int();

	return	true;
}

Config& Config::operator =(JSONNode& _node)
{
	this->settings_ = _node;

	return	*this;
}

std::ostream& operator<<(std::ostream& _out, const Config& _config) 
{
	_out << _config.settings_.write_formatted();

    return  _out;
}
