#include <iostream>
#include <iomanip>
#include <fstream>
#include "object.h"
#include "trace.h"
#include "time2.h"
#include "utils.h"

using namespace std;

Locker	global_locker;

static	Trace::Level	level_ = Trace::WARNING;

static	bool			debug_ = true;

Trace::Trace(Object* _object)
: master_(trace_master), object_(_object), class_name_(""), state_(ENABLE), level_(Trace::UNKNOWN), current_level_(Trace::UNKNOWN), continue_(false), debug_(::debug_), dump_(false), locker_()
{
}

Trace::~Trace()
{
}

bool	Trace::SetClassName(std::string const& _class_name)
{
	class_name_ = _class_name;

	return	true;
}


bool	Trace::Set(JSONNode const& _value)
{
	GetMemberValue(_value, "debug", debug_);
	GetMemberValue(_value, "dump", dump_);

	return	true;
}

	
bool	Trace::GetEnable()
{
	if(state_ == ENABLE)
	{
		return	true;
	}
	else if (state_ == DISABLE)
	{
		return	false;
	}

	return	master_.GetEnable();	
}

void	Trace::SetEnable(bool _enable)
{
	if (_enable)
	{
		state_ = ENABLE;
	}
	else
	{
		state_ = DISABLE;
	}
}

bool	Trace::SetState(State _state)
{
	state_ = _state;

	return	true;
}

Trace::State Trace::GetState()
{
	return	state_;
}

void	Trace::SetLevel(Trace::Level _level)
{
	level_ = _level;
}

Trace::Level 	Trace::GetLevel()
{
	if (level_ == Trace::UNKNOWN)
	{
		return	trace_master.GetLevel();
	}

	return	level_;
}

void	Trace::SetDefaultLevel(Trace::Level _level)
{
	::level_ = _level;
}

Trace::Level 	Trace::GetDefaultLevel()
{
	return	::level_;
}

void	Trace::SetDebug(bool _debug)
{
	debug_ = _debug;
}

bool	Trace::GetDebug()
{
	return	debug_;
}

void	Trace::SetDumpEnable(bool _enable)
{
	dump_ = _enable;
}

bool	Trace::GetDumpEnable()
{
	return	dump_;
}

void	Trace::SetDefaultDebug(bool _debug)
{
	::debug_ = _debug;
}

bool	Trace::GetDefaultDebug()
{
	return	::debug_;
}

Trace& Trace::Begin(Level _level, std::string const& _pretty_function, uint32_t _line)
{
	current_level_ = _level;
	headline_ = "";

	size_t colons, begin, end;

	colons = _pretty_function.find("::");
	if (colons != _pretty_function.npos)
	{
		begin = _pretty_function.substr(0,colons).rfind(" ") + 1;
	}
	else
	{
		begin = _pretty_function.substr(0,colons).find(" ");
	}

	end = _pretty_function.rfind("(") - begin;

	std::string function = _pretty_function.substr(begin,end);
	ostringstream	os;

	if (debug_)
	{
		Date	date;
		os << "[" << date ;
		os << "." << std::setw(6) << std::setfill('0') << date.GetMicroseconds() % 1000000 << std::setfill(' ') ;
		os << "]";
		os << "[" << setw(master_.GetFunctionNameSize()) << function.substr(0, master_.GetFunctionNameSize()) << "]";
		os << "[" << setw(master_.GetFunctionLineSize()) <<_line << "]";
	}

	if (object_)
	{
#if 0
		std::string	name = object_->GetName();

		if (master_.GetObjectNameSize() >= name.size())
		{
			os << "[" << setw(master_.GetObjectNameSize()) <<name << "]";
		}

		else if(master_.GetObjectNameSize() < 5)
		{
			os << "[" << setw(master_.GetObjectNameSize()) <<name.substr(0, 4) << "]";
		}
		else
		{
			int begin = (master_.GetObjectNameSize() - 3) / 2;
			int end   = master_.GetObjectNameSize() - begin - 3;
			os << "[" << setw(begin) << name.substr(0, begin) << "..." << name.substr(name.size() - end, end) << "]";
		}
#endif	
		std::string	id = object_->GetID();

		if (master_.GetObjectIDSize() >= id.size())
		{
			os << "[" << setw(master_.GetObjectIDSize()) <<id << "]";
		}

		else if(master_.GetObjectIDSize() < 5)
		{
			os << "[" << setw(master_.GetObjectIDSize()) <<id.substr(0, 4) << "]";
		}
		else
		{
			int begin = (master_.GetObjectIDSize() - 3) / 2;
			int end   = master_.GetObjectIDSize() - begin - 3;
			os << "[" << setw(begin) << id.substr(0, begin) << "..." << id.substr(id.size() - end, end) << "]";
		}
	}
	else
	{
		os << "[" << setw(master_.GetObjectNameSize()) << "global" << "]";
	}

	switch(current_level_)
	{
	case	UNKNOWN:	os << "[UNKN]"; break;
	case	DEBUG:	os << "[DEBG]"; break;
	case	INFO:	os << "[INFO]"; break;
	case	WARNING:os << "[\033[1;33mWARN\033[0m]"; break;
	case	ERROR:	os << "[\033[0;31mERRO\033[0m]"; break;
	case	CRITICAL:os <<"[\033[1;31mCRIT\033[0m]"; break;
	}

	os << " : ";

	headline_ = os.str();

	return	*this;
}

Trace& Trace::Begin(Level _level, std::string const& _pretty_function, uint32_t _line, Object *_object)
{
	if (_object != NULL)
	{
		return	_object->trace.Begin(_level, _pretty_function, _line);
	}
	return	Begin(_level, _pretty_function, _line);
}

Trace&	Trace::Dump(Level _level, std::string const& _pretty_function, uint32_t line, const uint8_t * _buffer, uint32_t _length)
{
	bool	binary_message = false;

	if (dump_)
	{
		try
		{
			if (libjson::is_valid((char *)_buffer))
			{
				JSONNode	json = libjson::parse((char *)_buffer);

				Begin(_level, _pretty_function, line) << json.write_formatted() << Trace::End;
			}
			else
			{
				binary_message = true;
			}
		}
		catch(std::invalid_argument)
		{
			binary_message = true;
		}

		if (binary_message)
		{
			for(uint32_t i = 0 ; i < _length ; i++)
			{
				if ((i % 16) == 0)
				{
					Begin(_level, _pretty_function, line) << std::hex << std::setw(4) << std::setfill('0') << i << " : ";
				}

				Begin(_level, _pretty_function, line) << std::hex << std::setw(2) << std::setfill('0') << (int)((uint8_t *)_buffer)[i] << " ";
				if ((i + 1) % 16 == 0)
				{
					Begin(_level, _pretty_function, line) << std::oct << Trace::End;	
				}
			}
			Begin(_level, _pretty_function, line) << std::oct << Trace::End;	
		}
	}
	return	*this;
}

Trace&	Trace::Dump(Level _level, std::string const& _pretty_function, uint32_t line, Object* _object, const uint8_t* _buffer, uint32_t _length)
{
	if (_object != NULL)
	{
		return	_object->trace.Dump(_level, _pretty_function, line, _buffer, _length);
	}

	return	Dump(_level, _pretty_function, line, _buffer, _length);
}

std::ostream& Trace::End(std::ostream& _trace)
{
	Trace* trace = dynamic_cast<Trace*>(&_trace);
	if (trace != NULL)
	{
		if (trace->GetEnable() && (trace->str().size() != 0))
		{
			trace_master.Write(trace->headline_, trace->headline_.size(), trace->str());
			trace->str("");
		}

		trace->continue_	= false;
	}

	_trace << std::setw(0);

	return	_trace;
}

std::ostream& Trace::NL(std::ostream& _trace)
{
	Trace* trace = dynamic_cast<Trace*>(&_trace);
	if (trace != NULL)
	{
		if (trace->GetEnable() && (trace->str().size() != 0))
		{
			trace_master.Write("", trace->headline_.size(), trace->str());
			trace->str("");
		}

		trace->continue_ = true;
	}

	return	_trace;
}

Trace& Trace::operator<< (streambuf* sb )
{
	ostringstream::operator<<(sb);	

	return	*this;
}

Trace& Trace::operator<< (ostream& (*pf)(ostream&))
{
	ostringstream::operator<<(pf);	

	return	*this;
}

Trace& Trace::operator<< (ios& (*pf)(ios&))
{
	ostringstream::operator<<(pf);	

	return	*this;
}

Trace& Trace::operator<< (ios_base& (*pf)(ios_base&))
{
	ostringstream::operator<<(pf);	

	return	*this;
}

