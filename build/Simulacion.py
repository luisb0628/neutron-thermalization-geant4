import uproot
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys

# ====================================================================
# CONFIGURACIÓN DE LA SIMULACIÓN Y ANÁLISIS
# ====================================================================

# ¡IMPORTANTE! Este número debe coincidir con el /run/beamOn de tu .mac
# Para tu "corrida de prueba", 100,000 es un buen número.
NEUTONES_SIMULADOS_TOTAL = 10000000

# Archivo ROOT de entrada
ARCHIVO_ROOT = "NeutronData.root"
NOMBRE_NTUPLE = "NeutronTracks" # Corregido en la versión anterior

# Columnas que queremos leer
COLUMNAS_A_LEER = [
    "KineticEnergy_eV",
    "FinalTime_ns",
    "TotalTrackLength_mm",
    "FinalPosX_mm",
    "FinalPosY_mm"
]

# Definiciones de energía
E_TERMICO_MAX_EV = 0.0253
E_EPITERMICO_MAX_EV = 1.0

# ====================================================================
# FUNCIÓN 1: CARGAR DATOS
# (Esta función no cambia, se omite por brevedad)
# ====================================================================
def cargar_datos(filename, ntuple_name, columns):
    """Carga los datos del archivo ROOT en un DataFrame de Pandas."""
    print(f"--- Cargando archivo: {filename}...")
    try:
        with uproot.open(filename) as file:
            # Verifica si la Ntuple existe
            if ntuple_name not in file:
                print(f"ERROR: No se encontró la Ntuple '{ntuple_name}' en el archivo.")
                # Imprime las Ntuples sin el ";1" para claridad
                disponibles_limpio = [k.split(';')[0] for k in file.keys()]
                print(f"Ntuples disponibles: {disponibles_limpio}")
                return None
            
            tree = file[ntuple_name]
            
            # Verifica si las columnas existen
            disponibles = set(tree.keys())
            solicitadas = set(columns)
            # Damos un poco de flexibilidad por si alguna columna no está
            columnas_finales = list(solicitadas.intersection(disponibles))
            columnas_faltantes = solicitadas.difference(disponibles)
            
            if columnas_faltantes:
                print(f"ADVERTENCIA: Faltan las siguientes columnas en la Ntuple: {columnas_faltantes}")
                print(f"Se cargarán solo las columnas disponibles: {columnas_finales}")
                if not columnas_finales:
                    print("ERROR: Ninguna de las columnas solicitadas existe. No se puede continuar.")
                    return None

            data_frame = tree.arrays(columnas_finales, library="pd")
            print(f"Datos cargados. {len(data_frame)} neutrones detectados.")
            return data_frame
            
    except FileNotFoundError:
        print(f"ERROR: Archivo no encontrado. Asegúrate de que '{filename}' está en el directorio.")
        return None
    except Exception as e:
        print(f"Ha ocurrido un error inesperado: {e}")
        return None

# ====================================================================
# FUNCIÓN 2: ANALIZAR DATOS (CON PROYECCIÓN)
# ====================================================================
def analizar_datos(df):
    """Realiza el análisis estadístico y la proyección de error."""
    print("\n--- 2. Análisis Estadístico ---")
    
    conteo_detectado = len(df)
    if conteo_detectado == 0:
        print("ADVERTENCIA: No se detectaron neutrones. No se puede continuar el análisis.")
        return None
        
    if "KineticEnergy_eV" not in df.columns:
        print("ERROR: La columna 'KineticEnergy_eV' es esencial y no se encontró.")
        return None

    # --- Clasificación por Energía ---
    df_termalizados = df[df["KineticEnergy_eV"] < E_TERMICO_MAX_EV]
    conteo_termalizado = len(df_termalizados)
    
    df_epitermicos = df[
        (df["KineticEnergy_eV"] >= E_TERMICO_MAX_EV) &
        (df["KineticEnergy_eV"] < E_EPITERMICO_MAX_EV)
    ]
    conteo_epitermico = len(df_epitermicos)
    
    df_rapidos = df[df["KineticEnergy_eV"] >= E_EPITERMICO_MAX_EV]
    conteo_rapido = len(df_rapidos)

    # --- Cálculos de Porcentaje ---
    eficiencia_deteccion = 0.0
    termalizados_sobre_total = 0.0
    eficiencia_termal_decimal = 0.0
    
    if NEUTONES_SIMULADOS_TOTAL > 0:
        eficiencia_deteccion = (conteo_detectado / NEUTONES_SIMULADOS_TOTAL) * 100
        termalizados_sobre_total = (conteo_termalizado / NEUTONES_SIMULADOS_TOTAL) * 100
        eficiencia_termal_decimal = conteo_termalizado / NEUTONES_SIMULADOS_TOTAL
    else:
        print("ADVERTENCIA: NEUTONES_SIMULADOS_TOTAL es 0. Los porcentajes sobre el total serán 0.")

    if conteo_detectado > 0:
        termalizados_sobre_detectados = (conteo_termalizado / conteo_detectado) * 100
    else:
        termalizados_sobre_detectados = 0.0

    # --- Impresión del Reporte ---
    print("=" * 50)
    print("REPORTE DE SIMULACIÓN")
    print("=" * 50)
    print(f"Neutrones Simulados Total:  {NEUTONES_SIMULADOS_TOTAL:,.0f}")
    print(f"Neutrones Detectados (Flujo): {conteo_detectado:,.0f}")
    if NEUTONES_SIMULADOS_TOTAL > 0:
        print(f"  Eficiencia de Detección:  {eficiencia_deteccion:.4f}%")
    print("-" * 50)
    print("DESGLOSE DE DETECCIONES:")
    print(f"  Neutrones Termalizados (< {E_TERMICO_MAX_EV} eV):  {conteo_termalizado:,.0f}")
    print(f"  Neutrones Epitérmicos (a {E_EPITERMICO_MAX_EV} eV): {conteo_epitermico:,.0f}")
    print(f"  Neutrones Rápidos (> {E_EPITERMICO_MAX_EV} eV):    {conteo_rapido:,.0f}")
    print("-" * 50)
    print("EFICIENCIA DE TERMALIZACIÓN:")
    print(f"  % Termalizados (respecto a DETECTADOS): {termalizados_sobre_detectados:.4f}%")
    if NEUTONES_SIMULADOS_TOTAL > 0:
        print(f"  % Termalizados (respecto a TOTAL):      {termalizados_sobre_total:.4f}%")
    print("-" * 50)
    
    print("ESTADÍSTICAS ADICIONALES (PROMEDIOS):")
    # ... (se omite por brevedad, no cambia)
    print(f"  Energía Media (todos):    {df['KineticEnergy_eV'].mean():.3f} eV")
    if conteo_termalizado > 0:
        print(f"  Energía Media (termal.):  {df_termalizados['KineticEnergy_eV'].mean():.5f} eV")
        if 'FinalTime_ns' in df_termalizados.columns:
            print(f"  Tiempo Vuelo (termal.): {df_termalizados['FinalTime_ns'].mean():.2f} ns")
        if 'TotalTrackLength_mm' in df_termalizados.columns:
            print(f"  Long. Traza (termal.):  {df_termalizados['TotalTrackLength_mm'].mean():.2f} mm")

    # ========================================================
    # --- ¡NUEVA SECCIÓN DE PROYECCIÓN ESTADÍSTICA! ---
    # ========================================================
    print("=" * 50)
    print("PROYECCIÓN ESTADÍSTICA")
    print("=" * 50)
    
    if conteo_termalizado > 0:
        error_relativo_actual = 1 / np.sqrt(conteo_termalizado)
        print(f"Estadística Actual (con {conteo_termalizado:,.0f} térmicos):")
        print(f"  Error Relativo Actual:   {error_relativo_actual * 100:.2f}%")
        print("-" * 50)
        
        # Proyección para 1% de error
        target_error_1 = 0.01
        n_termal_deseado_1 = (1 / target_error_1)**2
        n_total_necesario_1 = n_termal_deseado_1 / eficiencia_termal_decimal
        
        print("Proyección para 1% de Error Relativo (Recomendado):")
        print(f"  Neutrones Térmicos Necesarios: {n_termal_deseado_1:,.0f}")
        print(f"  Total de Eventos a Simular:  ~{n_total_necesario_1:,.0f} eventos")

        # Proyección para 0.1% de error
        target_error_01 = 0.001
        n_termal_deseado_01 = (1 / target_error_01)**2
        n_total_necesario_01 = n_termal_deseado_01 / eficiencia_termal_decimal
        
        print("\nProyección para 0.1% de Error Relativo (Alta Precisión):")
        print(f"  Neutrones Térmicos Necesarios: {n_termal_deseado_01:,.0f}")
        print(f"  Total de Eventos a Simular:  ~{n_total_necesario_01:,.0f} eventos")
    
    else:
        print("No se detectaron neutrones termalizados en esta corrida.")
        print("Aumenta el número de NEUTONES_SIMULADOS_TOTAL y vuelve a intentarlo.")

    print("=" * 50)
    
    return df_termalizados, df_epitermicos, df_rapidos


# ====================================================================
# FUNCIÓN 3: GENERAR GRÁFICOS
# (Esta función no cambia, se omite por brevedad)
# ====================================================================
def generar_graficos(df_full):
    """Genera y guarda los gráficos de los resultados."""
    print("\n--- 3. Generando Gráficos ---")
    
    # --- GRÁFICO 1: Espectro de Energía (Log-Log) ---
    if "KineticEnergy_eV" in df_full.columns:
        plt.figure(figsize=(10, 6))
        
        # Definir bines logarítmicos para la energía
        # Evitar log(0) o valores negativos si existen
        energias_positivas = df_full[df_full['KineticEnergy_eV'] > 0]['KineticEnergy_eV']
        if not energias_positivas.empty:
            e_min = max(1e-5, energias_positivas.min())
            e_max = energias_positivas.max()
            
            # Asegurarse de que e_min no sea mayor que e_max
            if e_min < e_max:
                bins_log = np.logspace(np.log10(e_min), np.log10(e_max), 150)
            else:
                bins_log = np.array([e_min, e_max]) # Fallback
            
            plt.hist(energias_positivas, bins=bins_log, color='blue', alpha=0.7)
        
        # Líneas de corte
        plt.axvline(E_TERMICO_MAX_EV, color='red', linestyle='--', label=f'Térmico ({E_TERMICO_MAX_EV} eV)')
        plt.axvline(E_EPITERMICO_MAX_EV, color='green', linestyle='--', label=f'Epitérmico ({E_EPITERMICO_MAX_EV} eV)')
        
        plt.xscale('log')
        plt.yscale('log')
        plt.title('Espectro de Energía de Neutrones Detectados', fontsize=16)
        plt.xlabel('Energía Cinética (eV)', fontsize=12)
        plt.ylabel('Cuentas por bin', fontsize=12)
        plt.legend()
        plt.grid(True, which='both', linestyle=':', linewidth=0.5)
        
        nombre_grafico_1 = "espectro_energia.png"
        plt.savefig(nombre_grafico_1)
        print(f"Gráfico guardado: {nombre_grafico_1}")
        plt.close()
    else:
        print("No se generó 'espectro_energia.png' (falta 'KineticEnergy_eV').")

    # --- GRÁFICO 2: Tiempo de Vuelo (Log-Lin) ---
    if "FinalTime_ns" in df_full.columns:
        plt.figure(figsize=(10, 6))
        plt.hist(df_full['FinalTime_ns'], bins=150, color='purple', alpha=0.7)
        
        plt.yscale('log') # Escala logarítmica en Y es útil para ver los eventos raros
        plt.title('Distribución de Tiempo de Vuelo (Todos los Neutrones)', fontsize=16)
        plt.xlabel('Tiempo de Vuelo (ns)', fontsize=12)
        plt.ylabel('Cuentas', fontsize=12)
        plt.grid(True, which='both', linestyle=':', linewidth=0.5)
        
        nombre_grafico_2 = "tiempo_de_vuelo.png"
        plt.savefig(nombre_grafico_2)
        print(f"Gráfico guardado: {nombre_grafico_2}")
        plt.close()
    else:
        print("No se generó 'tiempo_de_vuelo.png' (falta 'FinalTime_ns').")

    # --- GRÁFICO 3: Perfil del Haz (2D) ---
    if "FinalPosX_mm" in df_full.columns and "FinalPosY_mm" in df_full.columns:
        plt.figure(figsize=(8, 7))
        
        # Crear un histograma 2D
        plt.hist2d(df_full['FinalPosX_mm'], df_full['FinalPosY_mm'], 
                   bins=(100, 100), cmap='viridis', cmin=1) # cmin=1 ignora bines vacíos
                   
        plt.colorbar(label='Cuentas por bin')
        plt.title('Perfil del Haz en el Detector', fontsize=16)
        plt.xlabel('Posición X (mm)', fontsize=12)
        plt.ylabel('Posición Y (mm)', fontsize=12)
        plt.axis('equal') # Asegura que los ejes X e Y tengan la misma escala
        
        nombre_grafico_3 = "perfil_del_haz.png"
        plt.savefig(nombre_grafico_3)
        print(f"Gráfico guardado: {nombre_grafico_3}")
        plt.close()
    else:
        print("No se generó 'perfil_del_haz.png' (faltan 'FinalPosX_mm' o 'FinalPosY_mm').")


# ====================================================================
# SCRIPT PRINCIPAL
# ====================================================================
if __name__ == "__main__":
    
    print("Iniciando análisis de simulación Geant4...")
    
    # 1. Cargar
    df_total = cargar_datos(ARCHIVO_ROOT, NOMBRE_NTUPLE, COLUMNAS_A_LEER)
    
    if df_total is not None:
        # 2. Analizar
        resultados = analizar_datos(df_total)
        
        if resultados is not None:
            # 3. Graficar
            generar_graficos(df_total)
            print("\nAnálisis completado.")
        else:
            print("El análisis estadístico no se pudo completar.")
    else:
        print("El script no pudo continuar debido a un error al cargar los datos.")

