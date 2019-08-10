#ifndef	TRACE_H_
#define	TRACE_H_

#include <stdint.h>
#include <sstream>
#include <iostream>
#include <streambuf>
#include <libjson/libjson.h>
#include "locker.h"

class	Object;
class	TraceMaster;

class	Trace : public std::ostringstream
{
public:

	enum	Output
	{
		FILE_SINGLE,
		FILE_MULTIPLE,
		CONSOLE
	};

	enum	Level
	{
		UNKNOWN = 0,
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		CRITICAL
	};

	Trace(Object* _object = NULL);
	~Trace();

	enum	State
	{	
		UNDEFINED,
		ENABLE,
		DISABLE
	};

			bool	Set(JSONNode const& _settings);

			bool	GetEnable();
			void	SetEnable(bool on);

			bool	SetState(State _state);
			Trace::State	GetState();

			void	SetLevel(Level _level);
			Level	GetLevel();

	static	void	SetDefaultLevel(Level _level);
	static	Level	GetDefaultLevel();

			void	SetDebug(bool _debug);
			bool	GetDebug();

			void	SetDumpEnable(bool _enable);
			bool	GetDumpEnable();

	static	void	SetDefaultDebug(bool _debug);
	static	bool	GetDefaultDebug();

			Trace&	Begin(Level _level, std::string const& _pretty_function, uint32_t line);
			Trace&	Begin(Level _level, std::string const& _pretty_function, uint32_t line, Object* _object);
			Trace&	Dump(Level _level, std::string const& _pretty_function, uint32_t line, const uint8_t* _buffer, uint32_t _length);
			Trace&	Dump(Level _level, std::string const& _pretty_function, uint32_t line, Object* _object, const uint8_t* _buffer, uint32_t _length);

			bool	SetClassName(std::string const& _class_name);

			Trace& 	operator<< (std::streambuf* sb );
    		Trace& 	operator<< (std::ostream& (*pf)(std::ostream&));
			Trace& 	operator<< (std::ios& (*pf)(std::ios&));
			Trace& 	operator<< (std::ios_base& (*pf)(std::ios_base&));

	static 	std::ostream& 	End(std::ostream& _trace);
	static 	std::ostream& 	NL(std::ostream& _trace);


protected:

			TraceMaster&	master_;
			Object*			object_;
			std::string		class_name_;
			State			state_;
			Level			level_;
			Level			current_level_;
			bool			continue_;
			bool			debug_;
			bool			dump_;
			Locker			locker_;
			std::string		headline_;
};

class	TraceMaster : public std::ostringstream
{
public:
	friend	class	Trace;

	TraceMaster();
	~TraceMaster();

	bool		GetEnable()	{	return	enable_;	};
	void		SetEnable(bool on);

	uint32_t	GetFileSize()	{	return	file_size_;	}
	bool		SetFileSize(uint32_t _size);

	bool		Set(JSONNode const& _json);
				operator JSONNode() const;

	uint32_t	GetFunctionNameSize()	{	return	function_name_len_;	};
	uint32_t	GetFunctionLineSize()	{	return	function_line_len_;	};
	uint32_t	GetObjectNameSize()		{	return	object_name_len_;	};
	uint32_t	GetObjectIDSize()		{	return	object_id_len_;	};

	Trace::Level	GetLevel()				{	return	level_;		}
	Trace::Output	GetOutput()				{	return	output_;	}

	bool	SetOutputStream(std::ostream* _out);
	bool	SetOutputFile(std::string const& _file, uint32_t ulSize = 1024*1024);
	bool	SetOutputFile(std::string const& _path, std::string const& _prefix, uint32_t ulSize = 1024*1024);
	bool	SetOutputConsole();

protected:
	void	Write(std::string const& _file, std::string const& _headline, uint32_t _headline_max, std::string const& _log);
	void	Write(std::string const& _headline, uint32_t _headline_max, std::string const& _log);

	std::ostream*	out_;
	bool			enable_;
	Trace::Output	output_;
	std::string		file_path_;
	std::string		file_prefix_;
	std::string		file_name_;
	uint32_t		file_size_;
	Trace::Level	level_;
	bool			continue_;
	std::string		headline_;
	Locker			locker_;
	uint32_t		object_name_len_;
	uint32_t		object_id_len_;
	uint32_t		function_name_len_;
	uint32_t		function_line_len_;
};

extern	TraceMaster	trace_master;
extern	Trace		trace;
extern	Locker		global_locker;

#define	TRACE_DEBUG(x)	\
{	\
	if (trace.GetLevel() <= Trace::DEBUG) \
	{ \
		global_locker.Lock(); \
		std::ostringstream oss; \
		oss << x; \
		trace.Begin(Trace::DEBUG, __PRETTY_FUNCTION__, __LINE__) << oss.str() << Trace::End; \
		global_locker.Unlock(); \
	}\
}
#define	TRACE_INFO(x)	\
{	\
	if (trace.GetLevel() <= Trace::INFO) \
	{ \
		global_locker.Lock(); \
		std::ostringstream oss; \
		oss << x; \
		trace.Begin(Trace::INFO, __PRETTY_FUNCTION__, __LINE__) << oss.str() << Trace::End; \
		global_locker.Unlock(); \
	}\
}
#define	TRACE_WARN(x)\
{\
	if (trace.GetLevel() <= Trace::WARNING)\
	{ \
		global_locker.Lock(); \
		std::ostringstream oss; \
		oss << x; \
		trace.Begin(Trace::WARNING, __PRETTY_FUNCTION__, __LINE__) << oss.str() << Trace::End; \
		global_locker.Unlock(); \
	}\
}
#define	TRACE_ERROR(x)\
{	\
	if (trace.GetLevel() <= Trace::ERROR) \
	{\
		global_locker.Lock(); \
		std::ostringstream oss; \
		oss << x; \
		trace.Begin(Trace::ERROR, __PRETTY_FUNCTION__, __LINE__) << oss.str() << Trace::End; \
		global_locker.Unlock();\
	}\
}
#define	TRACE_CRITICAL(x)\
{\
	if (trace.GetLevel() <= Trace::CRITICAL)\
	{\
		global_locker.Lock();\
		std::ostringstream oss;\
		oss << x;\
		trace.Begin(Trace::CRITICAL, __PRETTY_FUNCTION__, __LINE__) << oss.str() << Trace::End;\
		global_locker.Unlock();\
	}\
}

#define	TRACE_DEBUG_DUMP(buffer, len)\
{\
	if (trace.GetLevel() <= Trace::DEBUG)\
	{\
		trace.Dump(Trace::DEBUG, __PRETTY_FUNCTION__, __LINE__, buffer, len) << Trace::End;\
	}\
}

#define	TRACE_INFO_DUMP(buffer, len)\
{\
	if (trace.GetLevel() <= Trace::INFO)\
	{\
		trace.Dump(Trace::INFO, __PRETTY_FUNCTION__, __LINE__, buffer, len) << Trace::End;\
	}\
}

#define	TRACE_INFO_JSON(x)\
{\
	if (::trace.GetLevel() <= Trace::INFO)\
	{\
		JSONNode json = libjson::parse(x);\
		::trace.Begin(Trace::INFO, __PRETTY_FUNCTION__, __LINE__) << json.write_formatted() << Trace::End;\
	}\
}

#define	TRACE_DEBUG2(x, y)\
{\
	if (::trace.GetLevel() <= Trace::DEBUG)\
	{\
		global_locker.Lock();\
		std::ostringstream oss;\
		oss << y;\
		::trace.Begin(Trace::DEBUG, __PRETTY_FUNCTION__, __LINE__, x) << oss.str() << Trace::End;\
		global_locker.Unlock();\
	}\
}

#define	TRACE_INFO2(x, y)\
{\
	if (::trace.GetLevel() <= Trace::INFO)\
	{\
		global_locker.Lock();\
		std::ostringstream oss;\
		oss << y;\
		::trace.Begin(Trace::INFO, __PRETTY_FUNCTION__, __LINE__, x) << oss.str() << Trace::End;\
		global_locker.Unlock();\
	}\
}

#define	TRACE_WARN2(x, y)\
{\
	if (::trace.GetLevel() <= Trace::INFO)\
	{\
		global_locker.Lock();\
		std::ostringstream oss;\
		oss << y;\
		::trace.Begin(Trace::WARNING, __PRETTY_FUNCTION__, __LINE__, x) << oss.str() << Trace::End;\
		global_locker.Unlock();\
	}\
}

#define	TRACE_ERROR2(x, y)\
{\
	if (::trace.GetLevel() <= Trace::INFO)\
	{\
		global_locker.Lock();\
		std::ostringstream oss;\
		oss << y; ::trace.Begin(Trace::ERROR, __PRETTY_FUNCTION__, __LINE__, x) << oss.str() << Trace::End;\
		global_locker.Unlock();\
	}\
}

#define	TRACE_CRITICAL2(x, y)\
{\
	if (::trace.GetLevel() <= Trace::INFO)\
	{\
		::trace.Begin(Trace::CRITICAL, __PRETTY_FUNCTION__, __LINE__, x) << y << Trace::End;\
	}\
}

#define	TRACE_INFO_DUMP2(x, buffer, len)\
{\
	if (::trace.GetLevel() <= Trace::INFO)\
	{\
		::trace.Dump(Trace::INFO, __PRETTY_FUNCTION__, __LINE__, x, buffer, len) << Trace::End;\
	}\
}


#define	TRACE_CREATE	TRACE_INFO("Create : " << this->GetClassName())
#define	TRACE_ENTRY		TRACE_INFO("Entry")
#define	TRACE_ENTRY2(x)	TRACE_INFO2(x, "Entry")
#define	TRACE_EXIT		TRACE_INFO("Exit")
#define	TRACE_EXIT2(x)	TRACE_INFO2(x, "Exit")

#endif
