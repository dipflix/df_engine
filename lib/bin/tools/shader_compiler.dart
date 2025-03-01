import 'dart:io';

class ShaderCompiler {
  final String inputDirectory;
  final String shaderDirectory;

  ShaderCompiler(this.inputDirectory, this.shaderDirectory);

  Future<void> compileAll() async {
    final inputDir = Directory(inputDirectory);
    if (!await inputDir.exists()) {
      throw Exception('Input directory does not exist: $inputDirectory');
    }

    final outputDir = Directory(shaderDirectory);
    if (!await outputDir.exists()) {
      outputDir.createSync();
      print('Shader directory created: $shaderDirectory');
    }

    final shaderFiles = inputDir
        .listSync()
        .where((entity) => entity is File && entity.path.endsWith('.glsl'))
        .cast<File>()
        .toList();

    final compileTasks = <Future<void>>[];
    for (var shaderFile in shaderFiles) {
      final shaderStage = _getShaderStage(shaderFile);
      if (shaderStage != null) {
        final outputFilePath = _getOutputFilePath(shaderFile);
        final shouldCompile = await _shouldCompile(shaderFile, outputFilePath);
        if (shouldCompile) {
          compileTasks
              .add(compile(shaderFile.path, outputFilePath, shaderStage));
        } else {
          print('Skipping unchanged shader: ${shaderFile.path}');
        }
      }
    }

    await Future.wait(compileTasks);
  }

  Future<void> compile(
      String shaderFilePath, String outputFilePath, String shaderStage) async {
    final command = [
      'glslc',
      '-fshader-stage=$shaderStage',
      shaderFilePath,
      '-o',
      outputFilePath,
    ];

    try {
      final result = await Process.run(command[0], command.sublist(1));
      if (result.exitCode == 0) {
        print('Shader compiled successfully: $outputFilePath');
      } else {
        print('Error compiling shader $shaderFilePath:');
        print('STDERR: ${result.stderr}');
        print('STDOUT: ${result.stdout}');
        throw Exception('Shader compilation failed');
      }
    } catch (e) {
      print('Exception during shader compilation: $e');
      rethrow;
    }
  }

  Future<bool> _shouldCompile(File shaderFile, String outputFilePath) async {
    final outputFile = File(outputFilePath);
    if (!await outputFile.exists()) {
      return true;
    }

    final shaderStat = await shaderFile.stat();
    final outputStat = await outputFile.stat();

    return shaderStat.modified.isAfter(outputStat.modified);
  }

  String? _getShaderStage(File shaderFile) {
    final fileName = shaderFile.uri.pathSegments.last.toLowerCase();
    if (fileName.contains('vertex')) {
      return 'vertex';
    } else if (fileName.contains('fragment')) {
      return 'fragment';
    }
    return null;
  }

  String _getOutputFilePath(File shaderFile) {
    final fileName = shaderFile.uri.pathSegments.last.split('.').first;
    return '$shaderDirectory/$fileName.spv';
  }
}
