#include <cpu.h>
#include <sys/utsname.h>
#include <fstream>
#include <json.hpp>
#include <set>

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
    int freq = 0;

    if (!(file >> freq) || freq <= 0) {
        cpu_max_frequency_ = 0;
        return;
    }

    cpu_max_frequency_ = freq;
}

void CPUInfo::getCPUCores() {
    std::ifstream file("/proc/cpuinfo");
    std::set<std::string> cores;

    std::string line;
    std::string physId, coreId;

    while (std::getline(file, line)) {
        if (line.empty()) {
            // koniec bloku jednego procesora
            if (!physId.empty() && !coreId.empty())
                cores.insert(physId + ":" + coreId);

            physId.clear();
            coreId.clear();
            continue;
        }

        if (line.rfind("physical id", 0) == 0)
            physId = line.substr(line.find(':') + 1);
        else if (line.rfind("core id", 0) == 0)
            coreId = line.substr(line.find(':') + 1);
    }

    if (!physId.empty() && !coreId.empty())
        cores.insert(physId + ":" + coreId);

    cpu_cores_ = cores.empty() ? -1 : static_cast<int>(cores.size());
}

void CPUInfo::getCPUThreads() {
    cpu_threads_ = sysconf(_SC_NPROCESSORS_ONLN);
}
