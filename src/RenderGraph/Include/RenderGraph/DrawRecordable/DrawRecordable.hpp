#ifndef DRAWRECORDABLE_HPP
#define DRAWRECORDABLE_HPP

namespace GVK {
class CommandBuffer;
}


namespace RG {

class DrawRecordable {
public:
    virtual ~DrawRecordable () = default;

    virtual void Record (GVK::CommandBuffer&) const = 0;
};

} // namespace RG

#endif