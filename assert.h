#ifndef	ASSERT_H__
#define	ASSERT_H__

#include <exception>
#include <string>
#include <sstream>

class	Asserted : public std::exception
{
public:
	Asserted(std::string const& _argument) throw();
	~Asserted() throw();

	virtual const char*		what() const throw();

protected:
	std::string	message_;
};

#define	ASSERT(x)	{	if (!(x)) { std::ostringstream oss; oss << __PRETTY_FUNCTION__ << ":" << __LINE__ << " - "<< #x; throw Asserted(oss.str());} }

#endif

