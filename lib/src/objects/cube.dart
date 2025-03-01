import '../graphics/render_object.dart';
import '../structs/vertex_3d.dart';

class Cube implements RenderObject {
  double x, y, z;
  double size;
  double r, g, b;

  Cube(this.x, this.y, this.z, this.size, this.r, this.g, this.b);

  @override
  List<Vertex3D> getVertices() {
    final halfSize = size / 2;
    return [
      // Передняя грань
      Vertex3D(x - halfSize, y - halfSize, z + halfSize, r, g, b),
      Vertex3D(x + halfSize, y - halfSize, z + halfSize, r, g, b),
      Vertex3D(x - halfSize, y + halfSize, z + halfSize, r, g, b),
      Vertex3D(x + halfSize, y - halfSize, z + halfSize, r, g, b),
      Vertex3D(x - halfSize, y + halfSize, z + halfSize, r, g, b),
      Vertex3D(x + halfSize, y + halfSize, z + halfSize, r, g, b),

      // Задняя грань
      Vertex3D(x - halfSize, y - halfSize, z - halfSize, r, g, b),
      Vertex3D(x + halfSize, y - halfSize, z - halfSize, r, g, b),
      Vertex3D(x - halfSize, y + halfSize, z - halfSize, r, g, b),
      Vertex3D(x + halfSize, y - halfSize, z - halfSize, r, g, b),
      Vertex3D(x - halfSize, y + halfSize, z - halfSize, r, g, b),
      Vertex3D(x + halfSize, y + halfSize, z - halfSize, r, g, b),

      // Левая грань
      Vertex3D(x - halfSize, y - halfSize, z - halfSize, r, g, b),
      Vertex3D(x - halfSize, y - halfSize, z + halfSize, r, g, b),
      Vertex3D(x - halfSize, y + halfSize, z - halfSize, r, g, b),
      Vertex3D(x - halfSize, y - halfSize, z + halfSize, r, g, b),
      Vertex3D(x - halfSize, y + halfSize, z - halfSize, r, g, b),
      Vertex3D(x - halfSize, y + halfSize, z + halfSize, r, g, b),

      // Правая грань
      Vertex3D(x + halfSize, y - halfSize, z - halfSize, r, g, b),
      Vertex3D(x + halfSize, y - halfSize, z + halfSize, r, g, b),
      Vertex3D(x + halfSize, y + halfSize, z - halfSize, r, g, b),
      Vertex3D(x + halfSize, y - halfSize, z + halfSize, r, g, b),
      Vertex3D(x + halfSize, y + halfSize, z - halfSize, r, g, b),
      Vertex3D(x + halfSize, y + halfSize, z + halfSize, r, g, b),

      // Верхняя грань
      Vertex3D(x - halfSize, y + halfSize, z - halfSize, r, g, b),
      Vertex3D(x + halfSize, y + halfSize, z - halfSize, r, g, b),
      Vertex3D(x - halfSize, y + halfSize, z + halfSize, r, g, b),
      Vertex3D(x + halfSize, y + halfSize, z - halfSize, r, g, b),
      Vertex3D(x - halfSize, y + halfSize, z + halfSize, r, g, b),
      Vertex3D(x + halfSize, y + halfSize, z + halfSize, r, g, b),

      // Нижняя грань
      Vertex3D(x - halfSize, y - halfSize, z - halfSize, r, g, b),
      Vertex3D(x + halfSize, y - halfSize, z - halfSize, r, g, b),
      Vertex3D(x - halfSize, y - halfSize, z + halfSize, r, g, b),
      Vertex3D(x + halfSize, y - halfSize, z - halfSize, r, g, b),
      Vertex3D(x - halfSize, y - halfSize, z + halfSize, r, g, b),
      Vertex3D(x + halfSize, y - halfSize, z + halfSize, r, g, b),
    ];
  }
}