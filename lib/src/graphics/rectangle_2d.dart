import '../structs/vertex_3d.dart';
import 'render_object.dart';


base class Rectangle2D implements RenderObject {
  double x, y, width, height;
  double r, g, b;

  Rectangle2D(this.x, this.y, this.width, this.height, this.r, this.g, this.b);

  @override
  List<Vertex3D> getVertices() {
    return [
      Vertex3D(x, y, 0.0, r, g, b),
      Vertex3D(x + width, y, 0.0, r, g, b),
      Vertex3D(x, y + height, 0.0, r, g, b),
      Vertex3D(x + width, y, 0.0, r, g, b),
      Vertex3D(x, y + height, 0.0, r, g, b),
      Vertex3D(x + width, y + height, 0.0, r, g, b),
    ];
  }
}
