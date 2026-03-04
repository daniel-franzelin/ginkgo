#ifndef SIMPLE_LOGGER_HPP
#define SIMPLE_LOGGER_HPP

#include <fstream>
#include <string>
#include <mutex>
#include <sstream>

class SimpleLogger {
public:
    // Constructor: Richtet Pfad und Datei ein
    SimpleLogger(const std::string& prefix = "");
    
    // Destructor: Schließt die Datei
    ~SimpleLogger();
    
    // Logging-Methoden
    void log(const std::string& message);
    void log(const std::string& message, const std::string& algorithm_phase);
    
    // Optional: Aktiviere/Deaktiviere Thread-ID Logging
    void set_log_thread_id(bool enable) { log_thread_id_ = enable; }
    
private:
    // Hilfsfunktionen
    std::string setup_log_path();
    uint64_t get_current_time_us() const;
    std::string get_thread_identifier() const;
    bool file_exists(const std::string& path) const;
    void create_directories(const std::string& path);
    
    // Member-Variablen
    std::ofstream log_file_;
    std::string prefix_;
    bool log_thread_id_;
    std::mutex write_mutex_;  // Thread-Safety
};

#endif // SIMPLE_LOGGER_HPP
