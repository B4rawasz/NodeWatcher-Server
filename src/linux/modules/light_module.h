#ifndef LIGHT_MODULE_H
#define LIGHT_MODULE_H

#include <chrono>

class ILightModule {
public:
    virtual ~ILightModule();

    virtual void collect() = 0;

    virtual std::chrono::milliseconds period() = 0;
};

#endif  // LIGHT_MODULE_H