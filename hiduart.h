#ifndef	HIDUART_H_
#define	HIDUART_H_

#include "SLABHIDtoUART.h"
#include "activeobject.h"
#include <string>

#define SILABS_VID                      0x10c4
#define CP2110_PID                      0xea80
#define CP2114_PID                      0xeab0

class	HIDUART : public ActiveObject
{
public:
	HIDUART();
	HIDUART(std::string const& _id, uint16_t _vid, uint16_t _pid, std::string const& _serial = "", Object* _parent = NULL);
	~HIDUART();

	virtual	bool	SetSerialID(std::string const& _serial);

	virtual	bool	OnRead(uint8_t* _data, uint32_t _length);
	virtual	bool	OnWrite(uint8_t* _data, uint32_t _length);

	static std::string	DecodeHidUartStatus(HID_UART_STATUS status);
	static	uint32_t	GetDeviceCount(uint16_t _pid = CP2110_PID, uint16_t _vid = SILABS_VID);
	static	bool		GetSerial(uint16_t _vid, uint16_t _pid, uint32_t _index, std::string& _serial);

protected:
	uint16_t	vid_ ;
	uint16_t	pid_ ;
    std::string	serial_;
	HID_UART_DEVICE uart_;
	HID_UART_STATUS	status_;
    uint32_t	baudRate_;
    uint8_t		dataBits_;
    uint8_t		parity_;
    uint8_t		stopBits_;
    uint8_t		flowControl_;

	virtual	void	Preprocess();
	virtual	void	Process();
	virtual	void	Postprocess();

	static	bool	SetSerialID(Object* _object, JSONNode const& _value);
};

#endif
