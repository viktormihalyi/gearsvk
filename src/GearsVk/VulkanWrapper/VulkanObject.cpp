#include "VulkanObject.hpp"

#include <set>


static std::set<VulkanObject*> objects;


VulkanObject::VulkanObject ()
{
    objects.insert (this);
}

VulkanObject::~VulkanObject ()
{
    objects.erase (this);
}
