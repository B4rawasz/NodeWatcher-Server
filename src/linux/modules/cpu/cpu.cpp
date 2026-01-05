#include <cpu.h>
#include <statgrab.h>
#include <sys/utsname.h>
#include <fstream>
#include <json.hpp>
#include <set>
#include <vector>

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
    message::CpuInfo cpu_info;
    double load1, load5, load15;
    getCPULoadAvg(load1, load5, load15);
    cpu_info.cpu_load_avg_1min = load1;
    cpu_info.cpu_load_avg_5min = load5;
    cpu_info.cpu_load_avg_15min = load15;
    cpu_info.cpu_usage = getCpuUsage();
    cpu_info.per_core_usage = getPerCoreUsage();
    cpu_info.cpu_frequency = getCPUFrequency();
    eventBus_.publish(cpu_info);
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

void CPUInfo::getCPULoadAvg(double& load1, double& load5, double& load15) {
    double load[3];
    if (getloadavg(load, 3) != 3) {
        load1 = 0.0;
        load5 = 0.0;
        load15 = 0.0;
        return;
    }

    load1 = load[0];
    load5 = load[1];
    load15 = load[2];
}

double CPUInfo::getCpuUsage() {
    size_t count = 0;
    sg_cpu_stats* cpu = sg_get_cpu_stats(&count);

    if (!cpu || count == 0)
        return 0.0;

    return 100.0 - cpu[0].idle - cpu[0].iowait;
}

std::vector<double> CPUInfo::getPerCoreUsage() {
    size_t count = 0;
    sg_cpu_stats* cpu = sg_get_cpu_stats(&count);
    std::vector<double> usages;

    if (!cpu || count == 0)
        return usages;

    for (size_t i = 1; i < count; ++i) {
        double usage = 100.0 - cpu[i].idle - cpu[i].iowait;
        usages.push_back(usage);
    }

    return usages;
}

int CPUInfo::getCPUFrequency() {
    long long sum = 0;
    int count = 0;

    for (int cpu = 0;; ++cpu) {
        std::ifstream f("/sys/devices/system/cpu/cpu" + std::to_string(cpu) +
                        "/cpufreq/scaling_cur_freq");

        if (!f.is_open())
            break;

        long khz = 0;
        f >> khz;

        if (khz > 0) {
            sum += khz;
            ++count;
        }
    }

    if (count == 0)
        return 0;

    return static_cast<int>(sum / count);
}