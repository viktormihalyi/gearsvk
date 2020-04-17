#include "core/Sequence.h"

#include <functional>

void InitializeEnvironment ();

void DestroyEnvironment ();

// not exported, only called from old api
void SetRenderGraphFromSequence (Sequence::P);

void StartRendering (const std::function<bool ()>&);
