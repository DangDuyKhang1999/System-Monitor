#include "format.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using std::string;
// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long times) {
  std::chrono::seconds seconds{times};

  // return std::chrono::format("%T", seconds); // in C++20 :-) (thank you)

  std::chrono::hours hours =
      std::chrono::duration_cast<std::chrono::hours>(seconds);

  seconds -= std::chrono::duration_cast<std::chrono::seconds>(hours);

  std::chrono::minutes minutes =
      std::chrono::duration_cast<std::chrono::minutes>(seconds);

  seconds -= std::chrono::duration_cast<std::chrono::seconds>(minutes);

  std::stringstream ss{};

  ss << std::setw(2) << std::setfill('0') << hours.count()  // HH
     << std::setw(1) << ":" << std::setw(2) << std::setfill('0')
     << minutes.count()  // HH
     << std::setw(1) << ":" << std::setw(2) << std::setfill('0')
     << seconds.count();  // HH

  return ss.str();
}
