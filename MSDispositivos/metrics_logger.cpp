// metrics_logger.cpp
#include "metrics_logger.h"

MetricsLogger* MetricsLogger::instance = nullptr;
std::mutex MetricsLogger::mutex;

MetricsLogger::MetricsLogger() {
    file.open("metrics.csv", std::ios::app);

    // Verificar si el archivo está vacío para escribir la cabecera
    headerWritten = (file.tellp() > 0);
    if (!headerWritten && file.is_open()) {
        file << "Nombre,Tiempo (s),Tiempo (ms),Memoria Dif (KB),Pico Memoria (KB),Uso CPU (%)\n";
        headerWritten = true;
    }
}

MetricsLogger::~MetricsLogger() {
    if (file.is_open()) {
        file.close();
    }
}

MetricsLogger* MetricsLogger::getInstance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new MetricsLogger();
    }
    return instance;
}

void MetricsLogger::logMetrics(const std::string& name, double timeSeconds, double timeMs,
                              long memoryKb, long peakMemoryKb, double cpuPercent) {
    std::lock_guard<std::mutex> lock(mutex);
    if (file.is_open()) {
        file << name << ","
             << timeSeconds << ","
             << timeMs << ","
             << memoryKb << ","
             << peakMemoryKb << ","
             << cpuPercent << "\n";
        file.flush(); // Asegurarse de que los datos se escriban en el archivo
    }
}

void MetricsLogger::close() {
    std::lock_guard<std::mutex> lock(mutex);
    if (file.is_open()) {
        file.close();
    }
}
