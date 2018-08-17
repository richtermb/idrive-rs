//
//  idevice.h
//  idrive
//
//  Created by Richter Brzeski on 8/15/18.
//  Copyright © 2018 Richter Brzeski. All rights reserved.
//

#ifndef idevice_h
#define idevice_h

#include <stdio.h>
#include <stdlib.h>

#include <libimobiledevice/afc.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

#ifndef IDRIVE_API
#define IDRIVE_API
#endif

struct idevice_handle {
    idevice_t handle;
    lockdownd_client_t lockdownd_client;
    afc_client_t afc_client;
    char *name;
};


/*
 * retrieves iOS devices that are fit to use for storage.
 * returns number of available devices. -1 on failure.
 */
IDRIVE_API int retrieve_available_devices(struct idevice_handle ***devices);


#endif /* idevice_h */
