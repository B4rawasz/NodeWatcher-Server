#ifndef STATIC_RESOURCE_H
#define STATIC_RESOURCE_H

#include <json.hpp>

class IStaticResource {
public:
    virtual ~IStaticResource();
    virtual message::MessageVariantOUT getStaticData() = 0;
};

#endif  // STATIC_RESOURCE_H