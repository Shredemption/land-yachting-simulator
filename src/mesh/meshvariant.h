#pragma once

#include <variant>

#include "mesh/mesh.hpp"
#include "mesh/mesh_defs.h"

using MeshVariant = std::variant<
    Mesh<VertexAnimated>,
    Mesh<VertexSimple>,
    Mesh<VertexTextured>>;