#include <cpu.h>
#include "json.hpp"

CPUInfo::CPUInfo(EventBus& eventBus, std::chrono::milliseconds period)
    : eventBus_(eventBus), period_(period) {
    getCPUModel();
    getCPUArchitecture();
    getCPUMaxFrequency();
    getCPUCores();
    getCPUThreads();
}

message::MessageVariantOUT CPUInfo::getStaticData() {
    message::CpuInfoStatic cpu_info_static(cpu_model_, cpu_architecture_,
                                           cpu_max_frequency_, cpu_cores_, cpu_threads_);
    return cpu_info_static;
}

void CPUInfo::collect() {
    // Implementation for collecting dynamic CPU data
}

std::chrono::milliseconds CPUInfo::period() {
    return period_;
}

void CPUInfo::getCPUModel() {
    // Implementation to retrieve CPU model
}

void CPUInfo::getCPUArchitecture() {
    // Implementation to retrieve CPU architecture
}

void CPUInfo::getCPUMaxFrequency() {
    // Implementation to retrieve CPU max frequency
}

void CPUInfo::getCPUCores() {
    // Implementation to retrieve number of CPU cores
}

void CPUInfo::getCPUThreads() {
    // Implementation to retrieve number of CPU threads
}
