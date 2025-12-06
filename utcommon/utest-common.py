#!/usr/bin/env python3

import os
import subprocess
import sys
from pathlib import Path

# Configuración
TEST_EXECUTABLE = "ut_renderer_common"
# DIRECTORIO RAIZ DE SALIDA: Ahora apunta a /workspace/out
OUT_DIR = Path("/workspace/out")
# DIRECTORIO BASE PARA LAS BUILDS: /workspace/out/build
BUILD_BASE_DIR = OUT_DIR / "build"
# Directorios de compilación específicos: /workspace/out/build/default y /workspace/out/build/clang-tidy
BUILD_DIRS = {
    "default": BUILD_BASE_DIR / "default",
    "clang-tidy": BUILD_BASE_DIR / "clang-tidy"
}
# Directorio donde se encuentra el CMakeLists.txt (directorio padre del script)
CURRENT_DIR = Path(__file__).parent
SOURCE_DIR = CURRENT_DIR


# FUNCIONES AUXILIARES

def run_command(command: list, cwd: Path = None, check_error: bool = True):
    """Ejecuta un comando del sistema y maneja errores."""
    print(f"-> Ejecutando: {' '.join(command)}")
    try:
        result = subprocess.run(
            command,
            cwd=cwd,
            check=check_error,
            text=True,
            capture_output=True
        )
        return result
    except subprocess.CalledProcessError as e:
        print(f"ERROR: Falló el comando. Código de salida: {e.returncode}")
        print(f"Directorio de ejecución: {cwd}")
        print("--- Salida Estándar (stdout) ---")
        print(e.stdout)
        print("--- Salida de Error (stderr) ---")
        print(e.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print(f"ERROR: El comando '{command[0]}' no se encontró. Asegúrate de que está en el PATH.")
        sys.exit(1)

# Pasos de ejecución

def setup_and_build(build_type: str):
    """Configura el entorno de CMake y compila el proyecto para un tipo de build específico."""
    build_dir = BUILD_DIRS[build_type]

    print("=" * 40)
    print(f"  CONFIGURANDO Y COMPILANDO [{build_type.upper()}]  ")
    print("=" * 40)

    # 1. Crear directorio de build
    if build_dir.exists():
        # *** AÑADE ESTA LÍNEA PARA LIMPIAR EL CACHÉ Y SOLUCIONAR EL ERROR ***
        import shutil
        print(f"-> Limpiando directorio existente: {build_dir}")
        shutil.rmtree(build_dir)
        
    build_dir.mkdir(parents=True, exist_ok=True)
        
    # 2. Configurar (cmake <SOURCE_DIR>)
    # El CMakeLists.txt está en el directorio fuente (SOURCE_DIR)
    cmake_args = ["cmake", str(SOURCE_DIR)]
    
    # Configuración específica para clang-tidy
    if build_type == "clang-tidy":
        print("-> Habilitando clang-tidy y compilador clang...")
        cmake_args.extend([
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", 
            "-DCMAKE_C_COMPILER=clang",
            "-DCMAKE_CXX_COMPILER=clang++",
            # Habilitar clang-tidy
            "-DCMAKE_CXX_CLANG_TIDY=/usr/bin/clang-tidy;--header-filter=.*" 
        ])

    run_command(cmake_args, cwd=build_dir)

    # 3. Compilar (make)
    run_command(["make"], cwd=build_dir)

def list_and_run_tests(build_type: str):
    """Lista las pruebas concretas y luego las ejecuta con CTest (método Google Test)."""
    build_dir = BUILD_DIRS[build_type]
    test_exe_path = build_dir / TEST_EXECUTABLE

    if not test_exe_path.exists():
        print(f"\nERROR: El ejecutable de pruebas '{test_exe_path}' no se encontró en la build '{build_type}'.")
        sys.exit(1)
        
    print("\n" + "=" * 40)
    print(f"   LISTA DE PRUEBAS CONCRETAS [{build_type.upper()}]    ")
    print("=" * 40)
    
    # 1. Listar pruebas usando el ejecutable de Google Test
    list_result = run_command([str(test_exe_path), "--gtest_list_tests"], check_error=False)
    print(list_result.stdout)
    
    # 2. Ejecutar pruebas
    print("\n" + "=" * 40)
    print(f"   EJECUTANDO PRUEBAS UNITARIAS [{build_type.upper()}]  ")
    print("=" * 40)
    # Usamos ctest para integración con CMake/CTest
    run_command(["ctest", "--output-on-failure"], cwd=build_dir)


# Script principal

if __name__ == "__main__":
    
    # 1. Asegurar que el directorio base exista
    BUILD_BASE_DIR.mkdir(parents=True, exist_ok=True)
    
    # Proceso para la build 'default' (para pruebas)
    BUILD_TYPE = "default"
    print(f"\n===== INICIANDO PROCESO PARA BUILD: {BUILD_TYPE.upper()} =====")
    setup_and_build(BUILD_TYPE)
    list_and_run_tests(BUILD_TYPE)
    
    # Proceso para la build 'clang-tidy' (solo configurar y compilar)
    BUILD_TYPE = "clang-tidy"
    print(f"\n===== INICIANDO PROCESO PARA BUILD: {BUILD_TYPE.upper()} =====")
    setup_and_build(BUILD_TYPE)
    
    print("\n[ÉXITO] Ejecución de configuración y pruebas completada.")