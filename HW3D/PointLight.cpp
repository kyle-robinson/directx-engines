#include "PointLight.h"
#include "Camera.h"
#include "Math.h"
#include "imgui/imgui.h"

PointLight::PointLight( Graphics& gfx, DirectX::XMFLOAT3 pos, float radius ) :
	mesh( gfx, radius ), cbuf( gfx )
{
	initial = {
		pos,
		{ 0.05f, 0.05f, 0.05f },
		{ 1.0f, 1.0f, 1.0f },
		1.0f,
		1.0f,
		0.045f,
		0.0075f
	};
	Reset();
	pCamera = std::make_shared<Camera>( gfx, "Light", cbData.lightPos, 0.0f, PI / 2.0f, 12.0f, 0.002f, true );
}

void PointLight::SpawnControlWindow( const char* name ) noexcept
{
	name = name ? name : "Light";
	if ( ImGui::Begin( name, FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		bool bufferSet = false;
		const auto linkCheck = [&bufferSet]( bool changed ) { bufferSet = bufferSet || changed; };
		
		linkCheck( ImGui::SliderFloat3( "Position", &cbData.lightPos.x, -50.0f, 50.0f, "%1.f" ) );

		if ( bufferSet )
			pCamera->SetPosition( cbData.lightPos );

		mesh.SpawnControls();

		if ( name == "Main Light" )
		{
			if ( ImGui::CollapsingHeader( "Color" ) )
			{
				ImGui::SliderFloat( "Intensity", &cbData.diffuseIntensity, 0.0f, 2.0f, "%.2f" );
				ImGui::ColorEdit3( "Ambient", &cbData.ambient.x );
				ImGui::ColorEdit3( "Diffuse", &cbData.diffuseColor.x );

				ImGui::Text( "Attenuation" );
				ImGui::SliderFloat( "Constant", &cbData.attConst, 0.05f, 10.0f, "%.2f", 4 );
				ImGui::SliderFloat( "Linear", &cbData.attLin, 0.0001f, 4.0f, "%.4f", 8 );
				ImGui::SliderFloat( "Quadratic", &cbData.attQuad, 0.0000001f, 1.0f, "%.7f", 10 );
			}
		}
				
		if ( ImGui::Button( "Reset" ) )
			Reset();
	}
	ImGui::End();
}

void PointLight::Reset() noexcept
{
	cbData = initial;
}

void PointLight::Submit( size_t channels ) const noexcept(!IS_DEBUG)
{
	mesh.SetPosition( cbData.lightPos );
	mesh.Submit( channels );
}

void PointLight::Bind( Graphics& gfx, DirectX::FXMMATRIX view ) const noexcept
{
	// update light world position to local camera position
	auto dataCopy = cbData;
	const auto pos = DirectX::XMLoadFloat3( &cbData.lightPos ); // load the world position of the light
	DirectX::XMStoreFloat3( &dataCopy.lightPos, DirectX::XMVector3Transform( pos, view ) ); // transform position using view matrix to local position
	cbuf.Update( gfx, dataCopy ); // update copied data using new local position matrix - light position is now relative to camera
	cbuf.Bind( gfx );
}

void PointLight::LinkTechniques( Rgph::RenderGraph& rg )
{
	mesh.LinkTechniques( rg );
}

std::shared_ptr<Camera> PointLight::ShareCamera() const noexcept
{
	return pCamera;
}