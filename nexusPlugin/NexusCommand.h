#pragma once
#include "NexusSolverNode.h"
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <string>

class NexusCommand : public MPxCommand
{
public:
    NexusCommand();
    virtual ~NexusCommand();
    static void* creator() { return new NexusCommand(); }
    MStatus doIt(const MArgList& args);
    static MSyntax newSyntax();
};
