#include "RenderingGLPch.h"
#include "RenderingGL/GLRenderer.h"

using namespace Helium;

/// Constructor.
GLRenderer::GLRenderer()
{
}

/// Destructor.
GLRenderer::~GLRenderer()
{
}

/// @copydoc Renderer::Initialize()
bool GLRenderer::Initialize()
{
	HELIUM_TRACE( TraceLevels::Info, "Initializing OpenGL rendering support.\n" );

	m_featureFlags = 0;

	// TODO: Select best depth texture format and enable depth texture feature flags.

	HELIUM_TRACE( TraceLevels::Info, "OpenGL initialized successfully.\n" );

	return true;
}

/// @copydoc Renderer::Shutdown()
void GLRenderer::Shutdown()
{
	HELIUM_TRACE( TraceLevels::Info, TXT( "Shutting down OpenGL rendering support.\n" ) );

	m_featureFlags = 0;

	HELIUM_TRACE( TraceLevels::Info, TXT( "OpenGL renderer shutdown complete.\n" ) );
}


/// @copydoc Renderer::CreateMainContext()
bool GLRenderer::CreateMainContext( const ContextInitParameters& rInitParameters )
{
	HELIUM_BREAK();

	return false;
}

/// @copydoc Renderer::ResetMainContext()
bool GLRenderer::ResetMainContext( const ContextInitParameters& rInitParameters )
{
	HELIUM_BREAK();

	return false;
}

/// @copydoc Renderer::GetMainContext()
RRenderContext* GLRenderer::GetMainContext()
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateSubContext()
RRenderContext* GLRenderer::CreateSubContext( const ContextInitParameters& rInitParameters )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::GetStatus()
Renderer::EStatus GLRenderer::GetStatus()
{
	HELIUM_BREAK();

	return STATUS_INVALID;
}

/// @copydoc Renderer::Reset()
Renderer::EStatus GLRenderer::Reset()
{
	HELIUM_BREAK();

	return STATUS_INVALID;
}

/// @copydoc Renderer::CreateRasterizerState()
RRasterizerState* GLRenderer::CreateRasterizerState( const RRasterizerState::Description& rDescription )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateBlendState()
RBlendState* GLRenderer::CreateBlendState( const RBlendState::Description& rDescription )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateDepthStencilState()
RDepthStencilState* GLRenderer::CreateDepthStencilState( const RDepthStencilState::Description& rDescription )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateSamplerState()
RSamplerState* GLRenderer::CreateSamplerState( const RSamplerState::Description& rDescription )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateDepthStencilSurface()
RSurface* GLRenderer::CreateDepthStencilSurface(
	uint32_t width,
	uint32_t height,
	ERendererSurfaceFormat format,
	uint32_t multisampleCount )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateVertexShader()
RVertexShader* GLRenderer::CreateVertexShader( size_t size, const void* pData )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreatePixelShader()
RPixelShader* GLRenderer::CreatePixelShader( size_t size, const void* pData )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateVertexBuffer()
RVertexBuffer* GLRenderer::CreateVertexBuffer( size_t size, ERendererBufferUsage usage, const void* pData )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateIndexBuffer()
RIndexBuffer* GLRenderer::CreateIndexBuffer(
	size_t size,
	ERendererBufferUsage usage,
	ERendererIndexFormat format,
	const void* pData )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateConstantBuffer()
RConstantBuffer* GLRenderer::CreateConstantBuffer(
	size_t size,
	ERendererBufferUsage /*usage*/,
	const void* pData )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateVertexDescription()
RVertexDescription* GLRenderer::CreateVertexDescription(
	const RVertexDescription::Element* pElements,
	size_t elementCount )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateVertexInputLayout()
RVertexInputLayout* GLRenderer::CreateVertexInputLayout(
	RVertexDescription* pDescription,
	RVertexShader* /*pShader*/ )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateTexture2d()
RTexture2d* GLRenderer::CreateTexture2d(
	uint32_t width,
	uint32_t height,
	uint32_t mipCount,
	ERendererPixelFormat format,
	ERendererBufferUsage usage,
	const RTexture2d::CreateData* pData )
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::CreateFence()
RFence* GLRenderer::CreateFence()
{
	HELIUM_BREAK();

	return NULL;
}

/// @copydoc Renderer::SyncFence()
void GLRenderer::SyncFence( RFence* pFence )
{
	HELIUM_BREAK();
}

/// @copydoc Renderer::TrySyncFence()
bool GLRenderer::TrySyncFence( RFence* pFence )
{
	HELIUM_BREAK();

	return false;
}

/// @copydoc Renderer::GetImmediateCommandProxy()
RRenderCommandProxy* GLRenderer::GetImmediateCommandProxy()
{
	return NULL;
}

/// @copydoc Renderer::CreateDeferredCommandProxy()
RRenderCommandProxy* GLRenderer::CreateDeferredCommandProxy()
{
	HELIUM_BREAK();

	return false;
}

/// @copydoc Renderer::Flush()
void GLRenderer::Flush()
{
	HELIUM_BREAK();
}

/// Create the static renderer instance.
///
/// @return  True if the renderer was created successfully, false if not or another renderer instance already
///          exists.
bool GLRenderer::CreateStaticInstance()
{
    if( sm_pInstance )
    {
        return false;
    }

    sm_pInstance = new GLRenderer;
    HELIUM_ASSERT( sm_pInstance );

    return ( sm_pInstance != NULL );
}