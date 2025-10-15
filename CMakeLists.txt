cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(Neutron_Thermalization)

# --- Encontrar Geant4 ---
find_package(Geant4 REQUIRED ui_all vis_all analysis)

# --- Incluir directorios de encabezados ---
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

# --- Archivos fuente ---
set(SOURCES
    src/ActionInitialization.cc
    src/DetectorConstruction.cc
    src/PrimaryGeneratorAction.cc
    src/RunAction.cc
    src/TransmittedSD.cc
    src/EventAction.cc       
)

# --- Ejecutable principal ---
add_executable(Neutron_Thermalization main.cc ${SOURCES})

# --- Enlazar librerías de Geant4 ---
target_link_libraries(Neutron_Thermalization ${Geant4_LIBRARIES})

# --- Copiar macros automáticamente al build ---
file(GLOB MACRO_FILES "${PROJECT_SOURCE_DIR}/macros/run1.mac")
foreach(_file ${MACRO_FILES})
    configure_file(${_file} ${PROJECT_BINARY_DIR}/run1.mac COPYONLY)
endforeach()

# --- Mensajes informativos ---
message(STATUS "Geant4 found at: ${Geant4_DIR}")
message(STATUS "Project built in: ${PROJECT_BINARY_DIR}")

# --- Opcional: instalación ---
install(TARGETS Neutron_Thermalization DESTINATION bin)

