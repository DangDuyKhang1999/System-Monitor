#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

template <typename T>
T findValueByKey(const std::string &keyFilter, const std::string &filename) {
  std::string line;
  std::string key;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == keyFilter) {
          return value;
        }
      }
    }
  }

  return value;
}

template <typename T>
T getValueOfFile(const std::string &filename) {
  T value;
  std::string line;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
  }

  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR *directory = opendir(kProcDirectory.c_str());
  struct dirent *file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  const std::string memTotal = "MemTotal:";
  const std::string memFree = "MemFree:";
  float total = findValueByKey<float>(kMeminfoFilename, memTotal);
  float free = findValueByKey<float>(kMeminfoFilename, memFree);
  return (total - free) / total;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  long uptimeInSeconds = 0;

  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    filestream >> uptimeInSeconds;
  }

  return uptimeInSeconds;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  string line;
  string cpuLabel;
  long user;
  long nice;
  long system;
  long idle;
  long iowait;
  long irq;
  long softirq;
  long steal;
  long guest;
  long guest_nice;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> cpuLabel;
      if (cpuLabel == kFilterCpu) {
        linestream >> user >> nice >> system >> idle >> iowait >> irq >>
            softirq >> steal >> guest >> guest_nice;
        break;  // We only need the values for the overall CPU
      }
    }
  }

  // Calculate the total jiffies
  long totalJiffies = user + nice + system + idle + iowait + irq + softirq +
                      steal + guest + guest_nice;

  return totalJiffies;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
// long LinuxParser::ActiveJiffies(int pid [[maybe_unused]]) { return 0; }

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::vector<std::string> cpuStats = LinuxParser::CpuUtilization();
  long activeJiffies = std::stol(cpuStats[LinuxParser::kUser_]) +
                       std::stol(cpuStats[LinuxParser::kNice_]) +
                       std::stol(cpuStats[LinuxParser::kSystem_]) +
                       std::stol(cpuStats[LinuxParser::kIRQ_]) +
                       std::stol(cpuStats[LinuxParser::kSoftIRQ_]) +
                       std::stol(cpuStats[LinuxParser::kSteal_]);

  return activeJiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  std::vector<std::string> cpuStats = CpuUtilization();

  long idleJiffies = std::stol(cpuStats[LinuxParser::kIdle_]) +
                     std::stol(cpuStats[LinuxParser::kIOwait_]);
  return idleJiffies;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  std::vector<std::string> jiffies;
  std::ifstream stream("/proc/stat");
  if (stream.is_open()) {
    std::string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    std::string key;
    linestream >> key;
    if (key == kFilterCpu) {
      while (linestream >> key) {
        jiffies.push_back(key);
      }
    }
  }
  return jiffies;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::string line;
    while (std::getline(stream, line)) {
      if (line.find(kFilterProcesses) == 0) {
        std::istringstream linestream(line);
        std::string key;
        int value;
        linestream >> key >> value;
        return value;
      }
    }
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  int runningProcesses = 0;
  string line;
  string key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == kFilterRunningProcesses) {
        linestream >> runningProcesses;
        break;
      }
    }
  }
  return runningProcesses;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) +
                       kCmdlineFilename);
  if (stream.is_open() && getline(stream, line)) {
    return line;
  }
  return string();
}
// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line;
  string key;
  string value;
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      if (linestream >> key >> value && key == "VmData:") {
        return to_string(stol(value) / 1000);
      }
    }
  }
  return string();
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string value;
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream linestream(line);
      if (linestream >> key >> value && key == "Uid:") {
        return value;
      }
    }
  }
  return string();
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  auto uid = LinuxParser::Uid(pid);
  std::string firstToken;
  std::string line;
  std::ifstream stream(kPasswordPath);

  if (stream.is_open())
  {
    while (getline(stream, line))
    {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::string secondToken, thirdToken, fourthToken;
      std::istringstream linestream(line);
      linestream >> firstToken >> secondToken >> thirdToken >> fourthToken;
      if (thirdToken == uid && fourthToken == uid)
        break;
    }
  }
  return firstToken;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  long sys_uptime = LinuxParser::UpTime();

  string value;
  string line;
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    if (getline(stream, line)) {
      std::istringstream linestream(line);
      for (int i = 0; i < 22 && linestream >> value; i++) {
        if (i == 21) {
          long process_starttime = stol(value);
          return sys_uptime - process_starttime / sysconf(_SC_CLK_TCK);
        }
      }
    }
  }
  // Return a default value if reading fails
  return 0;
}

