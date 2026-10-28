#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"
#include "GeneratorMessenger.hh"
#include "ModerationConfig.hh"
#include "SteppingAction.hh"

ActionInitialization::ActionInitialization(DetectorConstruction* detector)
 : fDetector(detector), fGeneratorMessenger(new GeneratorMessenger()),
   fModerationMessenger(new ModerationMessenger()) {}

ActionInitialization::~ActionInitialization()
{
    delete fModerationMessenger;
    delete fGeneratorMessenger;
}

void ActionInitialization::BuildForMaster() const
{
    SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
    SetUserAction(new PrimaryGeneratorAction(fDetector));
    auto* runAction = new RunAction();
    SetUserAction(runAction);
    SetUserAction(new EventAction(runAction));
    SetUserAction(new SteppingAction());
}
