import 'dart:math';

import '../src/core/scene.dart';
import '../src/engine_bindings.dart';
import '../src/graphics/camera_3d.dart';
import '../src/objects/cube.dart';
import 'tools/shader_compiler.dart';

Future<void> main() async {
  try {
    print('Compiling shaders...');
    final shaderCompiler = ShaderCompiler('shaders', '.shaders');
    await shaderCompiler.compileAll();
    print('Shader compilation completed.');

    final engine = GameEngine();
    print("Calling engine.initialize...");
    engine.initialize(800, 600, "Game Engine");
    print("Engine initialized successfully");

    final scene = Scene();
    scene.add(Cube(
        0.0,
        0.0,
        0.0,
        1.0,
        0.0,
        0.0,
        1.0));

    final camera = Camera3D(
      x: 0.0,
      y: 0.0,
      z: 2.0,
    );
    engine.setClearColor(0.0, 0.1, 0.1, 0.1);
    double totalTime = 0.0;
    engine.run((deltaTime) {
      totalTime += deltaTime;

      // if (totalTime % 4 < 1) {
      //   engine.setClearColor(0.0, 1.0, 0.0, 1.0);
      // } else {
      //   engine.setClearColor(1.0, 0.0, 0.0, 1.0);
      // }

      camera.z = sin(totalTime) * 2 + 2;
      camera.y = sin(totalTime) * 2 + 2;
      camera.x = cos(totalTime) * 2 + 2;
      camera.yaw = cos(totalTime) * 2 + 2;
      engine.setViewMatrix(camera);
      scene.render
      (
      engine
      );
    });

    engine.dispose();
  } catch (e) {
    print('Error: $e');
  }
}
