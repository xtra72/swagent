#include <iostream>
#include <thread>
#include"timer.h"

Timer::Timer()
    :   reference_date_(), time_(0)
{
}

void    Timer::Set(const Date& _date)
{
    reference_date_ = _date;
}

const Date& Timer::ReferenceTime() const
{
    return  reference_date_;
}

Time    Timer::RemainTime() const
{
    Date    current;
    uint64_t    remain = 0;

    if (reference_date_.value_ > current.value_)
    {
        remain = reference_date_.value_ - current.value_;
    }

    return  Time(remain);
}

Time    Timer::OverTime() const
{
    Date    current;
    uint64_t    over= 0;

    if (current.value_ > reference_date_.value_)
    {
        over = current.value_ - reference_date_.value_;
    }

    return  Time(over);
}

bool    Timer::Add(uint64_t _microseconds)
{
    reference_date_ += _microseconds;

    return  true;
}

////////////////////////////////////////////////////////////////////////////////////
// class	ActieTiemr
////////////////////////////////////////////////////////////////////////////////////
bool ActiveTimer::IsRun()
{
	return	this->run_;
}

void ActiveTimer::Stop() 
{
	this->clear_ = true;
}

////////////////////////////////////////////////////////////////////////////////////
// class	TimeoutTimer
////////////////////////////////////////////////////////////////////////////////////
void 	TimeoutTimer::SetTimeout(uint64_t _delay, ActiveTimerCallback _function, void * _params) 
{
	if (this->thread_)
	{
		if (this->run_)
		{
			this->Stop();

			for(int i = 0 ; i < 10 ; i++)
			{
				if(!this->IsRun())
				{
					break;
				}
				std::this_thread::sleep_for(std::chrono::microseconds(1000));
			}

			TRACE_INFO("TimeoutTimer does not stop!");
		}

		delete this->thread_;

		this->thread_ = NULL;
	}

	this->run_ = true;
	this->clear_ = false;
	
	if (_function)
	{
		this->function_ = _function;
	}

	if (_params)
	{
		this->params_ = _params;
	}

	
	this->thread_ = new std::thread([=]() 
	{
		Date	start_date = Date::GetCurrent();
		if(this->clear_) 
		{
			this->run_ = false;
			return;
		}
		
		while(true)
		{
			Date current_date = Date::GetCurrent();
			uint64_t	remain_delay = _delay - (current_date.GetMicroseconds() - start_date.GetMicroseconds());
			
			if ((remain_delay == 0) || (remain_delay > _delay))
			{
				break;
			}
			
			if (remain_delay > 1000)
			{
				remain_delay = 1000;	
			}

			std::this_thread::sleep_for(std::chrono::microseconds(remain_delay));
			if(this->clear_) 
			{
				break;
			}
		}

	
		if(this->clear_) 
		{
			this->run_ = false;
			return;
		}

		if (this->function_)
		{
			this->function_(this->params_);
		}	
		this->run_ = false;
	});

	this->thread_->detach();
}


////////////////////////////////////////////////////////////////////////////////////
// class	IntervalTimer
////////////////////////////////////////////////////////////////////////////////////
bool	IntervalTimer::SetInterval(uint64_t _interval, ActiveTimerCallback _function, void * _params, bool _start_after_call) 
{
	if (_function == NULL)
	{
		return	false;
	}

	if (this->thread_)
	{
		if (this->run_)
		{
			this->Stop();

			for(int i = 0 ; i < 10 ; i++)
			{
				if(!this->IsRun())
				{
					break;
				}
				std::this_thread::sleep_for(std::chrono::microseconds(1000));
			}

			TRACE_INFO("TimeoutTimer does not stop!");
		}

		delete this->thread_;

		this->thread_ = NULL;
	}

	this->run_ = true;
	this->clear_ = false;
	this->function_ = _function;

	if (_params)
	{
		this->params_ = _params;
	}

	this->thread_ = new std::thread([=]() 
	{ 
		Date	start_date = Date::GetCurrent();

		if (_start_after_call)
		{
			this->function_(this->params_);
		}
		while(true) 
		{
			if(this->clear_) 
			{
				break;
			}
			

			while(true)
			{
				Date current_date = Date::GetCurrent();
				uint64_t	remain_interval = _interval - (current_date.GetMicroseconds() - start_date.GetMicroseconds());
				
				if ((remain_interval == 0) || (remain_interval > _interval))
				{
					break;
				}
				
				if (remain_interval > 1000)
				{
					remain_interval = 1000;	
				}

				std::this_thread::sleep_for(std::chrono::microseconds(remain_interval));
				if(this->clear_) 
				{
					break;
				}
			}

			start_date += _interval;	
			if (this->function_)
			{
				this->function_(this->params_);
			}
		}
		this->run_ = false;
	});

	this->thread_->detach();

	return	true;
}

