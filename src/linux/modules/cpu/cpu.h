#ifndef CPU_H
#define CPU_H

#include <event_bus.h>
#include <light_module.h>
#include <static_resource.h>
#include <string>

struct CpuTimes {
    long long user, nice, system, idle, iowait, irq, softirq, steal;
};

class CPUInfo : public IStaticResource, public ILightModule {
public:
    CPUInfo(EventBus& eventBus, std::chrono::milliseconds period);
    message::MessageVariantOUT getStaticData() override;
    void collect() override;
    std::chrono::milliseconds period() override;

private:
    // Static system information retrieval methods
    void getCPUModel();
    void getCPUArchitecture();
    void getCPUMaxFrequency();
    void getCPUCores();
    void getCPUThreads();

    // Dynamic system information retrieval methods
    void getCPULoadAvg(double& load1, double& load5, double& load15);
    double getCpuUsage();
    std::vector<double> getPerCoreUsage();
    int getCPUFrequency();

    // Helpers
    CpuTimes readCpuTimes(int core = -1);
    double calcCpuUsage(const CpuTimes& a, const CpuTimes& b);

    EventBus& eventBus_;
    std::chrono::milliseconds period_;

    std::string cpu_model_;
    std::string cpu_architecture_;
    int cpu_max_frequency_;
    int cpu_cores_;
    int cpu_threads_;
    bool cpu_initialized_ = false;
    bool per_core_initialized_ = false;
    CpuTimes previous_total_times_;
    std::vector<CpuTimes> previous_per_core_times_;
};

#endif  // CPU_H