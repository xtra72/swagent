#ifndef	TIME2_H_
#define	TIME2_H_

#include <sys/time.h>
#include <stdint.h>
#include <string>

class	Time
{
public:
	Time();
	Time(const Time& _time);
	Time(uint64_t _microseconds);


			void		Set(uint64_t _microseconds);
			uint64_t	Get() const;
			uint64_t	GetMilliseconds() const;
			uint32_t	GetSeconds() const;
	
	virtual	std::string	ToString() const;

						operator uint64_t() const;
			Time		operator+(const Time& _time);
			Time		operator-(const Time& _time);
	const	Time&		operator+=(const Time& _time);
	const	Time&		operator-=(const Time& _time);

    static  uint32_t    SECOND;
    static  uint32_t    MILLISECOND;
protected:
	friend	class	Date;
	friend	class	Timer;

	uint64_t	value_;	
};

class	Date
{
	friend	class	Timer;

public:
	Date();
	Date(Date const& _date);
	Date(std::string const& _date);
	Date(time_t _time);


			bool		IsValid() const;

	virtual	const std::string	ToString() const;

			Date		operator+(uint64_t _microseconds);
			Date		operator+(const Time& _time);
			Date		operator-(const Time& _time);
			Time		operator-(const Date& _date);
	const	Date&		operator-=(const Time& _time);
	const	Date&		operator+=(const Time& _time);
	const	Date&		operator+=(uint64_t _microseconds);

	const	Date&		operator=(uint64_t _microseconds);
	const	Date&		operator=(Date const& _date);
	const	Date&		operator=(std::string const& _date);
						operator time_t() const;
			uint32_t	GetSeconds() const;
			uint64_t	GetMicroseconds() const;

	friend	std::ostream&	::operator<<(std::ostream& os, Date const& _date) ;

	static	Date	GetCurrent();
	

protected:
	uint64_t	value_;
};

bool	Compare(Date const & _date_a, Date const& _date_b);
#endif

