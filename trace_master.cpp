#include <iostream>
#include <iomanip> 
#include <fstream>
#include <string.h>
#include <libjson/libjson.h>
#include "trace.h"
#include "time2.h"
#include "utils.h"

#define	DEFAULT_CONST_LOG_FILE_PATH	"/tmp"
#define	DEFAULT_CONST_LOG_FILE_SIZE	(1024*1024)

extern	char*	program_invocation_short_name;
TraceMaster	trace_master;
Trace		trace;

TraceMaster::TraceMaster()
: enable_(false), output_(Trace::FILE_MULTIPLE), level_(Trace::INFO), continue_(false), locker_()
{
	out_ = &std::cout;
	file_path_ = std::string(DEFAULT_CONST_LOG_FILE_PATH);	
	file_prefix_= std::string(program_invocation_short_name);
	file_name_ = file_path_ + "/" + file_prefix_ + (".log");
	file_size_ = DEFAULT_CONST_LOG_FILE_SIZE;
	function_name_len_	= 16;
	object_name_len_	= 16;
	object_id_len_		= 16;
	function_line_len_	= 4;
}

TraceMaster::~TraceMaster()
{
}

bool	TraceMaster::Set(JSONNode const& _properties)
{
	if (_properties.type() != JSON_NODE)
	{
		TRACE_ERROR("Invalid json format");
		return	false;
	}

	bool	enable;
	if (GetMemberValue(_properties, "enable", enable))
	{
		SetEnable(enable);
	}

	std::string	level;
	if (GetMemberValue(_properties, "level", level))
	{
		if (level == "debug")
		{
			level_ = Trace::DEBUG;
		}
		else if (level == "info")
		{
			level_ = Trace::INFO;
		}
		else if (level == "warning")
		{
			level_ = Trace::WARNING;
		}
		else if (level == "error")
		{
			level_ = Trace::ERROR;
		}
		else if (level == "critical")
		{
			level_ = Trace::CRITICAL;
		}

	}

	std::string	output;
	if (GetMemberValue(_properties, "output", output))
	{
		if (output == "console")
		{
			SetOutputConsole();
		}
		else if (output == "file")
		{
			std::string	file_name;

			if (GetMemberValue(_properties, "file", file_name))
			{
				uint32_t		size;
			
				if (!GetMemberValue(_properties, "size", size))
				{
					size = file_size_;
				}

				SetOutputFile(file_name, size);
			}
			else
			{
				std::string		path;
				std::string		prefix;
				uint32_t		size;

				if (!GetMemberValue(_properties, "path", path))
				{
					path = file_path_;
				}

				if (!GetMemberValue(_properties, "prefix", prefix))
				{
					prefix = file_prefix_;
				}

				if (!GetMemberValue(_properties, "size", size))
				{
					size = file_size_;
				}

				SetOutputFile(path, prefix, size);
			}
		}
	}

	return	true;
}

bool	TraceMaster::SetFileSize(uint32_t _size)
{
	TRACE_INFO("Trace file size is changed from " << file_size_ << " to " << _size);
	file_size_ = _size;
	return	true;
}

TraceMaster::operator JSONNode() const
{
	JSONNode	root;

	root.push_back(JSONNode("enable", enable_));
	if (output_ == Trace::CONSOLE)
	{
		root.push_back(JSONNode("output", "console"));
	}
	else if (output_ == Trace::FILE_SINGLE)
	{
		root.push_back(JSONNode("output", "file"));
		root.push_back(JSONNode("file", file_path_));
		root.push_back(JSONNode("size", file_size_));
	}
	else if (output_ == Trace::FILE_MULTIPLE)
	{
		root.push_back(JSONNode("output", "file"));
		root.push_back(JSONNode("path", file_path_));
		root.push_back(JSONNode("prefix", file_prefix_));
		root.push_back(JSONNode("size", file_size_));
	}

	return	root;
}

void	TraceMaster::SetEnable(bool _enable)
{
	enable_ = _enable;
}

bool   TraceMaster::SetOutputStream(std::ostream* _out)
{
	out_ = _out;

	return	true;
}

bool	TraceMaster::SetOutputFile(std::string const& _file, uint32_t ulSize)
{
	output_ 	= Trace::FILE_SINGLE;
	file_name_ 	= _file;
	file_size_ 	= ulSize;
	
	return	true;
}

bool	TraceMaster::SetOutputFile(std::string const& _path, std::string const& _prefix, uint32_t _size)
{
	output_ 	= Trace::FILE_MULTIPLE;
	file_path_	= _path;
	file_prefix_= _prefix;
	file_size_ 	= _size;
	
	return	true;
}

bool	TraceMaster::SetOutputConsole()
{
	output_ = Trace::CONSOLE;
	return	true;
}

void	TraceMaster::Write(std::string const& _headline, uint32_t _headline_len, std::string const& _log)
{
	locker_.Lock();

	switch(output_)
	{
	case	Trace::FILE_SINGLE:	
		{
			Write(file_name_, _headline, _headline_len, _log);
		}
		break;

	case	Trace::FILE_MULTIPLE:	
		{
			std::string file_name = file_path_ + "/" + file_prefix_ + (".log");

			Write(file_name, _headline, _headline_len, _log);
		}
		break;

	case	Trace::CONSOLE:	
		{
			std::istringstream	message(_log);
			bool		first = true;

			while(!message.eof())
			{
				std::string	buffer;

				std::getline(message, buffer);

				for(uint32_t offset = 0 ; offset < buffer.length() ; )
				{
					if (first)
					{
						std::cout << std::setw(_headline_len) << _headline;
						first = false;
					}
					else
					{
						std::cout << std::setw(_headline_len) << std::setfill(' ') << "";
					}

					std::cout << buffer.substr(offset, 200 - _headline_len) << std::endl;

					offset += 200 - _headline_len;
				}
			}
		}
		break;
	}

	locker_.Unlock();
}

void	TraceMaster::Write(std::string const& _file_name, std::string const& _headline, uint32_t _headline_len, std::string const& _log)
{
	std::ofstream	ofs;
	std::ostringstream	oss;
	std::istringstream	message(_log);
	bool		first = true;

	ofs.open(_file_name.c_str(), std::ofstream::out | std::ofstream::app | std::ofstream::ate );
	if (!ofs.is_open())
	{
		std::cerr << "File open error : " << _file_name << std::endl;	
		return;
	}

	uint32_t	size = ofs.tellp();

	if (size + _headline_len + _log.length() > file_size_)
	{
		ofs.close();

		for(uint32_t i = 10 ; i > 0 ; i--)
		{
			oss.str("");
			oss.clear();
			oss << "mv -f " << _file_name << "." << i-1 << " " << _file_name << "." << i << " > /dev/null 2>&1 ";

			system(oss.str().c_str());
		}

		oss.str("");
		oss.clear();
		oss << "mv -f " << _file_name << " " << _file_name << ".0" << " > /dev/null 2>&1 ";

		system(oss.str().c_str());

		ofs.open(_file_name.c_str(), std::ofstream::out | std::ofstream::app);
	}

	while(!message.eof())
	{
		std::string	buffer;

		std::getline(message, buffer);
		for(uint32_t offset = 0 ; offset < buffer.length() ; )
		{
			if (first)
			{
				ofs << std::setw(_headline_len) << _headline;
				first = false;
			}
			else
			{
				ofs << std::setw(_headline_len) << std::setfill(' ') << "";
			}

			ofs << buffer.substr(offset, 200 - _headline_len) << std::endl;

			offset += 200 - _headline_len;
		}
	}

	ofs.close();

}
