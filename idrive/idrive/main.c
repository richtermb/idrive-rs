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
    struct idevice_handle **devices = NULL;
    int device_count = retrieve_available_devices(&devices);
    printf("device_count: %d\n", device_count);
    
    if (device_count <= 0) {
        exit(1);
    }
    
    struct idrive_handle *idrive = NULL;
    int idx = -1;
    
    for (int i = 0; i < device_count; i++) {
        if (strcmp(devices[i]->name, "IDRV_001") == 0)
            idx = i;
    }
    
    int res = idrive_init(devices[idx], &idrive);
    
    struct idrive_operation operation1 = {
        .fp = fopen("/Users/richter/Downloads/monkaS.pdf", "rb"),
        .lpath = "/Users/richter/Downloads/monkaS.pdf",
        .len = 40626,
        .key = "monkaS_essay.pdf",
        .optype = WRITE,
        .state = UNSTARTED,
        .written = 0,
        .read = 0
    };
    
    struct idrive_operation operation2 = {
        .fp = fopen("/Users/richter/Downloads/monkaS_PDF.pdf", "wb+"),
        .lpath = "/Users/richter/Downloads/monkaS_PDF.pdf",
        .len = 40626,
        .key = "monkaS_essay.pdf",
        .optype = READ,
        .state = UNSTARTED,
        .written = 0,
        .read = 0
    };
    
    idrive_add_operation(idrive, &operation1);
    idrive_add_operation(idrive, &operation2);

    DBG_PRINT_OPERATIONS(idrive);
    idrive_process_operation(idrive);
    DBG_PRINT_OPERATIONS(idrive);
    idrive_process_operation(idrive);
    DBG_PRINT_OPERATIONS(idrive);
    
    return 0;
}
