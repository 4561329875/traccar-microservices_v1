// metrics_logger.cpp
#include "metrics_logger.h"

MetricsLogger* MetricsLogger::instance = nullptr;
std::mutex MetricsLogger::mutex;
int writeCount = 0;
const int FLUSH_THRESHOLD = 10;


MetricsLogger::MetricsLogger() {
    file.open("metrics.csv", std::ios::app);

    // Verificar si el archivo está vacío para escribir la cabecera
    headerWritten = (file.tellp() > 0);
    if (!headerWritten && file.is_open()) {
        file << "Nombre,Tiempo (ms)\n";
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

void MetricsLogger::logMetrics(const std::string& name, double timeMs) {
    std::lock_guard<std::mutex> lock(mutex);
    if (file.is_open()) {
        file << name << ","
             << timeMs <<  "\n";
        //file.flush(); // Asegurarse de que los datos se escriban en el archivo

        writeCount++;
            if (writeCount >= FLUSH_THRESHOLD) {
                file.flush();
                writeCount = 0;
            }
    }
}

void MetricsLogger::close() {
    std::lock_guard<std::mutex> lock(mutex);
    if (file.is_open()) {
        file.close();
    }
}
