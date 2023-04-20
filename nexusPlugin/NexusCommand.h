#pragma once

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <string>
#include <nexus/PBDSolver.h>
#include <nexus/NexusCloth.h>
class NexusCommand : public MPxCommand
{
public:
    NexusCommand();
    virtual ~NexusCommand();
    static void* creator() { return new NexusCommand(); }
    MStatus doIt(const MArgList& args);
    static MSyntax newSyntax();
};
