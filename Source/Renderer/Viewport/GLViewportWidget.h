#pragma once

#include <QOpenGLWidget>
#include <QOpenGLContext>

#include "Renderer/Buffer/Framebuffer.h"
#include "Renderer/Buffer/VertexArray.h"
#include "Renderer/Buffer/VertexBuffer.h"
#include "Renderer/Buffer/IndexBuffer.h"

#include "Renderer/Renderable/Shader.h"
#include "Renderer/Camera/OrbitCamera.h"

class GLViewportWidget : public QOpenGLWidget {
	Q_OBJECT

public:
	GLViewportWidget(QWidget* parent = nullptr);
	~GLViewportWidget() override;

protected:
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int width, int height) override;

signals:
	void OnGLADReady();

private:
	void CreateBlitResources();
	void DeleteBlitResources();

	// Static shared resources (persist across widget lifecycles)
	Ref<VertexArray>		blit_vao_;
	Ref<VertexBuffer>	blit_vbo_;
	Ref<IndexBuffer>		blit_ibo_;
	Ref<Shader>		blit_shader_;

	// Instance-specific resources
	Ref<Framebuffer> blit_framebuffer_;
	Ref<OrbitCamera> camera_;

	bool glad_initalized = false;
};