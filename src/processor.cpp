#include "processor.h"
#include "linux_parser.h"

Processor::Processor() :
    m_prevIdle(LinuxParser::IdleJiffies()),
    m_prevNonIdle(LinuxParser::ActiveJiffies()),
    m_prevTotal(m_prevIdle + m_prevNonIdle) {}
// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
    // Get the current CPU utilization data
    std::vector<std::string> currentStats = LinuxParser::CpuUtilization();

    // Calculate the differences between current and previous CPU utilization
    long currentIdle = std::stol(currentStats[LinuxParser::kIdle_]) + std::stol(currentStats[LinuxParser::kIOwait_]);
    long currentNonIdle = std::stol(currentStats[LinuxParser::kUser_]) + std::stol(currentStats[LinuxParser::kNice_]) +
                          std::stol(currentStats[LinuxParser::kSystem_]) + std::stol(currentStats[LinuxParser::kIRQ_]) +
                          std::stol(currentStats[LinuxParser::kSoftIRQ_]) + std::stol(currentStats[LinuxParser::kSteal_]);
    long currentTotal = currentIdle + currentNonIdle;

    long deltaTotal = currentTotal - m_prevTotal;
    long deltaIdle = currentIdle - m_prevIdle;

    // Update the previous values for the next iteration
    m_prevTotal = currentTotal;
    m_prevIdle = currentIdle;

    // Calculate CPU utilization percentage
    float cpuUsage = (deltaTotal == 0) ? 0.0f : static_cast<float>(deltaTotal - deltaIdle) / static_cast<float>(deltaTotal);

    return cpuUsage;
}