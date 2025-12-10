import os
import subprocess
import sys

# --- CONFIGURACIÓN DE RUTAS Y EJECUTABLES ---
ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
ASSETS_DIR = os.path.join(ROOT_DIR, 'archivos_ejemplo')
REFERENCES_DIR = os.path.join(ASSETS_DIR, 'referencias')
COMPARE_SCRIPT = os.path.join(ROOT_DIR, 'compare_images.py')
TEMP_OUTPUT_DIR = os.path.join(ROOT_DIR, 'test_output')

EXECUTABLES = {
    'par': os.path.join(ROOT_DIR, 'out', 'build', 'default', 'par', 'Release', 'render-par'),
    'soa': os.path.join(ROOT_DIR, 'out', 'build', 'default', 'soa', 'Release', 'render-soa')
}

# --- CASOS DE PRUEBA FUNCIONALES (Pasan si Exit Code == 0) ---
TEST_CASES = [
    (1, 'scene1.txt', 'config1.cfg', 's1.ppm'),
    (2, 'scene2.txt', 'config2.cfg', 's2.ppm'),
    (3, 'scene3.txt', 'config3.cfg', 's3.ppm'),
    (4, 'scene4.txt', 'config4.cfg', 's4.ppm'),
    (5, 'scene5.txt', 'config5.cfg', 's5-par.ppm'),
]

# --- CASOS DE PRUEBA DE ERROR (Pasan si Exit Code != 0) ---
# Usa config_valid.cfg y scene_valid.txt como archivos complementarios
ERROR_TEST_CASES = [
    # Config Errors (Bad Config, Good Scene)
    ('C1', 'scene_valid.txt', 'config1-error.cfg', "Config - 'imagewidth' Negativo"),
    ('C2', 'scene_valid.txt', 'config2-error.cfg', "Config - 'gamma' con Datos Extra"),
    ('C3', 'scene_valid.txt', 'config3-error.cfg', "Config - Clave Desconocida"),
    ('C4', 'scene_valid.txt', 'config4-error.cfg', "Config - 'cameraposition' Faltante"),

    # Scene Errors (Good Config, Bad Scene)
    ('S1', 'scene1-error.txt', 'config_valid.cfg', "Escena - Material Incompleto"),
    ('S2', 'scene2-error.txt', 'config_valid.cfg', "Escena - Esfera Radio Negativo"),
    ('S3', 'scene3-error.txt', 'config_valid.cfg', "Escena - Material Inexistente"),
    ('S4', 'scene4-error.txt', 'config_valid.cfg', "Escena - Entidad Desconocida ('unknown')"),
    ('S5', 'scene5-error.txt', 'config_valid.cfg', "Escena - Redefinición de Nombre ('mat1')"),
]

def run_test(test_id, impl_name, scene_file, config_file, reference_file):
    """Ejecuta un único test funcional (espera código de salida 0)."""
    
    scene_path = os.path.join(ASSETS_DIR, scene_file)
    config_path = os.path.join(ASSETS_DIR, config_file)
    reference_path = os.path.join(REFERENCES_DIR, reference_file)
    
    output_filename = f'test_{impl_name}_{test_id}.ppm'
    output_path = os.path.join(TEMP_OUTPUT_DIR, output_filename)
    
    diff_filename = f'diff_{impl_name}_{test_id}.ppm'
    diff_path = os.path.join(TEMP_OUTPUT_DIR, diff_filename)
    
    executable_path = EXECUTABLES.get(impl_name)
    
    test_name = f"Test {test_id} ({impl_name.upper()} - {scene_file.split('.')[0]})"
    print(f"\n--- Ejecutando {test_name} (Funcional) ---")

    # 1. Ejecutar el Renderer C++ (Comando: [CONFIG] [SCENE] [OUTPUT])
    print(f"  > 1. Renderizando... ({config_file} + {scene_file})") 
    try:
        render_command = [executable_path, config_path, scene_path, output_path] 
        result = subprocess.run(render_command, capture_output=True, text=True, check=False) 
        
        if result.returncode != 0:
            print(f"  [FAIL] El renderer C++ falló (Exit Code: {result.returncode}). Se esperaba 0 (Éxito).")
            last_lines = '\n'.join(result.stderr.splitlines()[-10:])
            print(f"         Últimas líneas de Stderr:\n{last_lines}")
            return False
            
    except FileNotFoundError:
        print(f"  [ERROR] El ejecutable no fue encontrado. Verifique la ruta: {executable_path}")
        return False
    except Exception as e:
        print(f"  [ERROR] Ocurrió un error inesperado al renderizar: {e}")
        return False

    # 2. Comparar la imagen generada con la referencia
    print(f"  > 2. Comparando imágenes con {os.path.basename(COMPARE_SCRIPT)}...")
    try:
        compare_command = [sys.executable, COMPARE_SCRIPT, output_path, reference_path, diff_path]
        
        compare_result = subprocess.run(compare_command, capture_output=True, text=True, check=False)

        if compare_result.returncode == 0:
            print(f"  [PASS] {test_name}: IMÁGENES IDÉNTICAS.")
            return True
        else:
            print(f"  [FAIL] {test_name}: IMÁGENES DIFERENTES.")
            
            # Asegura que la salida del comparador siempre se muestre si falla
            print("         Salida de Stderr/Stdout del comparador:")
            if compare_result.stdout:
                # Aquí es donde aparecerán los valores MaxDiff y RMSE
                print(f"         STDOUT: {compare_result.stdout.strip()}") 
            if compare_result.stderr:
                print(f"         STDERR: {compare_result.stderr.strip()}")
            
            print(f"         Se generó el archivo de diferencia en: {diff_path}")
            return False

    except FileNotFoundError:
        print(f"  [ERROR] El script de comparación no fue encontrado en: {COMPARE_SCRIPT}")
        return False
    except Exception as e:
        print(f"  [ERROR] Ocurrió un error inesperado al comparar: {e}")
        return False

def run_error_test(test_id, impl_name, scene_file, config_file, description):
    """Ejecuta un test de error (espera código de salida != 0)."""

    scene_path = os.path.join(ASSETS_DIR, scene_file)
    config_path = os.path.join(ASSETS_DIR, config_file)
    # Usamos un archivo de salida temporal que será ignorado
    output_path = os.path.join(TEMP_OUTPUT_DIR, f'error_test_{impl_name}_{test_id}.ppm')

    executable_path = EXECUTABLES.get(impl_name)

    test_name = f"Test {test_id} ({impl_name.upper()} - {description})"
    print(f"\n--- Ejecutando {test_name} (Error) ---")

    # Ejecutar el Renderer C++
    print(f"  > 1. Probando fallo con: ({config_file} + {scene_file})")
    try:
        render_command = [executable_path, config_path, scene_path, output_path]
        result = subprocess.run(render_command, capture_output=True, text=True, check=False)

        # La condición de éxito para un test de error es un código de salida distinto de 0
        if result.returncode != 0:
            print(f"  [PASS] {test_name}: Falló como se esperaba (Exit Code: {result.returncode}).")
            # Muestra el error para verificar que sea el correcto
            last_lines = '\n'.join(result.stderr.splitlines()[-10:])
            print(f"         Mensaje de error:\n{last_lines}")
            return True
        else:
            print(f"  [FAIL] {test_name}: Se esperaba un fallo, pero el programa terminó con ÉXITO (Exit Code: 0).")
            return False

    except FileNotFoundError:
        print(f"  [ERROR] El ejecutable no fue encontrado. Verifique la ruta: {executable_path}")
        return False
    except Exception as e:
        print(f"  [ERROR] Ocurrió un error inesperado: {e}")
        return False

def main():
    """Función principal para ejecutar todos los tests."""
    
    os.makedirs(TEMP_OUTPUT_DIR, exist_ok=True)
    
    total_tests = 0
    passed_tests = 0
    
    # --- BUCLE EXTERNO (CORRECTO) ---
    for impl_name in EXECUTABLES.keys():
        
        if not os.path.exists(EXECUTABLES[impl_name]):
            print(f"\n[ATENCIÓN] Ejecutable '{impl_name.upper()}' no encontrado...")
            continue

        # 1. Ejecutar Tests Funcionales (EXISTENTES)
        for test_id, scene_file, config_file, reference_file in TEST_CASES:
            total_tests += 1
            if run_test(test_id, impl_name, scene_file, config_file, reference_file):
                passed_tests += 1

        # 2. Ejecutar Tests de Errores (NUEVOS)
        for test_id, scene_file, config_file, description in ERROR_TEST_CASES:
            total_tests += 1
            if run_error_test(test_id, impl_name, scene_file, config_file, description):
                passed_tests += 1


    # Reporte final
    print("\n================================================")
    print(f"RESULTADOS FINALES: {passed_tests}/{total_tests} Tests superados")
    print("================================================")
    if total_tests > 0 and passed_tests < total_tests:
        print(f"NOTA: {total_tests - passed_tests} fallaron. Revise los logs de FALLO.")
    elif total_tests == 0:
        print("ADVERTENCIA: No se pudo ejecutar ningún test. Verifique las rutas de los ejecutables.")

if __name__ == "__main__":
    main()