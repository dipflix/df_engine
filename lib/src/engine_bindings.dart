import 'dart:ffi';
import 'dart:io' show Platform;

import 'package:df_engine/src/structs/engine.dart';
import 'package:ffi/ffi.dart';

import 'graphics/camera_3d.dart';
import 'structs/vertex_3d.dart';

typedef FrameCallbackC = Void Function(Float deltaTime);
typedef FrameCallbackDart = void Function(double deltaTime);

class GameEngine {
  late DynamicLibrary _lib;
  late Pointer<Engine> _engine;

  late final _createFunc = _lib.lookupFunction<
      Pointer<Engine> Function(Int32, Int32, Pointer<Utf8>),
      Pointer<Engine> Function(int, int, Pointer<Utf8>)>('engine_create');
  late final _destroyFunc = _lib.lookupFunction<Void Function(Pointer<Engine>),
      void Function(Pointer<Engine>)>('engine_destroy');
  late final _runFunc = _lib.lookupFunction<
      Void Function(Pointer<Engine>, Pointer<NativeFunction<FrameCallbackC>>),
      void Function(Pointer<Engine>,
          Pointer<NativeFunction<FrameCallbackC>>)>('engine_run');
  late final _setClearColorFunc = _lib.lookupFunction<
      Void Function(Pointer<Engine>, Float, Float, Float, Float),
      void Function(Pointer<Engine>, double, double, double,
          double)>('engine_set_clear_color');
  late final _setVerticesFunc = _lib.lookupFunction<
      Void Function(Pointer<Engine>, Pointer<Vertex3D>, Uint32),
      void Function(
          Pointer<Engine>, Pointer<Vertex3D>, int)>('engine_set_vertices');
  late final _setViewMatrixFunc = _lib.lookupFunction<
      Void Function(Pointer<Engine>, Pointer<Float>),
      void Function(Pointer<Engine>, Pointer<Float>)>('engine_set_view_matrix');

  GameEngine() {
    _lib = DynamicLibrary.open(Platform.isWindows
        ? 'vulkan_wrapper/build/Release/engine.dll'
        : throw UnsupportedError('Platform not supported'));
  }

  void initialize(int width, int height, String title) {
    final titlePtr = title.toNativeUtf8();
    _engine = _createFunc(width, height, titlePtr);
    malloc.free(titlePtr);
    if (_engine.address == 0) throw Exception("Failed to create engine");
  }

  void setClearColor(double r, double g, double b, double a) {
    _setClearColorFunc(_engine, r, g, b, a);
  }

  void setVertices(List<Vertex3D> vertices) {
    final vertexPtr = malloc<Vertex3D>(vertices.length);
    for (int i = 0; i < vertices.length; i++) {
      vertexPtr[i] = vertices[i];
    }
    _setVerticesFunc(_engine, vertexPtr, vertices.length);
    malloc.free(vertexPtr);
  }

  void setViewMatrix(Camera3D camera) {
    final matrixPtr = malloc<Float>(16);
    camera.update(matrixPtr);
    _setViewMatrixFunc(_engine, matrixPtr);
    malloc.free(matrixPtr);
  }

  void run(void Function(double deltaTime) callback) {
    final nativeCallback =
        NativeCallable<FrameCallbackC>.isolateLocal(callback);
    _runFunc(_engine, nativeCallback.nativeFunction);
    nativeCallback.close();
  }

  void dispose() {
    _destroyFunc(_engine);
  }
}
