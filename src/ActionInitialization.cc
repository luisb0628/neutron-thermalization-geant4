#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"


ActionInitialization::ActionInitialization() {}
ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const
{
    SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
    auto primaryGenerator = new PrimaryGeneratorAction();
    SetUserAction(primaryGenerator);

    auto runAction = new RunAction();
    SetUserAction(runAction);

    auto eventAction = new EventAction(runAction);
    SetUserAction(eventAction);


}
