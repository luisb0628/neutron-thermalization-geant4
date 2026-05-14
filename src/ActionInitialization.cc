#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

ActionInitialization::ActionInitialization(const DetectorConstruction* detector)
 : fDetector(detector) {}

ActionInitialization::~ActionInitialization() {}

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
}
