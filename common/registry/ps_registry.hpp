//
//  ps_registry.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_registry_hpp
#define ps_registry_hpp

#include <set>
#include <map>
#include <string>
#include "common/ps_root_class.hpp"
#include "queue/ps_queue_class.hpp"
#include "ps_registry_types.hpp"

class ps_registry : public ps_root_class {
  
public:
    ps_registry_datatype_t get_registry_type(const char *domain, const char *name);
    ps_registry_flags_t    get_registry_flags(const char *domain, const char *name);
    
	ps_result_enum add_new_registry_entry(const char *domain, const char *name,
                                      ps_registry_datatype_t type, ps_registry_flags_t flags);
    
    ps_result_enum set_new_registry_entry(const char *domain, const char *name,const ps_registry_api_struct& set_value);
    
	ps_result_enum set_registry_entry(const char *domain, const char *name, const ps_registry_api_struct& set_value);
	ps_result_enum get_registry_entry(const char *domain, const char *name, ps_registry_api_struct& get_value);
    
	ps_result_enum set_observer(const char *domain, const char *name, ps_registry_callback_t *callback, const void *arg);
    ps_result_enum interate_domain(const char *domain, ps_registry_callback_t *callback, const void *arg);

    ps_result_enum reset_registry();
    
    ////////////////////// MANAGEMENT
    ps_result_enum ps_registry_request_sync();

    ////////////////////// MESSAGE HANDLER
    void message_handler(ps_packet_source_t packet_source,
                         ps_packet_type_t   packet_type,
                         const void *msg, int length) override;
    //observer callbacks
    void process_observed_data(ps_root_class *src, const void *msg, int length) override {}
    void process_observed_event(ps_root_class *src, int event) override {}
    
    //save registry
    ps_result_enum save_registry_method(const char *domain, void *arg, void (*write_line_callback)(void *arg, const char *linebuffer));
    
protected:
	ps_registry();
    
    //shared update message
    ps_registry_update_packet_t updateMessage;
    
    ps_result_enum ps_registry_send_sync();

    void registry_thread_method();
    ps_queue_class *registryQueue {nullptr};
    
    void notify_domain_observers(const char *domain, const char *name);

    typedef std::map<std::string, ps_registry_entry_t*> domain_set_t;   //a set per domain, mapping name to registry data
	std::map<std::string, domain_set_t*> registry;                     //set of domains

	friend ps_registry& the_registry();
    
    std::set<std::string> local_domains;    //used to determine domain ownership
    
    int count_entries_in_domain(std::string domain);
    void resend_all_updates(std::string domain);

    DECLARE_MUTEX(registryMtx);
    
    friend ps_result_enum ps_registry_add_new(const char *domain, const char *name,
        ps_registry_datatype_t type, ps_registry_flags_t flags);
    friend ps_result_enum ps_registry_set_new(const char *domain, const char *name, const ps_registry_api_struct &data);
    friend ps_result_enum ps_registry_set(const char *domain, const char *name, const ps_registry_api_struct &data);
};

ps_registry& the_registry();

#endif /* ps_registry_hpp */
