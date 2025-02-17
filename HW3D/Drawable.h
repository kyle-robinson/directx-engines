#pragma once
#include "Graphics.h"
#include "Technique.h"
#include <memory>

class TechniqueProbe;
class Material;
struct aiMesh;

namespace Rgph
{
	class RenderGraph;
}

namespace Bind
{
	class IndexBuffer;
	class InputLayout;
	class Topology;
	class VertexBuffer;
}

class Drawable
{
public:
	Drawable() = default;
	Drawable( const Drawable& ) = delete;
	Drawable( Graphics& gfx, const Material& mat, const aiMesh& mesh, float scale = 1.0f ) noexcept;
	void AddTechnique( Technique tech_in ) noexcept;
	virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
	void Submit( size_t channelFilter ) const noexcept;
	void Bind( Graphics& gfx ) const noexcept(!IS_DEBUG);
	void Accept( TechniqueProbe& );
	UINT GetIndexCount() const noexcept(!IS_DEBUG);
	void LinkTechniques( Rgph::RenderGraph& );
	virtual ~Drawable();
protected:
	std::vector<Technique> techniques;
	std::shared_ptr<Bind::IndexBuffer> pIndices;
	std::shared_ptr<Bind::Topology> pTopology;
	std::shared_ptr<Bind::VertexBuffer> pVertices;
};