//
// Created by joy on 18-1-16.
//

#ifndef UVCCAMERA_MASTER_PROPERTY_H
#define UVCCAMERA_MASTER_PROPERTY_H

#define PROP_NAME_MAX   32
#define PROP_VALUE_MAX  92
#define PROPERTY_KEY_MAX   PROP_NAME_MAX
#define PROPERTY_VALUE_MAX  PROP_VALUE_MAX
#define SYSTEM_PROPERTY_PIPE_NAME       "/tmp/android-sysprop"

# define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)	      \
		      + strlen ((ptr)->sun_path))

enum {
    kSystemPropertyUnknown = 0,
    kSystemPropertyGet,
    kSystemPropertySet,
    kSystemPropertyList
};

#endif //UVCCAMERA_MASTER_PROPERTY_H