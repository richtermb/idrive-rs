//
//  idrive.c
//  idrive
//
//  Created by Richter Brzeski on 8/15/18.
//  Copyright © 2018 Richter Brzeski. All rights reserved.
//

#include "idrive.h"


char *IDRVCMPLTKEY(char *k)
{
    char tmp[255] = IDRVBASE;
    strcat(tmp, k);
    char *res = malloc(sizeof(char) * 255);
    strcpy(res, tmp);
    return res;
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
    if (handle->opcount == 0) {
        printf("No operations found.");
        return;
    }
    
    for (int i = 0; i < handle->opcount; i++) {
        printf("{\n");
        printf("\t\"operationType\": \"%s\",\n", DBG_OPTYPE_TO_STR(handle->operations[i]->optype));
        printf("\t\"operationState\": \"%s\"\n", DBG_OPSTATE_TO_STR(handle->operations[i]->state));
        printf("\t\"localPath\": \"%s\"\n", handle->operations[i]->lpath);
        printf("\t\"key\": \"%s\"\n", handle->operations[i]->key);
        printf("\t\"size\": %u\n", handle->operations[i]->len);
        printf("}\n");
    }
}


static int idrive_get_remaining_space(struct idrive_handle *handle)
{
    plist_t node = NULL;
    
    lockdownd_error_t error;
    
    if (handle->idh->lockdownd_client == NULL) {
        printf("No lockdownd clients activate for device.\n");
        return -1;
    }
    
    error = lockdownd_get_value(handle->idh->lockdownd_client, "com.apple.disk_usage", "TotalDataAvailable", &node);
    
    if (error != 0 || node == NULL) {
        printf("Error getting disk usage: %d\n", error);
        return -1;
    }
    
    plist_get_uint_val(node, &handle->bytes);
    
    return 0;
}


IDRIVE_API static int idrive_write(struct idrive_handle *handle, struct idrive_operation *operation)
{
    if (operation->optype == READ || operation->optype == STEALTH_MAP)
        return -1;
    
    afc_error_t error;
    uint64_t fd;
    operation->state = IN_PROGRESS;
    
    /* Creates a new file on the device */
    error = afc_file_open(handle->idh->afc_client, IDRVCMPLTKEY(operation->key), AFC_FOPEN_WR, &fd);
    
    if (error != AFC_E_SUCCESS) {
        printf("AFC Error: %d\n", error);
        return -1;
    }
    
    char *data = malloc(operation->len);
    
    if (data == NULL)
        return -1;
    
    size_t bytes_read = fread(data, sizeof(char), operation->len, operation->fp);
    
    if (bytes_read != operation->len)
        return -1;
    
    error = afc_file_write(handle->idh->afc_client, fd, data, operation->len, &operation->written);
    
    if (error != AFC_E_SUCCESS)
        return -1;

    return 0;
}


IDRIVE_API static int idrive_read(struct idrive_handle *handle, struct idrive_operation *operation)
{
    if (operation->optype == WRITE || operation->optype == STEALTH_MAP)
        return -1;
    
    afc_error_t error;
    uint64_t fd;
    operation->state = IN_PROGRESS;
    
    /* Creates a new file on the device */
    error = afc_file_open(handle->idh->afc_client, IDRVCMPLTKEY(operation->key), AFC_FOPEN_RDONLY, &fd);
    
    if (error != AFC_E_SUCCESS) {
        printf("AFC Error: %d\n", error);
        return -1;
    }
    
    char *data = malloc(operation->len);
    
    if (data == NULL)
        return -1;
    
    error = afc_file_read(handle->idh->afc_client, fd, data, operation->len, &operation->read);
    if (operation->read == (uint32_t)operation->len)
        return -1;

    /* write bytes out to file */
    size_t bytes_written = fwrite(data, sizeof(char), operation->len, operation->fp);
    
    if (bytes_written != operation->len )
        return -1;
    
    if (error != AFC_E_SUCCESS)
        return -1;
    
    return 0;
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


IDRIVE_API int idrive_process_operation(struct idrive_handle *handle)
{
    if (handle->opcount == 0 || handle->operations[0] == NULL) {
        printf("No operations queued.");
        return -1;
    }
    
    int status = 0;
    
    switch (handle->operations[0]->optype) {
        case WRITE:
            idrive_write(handle, handle->operations[0]);
            break;
        case READ:
            idrive_read(handle, handle->operations[0]);
            break;
        default:
            return -1;
            break;
    }
    
    if (status != 0) {
        printf("Error performing operation.\n");
        return -1;
    }
    
    /* Rotate operation list to the left by 1 */
    handle->operations[0] = NULL;
    
    for (int i = 1; i < handle->opcount; i++) {
        handle->operations[i - 1] = handle->operations[i];
    }
    
    handle->operations[handle->opcount - 1] = NULL;
    handle->opcount--;
    
    return 0;
}
