#include <cpu.h>
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
    double load1, load5, load15;
    getCPULoadAvg(load1, load5, load15);
    message::CpuInfo cpu_info(load1, load5, load15, getCpuUsage(), getPerCoreUsage(),
                              getCPUFrequency());
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
    if (!cpu_initialized_) {
        previous_total_times_ = readCpuTimes();
        cpu_initialized_ = true;
        return 0.0;
    }

    CpuTimes current_total_times = readCpuTimes();
    double usage = calcCpuUsage(previous_total_times_, current_total_times);
    previous_total_times_ = current_total_times;
    return usage;
}

std::vector<double> CPUInfo::getPerCoreUsage() {
    if (!per_core_initialized_) {
        int cpu_count = cpu_cores_ > 0 ? cpu_cores_ : sysconf(_SC_NPROCESSORS_ONLN);
        previous_per_core_times_.resize(cpu_count);
        for (int i = 0; i < cpu_count; ++i) {
            previous_per_core_times_[i] = readCpuTimes(i);
        }
        per_core_initialized_ = true;
        return std::vector<double>(cpu_count, 0.0);
    }
    int cpu_count = previous_per_core_times_.size();
    std::vector<double> usages(cpu_count);
    for (int i = 0; i < cpu_count; ++i) {
        CpuTimes current_times = readCpuTimes(i);
        usages[i] = calcCpuUsage(previous_per_core_times_[i], current_times);
        previous_per_core_times_[i] = current_times;
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

CpuTimes CPUInfo::readCpuTimes(int core) {
    std::ifstream f("/proc/stat");
    std::string line;

    while (std::getline(f, line)) {
        if ((core == -1 && line.starts_with("cpu ")) ||
            (core >= 0 && line.starts_with("cpu" + std::to_string(core)))) {
            CpuTimes t{};
            std::stringstream ss(line);
            std::string cpu;
            ss >> cpu >> t.user >> t.nice >> t.system >> t.idle >> t.iowait >> t.irq >>
                t.softirq >> t.steal;
            return t;
        }
    }
    return {};
}

double CPUInfo::calcCpuUsage(const CpuTimes& a, const CpuTimes& b) {
    long long idleA = a.idle + a.iowait;
    long long idleB = b.idle + b.iowait;

    long long totalA = idleA + a.user + a.nice + a.system + a.irq + a.softirq + a.steal;
    long long totalB = idleB + b.user + b.nice + b.system + b.irq + b.softirq + b.steal;

    return 100.0 * (1.0 - (double)(idleB - idleA) / (totalB - totalA));
}