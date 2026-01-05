#include <linux/limits.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <system.h>
#include <unistd.h>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include "event_bus.h"
#include "json.hpp"

SystemInfo::SystemInfo(EventBus& eventBus, std::chrono::milliseconds period)
    : eventBus_(eventBus), period_(period) {
    getHostname();
    getSystemName();
    getVersionID();
    getKernelVersion();
    getTimezone();
}

message::MessageVariantOUT SystemInfo::getStaticData() {
    message::SystemInfoStatic info(hostname_, system_name_, version_id_, kernel_version_,
                                   timezone_);
    return info;
}

void SystemInfo::collect() {
    message::SystemInfo info(getUptime(), getTime());
    eventBus_.publish(info);
}

std::chrono::milliseconds SystemInfo::period() {
    return period_;
}

void SystemInfo::getHostname() {
    char name[256];
    gethostname(name, sizeof(name));
    hostname_ = std::string(name);
}

void SystemInfo::getSystemName() {
    std::ifstream f("/etc/os-release");
    std::string line;
    while (std::getline(f, line)) {
        if (line.find("NAME=") == 0)
            system_name_ = line.substr(6, line.length() - 7);  // Remove quotes
    }
}

void SystemInfo::getVersionID() {
    std::ifstream f("/etc/os-release");
    std::string line;
    while (std::getline(f, line)) {
        if (line.find("VERSION_ID=") == 0)
            version_id_ = line.substr(12, line.length() - 13);  // Remove quotes
    }
}

void SystemInfo::getKernelVersion() {
    utsname u{};
    uname(&u);
    kernel_version_ = std::string(u.release);
}

void SystemInfo::getTimezone() {
    char buf[PATH_MAX];
    ssize_t len = readlink("/etc/localtime", buf, sizeof(buf) - 1);
    buf[len] = '\0';

    std::string tz = "";
    std::string path(buf);
    auto pos = path.find("zoneinfo/");
    if (pos != std::string::npos)
        tz = path.substr(pos + 9);

    tzset();
    std::time_t now = std::time(nullptr);
    std::string abbr = tzname[std::localtime(&now)->tm_isdst];

    std::time_t t = std::time(nullptr);
    std::tm l = *std::localtime(&t);
    std::tm g = *std::gmtime(&t);

    int off = (l.tm_hour - g.tm_hour) * 3600 + (l.tm_min - g.tm_min) * 60 +
              (l.tm_yday != g.tm_yday ? (l.tm_yday > g.tm_yday ? 86400 : -86400) : 0);

    std::ostringstream os;
    os << "Time zone: " << tz << " (" << abbr << ", " << (off >= 0 ? "+" : "-")
       << std::setw(2) << std::setfill('0') << std::abs(off) / 3600 << std::setw(2)
       << (std::abs(off) % 3600) / 60 << ")";

    timezone_ = os.str();
}

std::string SystemInfo::getUptime() {
    struct sysinfo sys{};
    sysinfo(&sys);
    long seconds = sys.uptime;

    long h = seconds / 3600;
    long m = (seconds % 3600) / 60;
    long s = seconds % 60;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << h << ":" << std::setw(2) << m << ":"
        << std::setw(2) << s;

    return oss.str();
}

std::string SystemInfo::getTime() {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    std::tm tm{};
    localtime_r(&ts.tv_sec, &tm);

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << tm.tm_hour << ":" << std::setw(2)
        << tm.tm_min << ":" << std::setw(2) << tm.tm_sec;

    return oss.str();
}
