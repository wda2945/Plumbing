//
//  can defines.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef can_defines_hpp
#define can_defines_hpp

/* CAN TX message buffer. */

/* Create a CMSGSID data type. */
typedef struct
{
    unsigned SID:11;
    unsigned :21;
}txcmsgsid;

/* Create a CMSGEID data type. */
typedef struct
{
    unsigned DLC:4;
    unsigned RB0:1;
    unsigned :3;
    unsigned RB1:1;
    unsigned RTR:1;
    unsigned EID:18;
    unsigned IDE:1;
    unsigned SRR:1;
    unsigned :2;
}txcmsgeid;

/* Create a CMSGDATA0 data type. */
typedef struct
{
    unsigned Byte0:8;
    unsigned Byte1:8;
    unsigned Byte2:8;
    unsigned Byte3:8;
}txcmsgdata0;

/* Create a CMSGDATA1 data type. */
typedef struct
{
    unsigned Byte4:8;
    unsigned Byte5:8;
    unsigned Byte6:8;
    unsigned Byte7:8;
}txcmsgdata1;

/* This is the main data structure. */
typedef union uCANTxMessageBuffer {
struct {
        txcmsgsid CMSGSID;
        txcmsgeid CMSGEID;
        txcmsgdata0 CMSGDATA0;
        txcmsgdata0 CMSGDATA1;
};
    int messageWord[4];
}CANTxMessageBuffer;

/* CAN RX Message Buffer */

/* Create a CMSGSID data type. */
typedef struct
{
    unsigned SID:11;
    unsigned FILHIT:5;
    unsigned CMSGTS:16;
}rxcmsgsid;

/* Create a CMSGEID data type. */
typedef struct
{
    unsigned DLC:4;
    unsigned RB0:1;
    unsigned :3;
    unsigned RB1:1;
    unsigned RTR:1;
    unsigned EID:18;
    unsigned IDE:1;
    unsigned SRR:1;
    unsigned :2;
}rxcmsgeid;

/* Create a CMSGDATA0 data type. */
typedef struct
{
    unsigned Byte0:8;
    unsigned Byte1:8;
    unsigned Byte2:8;
    unsigned Byte3:8;
}rxcmsgdata0;

/* Create a CMSGDATA1 data type. */ typedef struct
{
    unsigned Byte4:8;
    unsigned Byte5:8;
    unsigned Byte6:8;
    unsigned Byte7:8;
}rxcmsgdata1;

/* This is the main data structure. */
typedef union uCANRxMessageBuffer {
struct
    {
        rxcmsgsid CMSGSID;
        rxcmsgeid CMSGEID;
        rxcmsgdata0 CMSGDATA0;
        rxcmsgdata0 CMSGDATA1;
};
    int messageWord[4];
}CANRxMessageBuffer;

#endif /* can_defines_hpp */
