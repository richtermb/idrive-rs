//
//  main.c
//  idrive
//
//  Created by Richter Brzeski on 8/15/18.
//  Copyright © 2018 Richter Brzeski. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "idrive.h"

int main(int argc, const char * argv[]) {
    struct idevice_handle **devs = NULL;
    int available_device_count = retrieve_available_devices(&devs);
    
    struct idrive_handle *hdl = NULL;
    
    int idx = -1;
    
    for (int i = 0; i < available_device_count; i++) {
        if (strcmp(devs[i]->name, "IDRV_001") == 0)
            idx = i;
        
    }
    
    int res = idrive_init(devs[idx], &hdl);
    
    if (res == 0)
        printf("rem_space: %llu", hdl->bytes);
    
    return 0;
}
