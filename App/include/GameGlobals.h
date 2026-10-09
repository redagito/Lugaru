#pragma once

// This header used to declare every file-scope global in the Game namespace.
// The last of them, the console's text buffer, moved into Console; the rest
// went to GameState or GameAssets. Nothing is declared here now.
//
// The includes stay. Other translation units relied on this header pulling them
// in, and removing them is a separate piece of work from the migration that
// emptied it.

#include <string>

#include "Environment/Skybox.hpp"
#include "Graphic/Texture.hpp"
#include "Graphic/Text.hpp"
#include "Graphic/Models.hpp"

namespace Game
{
}
