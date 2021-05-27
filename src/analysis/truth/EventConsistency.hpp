#pragma once

#include "TH1.h"
#include "TClonesArray.h"
#include "classes/DelphesClasses.h"
#include "external/ExRootAnalysis/ExRootTreeReader.h"


#define PID_PARTICLE_DSPLUS 431
#define PID_PARTICLE_W 24
#define PID_PARTICLE_PHOTON 22


class TruthEventConsistency
{
  private:
    long long numTruthParticles;
    TClonesArray *truthParticles;

    TH1D* parent_momentum;
    TH1D* parent_pt;
    TH1D* wplus_momentum;
    TH1D* wminus_momentum;
    TH1D* wplus_pt;
    TH1D* wminus_pt;
    TH1D* ds_momentum;
    TH1D* ds_pt;
    TH1D* gamma_momentum;
    TH1D* gamma_pt;

    unsigned long long valid_events;
    unsigned long long invalid_events;

    bool HasPID(long long index, int pid);
    long long GetParent(long long  index);
    long long GetParentOfType(long long index, int pid);
    long long GetSibling(long long index);
    long long GetSiblingOfType(long long index, int pid);

  public:
    TruthEventConsistency(ExRootTreeReader*);
    void ProcessEvent();
    void Finalize();
};