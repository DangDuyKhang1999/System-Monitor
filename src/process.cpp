#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pID) : m_PID(pID) { m_ProcessCpuInfo = CpuUtilization(); }
// TODO: Return this process's ID
int Process::Pid() { return m_PID; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
  long sys_uptime = LinuxParser::UpTime();

  string value;
  string line;
  std::vector<std::string> stat;
  std::ifstream stream(LinuxParser::kProcDirectory + "/" +
                       to_string(this->m_PID) + LinuxParser::kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      for (uint i = 0; i < 22 && linestream >> value; i++) {
        stat.emplace_back(value);
      }
    }
  }

  long utime = std::stol(stat[13]);
  long stime = std::stol(stat[14]);
  long cutime = std::stol(stat[15]);
  long cstime = std::stol(stat[16]);
  long starttime = std::stol(stat[21]);
  long total_time = utime + stime + cutime + cstime;
  long seconds = sys_uptime - (starttime / sysconf(_SC_CLK_TCK));

  return (1.0 * (total_time / sysconf(_SC_CLK_TCK)) / seconds);
}

// eturn the command that generated this process
string Process::Command() {
  string command = LinuxParser::Command(m_PID);
  size_t maxLength = 50;

  if (command.length() > maxLength) {
    command = command.substr(0, maxLength - 3) + "...";
  }

  return command;
}


// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(m_PID); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(m_PID); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(m_PID); }

bool Process::operator<(Process const& a) const {
  Process& p1 = const_cast<Process&>(*this);
  Process& p2 = const_cast<Process&>(a);
  return p1.CpuUtilization() < p2.CpuUtilization();
}