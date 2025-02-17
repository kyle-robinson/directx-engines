#include "Projection.h"
#include "Graphics.h"
#include "imgui/imgui.h"

Projection::Projection( Graphics& gfx, float width, float height, float nearZ, float farZ )
	:
	width( width ), height( height ), nearZ( nearZ ), farZ( farZ ),
	initialWidth( width ), initialHeight( height ), initialNearZ( nearZ ), initialFarZ( farZ ),
	frust( gfx, width, height, nearZ, farZ )
{
	Reset( gfx );
}

DirectX::XMMATRIX Projection::GetMatrix() const
{
	return DirectX::XMMatrixPerspectiveLH( width, height, nearZ, farZ );
}

void Projection::RenderWidgets( Graphics& gfx )
{
	if ( ImGui::CollapsingHeader( "Projection" ) )
	{
		bool bufferSet = false;
		const auto linkCheck = [&bufferSet]( bool changed ) { bufferSet = bufferSet || changed; };

		linkCheck( ImGui::SliderFloat( "Width", &width, 0.01f, 4.0f, "%.2f", 1.5f ) );
		linkCheck( ImGui::SliderFloat( "Height", &height, 0.01f, 4.0f, "%.2f", 1.5f ) );
		linkCheck( ImGui::SliderFloat( "Near Z", &nearZ, 0.01f, farZ - 0.01f, "%.2f", 4.0f ) );
		linkCheck( ImGui::SliderFloat( "Far Z", &farZ, nearZ + 0.01f, 400.0f, "%.2f", 4.0f ) );

		if ( ImGui::Button( "Reset Projection" ) )
			Reset( gfx );

		if ( bufferSet )
			frust.SetVertices( gfx, width, height, nearZ, farZ );
	}
}

void Projection::SetPosition( DirectX::XMFLOAT3 pos )
{
	frust.SetPosition( pos );
}

void Projection::SetRotation( DirectX::XMFLOAT3 rot )
{
	frust.SetRotation( rot );
}

void Projection::LinkTechniques( Rgph::RenderGraph& rg )
{
	frust.LinkTechniques( rg );
}

void Projection::Submit( size_t channel ) const
{
	frust.Submit( channel );
}

void Projection::Reset( Graphics& gfx )
{
	width = initialWidth;
	height = initialHeight;
	nearZ = initialNearZ;
	farZ = initialFarZ;
	frust.SetVertices( gfx, width, height, nearZ, farZ );
}