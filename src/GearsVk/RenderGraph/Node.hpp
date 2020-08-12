#ifndef RG_NODE_HPP
#define RG_NODE_HPP

#include "Assert.hpp"
#include "Noncopyable.hpp"
#include "UUID.hpp"
#include <vector>

namespace RG {

USING_PTR (Node);
class GEARSVK_API Node : public Noncopyable {
private:
    GearsVk::UUID uuid;

    template<typename ToType, typename FromType>
    static std::vector<ToType*> CastAll (const std::vector<FromType*>& container)
    {
        std::vector<ToType*> result;
        for (auto elem : container) {
            if (ToType* casted = dynamic_cast<ToType*> (elem)) {
                result.push_back (casted);
            } else {
                ASSERT (true);
            }
        }
        return result;
    }

public:
    std::vector<Node*> pointingHere;
    std::vector<Node*> pointingTo;


    void AddConnectionTo (Node& to)
    {
        pointingTo.push_back (&to);
        to.pointingHere.push_back (this);
    }

    template<typename T>
    std::vector<T*> GetPointingHere () const
    {
        return CastAll<T> (pointingHere);
    }

    template<typename T>
    std::vector<T*> GetPointingTo () const
    {
        return CastAll<T> (pointingTo);
    }

    bool HasPointingHere () const
    {
        return !pointingHere.empty ();
    }

    bool HasPointingTo () const
    {
        return !pointingTo.empty ();
    }

    GearsVk::UUID GetUUID () const
    {
        return uuid;
    }
};

} // namespace RG

#endif