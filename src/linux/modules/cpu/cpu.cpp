#include <cpu.h>
#include <sys/utsname.h>
#include <fstream>
#include <json.hpp>

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
    std::ifstream file("/proc/cpuinfo");
    std::string line;

    cpu_model_ = "Unknown";

    while (std::getline(file, line)) {
        if (line.rfind("model name", 0) == 0) {
            cpu_model_ = line.substr(line.find(':') + 2);
        }
    }
}

void CPUInfo::getCPUArchitecture() {
    struct utsname u{};
    if (uname(&u) == 0)
        cpu_architecture_ = u.machine;  // np. x86_64, aarch64, armv7l
    else
        cpu_architecture_ = "unknown";
}

void CPUInfo::getCPUMaxFrequency() {
    std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    long freq_khz = 0;

    if (!(file >> freq_khz) || freq_khz <= 0) {
        cpu_max_frequency_ = "0.000";
        return;
    }

    double freq_mhz = freq_khz / 1000.0;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << freq_mhz;
    cpu_max_frequency_ = oss.str();
}

void CPUInfo::getCPUCores() {
    // Implementation to retrieve number of CPU cores
}

void CPUInfo::getCPUThreads() {
    // Implementation to retrieve number of CPU threads
}
