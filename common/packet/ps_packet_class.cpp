//
//  ps_packet_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_class.hpp"
#include "pubsub/ps_pubsub_class.hpp"

ps_packet_class::ps_packet_class(const char *name) : ps_root_class(name)
{
}

ps_packet_class::ps_packet_class(std::string name) : ps_root_class(name)
{
}

ps_packet_class::~ps_packet_class()
{

}
