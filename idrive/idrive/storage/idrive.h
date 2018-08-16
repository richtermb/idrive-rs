//
//  idrive.h
//  idrive
//
//  Created by Richter Brzeski on 8/15/18.
//  Copyright © 2018 Richter Brzeski. All rights reserved.
//

#ifndef idrive_h
#define idrive_h

#include <stdio.h>

#include "idevice.h"

#ifndef IDRIVE_API
#define IDRIVE_API
#endif

#define INITIAL_OPERATIONS_SIZE 10

enum OPERATION_TYPE {
    WRITE = 0,
    READ,
    STEALTH_MAP
};

enum OPERATION_STATE {
    UNSTARTED = 0,
    IN_PROGRESS,
    FINISHED
};


struct idrive_operation {
    enum OPERATION_TYPE optype;
    /* Pointer to file to save */
    FILE *fp;
    /* Size of file in bytes */
    int len;
    /* Bytes written */
    int *written;
    /* Op state */
    enum OPERATION_STATE state;
};


struct idrive_handle {
    struct idevice_handle *idh;
    
    /* Available storage space in bytes */
    uint64_t bytes;
    
    /* Mutable list of operations */
    struct idrive_operation **operations;
    
    /* maintains an operation count */
    int opcount;
    
    /* maximum items operations can currently store */
    int opsize;
};


IDRIVE_API int idrive_init(struct idevice_handle *dev, struct idrive_handle **handle);
IDRIVE_API int idrive_add_operation(struct idrive_handle *handle, struct idrive_operation operation);


#endif /* idrive_h */
