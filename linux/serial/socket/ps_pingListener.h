//
//  ps_pingListener.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_pingListener_h
#define ps_pingListener_h

#ifdef __cplusplus
extern "C" {
#endif

    typedef void (PingCallback_t)(char *robot, char *ip_address, void *args);
    
    void pingListener(PingCallback_t *callback, void *args);

#ifdef __cplusplus
}
#endif

#endif /* ps_pingListener_h */
