#include "ShadowRasterizer.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace Bind
{
	ShadowRasterizer::ShadowRasterizer( Graphics& gfx, int depthBias, float slopeBias, float clamp )
	{
		ChangeDepthBiasParameters( gfx, depthBias, slopeBias, clamp );
	}

	void ShadowRasterizer::ChangeDepthBiasParameters( Graphics& gfx, int depthBias, float slopeBias, float clamp )
	{
		INFOMANAGER( gfx );

		this->depthBias = depthBias;
		this->slopeBias = slopeBias;
		this->clamp = clamp;

		D3D11_RASTERIZER_DESC rasterDesc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT{} );
		rasterDesc.DepthBias = depthBias;
		rasterDesc.SlopeScaledDepthBias = slopeBias;
		rasterDesc.DepthBiasClamp = clamp;

		GFX_THROW_INFO( GetDevice( gfx )->CreateRasterizerState( &rasterDesc, &pRasterizer ) );
	}

	void ShadowRasterizer::Bind( Graphics& gfx ) noexcept(!IS_DEBUG)
	{
		INFOMANAGER_NOHR( gfx );
		GFX_THROW_INFO_ONLY( GetContext( gfx )->RSSetState( pRasterizer.Get() ) );
	}

	int ShadowRasterizer::GetDepthBias() const
	{
		return depthBias;
	}

	float ShadowRasterizer::GetSlopeBias() const
	{
		return slopeBias;
	}
	
	float ShadowRasterizer::GetClamp() const
	{
		return clamp;
	}
}