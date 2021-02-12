#include "VulkanObject.hpp"

#include <set>

namespace GVK {

static std::set<VulkanObject*> objects;


VulkanObject::VulkanObject ()
{
    objects.insert (this);
}

VulkanObject::~VulkanObject ()
{
    objects.erase (this);
}

}