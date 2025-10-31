import uproot
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys
import os

# --- 1. CONFIGURACIÓN INICIAL ---
# ¡IMPORTANTE! Ajusta este número al total de /run/beamOn que usaste
NEUTONES_SIMULADOS_TOTAL = 1000

# Archivo ROOT y Ntuple (Ajusta si tienen nombres diferentes)
ARCHIVO_ROOT = "NeutronData.root"
NOMBRE_NTUPLE = "NeutronTracks" 

# Definiciones de Energía (en eV)
E_TERMICA_MAX_EV = 0.025  # Límite clásico de energía térmica
E_EPITERMICA_MAX_EV = 1.0   # Límite para epitérmicos (puedes ajustarlo)

# --- 2. FUNCIONES DE ANÁLISIS Y GRÁFICOS ---

def cargar_datos(archivo_root, nombre_ntuple):
    """
    Carga los datos de la Ntuple del archivo ROOT.
    Detecta automáticamente si los datos angulares están presentes.
    """
    if not os.path.exists(archivo_root):
        print(f"ERROR: Archivo no encontrado. Asegúrate de que '{archivo_root}' está en el directorio.")
        return None, False

    with uproot.open(archivo_root) as file:
        if nombre_ntuple not in file:
            print(f"ERROR: No se encontró la Ntuple '{nombre_ntuple}' en el archivo.")
            disponibles = [key.split(';')[0] for key in file.keys() if isinstance(file[key], uproot.TTree)]
            print(f"Ntuples disponibles: {disponibles}")
            return None, False
        
        ntuple = file[nombre_ntuple]
        
        # Columnas que siempre deben estar
        columnas_base = [
            "KineticEnergy_eV", "FinalTime_ns", "TotalTrackLength_mm",
            "FinalPosX_mm", "FinalPosY_mm", "NumSteps"
        ]
        
        # Columnas nuevas (opcionales)
        columnas_angulares = ["DirX", "DirY", "DirZ"]
        
        columnas_disponibles = ntuple.keys()
        
        # Verificar que las columnas base existan
        for col in columnas_base:
            if col not in columnas_disponibles:
                print(f"ERROR: Falta la columna base '{col}' en la Ntuple.")
                return None, False
        
        columnas_a_cargar = columnas_base
        hay_datos_angulares = True
        
        # Verificar si las columnas angulares existen
        for col in columnas_angulares:
            if col not in columnas_disponibles:
                hay_datos_angulares = False
                print(f"Aviso: No se encontró la columna angular '{col}'. Se omitirá el análisis angular.")
                break
        
        if hay_datos_angulares:
            columnas_a_cargar.extend(columnas_angulares)
            print("Datos angulares (DirX, DirY, DirZ) encontrados y cargados.")
        
        # Cargar los datos en un DataFrame de Pandas
        df = ntuple.arrays(columnas_a_cargar, library="pd")
        return df, hay_datos_angulares

def generar_histograma_energia(df_todos, df_termicos):
    """Genera un histograma log-log del espectro de energía."""
    plt.figure(figsize=(10, 6))
    
    # Definir bines logarítmicos de 1e-3 (0.001 eV) a 1e7 (10 MeV)
    bins_log = np.logspace(np.log10(1e-3), np.log10(1e7), 200)
    
    # Histograma de todos los neutrones
    plt.hist(df_todos['KineticEnergy_eV'], bins=bins_log, histtype='step', 
             label=f'Todos los Neutrones (Total: {len(df_todos)})', color='blue')
    
    # Histograma de neutrones térmicos
    if not df_termicos.empty:
        plt.hist(df_termicos['KineticEnergy_eV'], bins=bins_log, histtype='stepfilled', 
                 label=f'Térmicos (< {E_TERMICA_MAX_EV} eV) (Total: {len(df_termicos)})', 
                 color='orange', alpha=0.7)

    plt.axvline(E_TERMICA_MAX_EV, color='red', linestyle='--', label=f'Corte Térmico ({E_TERMICA_MAX_EV} eV)')
    
    plt.xscale('log')
    plt.yscale('log')
    plt.title('Espectro de Energía de Neutrones en el Detector')
    plt.xlabel('Energía Cinética (eV)')
    plt.ylabel('Cuentas / Bin')
    plt.legend()
    plt.grid(True, which="both", ls="--", alpha=0.5)
    plt.savefig('espectro_energia.png', dpi=300)
    print("Gráfico 'espectro_energia.png' guardado.")
    plt.close()

def generar_histograma_tof(df_todos, df_termicos):
    """Genera un histograma del Tiempo de Vuelo (ToF)."""
    plt.figure(figsize=(10, 6))
    
    # Usar bines lineales para el tiempo
    max_time = df_todos['FinalTime_ns'].quantile(0.99) # Ignorar outliers
    bins_lin = np.linspace(0, max_time, 150)
    
    plt.hist(df_todos['FinalTime_ns'], bins=bins_lin, histtype='step', 
             label=f'Todos los Neutrones', color='blue')
    
    if not df_termicos.empty:
        plt.hist(df_termicos['FinalTime_ns'], bins=bins_lin, histtype='stepfilled', 
                 label=f'Neutrones Térmicos', color='orange', alpha=0.7)

    plt.title('Tiempo de Vuelo (ToF) de Neutrones en el Detector')
    plt.xlabel('Tiempo (nanosegundos)')
    plt.ylabel('Cuentas / Bin')
    plt.legend()
    plt.grid(True, ls="--", alpha=0.5)
    plt.savefig('tiempo_de_vuelo.png', dpi=300)
    print("Gráfico 'tiempo_de_vuelo.png' guardado.")
    plt.close()

def generar_perfil_haz(df_todos):
    """Genera un mapa de calor 2D del perfil del haz."""
    plt.figure(figsize=(8, 7))
    
    # Histograma 2D
    plt.hist2d(df_todos['FinalPosX_mm'], df_todos['FinalPosY_mm'], 
               bins=(100, 100), cmap='plasma', cmin=1) # cmin=1 ignora bines vacíos

    plt.colorbar(label='Cuentas de Neutrones')
    plt.title('Perfil del Haz en el Detector (Vista Frontal)')
    plt.xlabel('Posición X (mm)')
    plt.ylabel('Posición Y (mm)')
    plt.gca().set_aspect('equal', adjustable='box')
    plt.savefig('perfil_del_haz.png', dpi=300)
    print("Gráfico 'perfil_del_haz.png' guardado.")
    plt.close()

def generar_histograma_angular(df_todos, df_termicos):
    """
    Genera un histograma de la distribución angular (cos theta).
    cos(theta) es simplemente el componente Z de la dirección (DirZ).
    """
    if 'DirZ' not in df_todos.columns:
        print("No se puede generar gráfico angular, falta la columna 'DirZ'.")
        return

    plt.figure(figsize=(10, 6))
    
    # Bines lineales de -1 a 1 (rango completo de cos(theta))
    bins_lin = np.linspace(-1, 1, 100)
    
    # Histograma de Todos los Neutrones
    plt.hist(df_todos['DirZ'], bins=bins_lin, histtype='step', 
             label=f'Todos los Neutrones', color='blue', density=True)
    
    # Histograma de Neutrones Térmicos
    if not df_termicos.empty:
        plt.hist(df_termicos['DirZ'], bins=bins_lin, histtype='stepfilled', 
                 label=f'Neutrones Térmicos', color='orange', alpha=0.7, density=True)

    plt.title('Distribución Angular de Neutrones en el Detector')
    plt.xlabel('Coseno del Ángulo de Salida ($cos \theta$ o DirZ)')
    plt.ylabel('Cuentas (Normalizado)')
    plt.legend()
    plt.grid(True, ls="--", alpha=0.5)
    plt.savefig('distribucion_angular.png', dpi=300)
    print("Gráfico 'distribucion_angular.png' guardado.")
    plt.close()

def imprimir_estadisticas(df_todos, df_termicos, df_epitermicos, df_rapidos, hay_datos_angulares):
    """Imprime un resumen estadístico en la terminal."""
    
    conteo_detectado = len(df_todos)
    conteo_termalizado = len(df_termicos)
    conteo_epitermico = len(df_epitermicos)
    conteo_rapido = len(df_rapidos)

    if conteo_detectado == 0:
        print("ERROR: No se detectaron neutrones. No se pueden calcular estadísticas.")
        return

    # --- Cálculos de Porcentajes ---
    porc_deteccion_total = (conteo_detectado / NEUTONES_SIMULADOS_TOTAL) * 100
    
    porc_termal_detectado = (conteo_termalizado / conteo_detectado) * 100
    porc_epi_detectado = (conteo_epitermico / conteo_detectado) * 100
    porc_rapido_detectado = (conteo_rapido / conteo_detectado) * 100
    
    porc_termal_total = (conteo_termalizado / NEUTONES_SIMULADOS_TOTAL) * 100

    # --- Salida en Terminal ---
    print("\n" + "="*50)
    print("  INFORME DE ANÁLISIS DE TERMALIZACIÓN DE NEUTRONES")
    print("="*50)
    print(f"Neutrones Simulados Total:  {NEUTONES_SIMULADOS_TOTAL:,.0f}")
    print(f"Archivo Analizado:          {ARCHIVO_ROOT} (Ntuple: {NOMBRE_NTUPLE})")
    print("-"*50)
    print("1. CONTEO DE DETECCIÓN (FLUJO)")
    print(f"  Neutrones Detectados (Total):  {conteo_detectado:,.0f}")
    print(f"  Eficiencia de Detección (Flujo): {porc_deteccion_total:.4f}%")
    print("\n2. DESGLOSE DE ENERGÍA (RESPECTO A DETECTADOS)")
    print(f"  Térmicos   (< {E_TERMICA_MAX_EV} eV): {conteo_termalizado:,.0f}  ({porc_termal_detectado:.2f}%)")
    print(f"  Epitérmicos (entre {E_TERMICA_MAX_EV} y {E_EPITERMICA_MAX_EV} eV): {conteo_epitermico:,.0f}  ({porc_epi_detectado:.2f}%)")
    print(f"  Rápidos    (> {E_EPITERMICA_MAX_EV} eV):  {conteo_rapido:,.0f}  ({porc_rapido_detectado:.2f}%)")
    print("\n3. EFICIENCIA DE TERMALIZACIÓN (RESPECTO A SIMULADOS)")
    print(f"  % Termalizados (respecto a TOTAL):      {porc_termal_total:.4f}%")
    print("-"*50)
    print("4. ESTADÍSTICAS ADICIONALES (PROMEDIOS)")
    
    with pd.option_context('display.float_format', '{:,.2f}'.format):
        stats_df = df_todos[['KineticEnergy_eV', 'FinalTime_ns', 'TotalTrackLength_mm', 'NumSteps']]
        print(stats_df.describe().loc[['mean', 'std', 'min', 'max']])
    
    if hay_datos_angulares:
        # Calcular el Coseno de Theta promedio (DirZ)
        cos_theta_promedio = df_todos['DirZ'].mean()
        cos_theta_term_promedio = df_termicos['DirZ'].mean() if conteo_termalizado > 0 else 0
        print("\nEstadísticas Angulares:")
        print(f"  cos(theta) Promedio (Todos):    {cos_theta_promedio:.4f}")
        print(f"  cos(theta) Promedio (Térmicos): {cos_theta_term_promedio:.4f}")

    # --- Proyección Estadística ---
    if conteo_termalizado > 0:
        objetivo_conteo = 10000  # Objetivo para 1% de error estadístico (1/sqrt(N))
        eficiencia_termal = conteo_termalizado / NEUTONES_SIMULADOS_TOTAL
        eventos_necesarios = objetivo_conteo / eficiencia_termal
        
        print("-"*50)
        print("5. PROYECCIÓN ESTADÍSTICA")
        print(f"  Eficiencia de Termalización (Total): {eficiencia_termal:.6f}")
        print(f"  Para lograr {objetivo_conteo:,} neutrones térmicos (error ~1%):")
        print(f"  -> Necesitas simular un total de {eventos_necesarios:,.0f} eventos.")
    
    print("="*50)

# --- 3. EJECUCIÓN PRINCIPAL DEL SCRIPT ---

def main():
    print("Iniciando análisis de simulación Geant4...")
    print(f"--- Cargando archivo: {ARCHIVO_ROOT}...")
    
    df_neutrones, hay_datos_angulares = cargar_datos(ARCHIVO_ROOT, NOMBRE_NTUPLE)
    
    if df_neutrones is None:
        print("El script no pudo continuar debido a un error al cargar los datos.")
        sys.exit(1)
        
    print(f"¡Carga exitosa! Se encontraron {len(df_neutrones)} neutrones detectados.")
    print("--- Clasificando energías...")

    # Filtrar por categorías de energía
    df_termicos = df_neutrones[
        df_neutrones['KineticEnergy_eV'] < E_TERMICA_MAX_EV
    ]
    
    df_epitermicos = df_neutrones[
        (df_neutrones['KineticEnergy_eV'] >= E_TERMICA_MAX_EV) &
        (df_neutrones['KineticEnergy_eV'] < E_EPITERMICA_MAX_EV)
    ]
    
    df_rapidos = df_neutrones[
        df_neutrones['KineticEnergy_eV'] >= E_EPITERMICA_MAX_EV
    ]
    
    print("--- Generando gráficos...")
    generar_histograma_energia(df_neutrones, df_termicos)
    generar_histograma_tof(df_neutrones, df_termicos)
    generar_perfil_haz(df_neutrones)
    
    # Solo genera el gráfico angular si los datos existen
    if hay_datos_angulares:
        generar_histograma_angular(df_neutrones, df_termicos)
    
    print("--- Calculando estadísticas...")
    imprimir_estadisticas(df_neutrones, df_termicos, df_epitermicos, df_rapidos, hay_datos_angulares)
    
    print("\nAnálisis completado.")

if __name__ == "__main__":
    main()
