//
//  idrive.c
//  idrive
//
//  Created by Richter Brzeski on 8/15/18.
//  Copyright © 2018 Richter Brzeski. All rights reserved.
//

#include "idrive.h"


static int idrive_get_remaining_space(struct idrive_handle *handle)
{
    plist_t node = NULL;
    
    lockdownd_error_t error;
    
    if (handle->idh->client == NULL) {
        printf("No lockdownd clients activate for device.\n");
        return -1;
    }
    
    error = lockdownd_get_value(handle->idh->client, "com.apple.disk_usage", "TotalDataAvailable", &node);
    
    if (error != 0 || node == NULL) {
        printf("Error getting disk usage: %d\n", error);
        return -1;
    }
    
    plist_get_uint_val(node, &handle->bytes);
    
    return 0;
}


static const char *DBG_OPTYPE_TO_STR(enum OPERATION_TYPE optype)
{
    switch (optype) {
        case WRITE:
            return "WRITE";
            break;
        case READ:
            return "READ";
            break;
        default:
            return NULL;
            break;
    }
}


static const char *DBG_OPSTATE_TO_STR(enum OPERATION_STATE opstate)
{
    switch (opstate) {
        case UNSTARTED:
            return "UNSTARTED";
            break;
        case IN_PROGRESS:
            return "IN PROGRESS";
            break;
        case FINISHED:
            return "FINISHED";
            break;
        default:
            return NULL;
            break;
    }
}


IDRIVE_API void DBG_PRINT_OPERATIONS(struct idrive_handle *handle)
{
    for (int i = 0; i < handle->opcount; i++) {
        printf("{\n");
        printf("\t\"operationType\": \"%s\",\n", DBG_OPTYPE_TO_STR(handle->operations[i]->optype));
        printf("\t\"operationState\": \"%s\"\n", DBG_OPSTATE_TO_STR(handle->operations[i]->state));
        printf("}\n");
    }
}


IDRIVE_API int idrive_init(struct idevice_handle *dev, struct idrive_handle **handle)
{
    struct idrive_handle *new_drive = malloc(sizeof(struct idrive_handle));
    
    new_drive->idh = dev;
    new_drive->bytes = 0;

    new_drive->operations = malloc(sizeof(struct idrive_operation *) * INITIAL_OPERATIONS_SIZE);
    new_drive->opcount = 0;
    new_drive->opsize = INITIAL_OPERATIONS_SIZE;
    
    if (idrive_get_remaining_space(new_drive) < 0)
        return -1;
    
    if (new_drive->bytes == 0)
        return -1;

    *handle = new_drive;
    
    return 0;
}


IDRIVE_API int idrive_add_operation(struct idrive_handle *handle, struct idrive_operation operation)
{
    if (handle->opcount >= handle->opsize) {
        /* if the operations list is full, increase memory available to it by a factor of 2 */
        handle->operations = realloc(handle->operations, sizeof(struct idrive_operation) * handle->opcount * 2);
        
        if (handle->operations == NULL)
            return -1;
    }
    
    handle->operations[handle->opcount] = &operation;
    
    handle->opcount++;
    
    return 0;
}
