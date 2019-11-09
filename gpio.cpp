#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gpio.h"
#include "trace.h"

GPIO::GPIO(uint32_t _index, DIRECTION _dir)
:	index_(_index), dir_(_dir), lazy_init_(true), init_(false)
{
}


GPIO::~GPIO()
{
	Unexport(index_);
}

bool	GPIO::IsExist(uint32_t pin)
{
#define DIRECTION_MAX 35
	char path[DIRECTION_MAX];

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d", pin);

	return	( access( path, F_OK ) != -1 ) ;
}

bool	GPIO::Export(uint32_t pin)
{
	if (!IsExist(pin))
	{
#define BUFFER_MAX 3
		char buffer[BUFFER_MAX];
		ssize_t bytes_written;
		int fd;

		fd = open("/sys/class/gpio/export", O_WRONLY);
		if (-1 == fd) 
		{
			TRACE_ERROR("Failed to open export for writing!");
			return(false);
		}

		bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
		write(fd, buffer, bytes_written);
		close(fd);
	}

	return true;
}


bool	GPIO::Unexport(uint32_t pin)
{
	if (IsExist(pin))
	{
		char buffer[BUFFER_MAX];
		ssize_t bytes_written;
		int fd;
	
		fd = open("/sys/class/gpio/unexport", O_WRONLY);
		if (-1 == fd) 
		{
			TRACE_ERROR("Failed to open unexport for writing!");
			return(false);
		}
	
		bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
		write(fd, buffer, bytes_written);
		close(fd);
	}
	
	return(true);
}


bool	GPIO::Direction(uint32_t pin, DIRECTION dir)
{
	if (!Export(pin))
	{
		return	false;	
	}

	static const char s_directions_str[]  = "in\0out";

#define DIRECTION_MAX 35
	char path[DIRECTION_MAX];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) 
	{
		TRACE_ERROR("Failed to open gpio direction for writing!");
		return(false);
	}

	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) 
	{
		TRACE_ERROR("Failed to set direction!");
		return(false);
	}

	close(fd);

	init_ = true;

	return true;
}

bool	GPIO::Read(uint32_t pin)
{
	if (!init_)
	{
		if (!Export(pin))
		{
			return	false;	
		}

		Direction(pin, dir_);
	}

#define VALUE_MAX 30
	char path[VALUE_MAX];
	char value_str[3];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) 
	{
		TRACE_ERROR("Failed to open gpio value for reading!");
		return(false);
	}

	if (-1 == read(fd, value_str, 3)) 
	{
		TRACE_ERROR("Failed to read value!");
		return(false);
	}

	close(fd);

	return(atoi(value_str));
}

bool	GPIO::Write(uint32_t pin, bool value)
{
	if (!init_)
	{
		if (!Export(pin))
		{
			return	false;	
		}

		Direction(pin, dir_);
	}

	static const char s_values_str[] = "01";

	char path[VALUE_MAX];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) 
	{
		TRACE_ERROR("Failed to open gpio value for writing!");
		return(false);
	}

	if (1 != write(fd, &s_values_str[value ? 1 : 0], 1)) 
	{
		TRACE_ERROR("Failed to write value!");
		return(false);
	}

	close(fd);

	return(true);
}


GPIOIn::GPIOIn(uint32_t _index)
: GPIO(_index, IN)
{
	if (!lazy_init_)
	{
		Direction(_index, IN);
	}
}

bool	GPIOIn::Value()
{
	return	Read(index_);
}

GPIOOut::GPIOOut(uint32_t _index)
: GPIO(_index, OUT)
{
	if (!lazy_init_)
	{
		Direction(_index, OUT);
	}
}

bool	GPIOOut::Value(bool _value)
{
	return	Write(index_, _value);
}

