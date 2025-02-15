// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtGlobalMinExpression.h                    //
// ************************************************************************* //

#ifndef AVT_GLOBAL_MIN_FILTER_H
#define AVT_GLOBAL_MIN_FILTER_H

#include <avtGhostAwareUnaryMathExpression.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtGlobalMinExpression
//
//  Purpose:
//      An expression that calculates the minimum across the entire mesh and
//      paints the result on every node or zone.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtGlobalMinExpression : public avtGhostAwareUnaryMathExpression
{
  public:
                              avtGlobalMinExpression();
    virtual                  ~avtGlobalMinExpression();

    virtual const char       *GetType(void)   { return "avtGlobalMinExpression"; };
    virtual const char       *GetDescription(void) 
                                              { return "Calculating min across mesh"; };

  protected:
    virtual void              CalculateWithoutGhosts(vtkDataArray *in, 
                                                     std::vector<double> &results_per_component,
                                                     int ncomponents,
                                                     int ntuples) override;
    virtual void              CalculateWithGhosts(vtkDataArray *in, 
                                                  std::vector<double> &results_per_component,
                                                  int ncomponents,
                                                  int ntuples,
                                                  int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                                  vtkDataArray *ghostZones,
                                                  int *nodeShouldBeIgnoredPtr) override;
};


#endif


