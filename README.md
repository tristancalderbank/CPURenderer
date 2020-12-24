# TinyRenderer
Attempt at implementing the software renderer described here: https://github.com/ssloy/tinyrenderer

# Progress

### Points

This is just drawing RGB pixel points onto an image grid. The result is rendered to a BMP image.

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/points.PNG" width="400">

### Lines

If you can draw points you can draw lines using [Bresenham's line algorithm](https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm).

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/lines.PNG" width="400">

### Mesh

Being able to draw lines allows you to draw meshes given an `.obj` file containing vertices and faces (connections between the vertices). This is technically an orthographic projection, we just ignore the z-coordinate for everything.

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/mesh.PNG" width="400">

### Triangle Rasterization

Drawing wireframe triangles is easy because we can already draw lines.

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/triangles_unfilled.PNG" width="400">

The hard part is drawing filled-in triangles.

**Idea 1**: pick point A of a triangle, draw a line from A to every point on the line connecting points B,C

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/triangles_filled_failed.PNG" width="400">

Oops, this doesn't cover the whole triangle.
