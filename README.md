# Self_face_head

## Set up

* Operating & compiling system: Cmake + Visual Studio 2019(x64) in Windows11
* Framework: [OpenGL](https://github.com/opengl-tutorials/ogl)

## Pipeline (* what I did)

### Create the head model in blender*

### Integrate all functions into the GUI*

![GUI.png](https://github.com/jamesdemon923/Self_face_head/blob/main/img/GUI.png)

### Display face geometry as a triangular mesh*

![Face_geometry.png](https://github.com/jamesdemon923/Self_face_head/blob/main/img/Face_geometry.png)

### Map the facial pictures onto the head

![Face_quad.png](https://github.com/jamesdemon923/Self_face_head/blob/main/img/Face_quad.png)

![Face_texture.png](https://github.com/jamesdemon923/Self_face_head/blob/main/img/Face_texture.png)

### Use PN triangle tessellation to refine the model*

The tessellation shader components:

![Tessellation.png](https://github.com/jamesdemon923/Self_face_head/blob/main/img/Tessellation.png)

In our project, we set 9 levels of detail for our tessellation.



## Result
