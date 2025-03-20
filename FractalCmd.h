#ifndef CreateFractalCmd_H_
#define CreateFractalCmd_H_

#include <maya/MPxCommand.h>
#include <string>

class FractalCmd : public MPxCommand
{
public:
    FractalCmd();
    virtual ~FractalCmd();
    static void* creator() { return new FractalCmd(); }
    MStatus doIt(const MArgList& args);
};

#endif