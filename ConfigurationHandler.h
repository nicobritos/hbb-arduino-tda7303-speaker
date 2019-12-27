#ifndef _CONFIGURATION_HANDLER_H
    #define _CONFIGURATION_HANDLER_H

    #include "defines.h"

    void resetEEPROM();
    void saveSettings(actualConfigType *actualConfig);
    void restoreConfig(actualConfigType *actualConfig);

#endif
