#!/usr/bin/env python3

import os
import subprocess
import sys
from pathlib import Path

# Configuración
TEST_EXECUTABLE = "utest-aos"
BUILD_DIR_NAME = "build"
CURRENT_DIR = Path(__file__).parent
BUILD_DIR = CURRENT_DIR.parent / BUILD_DIR_NAME


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
        print("--- Salida Estándar (stdout) ---")
        print(e.stdout)
        print("--- Salida de Error (stderr) ---")
        print(e.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print(f"ERROR: El comando '{command[0]}' no se encontró. Asegúrate de que está en el PATH.")
        sys.exit(1)

# Pasos de ejecución

def setup_and_build():
    """Configura el entorno de CMake y compila el proyecto."""
    print("=" * 40)
    print("      CONFIGURANDO Y COMPILANDO PROYECTO      ")
    print("=" * 40)

    # 1. Crear directorio de build
    if not BUILD_DIR.exists():
        BUILD_DIR.mkdir()

    # 2. Configurar (cmake ..)
    run_command(["cmake", ".."], cwd=BUILD_DIR)

    # 3. Compilar (make)
    run_command(["make"], cwd=BUILD_DIR)

def list_and_run_tests():
    """Lista las pruebas concretas y luego las ejecuta con CTest."""
    # Para AOS, ejecutamos ctest directamente ya que los tests están registrados en CMake
    print("\n" + "=" * 40)
    print("        LISTA DE PRUEBAS AOS          ")
    print("=" * 40)
    
    # 1. Listar pruebas usando ctest -N
    list_result = run_command(["ctest", "-N"], cwd=BUILD_DIR, check_error=False)
    print(list_result.stdout)
    
    # 2. Ejecutar pruebas específicas de AOS
    print("\n" + "=" * 40)
    print("        EJECUTANDO PRUEBAS AOS        ")
    print("=" * 40)
    
    # Ejecutar solo los tests de AOS usando el filtro de ctest
    run_command(["ctest", "-R", "test_aos_", "--output-on-failure"], cwd=BUILD_DIR)

def run_individual_tests():
    """Ejecuta tests individuales si ctest no está disponible."""
    print("\n" + "=" * 40)
    print("    EJECUTANDO PRUEBAS AOS INDIVIDUALES    ")
    print("=" * 40)
    
    # Test executables for AOS
    test_executables = [
        "utest-aos/test_aos_camera",
        "utest-aos/test_aos_color", 
        "utest-aos/test_aos_image",
        "utest-aos/test_aos_ray",
        "utest-aos/test_main_aos",
        "utest-aos/test_render_aos"
    ]
    
    all_passed = True
    
    for test_exe in test_executables:
        test_path = BUILD_DIR / test_exe
        
        if not test_path.exists():
            print(f"ERROR: El ejecutable de pruebas '{test_path}' no se encontró.")
            all_passed = False
            continue
            
        print(f"\n-> Ejecutando: {test_exe}")
        
        try:
            result = run_command([str(test_path)], cwd=BUILD_DIR, check_error=False)
            
            if result.returncode == 0:
                print(f"   PRUEBA EXITOSA")
            else:
                print(f"   PRUEBA FALLIDA (código: {result.returncode})")
                all_passed = False
                
        except Exception as e:
            print(f"   ERROR: {e}")
            all_passed = False
    
    return all_passed

# Script principal

if __name__ == "__main__":
    setup_and_build()
    
    # Intentar usar ctest primero (método preferido)
    try:
        list_and_run_tests()
    except Exception as e:
        print(f"Falló ejecución con ctest: {e}")
        print("Intentando ejecución individual de tests...")
        success = run_individual_tests()
        if not success:
            sys.exit(1)
    
    print("\n[ÉXITO] Ejecución de pruebas AOS completada.")