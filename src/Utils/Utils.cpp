#include "Utils.hpp"

namespace Utils {

#ifdef PROJECT_ROOT_FULL_PATH
const std::filesystem::path PROJECT_ROOT (PROJECT_ROOT_FULL_PATH);
#else
#error "no project root path defined"
#endif

} // namespace Utils