//
//  ps_serial_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_serial_class.hpp"

ps_serial_class::ps_serial_class(const char *name)
	: ps_root_class(name)
{

}


ps_serial_status_enum ps_serial_class::get_serial_status()
{
	return serial_status;
}
