import '../src/engine_bindings.dart';
import '../src/core/scene.dart';
import '../src/graphics/camera_3d.dart';
import '../src/graphics/rectangle_2d.dart';
import '../src/graphics/triangle_3d.dart';
import '../src/objects/cube.dart';
import 'tools/shader_compiler.dart';

Future<void> main() async {
  try {
    print('Compiling shaders...');
    final shaderCompiler = ShaderCompiler('shaders', '.shaders');

    await shaderCompiler.compileAll();
    print('Shaders compiled successfully.');

    final engine = GameEngine();
    engine.initialize(800, 600, "Game Engine");

    final scene = Scene();
    scene.add(Rectangle2D(-0.5, -0.5, 1.0, 1.0, 1.0, 0.0, 0.0));
    scene.add(Triangle3D(0.0, 0.5, 0.0, -0.5, -0.5, 0.0, 0.5, -0.5, 0.0, 0.0, 1.0, 0.0));
    scene.add(Cube(0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0));
    final camera = Camera3D(x: 0.0, y: 0.0, z: 5.0); // Камера на расстоянии 5 по Z
    double totalTime = 0.0;
    engine.run((deltaTime) {
      totalTime += deltaTime;

      // if (totalTime % 4 < 1) {
      //   engine.setClearColor(0.0, 1.0, 0.0, 1.0);
      // } else {
      //   engine.setClearColor(1.0, 0.0, 0.0, 1.0);
      // }

      camera.yaw = totalTime * 10; // Вращение камеры для эффекта
      engine.setViewMatrix(camera);
      scene.render(engine);
    });

    engine.dispose();
  } catch (e) {
    print('Error: $e');
  }
}
