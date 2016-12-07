#include <hdf5.h>
#include <hdf5_hl.h>
#include <iostream>
#include "attributes.h"

Value readTable(hid_t link, const char* name);
