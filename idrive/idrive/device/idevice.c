//
//  idevice.c
//  idrive
//
//  Created by Richter Brzeski on 8/15/18.
//  Copyright © 2018 Richter Brzeski. All rights reserved.
//

#include "idevice.h"


static int retrieve_device_name(lockdownd_client_t client, char **name)
{
    lockdownd_error_t error;
    
    if (client == NULL)
        return -1;
    
    error = lockdownd_get_device_name(client, name);
    
    if (error < 0 || *name == NULL)
        return -1;
    
    return 0;
}


IDRIVE_API int retrieve_available_devices(struct idevice_handle ***devices)
{
    idevice_error_t id_error;
    lockdownd_error_t ld_error;
    
    char **udids = NULL;
    int dcount = 0;
    
    id_error = idevice_get_device_list(&udids, &dcount);
    
    if (id_error == IDEVICE_E_NO_DEVICE || udids == NULL || dcount == 0) {
        return -1;
    }
    
    struct idevice_handle **devs = malloc(sizeof(struct idevice_handle *) * dcount);
    
    if (devs == NULL)
        return -1;
    
    for (int i = 0; i < dcount; i++) {
        devs[i] = malloc(sizeof(struct idevice_handle));
        
        if (devs[i] == NULL)
            return -1;
        
        devs[i]->handle = NULL;
        devs[i]->client = NULL;
        devs[i]->name = NULL;
        
        id_error = idevice_new(&devs[i]->handle, udids[i]);
        
        if (id_error < 0 || devs[i]->handle == NULL)
            return -1;
        
        ld_error = lockdownd_client_new(devs[i]->handle, &devs[i]->client, "com.idrive.core");
        
        if (ld_error < 0)
            return -1;
        
        if (devs[i]->client == NULL)
            return -1;
        
        
        int nr_error = retrieve_device_name(devs[i]->client, &devs[i]->name);

        if (nr_error < 0)
            return -1;

    }
    
    *devices = devs;
    
    return dcount;
}

