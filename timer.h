#ifndef TIMER_H_
#define TIMER_H_

#include <thread>
#include <stdbool.h>
#include "object.h"
#include "time2.h"

class   Timer
{
public:
    Timer();

    void        Set(const Date& _date);
    const Date& ReferenceTime() const;
    Time        RemainTime() const;
    Time        OverTime() const;

    bool        Add(uint64_t _microseconds);
    //  const Timer&    operator+=(const Time& _time);
    //  const Timer&    operator+=(const uint64_t _microseconds);

protected:
    Date    reference_date_;
    Time    time_;  

};


class	Object;
typedef	void (*ActiveTimerCallback)(void*);

class ActiveTimer : public Object
{
public:
	bool	IsRun();
	void 	Stop();

protected:
	bool	run_ = false;
	bool 	clear_ = false;
	void*	params_ = NULL;
	ActiveTimerCallback	function_ = NULL;
	std::thread*	thread_ = NULL;
};

class TimeoutTimer : public ActiveTimer
{
public:
	void 	SetTimeout(uint64_t _delay, ActiveTimerCallback _function = NULL, void * _params = NULL);
};

class IntervalTimer : public ActiveTimer
{
public:
	void 	SetInterval(uint64_t _interval, ActiveTimerCallback _function = NULL, void * _params = NULL);
};

#endif
