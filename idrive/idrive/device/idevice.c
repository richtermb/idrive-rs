//
//  idevice.c
//  idrive
//
//  Created by Richter Brzeski on 8/15/18.
//  Copyright © 2018 Richter Brzeski. All rights reserved.
//

#include "idevice.h"


static int retrieve_device_name(lockdownd_client_t lockdownd_client, char **name)
{
    if (lockdownd_client == NULL)
        return -1;
    
    lockdownd_error_t error = lockdownd_get_device_name(lockdownd_client, name);
    if (error != LOCKDOWN_E_SUCCESS || *name == NULL)
        return -1;
    
    return 0;
}


IDRIVE_API int retrieve_available_devices(struct idevice_handle ***devices)
{
    idevice_error_t id_error;
    lockdownd_error_t ld_error;
    afc_error_t afc_error;
    
    char **udids = NULL;
    int dcount = 0;
    
    id_error = idevice_get_device_list(&udids, &dcount);
    
    if (id_error == IDEVICE_E_NO_DEVICE || udids == NULL || dcount == 0) {
        printf("No devices found. error: %d, dcount: %d", id_error, dcount);
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
        devs[i]->lockdownd_client = NULL;
        devs[i]->afc_client = NULL;
        devs[i]->name = NULL;
        
        id_error = idevice_new(&devs[i]->handle, udids[i]);
        if (id_error != IDEVICE_E_SUCCESS || devs[i]->handle == NULL) {
            printf("Error creating new device: %d\n", id_error);
            return -1;
        }
        
        ld_error = lockdownd_client_new_with_handshake(devs[i]->handle, &devs[i]->lockdownd_client, "com.idrive.core");
        if (ld_error != LOCKDOWN_E_SUCCESS) {
            printf("Error creating new lockdownd client: %d\n", ld_error);
            return -1;
        }
        
        if (devs[i]->lockdownd_client == NULL)
            return -1;
        
        lockdownd_service_descriptor_t service_descriptor = NULL;
        ld_error = lockdownd_start_service(devs[i]->lockdownd_client, "com.apple.afc", &service_descriptor);
        if (ld_error != LOCKDOWN_E_SUCCESS) {
            printf("Error creating new lockdownd service: %d\n", ld_error);
            return -1;
        }

        afc_error = afc_client_new(devs[i]->handle, service_descriptor, &devs[i]->afc_client);
        if (afc_error != AFC_E_SUCCESS || devs[i]->afc_client == NULL) {
            printf("Error creating new AFC client: %d\n", afc_error);
            return -1;
        }
        
        lockdownd_service_descriptor_free(service_descriptor);
        service_descriptor = NULL;
        int nr_error = retrieve_device_name(devs[i]->lockdownd_client, &devs[i]->name);
        if (nr_error != 0)
            return -1;

    }
    
    *devices = devs;
    return dcount;
}

