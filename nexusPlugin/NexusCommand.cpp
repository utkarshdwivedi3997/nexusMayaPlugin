#include "NexusCommand.h"


#include <maya/MGlobal.h>
#include <list>

const char* meshFlag = "-m", * meshLongFlag = "-mesh";
const char* angleFlag = "-a", * angleLongFlag = "-angle";
const char* grammarFlag = "-g", * grammarLongFlag = "-grammar";
const char* iterFlag = "-i", * iterLongFlag = "-iterFlag";

NexusCommand::NexusCommand() : MPxCommand()
{
}

NexusCommand::~NexusCommand()
{
}

MStatus NexusCommand::doIt(const MArgList& argList)
{
    MStatus status = MStatus::kSuccess;

    MString mesh;
    double angle = 0.0;
    MString grammar;
    int iter = 0;
    // <<<your code goes here>>> 
    MArgDatabase argData(newSyntax(), argList);

    if (argData.isFlagSet(meshFlag))
    {
        argData.getFlagArgument(meshFlag, 0, mesh);
    }

    if (argData.isFlagSet(angleFlag))
    {
        argData.getFlagArgument(angleFlag, 0, angle);
    }

    if (argData.isFlagSet(grammarFlag))
    {
        argData.getFlagArgument(grammarFlag, 0, grammar);
    }

    if (argData.isFlagSet(iterFlag))
    {
        argData.getFlagArgument(iterFlag, 0, iter);
    }

    
    //solver->addObject(std::move(cloth));
    //solver->precomputeConstraints();


    //    // message in scriptor editor
    //    std::string curveName = "curve" + to_string(i);
    //    std::string dialogTxt = "curve -d 1 -p " + getVec3AsString(branch.first) + " - p " + getVec3AsString(branch.second) + " -k 0 -k 1 - name " + curveName + "; select - r nurbsCircle1 " + curveName + "; extrude - ch true - rn false - po 1 - et 2 - ucp 1 - fpt 1 - upn 1 - rotation 0 - scale 1 - rsp 1 \"nurbsCircle1\" \"" + curveName + "\";";
    //    MString dialog = dialogTxt.c_str();
    //    status = MGlobal::executeCommand(dialog);
    //}

    // message in scriptor editor
    std::string dialogTxt = "print(\"hellu\");";// +std::to_string(solver->getObjects().size()) + ";";
    MString dialog = dialogTxt.c_str();
    status = MGlobal::executeCommand(dialog);

    return MStatus::kSuccess;
}

MSyntax NexusCommand::newSyntax()
{
    MSyntax syntax;

    syntax.addFlag(meshFlag, meshLongFlag, MSyntax::kString);
    syntax.addFlag(angleFlag, angleLongFlag, MSyntax::kDouble);
    syntax.addFlag(grammarFlag, grammarLongFlag, MSyntax::kString);
    syntax.addFlag(iterFlag, iterLongFlag, MSyntax::kLong);

    return syntax;
}
