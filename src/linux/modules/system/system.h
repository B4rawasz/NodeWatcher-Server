#ifndef SYSTEM_H
#define SYSTEM_H

#include <event_bus.h>
#include <light_module.h>
#include <static_resource.h>
#include <json.hpp>
#include <string>

class SystemInfo : public IStaticResource, public ILightModule {
public:
    SystemInfo(EventBus& eventBus, std::chrono::milliseconds period);
    message::MessageVariantOUT getStaticData() override;
    void collect() override;
    std::chrono::milliseconds period() override;

private:
    // Static system information retrieval methods
    void getHostname();
    void getSystemName();
    void getVersionID();
    void getKernelVersion();
    void getTimezone();

    // Dynamic system information retrieval methods
    std::string getUptime();
    std::string getTime();

    std::chrono::milliseconds period_;
    EventBus& eventBus_;

    std::string hostname_;
    std::string system_name_;
    std::string version_id_;
    std::string kernel_version_;
    std::string timezone_;
};

#endif  // SYSTEM_H