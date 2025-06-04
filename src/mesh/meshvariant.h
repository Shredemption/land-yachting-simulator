#ifndef MESHVARIANT_H
#define MESHVARIANT_H

#include <variant>

#include "mesh/mesh.hpp"
#include "mesh/mesh_defs.h"

using MeshVariant = std::variant<
    Mesh<VertexAnimated>,
    Mesh<VertexSimple>,
    Mesh<VertexTextured>>;

#endif