#ifndef VULKANWRAPPER_COMMAND_HPP
#define VULKANWRAPPER_COMMAND_HPP

#include "VulkanWrapperExport.hpp"

#include <string>

namespace GVK {

class CommandBuffer;

class VULKANWRAPPER_DLL_EXPORT Command {
private:
    std::string name;

public:
    virtual ~Command () = default;

    void SetName (const std::string& value) { name = value; }

    virtual void        Record (CommandBuffer&)             = 0;
    virtual bool        IsEquivalent (const Command& other) = 0;
    virtual std::string ToString () const { return ""; }
};

} // namespace GVK

#endif