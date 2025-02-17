#pragma once
#include "Vertex.h"
#include "Technique.h"
#include "Graphics.h"
#include <filesystem>
#include <vector>

struct aiMaterial;
struct aiMesh;

namespace Bind
{
	class VertexBuffer;
	class IndexBuffer;
}

class Material
{
public:
	Material( Graphics& gfx, const aiMaterial& material, const std::filesystem::path& path ) noexcept(!IS_DEBUG);
	VertexMeta::VertexBuffer ExtractVertices( const aiMesh& mesh ) const noexcept;
	std::vector<unsigned short> ExtractIndices( const aiMesh& mesh ) const noexcept;
	std::shared_ptr<Bind::VertexBuffer> MakeVertexBindable( Graphics& gfx, const aiMesh& mesh, float scale = 1.0f ) const noexcept(!IS_DEBUG);
	std::shared_ptr<Bind::IndexBuffer> MakeIndexBindable( Graphics& gfx, const aiMesh& mesh ) const noexcept(!IS_DEBUG);
	std::vector<Technique> GetTechniques() const noexcept;
private:
	std::string MakeMeshTag( const aiMesh& mesh ) const noexcept;
private:
	VertexMeta::VertexLayout layout;
	std::vector<Technique> techniques;
	std::string modelPath;
	std::string name;
};