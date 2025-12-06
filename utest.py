#!/usr/bin/env python3

import os
import subprocess
import sys
from pathlib import Path

# Configuración
BUILD_DIR_NAME = "build"
CURRENT_DIR = Path(__file__).parent
BUILD_DIR = CURRENT_DIR / BUILD_DIR_NAME

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
        return None
    except FileNotFoundError:
        print(f"ERROR: El comando '{command[0]}' no se encontró. Asegúrate de que está en el PATH.")
        return None

def setup_and_build():
    """Configura el entorno de CMake y compila el proyecto."""
    print("=" * 50)
    print("      CONFIGURANDO Y COMPILANDO PROYECTO      ")
    print("=" * 50)

    # 1. Crear directorio de build
    if not BUILD_DIR.exists():
        BUILD_DIR.mkdir()

    # 2. Configurar (cmake ..)
    result = run_command(["cmake", ".."], cwd=BUILD_DIR)
    if result is None:
        return False

    # 3. Compilar (make)
    result = run_command(["make"], cwd=BUILD_DIR)
    return result is not None

def run_ctest_tests(test_filter: str = None):
    """Ejecuta pruebas usando CTest con un filtro opcional."""
    if test_filter:
        print(f"\n-> Ejecutando pruebas con filtro: {test_filter}")
        return run_command(["ctest", "-R", test_filter, "--output-on-failure"], cwd=BUILD_DIR)
    else:
        print(f"\n-> Ejecutando todas las pruebas")
        return run_command(["ctest", "--output-on-failure"], cwd=BUILD_DIR)

def run_individual_test(test_path: Path):
    """Ejecuta un test individual."""
    if not test_path.exists():
        print(f"ERROR: El ejecutable de pruebas '{test_path}' no se encontró.")
        return False
        
    print(f"-> Ejecutando: {test_path.name}")
    
    result = run_command([str(test_path)], cwd=BUILD_DIR, check_error=False)
    
    if result and result.returncode == 0:
        print(f"   PRUEBA EXITOSA")
        return True
    else:
        print(f"   PRUEBA FALLIDA (código: {result.returncode if result else 'N/A'})")
        return False

def run_common_tests():
    """Ejecuta las pruebas comunes."""
    print("\n" + "=" * 50)
    print("           PRUEBAS UNITARIAS COMMON           ")
    print("=" * 50)
    
    # Intentar usar ctest primero
    success = True
    try:
        result = run_ctest_tests("test_common_")
        if result is None:
            success = False
    except Exception as e:
        print(f"Falló ejecución con ctest: {e}")
        print("Intentando ejecución individual de tests...")
        
        # Tests individuales para common
        test_executables = [
            "utest-common/test_vector",
            "utest-common/test_utils",
            # Añadir aquí otros tests comunes según se desarrollen
        ]
        
        for test_exe in test_executables:
            test_path = BUILD_DIR / test_exe
            if not run_individual_test(test_path):
                success = False
    
    return success

def run_soa_tests():
    """Ejecuta las pruebas SOA."""
    print("\n" + "=" * 50)
    print("           PRUEBAS UNITARIAS SOA           ")
    print("=" * 50)
    
    # Intentar usar ctest primero
    success = True
    try:
        result = run_ctest_tests("test_soa_")
        if result is None:
            success = False
    except Exception as e:
        print(f"Falló ejecución con ctest: {e}")
        print("Intentando ejecución individual de tests...")
        
        # Tests individuales para SOA
        test_executables = [
            "utest-soa/test_soa_camera",
            "utest-soa/test_soa_color", 
            "utest-soa/test_soa_image",
            "utest-soa/test_soa_ray",
            "utest-soa/test_main_soa",
            "utest-soa/test_render_soa"
        ]
        
        for test_exe in test_executables:
            test_path = BUILD_DIR / test_exe
            if not run_individual_test(test_path):
                success = False
    
    return success

def run_aos_tests():
    """Ejecuta las pruebas AOS."""
    print("\n" + "=" * 50)
    print("           PRUEBAS UNITARIAS AOS           ")
    print("=" * 50)
    
    # Intentar usar ctest primero
    success = True
    try:
        result = run_ctest_tests("test_aos_")
        if result is None:
            success = False
    except Exception as e:
        print(f"Falló ejecución con ctest: {e}")
        print("Intentando ejecución individual de tests...")
        
        # Tests individuales para AOS
        test_executables = [
            "utest-aos/test_aos_camera",
            "utest-aos/test_aos_color", 
            "utest-aos/test_aos_image",
            "utest-aos/test_aos_ray",
            "utest-aos/test_main_aos",
            "utest-aos/test_render_aos"
        ]
        
        for test_exe in test_executables:
            test_path = BUILD_DIR / test_exe
            if not run_individual_test(test_path):
                success = False
    
    return success

def list_all_tests():
    """Lista todas las pruebas disponibles."""
    print("\n" + "=" * 50)
    print("        LISTA DE TODAS LAS PRUEBAS         ")
    print("=" * 50)
    
    list_result = run_command(["ctest", "-N"], cwd=BUILD_DIR, check_error=False)
    if list_result:
        print(list_result.stdout)

# Script principal

if __name__ == "__main__":
    # Configurar y compilar
    if not setup_and_build():
        print("ERROR: Falló la configuración o compilación del proyecto.")
        sys.exit(1)
    
    # Listar todas las pruebas disponibles
    list_all_tests()
    
    # Ejecutar todas las pruebas
    all_success = True
    
    # Pruebas comunes
    if not run_common_tests():
        all_success = False
    
    # Pruebas SOA
    if not run_soa_tests():
        all_success = False
    
    # Pruebas AOS
    if not run_aos_tests():
        all_success = False
    
    # Resumen final
    print("\n" + "=" * 50)
    print("              RESUMEN FINAL               ")
    print("=" * 50)
    
    if all_success:
        print("[ÉXITO] Todas las pruebas se completaron correctamente.")
    else:
        print("[ERROR] Algunas pruebas fallaron.")
        sys.exit(1)