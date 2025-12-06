import sys
import math

# --- LÓGICA DE PARSING PPM P3/P6 (SIN CAMBIOS) ---

def parse_ppm(file_path):
    """
    Lee un archivo PPM (P3 o P6) y devuelve una lista de tuplas RGB.
    """
    
    # Abrir en modo binario 'rb'
    with open(file_path, 'rb') as f:
        # Leer el número mágico (P3 o P6)
        magic_number = f.readline().strip().decode('ascii')
        
        if magic_number not in [b'P3'.decode('ascii'), b'P6'.decode('ascii')]:
            raise ValueError(f"Formato no soportado. Se esperaba 'P3' o 'P6', se encontró '{magic_number}'.")

        # Saltar comentarios
        while True:
            line = f.readline()
            if not line.startswith(b'#'):
                break
        
        # Leer ancho y alto
        try:
            width, height = map(int, line.split())
        except ValueError:
            raise ValueError("Cabecera PPM mal formada (W o H).")

        # Leer valor máximo de color
        max_color = int(f.readline().strip())
        if max_color != 255:
             raise ValueError(f"El valor máximo de color no es 255 (encontrado {max_color}).")
             
        pixels = []
        
        if magic_number == 'P3':
            # --- Lógica de P3 (ASCII) ---
            data = f.read().split()
            if len(data) != width * height * 3:
                raise ValueError("Número de píxeles P3 incorrecto.")
            
            for i in range(0, len(data), 3):
                r = int(data[i].decode('ascii'))
                g = int(data[i+1].decode('ascii'))
                b = int(data[i+2].decode('ascii'))
                pixels.append((r, g, b))

        elif magic_number == 'P6':
            # --- Lógica de P6 (BINARIO) ---
            data = f.read()
            expected_bytes = width * height * 3
            if len(data) != expected_bytes:
                 raise ValueError(f"Número de bytes P6 incorrecto. Esperado {expected_bytes}, encontrado {len(data)}.")
            
            for i in range(0, expected_bytes, 3):
                r = data[i]
                g = data[i+1]
                b = data[i+2]
                pixels.append((r, g, b))

    return pixels

# --- FUNCIÓN PRINCIPAL DE COMPARACIÓN Y SALIDA ---

def compare_images_and_exit(is_manual_run=False):
    
    # 1. Validación de Argumentos
    # is_manual_run se activa si se pasan 2 o 3 argumentos.
    if len(sys.argv) not in [3, 4]:
        print(f"Uso: python3 {sys.argv[0]} <ruta_output.ppm> <ruta_referencia.ppm> [<ruta_diff.ppm>]")
        sys.exit(1)

    img1_path = sys.argv[1] 
    img2_path = sys.argv[2] 
    
    # 2. Parsing de Imágenes
    try:
        pixels1 = parse_ppm(img1_path)
        pixels2 = parse_ppm(img2_path)
    except Exception as e:
        if is_manual_run:
            print(f"❌ ERROR: Fallo al procesar los archivos PPM: {e}")
        # En la ejecución automática, solo se muestra el error capturado por run_test.py
        sys.exit(1)

    if len(pixels1) != len(pixels2):
        if is_manual_run:
            print(f"❌ ERROR: El número de píxeles es diferente.")
        sys.exit(1)

    # 3. Cálculo de Métricas
    max_diff = 0
    sum_squared_diff = 0
    total_pixels = len(pixels1)

    for p1, p2 in zip(pixels1, pixels2):
        diff = (abs(p1[0] - p2[0]) + abs(p1[1] - p2[1]) + abs(p1[2] - p2[2])) / 3.0
        max_diff = max(max_diff, diff)
        squared_diff = sum((c1 - c2)**2 for c1, c2 in zip(p1, p2))
        sum_squared_diff += squared_diff

    rmse = math.sqrt(sum_squared_diff / total_pixels)

    # 4. Umbrales y Decisión
    MAX_DIFF_THRESHOLD = 150
    RMSE_THRESHOLD = 10

    max_diff_pass = max_diff < MAX_DIFF_THRESHOLD
    rmse_pass = rmse < RMSE_THRESHOLD
    
    # 5. Salida de Consola (Solo si es ejecución manual)
    if is_manual_run:
        print(f"✅ Ambas imágenes: {int(math.sqrt(len(pixels1)/3))}x{int(math.sqrt(len(pixels1)/3))}") # Nota: Asumo que las imágenes son cuadradas para la salida
        print(f"\n📊 Resultados:")
        print(f"   Diferencia máxima: {max_diff:.2f}")
        print(f"   Error cuadrático medio (RMSE): {rmse:.2f}")

        print(f"\n📏 Umbrales de aceptación:")
        print(f"   Diferencia máxima < {MAX_DIFF_THRESHOLD}: {'✅ PASS' if max_diff_pass else '❌ FAIL'}")
        print(f"   RMSE < {RMSE_THRESHOLD}: {'✅ PASS' if rmse_pass else '❌ FAIL'}")

    # 6. Salida de Proceso (Obligatoria para la automatización)
    if max_diff_pass and rmse_pass:
        if is_manual_run:
            print(f"\n✅ ¡IMAGEN ACEPTABLE!")
        sys.exit(0) # ÉXITO
    else:
        if is_manual_run:
            print(f"\n❌ Imagen NO cumple umbrales")
        sys.exit(1) # FALLO


if __name__ == "__main__":
    # Si se pasan 2 o 3 argumentos (ejecución manual), se activa la salida detallada.
    # Si se pasan 4 (ejecución automática desde run_test.py), no se imprime nada a menos que haya un ERROR.
    is_manual = len(sys.argv) == 3 
    compare_images_and_exit(is_manual_run=is_manual)