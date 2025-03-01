import 'package:df_engine/src/graphics/render_object.dart';

import '../structs/vertex_3d.dart';

class Triangle3D implements RenderObject {
  double x1, y1, z1;
  double x2, y2, z2;
  double x3, y3, z3;
  double r, g, b;

  Triangle3D(this.x1, this.y1, this.z1, this.x2, this.y2, this.z2, this.x3,
      this.y3, this.z3, this.r, this.g, this.b);

  @override
  List<Vertex3D> getVertices() {
    return [
      Vertex3D(x1, y1, z1, r, g, b),
      Vertex3D(x2, y2, z2, r, g, b),
      Vertex3D(x3, y3, z3, r, g, b),
    ];
  }
}
