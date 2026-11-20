#ifndef __RELAY_MODULE_H_
#define __RELAY_MODULE_H_

#include "global.h"

#define RELAY_MODULE_MAX_COUNT 16
#define RELAY_MODULE_CHANNEL_COUNT 8

void downloadRelayModuleConfig(void);
void relayModuleService(void);

#endif
