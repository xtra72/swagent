#include <unistd.h>
#include "hiduart.h"


#define ERROR_LEVEL_SUCCESS             0
#define ERROR_LEVEL_INVALID_ARG         1
#define ERROR_LEVEL_API_CODE            2

HIDUART::HIDUART()
: ActiveObject(), vid_(0), pid_(0), port_(255),serial_(""), uart_(NULL), baudRate_(921600), dataBits_(3), parity_(HID_UART_NO_PARITY), stopBits_(0), flowControl_(0), serialRxTimeout_(20), serialTxTimeout_(1000)
{
	properties_map_["serial_id"] = SetSerialID;
	properties_map_["port"] = SetPort;
	properties_map_["serial_rx_timeout"] = SetSerialRxTimeout;
	properties_map_["serial_tx_timeout"] = SetSerialTxTimeout;
}

HIDUART::HIDUART(std::string const& _id, uint16_t _vid, uint16_t _pid, std::string const& _serial, Object* _parent)
: ActiveObject(_id, _parent), vid_(_vid), pid_(_pid), port_(255), serial_(_serial), uart_(NULL), baudRate_(921600), dataBits_(3), parity_(HID_UART_NO_PARITY), stopBits_(0), flowControl_(0), serialRxTimeout_(20), serialTxTimeout_(1000)
{
	properties_map_["serial_id"] = SetSerialID;
	properties_map_["port"] = SetPort;
	properties_map_["serial_rx_timeout"] = SetSerialRxTimeout;
	properties_map_["serial_tx_timeout"] = SetSerialTxTimeout;
}

HIDUART::~HIDUART()
{
}

void	HIDUART::Preprocess()
{
	HID_UART_STATUS	status;

	loop_interval_ = 100;

	if (port_ == 255)
	{
		TRACE_DEBUG("HIDUART serial : " << this->serial_);
		if (this->serial_ != "")
		{
			bool		found = false;
			uint32_t	count = 0;

			count = HIDUART::GetDeviceCount(this->pid_, this->vid_);
			for (uint32_t i = 0; i < count; i++)
			{
				std::string serial;

				if (HIDUART::GetSerial(this->vid_, this->pid_, i, serial))
				{
					if (this->serial_ == serial)
					{
						found = true;
						port_ = i;
					}
				}
			}

			if (!found)
			{
				TRACE_DEBUG("HIDUART deivce not found!");
				return;
			}
		}
	}

	TRACE_DEBUG("HIDUART Open : " << port_ << ", " << this->vid_ << ", " << this->pid_);
	status = HidUart_Open(&this->uart_, port_, this->vid_, this->pid_);
    if (status != HID_UART_SUCCESS)
    {
		TRACE_ERROR("Open failed!");		
		return;
	}
	else
	{
		status = HidUart_SetUartConfig(this->uart_, this->baudRate_, this->dataBits_, this->parity_, this->stopBits_, this->flowControl_);
		if (status != HID_UART_SUCCESS)
		{
			TRACE_ERROR("Failed setting UART config: " << DecodeHidUartStatus(status).c_str());
		}
    
		status = HidUart_SetTimeouts(this->uart_, this->serialRxTimeout_, this->serialTxTimeout_);
	}

}

bool	HIDUART::SetSerialID(std::string const& _serial)
{
	this->serial_ = _serial;

	return	true;
}

bool	HIDUART::SetPort(uint32_t _port)
{
	this->port_ = _port;

	return	true;
}

bool	HIDUART::SetSerialRxTimeout(uint32_t _timeout)
{
	this->serialRxTimeout_ = _timeout;

	return	true;
}

bool	HIDUART::SetSerialTxTimeout(uint32_t _timeout)
{
	this->serialTxTimeout_ = _timeout;

	return	true;
}

void	HIDUART::Process()
{
	static	uint8_t		rxBuffer[HID_UART_MAX_READ_SIZE + 1];
	static	uint8_t		buffer[HID_UART_MAX_READ_SIZE + 1];
	static	uint32_t	remainLength = 0;
	uint32_t			rxLength = 0;
	uint32_t			length = 0;


	HID_UART_STATUS status = HidUart_Read(uart_, &buffer[remainLength], HID_UART_MAX_READ_SIZE - remainLength , &length);

	length += remainLength;
	// Read at least 1 byte
	if ((status == HID_UART_SUCCESS || status == HID_UART_READ_TIMED_OUT) && (length > 0))
	{
		buffer[length] = 0;
		for(uint32_t i = 0; i < length ; i++)
		{
			if ((buffer[i] == '\n') || (buffer[i] == '\r'))
			{
				if (rxLength)
				{
					rxBuffer[rxLength] = 0;
					OnRead(rxBuffer, rxLength);
					rxLength = 0;
					remainLength = 0;
				}
			}
			else
			{
				if (rxLength < HID_UART_MAX_READ_SIZE)
				{
					rxBuffer[rxLength++] = buffer[i];
					if (rxLength == HID_UART_MAX_READ_SIZE)
					{
						//rxBuffer[rxLength] = 0;
						//OnRead(rxBuffer, rxLength);
						//rxLength = 0;
					}
				}
			}
		}
	}

	if (rxLength != 0)
	{
		memcpy(buffer, rxBuffer, rxLength);
		remainLength = rxLength;
	}
}

void	HIDUART::Postprocess()
{
	if (uart_)
	{
		HidUart_Close(uart_);
		uart_ = NULL;
	}
}

bool	HIDUART::OnRead(uint8_t* _data, uint32_t _length)
{
	return	true;
}

bool	HIDUART::OnWrite(uint8_t* _data, uint32_t _length)
{

	uint32_t	writeLength = 0;

	TRACE_DEBUG("OnWrite(" << (char *)_data << ")");
	HidUart_Write(uart_, _data, _length, &writeLength);

	return	(_length == writeLength);
}

// Convert HID_UART_STATUS value to a printable string
std::string HIDUART::DecodeHidUartStatus(HID_UART_STATUS status)
{
    std::string text;
    char temp[100];
    
    switch (status)
    {
    case HID_UART_SUCCESS:                      text = "HID_UART_SUCCESS"; break;
    case HID_UART_DEVICE_NOT_FOUND:             text = "HID_UART_DEVICE_NOT_FOUND"; break;
    case HID_UART_INVALID_HANDLE:               text = "HID_UART_INVALID_HANDLE"; break;
    case HID_UART_INVALID_DEVICE_OBJECT:        text = "HID_UART_INVALID_DEVICE_OBJECT"; break;
    case HID_UART_INVALID_PARAMETER:            text = "HID_UART_INVALID_PARAMETER"; break;
    case HID_UART_INVALID_REQUEST_LENGTH:       text = "HID_UART_INVALID_REQUEST_LENGTH"; break;
    case HID_UART_READ_ERROR:                   text = "HID_UART_READ_ERROR"; break;
    case HID_UART_WRITE_ERROR:                  text = "HID_UART_WRITE_ERROR"; break;
    case HID_UART_READ_TIMED_OUT:               text = "HID_UART_READ_TIMED_OUT"; break;
    case HID_UART_WRITE_TIMED_OUT:              text = "HID_UART_WRITE_TIMED_OUT"; break;
    case HID_UART_DEVICE_IO_FAILED:             text = "HID_UART_DEVICE_IO_FAILED"; break;
    case HID_UART_DEVICE_ACCESS_ERROR:          text = "HID_UART_DEVICE_ACCESS_ERROR"; break;
    case HID_UART_DEVICE_NOT_SUPPORTED:         text = "HID_UART_DEVICE_NOT_SUPPORTED"; break;
    case HID_UART_UNKNOWN_ERROR:                text = "HID_UART_UNKNOWN_ERROR"; break;
    default:
        sprintf(temp, "Error Status (0x%02X)", status);
        text = temp;
        break;
    }
    
    return text;
}


uint32_t	HIDUART::GetDeviceCount(uint16_t _pid, uint16_t _vid)
{
	HID_UART_STATUS	status;
	uint32_t		count = 0;
		
	status = HidUart_GetNumDevices(&count, _vid, _pid);

	if (status == HID_UART_SUCCESS)
	{
		return	count;
	}

	return	0;
}

bool	HIDUART::GetSerial(uint16_t _vid, uint16_t _pid, uint32_t _index, std::string& _serial)
{
	char	serial[HID_UART_DEVICE_STRLEN];

	if (HidUart_GetString(_index, _vid, _pid, serial, HID_UART_GET_SERIAL_STR) != HID_UART_SUCCESS)
	{
		return	false;
	}

	_serial = serial;	

	return	true;
}

bool	HIDUART::SetSerialID(Object* _object, JSONNode const& _value)
{
	HIDUART*	hiduart = dynamic_cast<HIDUART*>(_object);
	if (!hiduart)
	{
		return	false;	
	}

	return	hiduart->SetSerialID(_value.as_string());
}

bool	HIDUART::SetPort(Object* _object, JSONNode const& _value)
{
	HIDUART*	hiduart = dynamic_cast<HIDUART*>(_object);
	if (!hiduart)
	{
		return	false;	
	}

	return	hiduart->SetPort(_value.as_int());
}

bool	HIDUART::SetSerialRxTimeout(Object* _object, JSONNode const& _value)
{
	HIDUART*	hiduart = dynamic_cast<HIDUART*>(_object);
	if (!hiduart)
	{
		return	false;	
	}

	return	hiduart->SetSerialRxTimeout(_value.as_int());
}

bool	HIDUART::SetSerialTxTimeout(Object* _object, JSONNode const& _value)
{
	HIDUART*	hiduart = dynamic_cast<HIDUART*>(_object);
	if (!hiduart)
	{
		return	false;	
	}

	return	hiduart->SetSerialTxTimeout(_value.as_int());
}
