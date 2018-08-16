//
//  main.c
//  idrive
//
//  Created by Richter Brzeski on 8/15/18.
//  Copyright © 2018 Richter Brzeski. All rights reserved.
//

#include <stdio.h>
#include "idevice.h"

int main(int argc, const char * argv[]) {
    struct idevice_handle **devs = NULL;
    int aDs = retrieve_available_devices(&devs);
    printf(devs[0]->name);
    return 0;
}
