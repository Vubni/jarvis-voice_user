#ifndef INSTALLED_PROGRAMS_H
#define INSTALLED_PROGRAMS_H

#include <nlohmann/json.hpp>
#include <string>

namespace InstalledPrograms {
    nlohmann::json GetInstalledPrograms(bool filter=true);
}

#endif // INSTALLED_PROGRAMS_H