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

struct idrive_handle {
    struct idevice_handle *idh;
    /* Available storage space in bytes */
    int bytes;
    /* Maintains a count of objects stored */
    int object_count;
};


#endif /* idrive_h */
