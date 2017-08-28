//
//  ps_serial_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "serial/linux_serial/ps_serial_linux.hpp"
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

ps_serial_linux::ps_serial_linux(const char *_name, const char *devicePath, unsigned int baudrate)
: ps_serial_class(_name)
{
	struct termios settings;

	//initialize UART
	while ((FD = open(devicePath, O_RDWR | O_NOCTTY)) < 0)
	{
		PS_ERROR("ser: %s failed to open %s - %s", _name, devicePath, strerror(errno));
		sleep(1);
	}


	if (tcgetattr(FD, &settings) < 0)
	{
		PS_ERROR("ser: %s tcgetattr fail %s - %s", _name, devicePath, strerror(errno));
	}

	//no processing
	settings.c_iflag = 0;
	settings.c_oflag = 0;
	settings.c_lflag = 0;
	settings.c_cflag = CLOCAL | CREAD | CS8;        //no modem, 8-bits

	if (baudrate)
	{
		//baudrate
		if ((cfsetospeed(&settings, baudrate) < 0) || (cfsetispeed(&settings, baudrate) < 0))
		{
			PS_ERROR("ser: %s cfsetospeed fail %s - %s", _name, devicePath, strerror(errno));
		}
	}
	if (tcsetattr(FD, TCSANOW, &settings) < 0)
	{
		PS_ERROR("ser: %s tcgetattr fail %s - %s", _name, devicePath, strerror(errno));
	}
}

ps_serial_linux::ps_serial_linux(const char *_name, int fd) : ps_serial_class(_name)
{
	FD = fd;
}

ps_serial_linux::~ps_serial_linux()
{

}

bool ps_serial_linux::data_available()
{
	return true;
}

//receive bytes
int ps_serial_linux::read_bytes(void *data, int length)
{
	int readchars = 0;
	int len = length;
	uint8_t *next = (uint8_t *) data;

	do {
		int chars = read(FD, next, len);
		if (chars > 0)
		{
			readchars += chars;
			next += chars;
			len -= chars;
		}
		else if (chars < 0)
		{
			if (errno != EAGAIN) return -1;
		}
	} while (len > 0);

	return readchars;
}

//send bytes
ps_result_enum ps_serial_linux::write_bytes(const void *data, int _length)
{
	int written;
	int length = _length;
	unsigned char *next = const_cast<unsigned char *>(static_cast<const unsigned char *>(data));

	do {
		written = write(FD, next, length);
		if (written >= 0)
		{
			next += written;
			length -= written;
		}
		else if (written != EAGAIN)
		{
			return PS_IO_ERROR;
		}
	} while (length > 0);

	return PS_OK;
}

