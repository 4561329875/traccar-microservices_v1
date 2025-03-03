// metrics_logger.h
#ifndef METRICS_LOGGER_H
#define METRICS_LOGGER_H

#include <fstream>
#include <string>
#include <mutex>

class MetricsLogger {
private:
    static MetricsLogger* instance;
    static std::mutex mutex;
    std::ofstream file;
    bool headerWritten;

    MetricsLogger();
    ~MetricsLogger();

public:
    static MetricsLogger* getInstance();
    void logMetrics(const std::string& name, double timeSeconds, double timeMs,
                    long memoryKb, long peakMemoryKb, double cpuPercent);
    void close();
    static void cleanup() {
        if (instance) {
            instance->close();
            delete instance;
            instance = nullptr;
        }
    }
};

#endif
