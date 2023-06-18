# Self_face_head

## Set up

* Operating & compiling system: Cmake + Visual Studio 2019(x64) in Windows11
* Framework: [OpenGL](https://github.com/opengl-tutorials/ogl)

## Pipeline (* what I did)

### Create the head model in blender*

### Integrate all functions into the GUI*

<img src="img/GUI.png" alt="GUI.png" style="zoom:67%;" />

### Display face geometry as a triangular mesh*

<img src="img/Face_geometry.png" alt="Face_geometry.png" style="zoom:67%;" />

### Map the facial pictures onto the head

<img src="img/Face_quad.png" alt="Face_quad.png" style="zoom:67%;" />

<img src="img/Face_texture.png" alt="Face_texture.png" style="zoom:67%;" />

### Use PN triangle tessellation to refine the model*

The tessellation shader components:

<img src="img/Tessellation.png" alt="Tessellation.png" style="zoom:67%;" />

In our project, we set 9 levels of detail for our tessellation.



## Result
