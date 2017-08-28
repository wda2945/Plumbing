//
//  psRegistry.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <string.h>
#include <string>
#include <exception>
#include <typeinfo>
#include <stdio.h>
#include "ps_registry.hpp"
#include "ps_config.h"
#include "pubsub/ps_pubsub_class.hpp"
#include "notify/ps_notify.hpp"

//#undef PS_TRACE
//#define PS_TRACE(...) PS_DEBUG(__VA_ARGS__)

#define SYNC_SKIP_COUNT 5

ps_registry::ps_registry() :
		ps_root_class(std::string("registry")) {
	PS_TRACE("registry: new");

	the_broker().register_object(REGISTRY_UPDATE_PACKET, this);
	the_broker().register_object(REGISTRY_SYNC_PACKET, this);

	INIT_MUTEX(registryMtx);
}

//helpers

const char *typeNames[] = REGISTRY_TYPE_NAMES;

std::string get_domain_string(const char *_domain) {
	char domain[REGISTRY_DOMAIN_LENGTH + 1];
	strncpy(domain, _domain, REGISTRY_DOMAIN_LENGTH);
	domain[REGISTRY_DOMAIN_LENGTH] = '\0';
	return std::string(domain);
}

std::string get_name_string(const char *_name) {
	char name[REGISTRY_NAME_LENGTH + 1];
	strncpy(name, _name, REGISTRY_NAME_LENGTH);
	name[REGISTRY_NAME_LENGTH] = '\0';
	return std::string(name);
}

ps_registry_datatype_t ps_registry::get_registry_type(const char *domain,
		const char *name) {

	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(get_domain_string(domain)); //find the domain set<>
	if (domain_pos == registry.end()) {
		UNLOCK_MUTEX(registryMtx);
		PS_DEBUG("reg: get_type: unknown domain %s", domain);
		return PS_REGISTRY_ANY_TYPE; //unknown domain
	}

	domain_set_t *domain_set = domain_pos->second;

	auto pos = domain_set->find(get_name_string(name)); //find the name object (ps_registry_entry_class)
	if (pos == domain_set->end()) {
		UNLOCK_MUTEX(registryMtx);
		PS_DEBUG("reg: get_type: unknown name %s", name);
		return PS_REGISTRY_ANY_TYPE; //unknown name
	}
	ps_registry_entry_t *reg_entry = pos->second;
	ps_registry_datatype_t type = reg_entry->entry.datatype;

	UNLOCK_MUTEX(registryMtx);

	PS_DEBUG("reg: %s type %s/%s", typeNames[type], domain, name);

	return type;
}

ps_registry_flags_t ps_registry::get_registry_flags(const char *domain,
		const char *name) {
	PS_TRACE("reg: get flags %s/%s", domain, name);
	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(get_domain_string(domain)); //find the domain set<>
	if (domain_pos == registry.end()) {
		UNLOCK_MUTEX(registryMtx);
		PS_DEBUG("reg: get_flags: unknown domain %s", domain);
		return PS_REGISTRY_INVALID_FLAGS; //unknown domain
	}

	domain_set_t *domain_set = domain_pos->second;

	auto pos = domain_set->find(get_name_string(name)); //find the name object (ps_registry_entry_class)
	if (pos == domain_set->end()) {
		UNLOCK_MUTEX(registryMtx);
		PS_DEBUG("reg: get_flags: unknown name %s", name);
		return PS_REGISTRY_INVALID_FLAGS; //unknown name
	}
	ps_registry_entry_t *reg_entry = pos->second;
	ps_registry_flags_t flags = reg_entry->entry.flags;

	UNLOCK_MUTEX(registryMtx);

	PS_TRACE("reg: %s/%s flags %x", domain, name, flags);

	return flags;
}

//add a new entry into the registry
//Just the entry - initialized data

ps_result_enum ps_registry::add_new_registry_entry(const char *_domain,
		const char *_name, ps_registry_datatype_t type,
		ps_registry_flags_t flags) {

	PS_DEBUG("reg: add_new: %s/%s (%s)", _domain, _name, typeNames[type]);

	domain_set_t *domain_set;
	std::string domain = get_domain_string(_domain);
	std::string name = get_name_string(_name);

	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain); //get the domain set
	if (domain_pos == registry.end()) {
		//new domain
		domain_set = new domain_set_t();

		if (domain_set == nullptr) {
			UNLOCK_MUTEX(registryMtx);
			PS_ERROR("reg: add_new: %s/%s : no memory", _domain, _name);
			return PS_NO_MEMORY;
		}

		//note it as local
		local_domains.insert(domain);

		PS_DEBUG("reg: add_new: new %s", _domain);
		try {
			registry.insert(std::make_pair(domain, domain_set));
		} catch (std::exception &e) {
			UNLOCK_MUTEX(registryMtx);
			PS_ERROR("reg: exception: %s - %s", e.what(), typeid (e).name());
			return PS_EXCEPTION;
		}
	} else {
		domain_set = domain_pos->second;
	}

	auto pos = domain_set->find(name); //get the name set
	if (pos == domain_set->end()) {
		//new entry
		ps_registry_entry_t *new_entry = new_registry_entry(type);

		if (new_entry == nullptr) {
			UNLOCK_MUTEX(registryMtx);
			PS_ERROR("reg: add_new: %s/%s : no memory", _domain, _name);
			return PS_NO_MEMORY;
		}

		new_entry->entry.flags = flags;
		new_entry->entry.serial = 1;

		try {
			domain_set->insert(std::make_pair(name, new_entry));
		} catch (std::exception &e) {
			UNLOCK_MUTEX(registryMtx);
			PS_ERROR("reg: exception: %s - %s", e.what(), typeid (e).name());
			return PS_EXCEPTION;
		}

		UNLOCK_MUTEX(registryMtx);

		return PS_OK;
	} else {
		UNLOCK_MUTEX(registryMtx);
		PS_DEBUG("reg: name %s exists", _name);
		return PS_NAME_EXISTS;
	}
}

//Add new entry and include data values
ps_result_enum ps_registry::set_new_registry_entry(const char *_domain,
		const char *_name, const ps_registry_api_struct& set_value) {
	PS_DEBUG("reg: set_new: %s/%s (%s)", _domain, _name,
			typeNames[set_value.datatype]);

	domain_set_t *domain_set;
	std::string domain = get_domain_string(_domain);
	std::string name = get_name_string(_name);

	ps_registry_datatype_t type = set_value.datatype;

	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain); //get the domain set
	if (domain_pos == registry.end()) {
		//new domain
		domain_set = new domain_set_t();

		if (domain_set == nullptr) {
			PS_ERROR("reg: set_new: %s/%s : no memory", _domain, _name);
			UNLOCK_MUTEX(registryMtx);
			return PS_NO_MEMORY;
		}

		//note it as local
		local_domains.insert(domain);

		PS_DEBUG("reg: set_new: new %s", _domain);

		try {
			registry.insert(std::make_pair(domain, domain_set));
		} catch (std::exception &e) {
			UNLOCK_MUTEX(registryMtx);
			PS_ERROR("reg: exception: %s - %s", e.what(), typeid (e).name());
			return PS_EXCEPTION;
		}
	} else {
		domain_set = domain_pos->second;
	}

	auto pos = domain_set->find(name); //get the name set
	if (pos == domain_set->end()) {

		//not found
		ps_registry_entry_t *new_entry = new_registry_entry(type);

		if (new_entry == nullptr) {
			PS_ERROR("reg: set_new: %s/%s : no memory", _domain, _name);
			UNLOCK_MUTEX(registryMtx);
			return PS_NO_MEMORY;
		}

		new_entry->entry.flags = set_value.flags;
		new_entry->entry.serial = 1;

		copy_data_in(new_entry->entry, set_value);

		try {
			domain_set->insert(std::make_pair(name, new_entry));
		} catch (std::exception &e) {
			UNLOCK_MUTEX(registryMtx);
			PS_ERROR("reg: exception: %s - %s", e.what(), typeid (e).name());
			return PS_EXCEPTION;
		}

		int update_pkt_size = copy_update_packet(updateMessage,
				new_entry->entry, _domain, _name);

		if ((set_value.flags & PS_REGISTRY_LOCAL) == 0) {
			//publish
			PS_DEBUG("reg: update set_new: %s/%s", _domain, _name);
			the_broker().publish_packet(REGISTRY_UPDATE_PACKET, &updateMessage,
					update_pkt_size);
		}

		UNLOCK_MUTEX(registryMtx);

		notify_domain_observers(_domain, _name);

		return PS_OK;
	} else {
		UNLOCK_MUTEX(registryMtx);
		PS_DEBUG("reg: name %s exists", _name);
		return PS_NAME_EXISTS;
	}
}

//copy value into the registry

ps_result_enum ps_registry::set_registry_entry(const char *_domain,
		const char *_name, const ps_registry_api_struct &new_value) {
	PS_TRACE("reg: set %s/%s", _domain, _name);

	std::string domain = get_domain_string(_domain);
	std::string name = get_name_string(_name);

	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain); //get the domain set
	if (domain_pos == registry.end()) {
		UNLOCK_MUTEX(registryMtx);
		PS_ERROR("reg: set: %s not found", _domain);
		return PS_NAME_NOT_FOUND;
	}

	domain_set_t *domain_set = domain_pos->second;

	auto pos = domain_set->find(name);
	if (pos == domain_set->end()) {
		UNLOCK_MUTEX(registryMtx);
		PS_ERROR("reg: set: %s not found", _name);
		return PS_NAME_NOT_FOUND;
	} else {

		//copy value if type matches
		ps_registry_entry_t *reg_entry = pos->second;

		//returns true if data actually changed
		bool dataCopied = copy_data_in(reg_entry->entry, new_value);

		if (dataCopied) {
			//entry changed
			reg_entry->entry.serial++;

			ps_registry_flags_t flags = reg_entry->entry.flags;

			//collect observers to call after the mutex is released
			std::set<ps_observer_class*> _observers;
			if (reg_entry->observers) {
				//copy the observers (refs))
				for (ps_observer_class *obs : *reg_entry->observers) {
					_observers.insert(obs);
				}
			}

			int update_pkt_size = copy_update_packet(updateMessage,
					reg_entry->entry, _domain, _name);

			if ((flags & PS_REGISTRY_LOCAL) == 0) {
				PS_DEBUG("reg: update: %s/%s", _domain, _name);
				the_broker().publish_packet(REGISTRY_UPDATE_PACKET,
						&updateMessage, update_pkt_size);
			}

			UNLOCK_MUTEX(registryMtx);

			//notify observers
			for (auto obs : _observers) {
				(obs->observer_callback)(_domain, _name, obs->arg);
			}

			notify_domain_observers(_domain, _name);

			return PS_OK;
		} else {
			UNLOCK_MUTEX(registryMtx);
			return PS_INVALID_PARAMETER;
		}
	}
}

//copy entry from the registry

ps_result_enum ps_registry::get_registry_entry(const char *_domain,
		const char *_name, ps_registry_api_struct &get_value) {
	PS_TRACE("reg: get %s/%s", _domain, _name);

	domain_set_t *domain_set;
	std::string domain = get_domain_string(_domain);
	std::string name = get_name_string(_name);

	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain); //get the domain set
	if (domain_pos == registry.end()) {
		UNLOCK_MUTEX(registryMtx);
		PS_DEBUG("reg: get: %s not found", _domain);
		return PS_NAME_NOT_FOUND;
	} else {
		domain_set = domain_pos->second;
	}

	auto pos = domain_set->find(name);
	if (pos == domain_set->end()) {
		UNLOCK_MUTEX(registryMtx);
		PS_DEBUG("reg: get: %s not found", _name);
		return PS_NAME_NOT_FOUND;
	} else {
		ps_registry_entry_t *reg_entry = pos->second;
		copy_data_out(get_value, reg_entry->entry);
	}
	UNLOCK_MUTEX(registryMtx);
	return PS_OK;
}

ps_result_enum ps_registry::set_observer(const char *_domain, const char *_name,
		ps_registry_callback_t *callback, const void *arg) {
	PS_DEBUG("reg: set_observer: %s/%s", _domain, _name);

	domain_set_t *domain_set;
	std::string domain = get_domain_string(_domain);
	std::string name = get_name_string(_name);

	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain); //get the domain set
	if (domain_pos == registry.end()) {
		//make an entry for domain
		domain_set = new domain_set_t();

		if (domain_set == nullptr) {
			PS_ERROR("set_observer: %s : no memory", _domain);
			UNLOCK_MUTEX(registryMtx);
			return PS_NO_MEMORY;
		}
		try {
			registry.insert(std::make_pair(domain, domain_set));
		} catch (std::exception &e) {
			UNLOCK_MUTEX(registryMtx);
			PS_ERROR("reg: exception: %s - %s", e.what(), typeid (e).name());
			return PS_EXCEPTION;
		}
	} else {
		domain_set = domain_pos->second;
	}

	auto pos = domain_set->find(name);

	if ((pos == domain_set->end()) && (name == "any")) {
		//make an entry for the domain
		ps_registry_entry_t *new_entry = new_registry_entry(
				PS_REGISTRY_ANY_TYPE);

		if (new_entry == nullptr) {
			PS_ERROR("set_observer: %s/%s : no memory", _domain, _name);
			UNLOCK_MUTEX(registryMtx);
			return PS_NO_MEMORY;
		}
		new_entry->entry.flags = PS_REGISTRY_LOCAL;

		try {
			domain_set->insert(std::make_pair(name, new_entry));
		} catch (std::exception &e) {
			UNLOCK_MUTEX(registryMtx);
			PS_ERROR("reg: exception: %s - %s", e.what(), typeid (e).name());
			return PS_EXCEPTION;
		}

		pos = domain_set->find(name);
	}

	if (pos != domain_set->end()) {
		ps_observer_class *obs = new ps_observer_class(callback, arg);

		if (obs == nullptr) {
			PS_ERROR("set_observer: %s : no memory", _name);
			UNLOCK_MUTEX(registryMtx);
			return PS_NO_MEMORY;
		}

		ps_registry_entry_t *reg_entry = pos->second;

		if (reg_entry->observers == nullptr) {
			reg_entry->observers = new observer_set_t;

			if (reg_entry->observers == nullptr) {
				PS_ERROR("set_observer: %s/%s : no memory", _domain, _name);
				UNLOCK_MUTEX(registryMtx);
				return PS_NO_MEMORY;
			}
		}
		try {
			reg_entry->observers->insert(obs);
		} catch (std::exception &e) {
			UNLOCK_MUTEX(registryMtx);
			PS_ERROR("reg: exception: %s - %s", e.what(), typeid (e).name());
			return PS_EXCEPTION;
		}

		UNLOCK_MUTEX(registryMtx);
		return PS_OK;
	} else {
		PS_DEBUG("reg: set_observer: %s/%s not found", domain.c_str(),
				name.c_str());
		UNLOCK_MUTEX(registryMtx);
		return PS_NAME_NOT_FOUND;
	}
}

void ps_registry::notify_domain_observers(const char *_domain,
		const char *_name) {
	PS_TRACE("reg: domain_observers: %s/%s", _domain, _name);

	domain_set_t *domain_set;
	std::set<ps_observer_class*> _observers;

	std::string domain = get_domain_string(_domain);
	std::string any = get_name_string("any");

	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain); //get the domain set
	if (domain_pos != registry.end()) {
		domain_set = domain_pos->second;

		auto pos = domain_set->find(any);
		if (pos != domain_set->end()) {

			ps_registry_entry_t *reg_entry = pos->second;
			if (reg_entry->observers) {
				for (auto obs : *reg_entry->observers) {
					_observers.insert(obs);
				}
			}
		}
	}

	UNLOCK_MUTEX(registryMtx);

	//notify observers
	for (auto obs : _observers) {
		(obs->observer_callback)(_domain, _name, obs->arg);
	}

	PS_TRACE("reg: notify %i domain_obs: %s/%s", (int)_observers.size(), _domain, _name);
}

ps_result_enum ps_registry::interate_domain(const char *_domain,
		ps_registry_callback_t *callback, const void *arg) {
	PS_DEBUG("reg: iterate: %s", _domain);

	domain_set_t *domain_set;
	std::string domain = get_domain_string(_domain);

	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain); //get the domain set
	if (domain_pos == registry.end()) {
		UNLOCK_MUTEX(registryMtx);
		PS_DEBUG("reg: iterate: %s not found", _domain);
		return PS_NAME_NOT_FOUND;
	} else {
		domain_set = domain_pos->second;
	}

	for (auto pos = domain_set->begin(); pos != domain_set->end(); pos++) {
		std::string name = pos->first;

		UNLOCK_MUTEX(registryMtx);
		(callback)(_domain, name.c_str(), arg);
		LOCK_MUTEX(registryMtx);
	}
	UNLOCK_MUTEX(registryMtx);
	return PS_OK;
}

ps_result_enum ps_registry::reset_registry() {

#ifdef MEMORY_FREE
	PS_DEBUG("reg: reset_registry");

	LOCK_MUTEX(registryMtx);

	//free data storage
	for (auto domain_pos : registry) {
		for (auto name_pos : *domain_pos.second) {

			ps_registry_entry_t *reg_entry = name_pos.second;

			switch (reg_entry->entry.datatype) {
				case PS_REGISTRY_TEXT_TYPE:
				MEMORY_FREE(reg_entry->entry.string_value);
				reg_entry->entry.string_value = nullptr;
				break;
				case PS_REGISTRY_SETTING_TYPE:
				MEMORY_FREE(reg_entry->entry.setting);
				reg_entry->entry.setting = nullptr;
				break;
				default:
				break;
			}
			reg_entry->entry.datatype = PS_REGISTRY_ANY_TYPE;

			delete reg_entry->observers;
			reg_entry->observers = nullptr;
		}
	}

	registry.clear();
	local_domains.clear();

	UNLOCK_MUTEX(registryMtx);
	return PS_OK;
#else
	PS_ERROR("reg: reset_registry not implemented");
	return PS_NOT_IMPLEMENTED;
#endif
}

int ps_registry::count_entries_in_domain(std::string domain) {

	int count = 0;

	LOCK_MUTEX(registryMtx);

	auto regIterator = registry.find(domain);

	if (regIterator != registry.end()) {
		domain_set_t *domain_set = regIterator->second;
		count = (int) domain_set->size();

		std::string any = get_name_string("any");

		if (domain_set->find(any) != domain_set->end()) {
			count--;	//don't count the dummy put in for domain observers
		}
	}
	UNLOCK_MUTEX(registryMtx);

	PS_TRACE("reg: %i entries in %s", count, domain.c_str());

	return count;
}

//send sync packets
ps_result_enum ps_registry::ps_registry_request_sync() {
	ps_packet_source_t packet_source = SOURCE;
	ps_packet_type_t packet_type = REGISTRY_SYNC_REQUEST;

	registryQueue->copy_2message_parts_to_q(&packet_source,
			sizeof(ps_packet_source_t), &packet_type, sizeof(ps_packet_type_t));

	return PS_OK;
}

ps_result_enum ps_registry::ps_registry_send_sync() {
#ifndef REGISTRY_LOCAL_ONLY
//	PS_DEBUG("reg: send sync");

	ps_registry_sync_packet_t sync;

	LOCK_MUTEX(registryMtx);

	auto regIterator = registry.begin();

	while (regIterator != registry.end()) {

		std::string domain = regIterator->first;

		auto local_pos = local_domains.find(domain);
		if (local_pos == local_domains.end()) {
			//a non local one that I need

			domain_set_t *domain_set = regIterator->second;
			std::string any = get_name_string("any");

			//get the entry for "any"
			auto any_pos = domain_set->find(any);

			if (any_pos != domain_set->end()) {
				UNLOCK_MUTEX(registryMtx);

				ps_registry_entry_t *reg_entry = any_pos->second;

				//check the skip count
				if (reg_entry->entry.skip_count <= 0) {
					strncpy(sync.domain, domain.c_str(),
							REGISTRY_DOMAIN_LENGTH);
					sync.domain[REGISTRY_DOMAIN_LENGTH] = '\0';

					sync.entry_count = count_entries_in_domain(domain);
					sync.domain_owner = false;

					the_broker().transmit_packet(REGISTRY_SYNC_PACKET, &sync,
							get_sync_packet_length(sync));

					PS_DEBUG("reg: send sync: %s, count %i", domain.c_str(),
							sync.entry_count);
					SLEEP_MS(500);
				} else {
					reg_entry->entry.skip_count--;
				}

				LOCK_MUTEX(registryMtx);
			} else {
				PS_ERROR("reg: sync %s/any not found", domain.c_str());
			}

		}

		regIterator++;
	}

	UNLOCK_MUTEX(registryMtx);
#endif
	return PS_OK;
}

void ps_registry::message_handler(ps_packet_source_t packet_source,
		ps_packet_type_t packet_type, const void *msg, int length) {

	PS_TRACE("registry: message_handler");

	if (packet_source != SOURCE) {
		switch (packet_type) {
		case REGISTRY_SYNC_PACKET:
			//receive registry sync packet
			//pass to thread for checking
			registryQueue->copy_3message_parts_to_q(&packet_source,
					sizeof(ps_packet_source_t), &packet_type,
					sizeof(ps_packet_type_t), msg, length);
			break;
		case REGISTRY_UPDATE_PACKET:
			//receive registry update
		{
			bool new_reg_entry = false;

			ps_registry_update_packet_t *pkt =
					(ps_registry_update_packet_t*) msg;
			if (length >= get_update_packet_length(*pkt)) {
				PS_DEBUG("reg: rx update: %s/%s", pkt->domain, pkt->name);

				std::set<ps_observer_class*> _observers;

				domain_set_t *domain_set;
				std::string domain = get_domain_string(pkt->domain);
				std::string name = get_name_string(pkt->name);

				LOCK_MUTEX(registryMtx);

				auto domain_pos = registry.find(domain); //get the domain set
				if (domain_pos == registry.end()) {
					//new domain
					//not wanted
					UNLOCK_MUTEX(registryMtx);
					PS_DEBUG("reg: rx update. %s unknown", pkt->domain);
					return;
				} else {
					domain_set = domain_pos->second;
				}

				auto pos = domain_set->find(name);
				if (pos == domain_set->end()) {
					//new name entry
#ifdef REGISTRY_LOCAL_ONLY
					PS_DEBUG("reg: rx update. %s unknown", pkt->name);
					UNLOCK_MUTEX(registryMtx);
					return;
#else
					ps_registry_entry_t *new_entry = new_registry_entry(
							pkt->value.datatype);

					if (new_entry == nullptr) {
						PS_ERROR("reg: rx update: %s/%s : no memory",
								pkt->domain, pkt->name);
						UNLOCK_MUTEX(registryMtx);
						return;
					}

					new_reg_entry = true;
					new_entry->entry.flags = pkt->value.flags;
					new_entry->entry.source = pkt->value.source;
					try {
						domain_set->insert(std::make_pair(name, new_entry));
					} catch (std::exception &e) {
						UNLOCK_MUTEX(registryMtx);
						PS_ERROR("reg: exception: %s - %s", e.what(),
								typeid (e).name());
						return;
					}
					pos = domain_set->find(name);
#endif
				}
				ps_registry_entry_t *reg_entry = pos->second;

				//copy value if type matches
				bool dataCopied = false;

				if ((reg_entry->entry.serial < pkt->value.serial)
						|| new_reg_entry) {
					dataCopied = copy_data_in(reg_entry->entry, pkt->value);
					reg_entry->entry.serial = pkt->value.serial;
				}

				if (reg_entry->observers) {
					for (auto obs : *reg_entry->observers) {
						_observers.insert(obs);
					}
				}

				UNLOCK_MUTEX(registryMtx);

				if (dataCopied || new_reg_entry) {
					//notify observers
					for (auto obs : _observers) {
						(obs->observer_callback)(pkt->domain, pkt->name,
								obs->arg);
					}

					notify_domain_observers(pkt->domain, pkt->name);
				} else {
					PS_ERROR("reg: No data copied! %s/%s", pkt->domain,
							pkt->name)
				}
			}
		}
			break;
		default:
			break;
		}
	}
}

//convenience struct for thread
typedef struct {
	ps_packet_source_t packet_source;
	ps_packet_type_t packet_type;
	union {
		uint8_t msg;
		ps_registry_sync_packet_t sync_pkt;
	};
} reg_sync_packet_t;

//thread for sync operations
void ps_registry::registry_thread_method() {

	PS_DEBUG("reg: Registry Thread Started");

	try {

		while (1) {
			int length;

			reg_sync_packet_t *packet =
					(reg_sync_packet_t*) registryQueue->get_next_message(
							REGISTRY_SYNC_INTERVAL * 1000, &length);

			if (!packet
					|| (packet && (packet->packet_type == REGISTRY_SYNC_REQUEST))) {
				//send sync messages
				PS_TRACE("reg: sync request");
				ps_registry_send_sync();
			}

			if (packet) {
				int payloadLength = length - sizeof(ps_packet_source_t)
						- sizeof(ps_packet_type_t);

				switch (packet->packet_type) {
				case REGISTRY_SYNC_PACKET: {
					if (payloadLength
							>= get_sync_packet_length(packet->sync_pkt)) {
						PS_TRACE("reg: rx sync: %s. count %i", packet->sync_pkt.domain, packet->sync_pkt.entry_count);

						std::string domain = get_domain_string(
								packet->sync_pkt.domain);

						//look up to see if I own this domain
						auto local_pos = local_domains.find(domain);
						if (local_pos == local_domains.end()) {
#ifndef REGISTRY_LOCAL_ONLY
							//not locally owned
							//do I want it?
							//I do if someone is observing it
							auto registry_pos = registry.find(domain);
							if (registry_pos == registry.end()) {
								//not in registry. no observers. don't want.
								PS_TRACE("reg: rx sync: not wanted");
							} else {
								//I have an observer
								if (packet->sync_pkt.domain_owner != 0) {
									//from the domain owner - a definitive count
									int count = count_entries_in_domain(domain);
									if (count != packet->sync_pkt.entry_count) {
										PS_TRACE("reg: rx sync: my count %i. send sync", count);
										//I don't have it all - report what I have to cause a re-send
										packet->sync_pkt.entry_count = count;
										packet->sync_pkt.domain_owner = 0;
										the_broker().transmit_packet(
												REGISTRY_SYNC_PACKET,
												&packet->sync_pkt,
												payloadLength);
									} else {
										//count good
										//we're done here
										domain_set_t *domain_set =
												registry_pos->second;
										std::string any = get_name_string(
												"any");
										auto pos = domain_set->find(any);
										if (pos != domain_set->end()) {
											ps_registry_entry_t *reg_entry =
													pos->second;
											reg_entry->entry.skip_count =
													SYNC_SKIP_COUNT;
										}
										//TODO: remember it's done - for now
										PS_TRACE("reg: rx sync: count OK");
									}
								} else {
									//not from the domain owner. from some other node. count may be wrong. don't care
									PS_TRACE("reg: rx sync: not domain owner");
								}
							}
#endif
						} else {
							//local domain - one of mine
							int count = count_entries_in_domain(domain);
							if (count != packet->sync_pkt.entry_count) {
								PS_TRACE("reg: rx sync: my count %i. send all", count);
								//He doesn't have them all
								resend_all_updates(domain);
								//send sync count again
								packet->sync_pkt.entry_count = count;
								packet->sync_pkt.domain_owner = 1;
								the_broker().transmit_packet(
										REGISTRY_SYNC_PACKET, &packet->sync_pkt,
										sizeof(ps_registry_sync_packet_t));
							}
						}
					}
				}
					break;
				default:
					break;
				}

				registryQueue->done_with_message(packet);
			}
		}
	} catch (std::exception &e) {
		PS_ERROR("reg: thread exception: %s", e.what());
	}
}

//resend all the entries in a domain

void ps_registry::resend_all_updates(std::string domain) {

	PS_DEBUG("reg: resend_all %s", domain.c_str());

	ps_registry_update_packet_t resendUpdateMessage;

	LOCK_MUTEX(registryMtx);

	auto regIterator = registry.find(domain);

	if (regIterator != registry.end()) {
		domain_set_t *domain_set = regIterator->second;

		auto domainIterator = domain_set->begin();

		while (domainIterator != domain_set->end()) {

			std::string name = domainIterator->first;

			ps_registry_entry_t *reg_entry = domainIterator->second;

			int size_of_update_packet = copy_update_packet(resendUpdateMessage,
					reg_entry->entry, domain.c_str(), name.c_str());

			int src = reg_entry->entry.source;
			int flags = reg_entry->entry.flags;

			UNLOCK_MUTEX(registryMtx);

			if ((src == SOURCE) && ((flags & PS_REGISTRY_LOCAL) == 0)) {

				PS_DEBUG("reg: resend: %s/%s (%s)", resendUpdateMessage.domain,
						resendUpdateMessage.name,
						typeNames[resendUpdateMessage.value.datatype]);
				the_broker().transmit_packet(REGISTRY_UPDATE_PACKET,
						&resendUpdateMessage, size_of_update_packet);
				SLEEP_MS(250); //pace it
			}

			LOCK_MUTEX(registryMtx);

			domainIterator++;
		}
	}
	UNLOCK_MUTEX(registryMtx);
}

ps_result_enum ps_registry::save_registry_method(const char *domain, void *arg,
		void (*write_line_callback)(void *arg, const char *linebuffer)) {
	char line_buffer[100];

	LOCK_MUTEX(registryMtx);

	for (auto domain_pos : registry) {
		if ((domain_pos.first == std::string(domain)) || (domain == NULL)) {
			for (auto name_pos : *domain_pos.second) {

				ps_registry_entry_t *reg_entry = name_pos.second;

				switch (reg_entry->entry.datatype) {
				case PS_REGISTRY_TEXT_TYPE:
					snprintf(line_buffer, 100, "text %s %s %s\n",
							domain_pos.first.c_str(), name_pos.first.c_str(),
							reg_entry->entry.string_value);
					PS_DEBUG("text %s %s %s", domain_pos.first.c_str(),
							name_pos.first.c_str(),
							reg_entry->entry.string_value)
					;
					break;
				case PS_REGISTRY_INT_TYPE:
					snprintf(line_buffer, 100, "int %s %s %i\n",
							domain_pos.first.c_str(), name_pos.first.c_str(),
							reg_entry->entry.int_value);
					PS_DEBUG("int %s %s %i", domain_pos.first.c_str(),
							name_pos.first.c_str(), reg_entry->entry.int_value)
					;
					break;
				case PS_REGISTRY_REAL_TYPE:
					snprintf(line_buffer, 100, "real %s %s %f\n",
							domain_pos.first.c_str(), name_pos.first.c_str(),
							reg_entry->entry.real_value);
					PS_DEBUG("real %s %s %f", domain_pos.first.c_str(),
							name_pos.first.c_str(), reg_entry->entry.real_value)
					;
					break;
				case PS_REGISTRY_BOOL_TYPE:
					snprintf(line_buffer, 100, "text %s %s %i\n",
							domain_pos.first.c_str(), name_pos.first.c_str(),
							reg_entry->entry.bool_value);
					PS_DEBUG("text %s %s %i", domain_pos.first.c_str(),
							name_pos.first.c_str(), reg_entry->entry.bool_value)
					;
					break;
				case PS_REGISTRY_SETTING_TYPE:
					snprintf(line_buffer, 100, "setting %s %s %f %f %f\n",
							domain_pos.first.c_str(), name_pos.first.c_str(),
							reg_entry->entry.setting->minimum,
							reg_entry->entry.setting->maximum,
							reg_entry->entry.setting->value);
					PS_DEBUG("real %s %s %f %f %f", domain_pos.first.c_str(),
							name_pos.first.c_str(),
							reg_entry->entry.setting->minimum,
							reg_entry->entry.setting->maximum,
							reg_entry->entry.setting->value)
					;
					break;
				default:
					break;
				}

				(*write_line_callback)(arg, line_buffer);
			}
		}
	}

	UNLOCK_MUTEX(registryMtx);
	return PS_OK;
}

//////////////////////C API

//////////////Adding new entries

//just add with no data

ps_result_enum ps_registry_add_new(const char *domain, const char *name,
		ps_registry_datatype_t type, ps_registry_flags_t flags) {
	ps_result_enum result;
	try {
		result = the_registry().add_new_registry_entry(domain, name, type,
				flags);
	} catch (std::exception &e) {
		PS_ERROR("reg: ps_registry_add_new: %s - %s", e.what(),
				typeid (e).name());
		UNLOCK_MUTEX(the_registry().registryMtx);
		return PS_EXCEPTION;
	}
	return result;
}
//add and set data

ps_result_enum ps_registry_set_new(const char *domain, const char *name,
		const ps_registry_api_struct &data) {
	ps_result_enum result;
	try {
		result = the_registry().set_new_registry_entry(domain, name, data);
	} catch (std::exception &e) {
		PS_ERROR("reg: ps_registry_set_new: %s - %s", e.what(),
				typeid (e).name());
		UNLOCK_MUTEX(the_registry().registryMtx);
		return PS_EXCEPTION;
	}
	return result;
}

//get metadata

ps_registry_datatype_t ps_registry_get_type(const char *domain,
		const char *name) {
	return the_registry().get_registry_type(domain, name);
}

ps_registry_flags_t ps_registry_get_flags(const char *domain,
		const char *name) {
	return the_registry().get_registry_flags(domain, name);
}

/////////////Changing values of existing entries

ps_result_enum ps_registry_set(const char *domain, const char *name,
		const ps_registry_api_struct &data) {
	ps_result_enum result;
	try {
		result = the_registry().set_registry_entry(domain, name, data);
	} catch (std::exception &e) {
		PS_ERROR("reg: ps_registry_set: %s - %s", e.what(), typeid (e).name());
		UNLOCK_MUTEX(the_registry().registryMtx);
		return PS_EXCEPTION;
	}

	return result;
}

ps_result_enum ps_registry_set_text(const char *domain, const char *name,
		const char *string_value) {
	ps_registry_api_struct data;
	data.datatype = PS_REGISTRY_TEXT_TYPE;
	strncpy(data.string_value, string_value, REGISTRY_TEXT_LENGTH);
	data.string_value[REGISTRY_TEXT_LENGTH - 1] = '\0';
	return ps_registry_set(domain, name, data);
}

ps_result_enum ps_registry_set_string(const char *domain, const char *name,
		std::string string_value) {
	ps_registry_api_struct data;
	data.datatype = PS_REGISTRY_TEXT_TYPE;
	strncpy(data.string_value, string_value.c_str(), REGISTRY_TEXT_LENGTH);
	data.string_value[REGISTRY_TEXT_LENGTH - 1] = '\0';
	return ps_registry_set(domain, name, data);
}

ps_result_enum ps_registry_set_int(const char *domain, const char *name,
		int _value) {
	ps_registry_api_struct data;
	data.datatype = PS_REGISTRY_INT_TYPE;
	data.int_value = _value;
	return ps_registry_set(domain, name, data);
}

ps_result_enum ps_registry_set_real(const char *domain, const char *name,
		float _value) {
	ps_registry_api_struct data;
	data.datatype = PS_REGISTRY_REAL_TYPE;
	data.real_value = _value;
	return ps_registry_set(domain, name, data);
}

ps_result_enum ps_registry_set_bool(const char *domain, const char *name,
		bool _value) {
	ps_registry_api_struct data;
	data.datatype = PS_REGISTRY_BOOL_TYPE;
	data.bool_value = _value;
	return ps_registry_set(domain, name, data);
}

////////////Getting current values

ps_result_enum ps_registry_get(const char *domain, const char *name,
		ps_registry_api_struct &data) {
	return the_registry().get_registry_entry(domain, name, data);
}

ps_result_enum ps_registry_get_text(const char *domain, const char *name,
		char *buff, int len) {
	ps_registry_api_struct value;
	ps_result_enum reply = ps_registry_get(domain, name, value);

	if (reply == PS_OK) {
		if (value.datatype == PS_REGISTRY_TEXT_TYPE) {
			strncpy(buff, value.string_value, len);
			return PS_OK;
		} else {
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_get_string(const char *domain, const char *name,
		std::string *_value) {
	ps_registry_api_struct value;
	ps_result_enum reply = ps_registry_get(domain, name, value);

	if (reply == PS_OK) {
		if (value.datatype == PS_REGISTRY_TEXT_TYPE) {
			*_value = std::string(value.string_value);
			return PS_OK;
		} else {
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_get_int(const char *domain, const char *name,
		int *buff) {
	ps_registry_api_struct value;
	ps_result_enum reply = ps_registry_get(domain, name, value);

	if (reply == PS_OK) {
		if (value.datatype == PS_REGISTRY_INT_TYPE) {
			*buff = value.int_value;
			return PS_OK;
		} else {
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_get_real(const char *domain, const char *name,
		float *buff) {
	ps_registry_api_struct value;
	ps_result_enum reply = ps_registry_get(domain, name, value);

	if (reply == PS_OK) {
		if (value.datatype == PS_REGISTRY_REAL_TYPE) {
			*buff = value.real_value;
			return PS_OK;
		} else {
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_get_bool(const char *domain, const char *name,
		bool *buff) {
	ps_registry_api_struct value;
	ps_result_enum reply = ps_registry_get(domain, name, value);

	if (reply == PS_OK) {
		if (value.datatype == PS_REGISTRY_BOOL_TYPE) {
			*buff = value.bool_value;
			return PS_OK;
		} else {
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_set_observer(const char *domain, const char *name,
		ps_registry_callback_t *callback, void *arg) {
	return the_registry().set_observer(domain, name, callback, arg);
}

ps_result_enum ps_registry_iterate_domain(const char *domain,
		ps_registry_callback_t *callback, void *arg) {
	return the_registry().interate_domain(domain, callback, arg);
}

ps_result_enum ps_registry_send_sync() {
	return the_registry().ps_registry_request_sync();
}

ps_result_enum ps_reset_registry() {
	return the_registry().reset_registry();
}
