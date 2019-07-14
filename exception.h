#ifndef	EXCEPTION_H_
#define	EXCEPTION_H_

#include <exception>
#include <string>
#include <sstream>

class	ShowUsage : public std::exception
{
public:
	ShowUsage(std::string const& _argument) throw();
	ShowUsage(std::string const& _name, std::string const& _value) throw();
	~ShowUsage() throw();

	virtual const char*		what() const throw();

protected:
	std::string	message_;
};

#define	SHOW_USAGE(x)	{	if (!(x)) { std::ostringstream oss; oss << #x; throw ShowUsage(oss.str());} }

class	InvalidConfig: public std::exception
{
public:
	InvalidConfig(std::string const& _message) throw();
	~InvalidConfig() throw();

	virtual const char*		what() const throw();

protected:
	std::string	message_;
};

#define	THROW_INVALID_CONFIG(x)	{	std::ostringstream oss; oss << x; throw InvalidConfig(oss.str()); }

class	InvalidArgument : public std::exception
{
public:
	InvalidArgument(std::string const& _argument) throw();
	InvalidArgument(std::string const& _name, std::string const& _value) throw();
	~InvalidArgument() throw();

	virtual const char*		what() const throw();

protected:
	std::string	message_;
};

#define	THROW_INVALID_ARGUMENT(x)	{	std::ostringstream oss; oss << x; throw InvalidArgument(oss.str()); }

class	ObjectNotFound : public std::exception
{
public:
	ObjectNotFound(std::string const& _object_id) throw();
	~ObjectNotFound() throw();

	const	std::string&	GetObjectID()		{	return	object_id_;		}
	virtual const char*		what() const throw();

protected:
	std::string	object_id_;
	std::string	message_;
};

#define	THROW_OBJECT_NOT_FOUND(x)	{	std::ostringstream oss; oss << x; throw ObjectNotFound(oss.str()); }

class	NotInitialized : public std::exception
{
public:
	NotInitialized(std::string const& _message) throw();
	~NotInitialized() throw();

protected:
	std::string	message_;
};

class	RequestTimeout : public std::exception
{
public:
	RequestTimeout(std::string const& _argument) throw();
	RequestTimeout(std::string const& _name, std::string const& _value) throw();
	~RequestTimeout() throw();

	virtual const char*		what() const throw();

protected:
	std::string	message_;
};

#define	THROW_REQUEST_TIMEOUT(x)	{	std::ostringstream oss; oss << x; throw RequestTimeout(oss.str()); }

class	ConnectTimeout : public std::exception
{
public:
	ConnectTimeout(std::string const& _argument) throw();
	ConnectTimeout(std::string const& _name, std::string const& _value) throw();
	~ConnectTimeout() throw();

	virtual const char*		what() const throw();

protected:
	std::string	message_;
};

#define	THROW_CONNECT_TIMEOUT(x)	{	std::ostringstream oss; oss << x; throw ConnectTimeout(oss.str()); }
#endif

