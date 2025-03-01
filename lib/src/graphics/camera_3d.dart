import 'dart:ffi';
import 'dart:math';

class Camera3D {
  double x, y, z;
  double yaw, pitch;
  double fov;
  double aspect;
  double near, far;

  Camera3D({
    this.x = 0.0,
    this.y = 0.0,
    this.z = 5.0,
    this.yaw = 0.0,
    this.pitch = 0.0,
    this.fov = 45.0,
    this.aspect = 800.0 / 600.0,
    this.near = 0.1,
    this.far = 100.0,
  });

  List<double> _getProjectionMatrix() {
    final f = 1.0 / tan(fov * pi / 360.0);
    return [
      f / aspect, 0.0, 0.0, 0.0,
      0.0, f, 0.0, 0.0,
      0.0, 0.0, (far + near) / (near - far), -1.0,
      0.0, 0.0, (2.0 * far * near) / (near - far), 0.0,
    ];
  }

  List<double> _getViewMatrix() {
    final cosPitch = cos(pitch * pi / 180.0);
    final sinPitch = sin(pitch * pi / 180.0);
    final cosYaw = cos(yaw * pi / 180.0);
    final sinYaw = sin(yaw * pi / 180.0);

    final forwardX = cosPitch * sinYaw;
    final forwardY = sinPitch;
    final forwardZ = cosPitch * cosYaw;

    final rightX = cosYaw;
    final rightY = 0.0;
    final rightZ = -sinYaw;

    final upX = -sinPitch * sinYaw;
    final upY = cosPitch;
    final upZ = -sinPitch * cosYaw;

    return [
      rightX, upX, -forwardX, 0.0,
      rightY, upY, -forwardY, 0.0,
      rightZ, upZ, -forwardZ, 0.0,
      -(rightX * x + rightY * y + rightZ * z),
      -(upX * x + upY * y + upZ * z),
      -(forwardX * x + forwardY * y + forwardZ * z),
      1.0,
    ];
  }

  List<double> getViewProjMatrix() {
    final proj = _getProjectionMatrix();
    final view = _getViewMatrix();
    return _multiplyMatrices(proj, view);
  }

  void update(Pointer<Float> matrixPtr) {
    final matrix = getViewProjMatrix();
    for (int i = 0; i < 16; i++) {
      matrixPtr[i] = matrix[i];
    }
  }

  List<double> _multiplyMatrices(List<double> a, List<double> b) {
    final result = List<double>.filled(16, 0.0);
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 4; k++) {
          result[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
        }
      }
    }
    return result;
  }
}