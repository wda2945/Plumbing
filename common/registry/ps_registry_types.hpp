//
//  ps_registry_types.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_registry_types_hpp
#define ps_registry_types_hpp

#include <set>
#include <map>
#include <string>
#include <string.h>
#include "common/ps_root_class.hpp"
#include "ps_api/ps_registry.h"
#include "ps_registry_message.h"

/////////////////// Observer class

class ps_observer_class {
public:
	ps_observer_class( ps_registry_callback_t *_callback, const void *_arg){
		observer_callback = _callback;
		arg = const_cast<void*>(_arg);
	}

	ps_registry_callback_t *observer_callback;
	void *arg;
};

/////////////////////Registry entry struct
//data as held in the set registry

typedef std::set<ps_observer_class*> observer_set_t;

typedef struct {
	float minimum, maximum, value;
} setting_t;

typedef     struct {
    ps_registry_serial_t serial {1};
    Source_t source {SRC_UNKNOWN};
    ps_registry_datatype_t datatype {PS_REGISTRY_ANY_TYPE};
    ps_registry_flags_t flags {PS_REGISTRY_DEFAULT};

    union {
        char *string_value;
        int int_value;
        float real_value;
        bool bool_value;
        setting_t *setting;
        int skip_count;
    };
} registry_data_struct;

typedef struct  {
	registry_data_struct entry;
    observer_set_t *observers;
} ps_registry_entry_t;

/////////////////// Registry data value helpers
//ps_registry_api_struct used by C api

//new
ps_registry_entry_t *new_registry_entry(int dataType);

//copying
//copying in
    bool copy_data_in(registry_data_struct& to, const ps_registry_api_struct& from);
//copying out
    void copy_data_out(ps_registry_api_struct& to, const registry_data_struct& from);
//copy for update packet
    int copy_update_packet(ps_registry_update_packet_t& to, const registry_data_struct& from, const char *_domain, const char* _name);

#endif /* ps_registry_types_hpp */
