#include <ginkgo/core/log/xbat_logger.hpp>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <sstream>
#include <iomanip>
#include <sched.h> // Required for sched_getcpu

SimpleLogger::SimpleLogger(const std::string& prefix)
    : prefix_(prefix), log_thread_id_(true)
{
    std::string log_path = setup_log_path();
    
    log_file_.open(log_path, std::ios::out | std::ios::app);
    
    if (!log_file_.is_open()) {
        std::cerr << "FEHLER: Konnte Log-Datei nicht öffnen: " 
                  << log_path << std::endl;
        throw std::runtime_error("Failed to open log file");
    }
    
    std::cout << "Logger initialisiert. Logge nach: " << log_path << std::endl;
    
    // CSV Header schreiben (nur wenn Datei leer ist)
    log_file_.seekp(0, std::ios::end);
    if (log_file_.tellp() == 0) {
        log_file_ << "timestamp_us,thread_id,algorithm_phase,message" << std::endl;
    }
}

SimpleLogger::~SimpleLogger()
{
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

std::string SimpleLogger::setup_log_path()
{
    const char* env_home = std::getenv("HOME");
    const char* env_job_id = std::getenv("SLURM_JOB_ID");
    
    std::string log_dir_base;
    std::string log_dir_full;
    std::string log_filename;
    
    if (env_home) {
        std::string home(env_home);
        log_dir_base = home + "/.xbat";
        log_dir_full = log_dir_base + "/timestamps";
    } else {
        log_dir_full = ".";
        std::cerr << "Warnung: HOME Umgebungsvariable nicht gesetzt. "
                  << "Nutze aktuelles Verzeichnis." << std::endl;
    }
    
    if (env_job_id) {
        log_filename = std::string(env_job_id) + ".csv";
    } else {
        log_filename = "manual_run.csv";
    }
    
    // Verzeichnisse erstellen
    if (env_home) {
        create_directories(log_dir_base);
        create_directories(log_dir_full);
    }
    
    return log_dir_full + "/" + log_filename;
}

void SimpleLogger::create_directories(const std::string& path)
{
    if (!file_exists(path)) {
        if (mkdir(path.c_str(), 0777) != 0) {
            std::cerr << "Warnung: Konnte Verzeichnis nicht erstellen: " 
                      << path << std::endl;
        }
    }
}

bool SimpleLogger::file_exists(const std::string& path) const
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

uint64_t SimpleLogger::get_current_time_us() const
{
    using namespace std::chrono;
    auto now = system_clock::now();
    return duration_cast<microseconds>(now.time_since_epoch()).count();
}

std::string SimpleLogger::get_thread_identifier() const
{
    std::ostringstream oss;

    oss << "HW:" << sched_getcpu();
    
    return oss.str();
}

void SimpleLogger::log(const std::string& message)
{
    log(message, "");
}

void SimpleLogger::log(const std::string& message, 
                       const std::string& algorithm_phase)
{
    std::lock_guard<std::mutex> lock(write_mutex_);
    
    uint64_t timestamp = get_current_time_us();
    std::string thread_id = log_thread_id_ ? get_thread_identifier() : "-";
    std::string phase = algorithm_phase.empty() ? "-" : algorithm_phase;
    
    // CSV-Format: timestamp,thread_id,phase,message
    log_file_ << timestamp << ","
              << thread_id << ","
              << phase << ","
              << prefix_ << message << std::endl;
    
    // Flush für wichtige Events (kann bei Bedarf optimiert werden)
    log_file_.flush();
}
