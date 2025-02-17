#pragma once
#include "GraphicsResource.h"

class Drawable;
class TechniqueProbe;
class Graphics;

namespace Bind
{
	class Bindable : public GraphicsResource
	{
	public:
		virtual void Bind( Graphics& gfx ) noexcept(!IS_DEBUG) = 0;
		virtual void InitializeParentReference( const Drawable& ) noexcept {};
		virtual void Accept( TechniqueProbe& ) { }
		virtual std::string GetUID() const noexcept
		{
			assert( false );
			return "";
		}
		virtual ~Bindable() = default;
	};

	class CloningBindable : public Bindable
	{
	public:
		virtual std::unique_ptr<CloningBindable> Clone() const noexcept = 0;
	};
}