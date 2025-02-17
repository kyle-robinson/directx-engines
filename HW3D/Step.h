#pragma once
#include "Bindable.h"
#include "Graphics.h"
#include <vector>
#include <memory>

class TechniqueProbe;
class Drawable;

namespace Rgph
{
	class RenderQueuePass;
	class RenderGraph;
}

class Step
{
public:
	Step( std::string targetPassName );
	Step( Step&& ) = default;
	Step( const Step& src ) noexcept;
	Step& operator=( const Step& ) = delete;
	Step& operator=( Step&& ) = delete;
	void AddBindable( std::shared_ptr<Bind::Bindable> bind_in ) noexcept;
	void Submit( const class Drawable& drawable ) const;
	void Bind( Graphics& gfx ) const noexcept(!IS_DEBUG);
	void InitializeParentReferences( const class Drawable& parent ) noexcept;
	void Accept( TechniqueProbe& probe );
	void Link( Rgph::RenderGraph& rg );
private:
	std::string targetPassName;
	Rgph::RenderQueuePass* pTargetPass = nullptr;
	std::vector<std::shared_ptr<Bind::Bindable>> bindables;
};