#include "pch.h"
#include "GLViewportWidget.h"
#include "Renderer/ViewportManager.h" // TODO : Transition to ViewportRegistry

#include <glad/glad.h>

GLViewportWidget::GLViewportWidget(QWidget *parent) : QOpenGLWidget(parent) {
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(4, 6);
	format.setProfile(QSurfaceFormat::CoreProfile);
}

GLViewportWidget::~GLViewportWidget() {
	// Only delete instance-specific resources
	makeCurrent();
	DeleteBlitResources();
	doneCurrent();
}

void GLViewportWidget::initializeGL() {
	QOpenGLWidget::initializeGL();

	int success = gladLoadGL();
	if (!success)
		NVIZ_ERROR("Failed to initialize GLAD...");

	NVIZ_INFO("OpenGL Info:");
	NVIZ_INFO("  Vendor: {0}",		(const char*)glGetString(GL_VENDOR));
	NVIZ_INFO("  Renderer: {0}",		(const char*)glGetString(GL_RENDERER));
	NVIZ_INFO("  Version: {0}",		(const char*)glGetString(GL_VERSION));

	if (glad_initalized)
		return;

	emit OnGLADReady();
	// Create instance-specific resources
	CreateBlitResources();

	glad_initalized = true;
}

void GLViewportWidget::CreateBlitResources() {
	// Create the framebuffer that will be rendered to
	FramebufferSpecification fb_spec;
	fb_spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
	fb_spec.Width = size().width();
	fb_spec.Height = size().height();
	blit_framebuffer_ = CreateRef<Framebuffer>(fb_spec);

	// Create the camera
	camera_ = CreateRef<OrbitCamera>();
	camera_->SetViewportSize(size().width(), size().height());

	// Register the viewport with the viewport manager
	ViewportManager::RegisterViewport(ViewportType::AnatomyViewport, { .Camera = camera_.get(), .Framebuffer = blit_framebuffer_.get() });

	// Fullscreen quad vertices in NDC space (-1 to 1)
	// Position (x, y) and TexCoord (u, v)
	float quadVertices[] = {
		// Positions   // TexCoords
		-1.0f,  1.0f,  0.0f, 1.0f, // Top-left
		-1.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
		 1.0f, -1.0f,  1.0f, 0.0f, // Bottom-right
		 1.0f,  1.0f,  1.0f, 1.0f  // Top-right
	};

	// Indices for two triangles forming a quad
	unsigned int quadIndices[] = {
		0, 1, 2, // First triangle
		2, 3, 0  // Second triangle
	};

	// Create and configure the vertex array
	blit_vao_ = CreateRef<VertexArray>();

	// Create vertex buffer
	blit_vbo_ = CreateRef<VertexBuffer>(quadVertices, sizeof(quadVertices));
	blit_vbo_->SetLayout({
		{ ShaderDataType::Float2, "aPos" },
		{ ShaderDataType::Float2, "aTexCoord" }
	});

	// Create index buffer
	blit_ibo_ = CreateRef<IndexBuffer>(quadIndices, 6);

	// Add buffers to VAO
	blit_vao_->AddVertexBuffer(blit_vbo_);
	blit_vao_->SetIndexBuffer(blit_ibo_);

	// Create the blit shader
	blit_shader_ = CreateRef<Shader>(
		"C:/dev/NIRSViz/Assets/Shaders/Blit.vert",
		"C:/dev/NIRSViz/Assets/Shaders/Blit.frag"
	);
}

void GLViewportWidget::paintGL() {
	if (!blit_ibo_)
		NVIZ_ERROR("blit_ibo_ is null");
	if (!blit_vao_)
		NVIZ_ERROR("blit_vao_ is null");
	if (!blit_shader_)
		NVIZ_ERROR("blit_shader_ is null");
	if (!camera_)
		NVIZ_ERROR("camera_ is null");

	// Clear the default framebuffer
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Disable depth testing for the blit operation (we're rendering a fullscreen quad)
	glDisable(GL_DEPTH_TEST);

	// Bind the shared blit shader
	blit_shader_->Bind();

	// Bind the framebuffer's color attachment as a texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, blit_framebuffer_->GetColorAttachmentRendererID(0));
	blit_shader_->SetUniform1i("u_Texture", 0);

	// Draw the fullscreen quad using shared VAO
	blit_vao_->Bind();
	glDrawElements(GL_TRIANGLES, blit_ibo_->GetCount(), GL_UNSIGNED_INT, nullptr);
	blit_vao_->Unbind();

	// Re-enable depth testing for other rendering
	glEnable(GL_DEPTH_TEST);
}

void GLViewportWidget::resizeGL(int width, int height) {
	QOpenGLWidget::resizeGL(width, height);

	if (blit_framebuffer_)
		blit_framebuffer_->Resize(width, height);

	if (camera_)
		camera_->SetViewportSize(width, height);
}


void GLViewportWidget::DeleteBlitResources() {
	NVIZ_CRITICAL("GLViewportWidget::DeleteBlitResources");
}
