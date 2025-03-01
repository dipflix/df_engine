import 'package:df_engine/src/structs/vertex_3d.dart';

import '../engine_bindings.dart';
import '../graphics/render_object.dart';
import '../structs/structs.dart';

class Scene {
  List<RenderObject> objects = [];

  void add(RenderObject obj) {
    objects.add(obj);
  }

  void render(GameEngine engine) {
    final allVertices = <Vertex3D>[];
    for (var obj in objects) {
      allVertices.addAll(obj.getVertices());
    }
    if (allVertices.isNotEmpty) {
      engine.setVertices(allVertices);
    }
  }
}