#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4Threading.hh"
#include "G4UImanager.hh"
#include "QGSP_BERT_HP.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4Timer.hh"

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"

int main(int argc, char** argv) {
    // Interfaz de usuario (modo interactivo si no hay macro)
    G4UIExecutive* ui = nullptr;
    if (argc == 1) ui = new G4UIExecutive(argc, argv);

    // Crear el Run Manager multihilo (usa todos los núcleos disponibles;
    // cae a modo serial automáticamente si Geant4 no se compiló con MT)
    G4RunManager* runManager =
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::MT, false);
    runManager->SetNumberOfThreads(G4Threading::G4GetNumberOfCores());

    // Construcción del detector
    auto* detector = new DetectorConstruction();
    runManager->SetUserInitialization(detector);

    // Lista de física
    runManager->SetUserInitialization(new QGSP_BERT_HP);

    // Inicialización de acciones (PrimaryGenerator, RunAction, EventAction, etc.)
    runManager->SetUserInitialization(new ActionInitialization(detector));

    // Inicializar el sistema de visualización
    auto* visManager = new G4VisExecutive();
    visManager->Initialize();

    // Obtener el gestor de comandos
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    // Iniciar cronometro
    G4Timer* timer = new G4Timer();
    timer->Start();


    if (!ui) {
        // Modo batch (ejecutar macro desde línea de comandos)
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    } else {
        // Modo interactivo (interfaz gráfica)
        UImanager->ApplyCommand("/control/execute ../macros/vis2.mac");
        ui->SessionStart();
        delete ui;
    }

    //Detener cronometro
    timer->Stop();

       G4cout << "\n" << "************************************************************" << G4endl;
    G4cout << "  Simulación completada." << G4endl;
    G4cout << "  Tiempo Real (Wall Clock): " << timer->GetRealElapsed()  << " segundos." << G4endl;
    G4cout << "  Tiempo de CPU (User):     " << timer->GetUserElapsed()  << " segundos." << G4endl;
    G4cout << "************************************************************" << G4endl;

    delete timer;
    // ----------------------------------------------------


    // Limpieza
    delete visManager;
    delete runManager;
    return 0;
}
