//
//  psRegistry.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <string.h>
#include <string>
#include <stdio.h>
#include "ps_registry_linux.hpp"

#include "queue/ps_queue_linux.hpp"

ps_registry& the_registry() {
    static ps_registry_linux the_registry;
    return the_registry;
}

ps_registry_linux::ps_registry_linux()
{
	//queue for sync messages
	registryQueue = new ps_queue_linux(MAX_PS_PACKET, 0);

    if (!registryQueue)
    {
        PS_ERROR("registry: registry Queue FAIL!");
    }
	//start broker thread

    registry_thread = new std::thread([this](){registry_thread_method();});
}

ps_result_enum load_registry(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == 0)
        return PS_NAME_NOT_FOUND;
    
    char *buff = NULL;
    size_t buffsize = 0;
    
    PS_DEBUG("reg: pre-loading from %s", path);
    
	while (getline(&buff, &buffsize, fp) > 0)
    {
        char value_type[20] {""};
        sscanf(buff, "%19s", value_type);
        
        char domain[100];
        char name[100];
        
        if (strcmp(value_type, "setting") == 0)
        {
            ps_registry_api_struct val;

            sscanf(buff, "%19s %99s %99s %f %f %f", value_type, domain, name,
            		&val.setting.minimum, &val.setting.maximum, &val.setting.value);

            val.datatype = PS_REGISTRY_INT_TYPE;
            val.flags = PS_REGISTRY_ANY_WRITE;
            
            ps_registry_set_new(domain, name, val);
            
            PS_DEBUG(">> int %s %s %f %f %f ", domain, name,  val.setting.minimum, val.setting.maximum, val.setting.value);
        }
        else if (strcmp(value_type, "bool") == 0)
        {
            int v;
            
            sscanf(buff, "%19s %99s %99s %i", value_type, domain, name, &v);
            
            ps_registry_add_new(domain, name, PS_REGISTRY_BOOL_TYPE, PS_REGISTRY_ANY_WRITE);
            
            ps_registry_set_bool(domain, name, v);
            
            PS_DEBUG(">> bool %s %s %i", domain, name, v);
        }
        if (strcmp(value_type, "real") == 0)
        {
            float v;
            ps_registry_flags_t flags;

            sscanf(buff, "%19s %c %99s %99s %f", value_type, &flags, domain, name, &v);

            ps_registry_add_new(domain, name, PS_REGISTRY_REAL_TYPE, flags);
            
            ps_registry_set_real(domain, name,  v);

            PS_DEBUG(">> real %s %s %f", domain, name, v);
        }
        else if (strcmp(value_type, "int") == 0)
        {
            int v;
            ps_registry_flags_t flags;

            sscanf(buff, "%19s %c %99s %99s %i", value_type, &flags, domain, name, &v);

            ps_registry_add_new(domain, name, PS_REGISTRY_INT_TYPE, flags);

            ps_registry_set_int(domain, name,  v);
            
            PS_DEBUG(">> int %s %s %i", domain, name, v);
        }
        else if (strcmp(value_type, "bool") == 0)
        {
            int v;
            ps_registry_flags_t flags;

            sscanf(buff, "%19s %c %99s %99s %i", value_type, &flags, domain, name, &v);
            
            ps_registry_add_new(domain, name, PS_REGISTRY_BOOL_TYPE, flags);
            
            ps_registry_set_bool(domain, name, v);
            
            PS_DEBUG(">> bool %s %s %i", domain, name, v);
        }
        else if (strcmp(value_type, "text") == 0)
        {
            char value[100];
            ps_registry_flags_t flags;

            sscanf(buff, "%19s %c %99s %99s %99s", value_type, &flags, domain, name, value);
            
            ps_registry_add_new(domain, name, PS_REGISTRY_TEXT_TYPE, flags);
            ps_registry_set_text(domain, name, value);
            
            PS_DEBUG(">> text %s %s %s", domain, name, value);
        }
        //else ignore
    }
    
    fclose(fp);
    return PS_OK;
}

void write_line(void *arg, const char *linebuffer)
{
    
}

ps_result_enum save_registry(const char *path, const char *domain)
{
    FILE *fp;
    
    fp = fopen(path, "w");
    if (fp == 0)
        return PS_NAME_NOT_FOUND;
    
    PS_DEBUG("reg: saving <%s> to %s", domain, path);
    
    the_registry().save_registry_method(domain, fp, &write_line);
    
    fclose(fp);
    
    return PS_OK;
}
