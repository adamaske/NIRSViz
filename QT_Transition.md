# QT6 Integration

We have been using ImGUI previously and that has worked great, 
however we are moving towards this being a real application
and this needs a different UI framework.

Immediate Mode GUI renders the GUI fully each frame, this has
enabled rapid iteration and easy logic coupling. But, this is
inefficient and results in the whole application being a "real-time"
system. 

Therefore, we are moving towards QT6 which has built-in OpenGL integration.
My current renderer always renders to a Framebuffer (Renderer/Buffer/Framebuffer.cpp).
Therefore we can simplify this process alot by instead of rendering directly onto
the widget we rather place a render the framebuffer onto a quad as a texture.
This approach lets the framebuffer data stay on the graphics unit. 

And the renderer::ExecuteQueue occurs inside the main application loop. We may need to 
move the execute command towards the paintGL function? 
I would really prefer to have GLAD still be initalized because that lets me avoid
changing my whole rendering system. 

## Plan for Implementation
Currently the application itself launches a glfw window and
initialized GLAD. In a QT framework we replace this window with a
QMainWindow (Application : public QMainWindow). 

### Limitation 1
To simplify this implementation I limit the application to a single
OpenGLWidget, this eliminates the need for complex rendering logic
and context switching. 

## Channel Selection
This really is a 2D scene which can be quite simply rendered
more traditionally rather than with OpenGL. (Systems/ChannelSelectorSystem)

## Plan
1. Create a QOpenGLWidget subclass (GLViewportWidget). We also need a QDockWidget which is fed the new subclass
We stop using the ImGUISystem and avoid calling the OnGUIRender in the main application loop.
Here we use a "blit" render technique, where we only actually render a quad geometry and then display on it a texture gotten from the framebuffer. 

2. OpenGL-widget specific.  
3. Turn Application into a QMainWindow subclass. Have initalize and set the GLViewpoertWidget (main_viewport_) as the central widget.

## Inital test run
Render the "AnatomyViewport" framebuffer onto the glviewport.