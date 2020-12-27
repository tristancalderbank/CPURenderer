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

**Idea 1**: Pick point A of a triangle, draw a line from A to every point on the line connecting points B,C.

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/triangles_filled_failed.PNG" width="400">

Oops, this doesn't cover the whole triangle.

A different approach is to instead just loop through pixels in the image and check "is pixel inside triangle?". There are various way to perform that test. To be more efficient we can also calculate a "bounding box" around each triangle and only check those pixels.

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/triangles_bounding_box.PNG" width="400">

Now how do you check if a point is inside a triangle? It turns out thats pretty hard.

**Idea 1**: Connect the point to each vertex on the triangle. This forms three smaller triangles. If the point is in the triangle then the sum of the areas of the three small triangles will equal the area of the main triangle.

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/triangles_filled_area_failed.PNG" width="400">

Uhhh.....

I did eventually get this method working but it had some artifacts on the edges of the triangles which seemed related to some sort of floating point error.

**Idea 2**: A more common approach is to use something called "barycentric coordinates" which sounds complicated but is actually simple. The idea is to calculate the coordinates of the point as a weighted sum of the triangle vertices. If the coordinates end up negative or too big then the point is outside the triangle. [Here](https://www.youtube.com/watch?v=HYAgJN3x4GA) is a really good video visually explaining this.

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/triangles_filled.PNG" width="400">

Nice!

And just to show this off more here's the head model from earlier with random colors for each filled triangle (new album art?).

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/triangles_filled_head_backwards.PNG" width="400">

Alternatively we can add a basic light vector pointing into the image. We can then calculate a normal vector for each triangle using a cross-product of the sides. We can then determine the brightness of the triangles by taking the dot product of the light vector with each triangle normal.

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/triangles_filled_head_lighting.PNG" width="400">

This is a really basic way to do lighting (flat shading). There are smarter ways which use interpolated normals across pixels/vertices to get smoother lighting.

### Depth Testing (z-buffer)

A problem with the renders above is that we aren't keep track of which triangles are in front of each other, we're just rendering them all in the order they happen to be in the `.obj` model file. For example the mouth looks weird because we're rendering the inside triangles on top of the lips triangles.

An easy way to deal with this is to keep another data structure around called a z-buffer. The z-buffer is basically the same as the main image (a 2D array) but instead of RGB for each pixel we store the current depth of the corresponding pixel in the main image.  

Whenever we're about to draw a pixel for a triangle we calculate a z-value for the pixel (using some interpolation based on the vertex z values) and only draw it if its greater than the current value in the z-buffer.

The z-buffer stores floating point z-values but if we convert it to RGB values between 0-255 this is what we see.

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/zbuffer_buffer.PNG" width="400">

Now applying this to the main image, if you compare the mouth of the model rendered you can see we now correctly render the closer triangles on top of the image.

<img src="https://github.com/tristancalderbank/TinyRenderer/blob/master/TinyRenderer/images/png/zbuffer.PNG" width="400">


