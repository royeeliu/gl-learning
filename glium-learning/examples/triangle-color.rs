#[macro_use]
extern crate glium;

use glium::{glutin, Surface};

#[derive(Copy, Clone)]
struct Vertex {
    position: [f32; 2],
    color: [f32; 3],
}
implement_vertex!(Vertex, position, color);

fn main() {
    let vertices = vec![
        Vertex {
            position: [-0.5, -0.5],
            color: [1.0, 0.0, 0.0],
        },
        Vertex {
            position: [0.0, 0.5],
            color: [0.0, 1.0, 0.0],
        },
        Vertex {
            position: [0.5, -0.5],
            color: [0.0, 0.0, 1.0],
        },
    ];

    let vertex_shader_src = r#"
        #version 330 core
        layout (location = 0) in vec2 position;
        layout (location = 1) in vec3 color;
        out vec3 ourColor;

        void main()
        {
           gl_Position = vec4(position, 0.0, 1.0);
           ourColor = color;
        }
    "#;

    let fragment_shader_src = r#"
        #version 330 core    
        in vec3 ourColor;
        out vec4 color;

        void main() {
            color = vec4(ourColor, 1.0);
        }
    "#;

    let event_loop = glutin::event_loop::EventLoop::new();
    let wb = glutin::window::WindowBuilder::new();
    let cb = glutin::ContextBuilder::new();
    let display = glium::Display::new(wb, cb, &event_loop).unwrap();

    let vertex_buffer = glium::VertexBuffer::new(&display, &vertices).unwrap();
    let indices = glium::index::NoIndices(glium::index::PrimitiveType::TrianglesList);
    let program =
        glium::Program::from_source(&display, vertex_shader_src, fragment_shader_src, None)
            .unwrap();

    event_loop.run(move |event, _, control_flow| {
        let next_frame_time =
            std::time::Instant::now() + std::time::Duration::from_nanos(16_666_667);
        *control_flow = glutin::event_loop::ControlFlow::WaitUntil(next_frame_time);

        match event {
            glutin::event::Event::WindowEvent { event, .. } => match event {
                glutin::event::WindowEvent::CloseRequested => {
                    *control_flow = glutin::event_loop::ControlFlow::Exit;
                }
                _ => {}
            },
            _ => (),
        }

        let mut target = display.draw();
        target.clear_color(0.0, 0.1, 0.1, 1.0);
        target
            .draw(
                &vertex_buffer,
                &indices,
                &program,
                &glium::uniforms::EmptyUniforms,
                &Default::default(),
            )
            .unwrap();
        target.finish().unwrap();
    });
}
