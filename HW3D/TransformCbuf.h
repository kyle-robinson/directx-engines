#pragma once
#include "ConstantBuffers.h"
#include "Drawable.h"

namespace Bind
{
	class TransformCbuf : public CloningBindable
	{
	public:
		TransformCbuf( Graphics& gfx, UINT slot = 0u );
		void Bind( Graphics& gfx ) noexcept(!IS_DEBUG) override;
		void InitializeParentReference( const Drawable& parent ) noexcept override;
		std::unique_ptr<CloningBindable> Clone() const noexcept override;
	protected:
		struct Transforms
		{
			DirectX::XMMATRIX model;
			DirectX::XMMATRIX modelView;
			DirectX::XMMATRIX modelViewProj;
		};
		void UpdateBind( Graphics& gfx, const Transforms& tf ) noexcept;
		Transforms GetTransforms( Graphics& gfx ) noexcept;
	private:
		static std::unique_ptr<VertexConstantBuffer<Transforms>> pVcbuf;
		const Drawable* pParent = nullptr;
	};
}