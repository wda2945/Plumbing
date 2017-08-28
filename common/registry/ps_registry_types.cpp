//
//  psRegistry_types.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_registry_types.hpp"

//////////////////METHODS FOR PS_REGISTRY_VALUE

////New
ps_registry_entry_t *new_registry_entry(int dataType)
{
    if (dataType >= 0 && dataType < PS_DATATYPE_COUNT)
    {
        ps_registry_entry_t *new_reg = (ps_registry_entry_t*) MEMORY_ALLOC(sizeof(ps_registry_entry_t));
        if (new_reg)
        {
            new_reg->entry.serial   = 0;
            new_reg->entry.source   = SOURCE;
            new_reg->entry.datatype = dataType;
            new_reg->entry.flags 	= PS_REGISTRY_DEFAULT;
            
            switch(dataType)
            {
                case PS_REGISTRY_ANY_TYPE:
                    new_reg->entry.skip_count = 0;
                    break;
                case PS_REGISTRY_INT_TYPE:
                    new_reg->entry.int_value = 0;
                    break;
                case PS_REGISTRY_REAL_TYPE:
                    new_reg->entry.real_value = 0;
                    break;
                case PS_REGISTRY_TEXT_TYPE:
                    new_reg->entry.string_value = (char*) MEMORY_ALLOC(REGISTRY_TEXT_LENGTH);
                    if (!new_reg->entry.string_value)
                    {
#ifdef MEMORY_FREE
                        MEMORY_FREE(new_reg);
#endif
                        return nullptr;
                    }
                    new_reg->entry.string_value[0] = '\0';
                    break;
                case PS_REGISTRY_BOOL_TYPE:
                    new_reg->entry.bool_value = false;
                    break;
                case PS_REGISTRY_SETTING_TYPE:
                    new_reg->entry.setting = (setting_t*) MEMORY_ALLOC(sizeof(setting_t));
                    if (!new_reg->entry.setting)
                    {
#ifdef MEMORY_FREE
                        MEMORY_FREE(new_reg);
#endif
                        return nullptr;
                    }
                    new_reg->entry.setting->value   = 0;
                    new_reg->entry.setting->minimum = 0;
                    new_reg->entry.setting->maximum = 1.0;
                    break;
                default:
                    break;
            }
		}
        else {
            return nullptr;
        }
		new_reg->observers = nullptr;
		return new_reg;
	}
	else
	{
		return nullptr;
	}
}


//update values if type matches
//returns true if data changed
bool copy_data_in(registry_data_struct& to, const ps_registry_api_struct& from)
{
	bool dataCopied = false;

	if (to.datatype == from.datatype)
	{
		switch(to.datatype)
		{
		case PS_REGISTRY_TEXT_TYPE:
			if (strncmp(to.string_value, from.string_value, REGISTRY_TEXT_LENGTH) != 0)
			{
				strncpy(to.string_value, from.string_value, REGISTRY_TEXT_LENGTH);
				dataCopied = true;
			}
			break;
		case PS_REGISTRY_INT_TYPE:
			if (to.int_value != from.int_value)
			{
				to.int_value = from.int_value;
				dataCopied = true;
			}
			break;
		case PS_REGISTRY_REAL_TYPE:
			if (to.real_value != from.real_value)
			{
				to.real_value = from.real_value;
				dataCopied = true;
			}
			break;
		case PS_REGISTRY_BOOL_TYPE:
			if (to.bool_value != from.bool_value)
			{
				to.bool_value = from.bool_value;
				dataCopied = true;
			}
			break;
		case PS_REGISTRY_SETTING_TYPE:
			if (to.setting->value != from.setting.value)
			{
				to.setting->value   = from.setting.value;
				to.setting->maximum = from.setting.maximum;
				to.setting->minimum = from.setting.minimum;
				dataCopied = true;
			}
			break;
		default:
			break;
		}
		return dataCopied;
	}
	else
	{
		return false;
	}
}

////copying out

void copy_data_out(ps_registry_api_struct& to, const registry_data_struct& from)
{
	to.datatype = from.datatype;
	to.flags    = from.flags;
	to.source   = from.source;
	to.serial   = from.serial;

	switch(to.datatype)
	{
	case PS_REGISTRY_TEXT_TYPE:
		strncpy(to.string_value, from.string_value, REGISTRY_TEXT_LENGTH);
		break;
	case PS_REGISTRY_INT_TYPE:
		to.int_value = from.int_value;
		break;
	case PS_REGISTRY_REAL_TYPE:
		to.real_value = from.real_value;
		break;
	case PS_REGISTRY_BOOL_TYPE:
		to.bool_value = from.bool_value;
		break;
	case PS_REGISTRY_SETTING_TYPE:
		to.setting.value    = from.setting->value;
		to.setting.maximum  = from.setting->maximum;
		to.setting.minimum  = from.setting->minimum;
		break;
	default:
		break;
	}
}

// Create Update Packet to publish registry change

int copy_update_packet(ps_registry_update_packet_t& to, const registry_data_struct& from, const char *_domain, const char* _name)
{
	to.value.datatype = from.datatype;
	to.value.flags    = from.flags;
	to.value.source   = from.source;
	to.value.serial   = from.serial;

	switch(from.datatype)
	{
	case PS_REGISTRY_TEXT_TYPE:
		strncpy(to.value.string_value, from.string_value, REGISTRY_TEXT_LENGTH);
		break;
	case PS_REGISTRY_INT_TYPE:
		to.value.int_value = from.int_value;
		break;
	case PS_REGISTRY_REAL_TYPE:
		to.value.real_value = from.real_value;
		break;
	case PS_REGISTRY_BOOL_TYPE:
		to.value.bool_value = from.bool_value;
		break;
	case PS_REGISTRY_SETTING_TYPE:
		to.value.setting.value      = from.setting->value;
		to.value.setting.maximum    = from.setting->maximum;
		to.value.setting.minimum    = from.setting->minimum;
		break;
	default:
		break;
	}
    strncpy(to.domain, _domain, REGISTRY_DOMAIN_LENGTH);
    strncpy(to.name, _name, REGISTRY_NAME_LENGTH);

	return get_update_packet_length(to);
}


/////////////////// Message sizing

int get_update_packet_length(ps_registry_update_packet_t& pkt)
{
	int value_size = 0;

	switch(pkt.value.datatype)
	{
	case PS_REGISTRY_INT_TYPE:
		value_size = sizeof(int);
		break;
	case PS_REGISTRY_REAL_TYPE:
		value_size = sizeof(float);
		break;
	case PS_REGISTRY_TEXT_TYPE:
		pkt.value.string_value[REGISTRY_TEXT_LENGTH-1] = '\0';      // make sure we don't overrun
		value_size = (int) strlen(pkt.value.string_value) + 1;      // +1 to include '\0'
		break;
	case PS_REGISTRY_BOOL_TYPE:
		value_size = sizeof(bool);
		break;
	case PS_REGISTRY_SETTING_TYPE:
		value_size = sizeof(setting_t);
		break;
	default:
		value_size = 0;
		break;
	}

	return (value_size + REGISTRY_DOMAIN_LENGTH + REGISTRY_NAME_LENGTH
			+ sizeof(ps_registry_serial_t)
			+ sizeof(Source_t)
			+ sizeof(ps_registry_datatype_t)
			+ sizeof(ps_registry_flags_t));
}

int get_sync_packet_length(ps_registry_sync_packet_t& pkt)
{
	pkt.domain[REGISTRY_DOMAIN_LENGTH] = '\0';
	return (int)(strlen(pkt.domain) + 3);       // +1 to include '\0'
}
