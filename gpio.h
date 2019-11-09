#ifndef	GPIO_H_
#define	GPIO_H_

class	GPIO
{
public:	
	enum	DIRECTION
	{
		IN = 0,
		OUT = 1
	};

	GPIO(uint32_t _index, DIRECTION _dir);
	~GPIO();

protected:
	uint32_t	index_;
	DIRECTION	dir_;
	bool		lazy_init_;
	bool		init_;

	bool	IsExist(uint32_t pin);
	bool	Export(uint32_t pin);
	bool	Unexport(uint32_t pin);
	bool	Direction(uint32_t pin, DIRECTION dir);
	bool	Read(uint32_t pin);
	bool	Write(uint32_t pin, bool value);
};

class	GPIOIn : public GPIO
{
public:
	GPIOIn(uint32_t	_index);

	bool	Value();
};

class	GPIOOut : public GPIO
{
public:
	GPIOOut(uint32_t	_index);

	bool	Value(bool value);
};

#endif

