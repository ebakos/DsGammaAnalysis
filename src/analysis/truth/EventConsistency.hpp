#pragma once

#include "TH1.h"
#include "TH2.h"
#include "TClonesArray.h"
#include "classes/DelphesClasses.h"
#include "external/ExRootAnalysis/ExRootTreeReader.h"

#include "analysis/AnalysisTool.hpp"


#define PID_PARTICLE_DSPLUS 431
#define PID_PARTICLE_W 24
#define PID_PARTICLE_PHOTON 22


class TruthEventConsistency: AnalysisTool
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

    TH1D* delta_phi_ds_gamma;
    TH1D* delta_eta_ds_gamma;
    TH1D* delta_r_ds_gamma;
    TH2D* delta_ds_gamma;

    unsigned long long valid_events;
    unsigned long long invalid_events;

    bool HasPID(long long index, int pid);
    long long GetParent(long long  index);
    long long GetParentOfType(long long index, int pid);
    long long GetSibling(long long index);
    long long GetSiblingOfType(long long index, int pid);

  public:
    TruthEventConsistency(ExRootTreeReader*);
    virtual void ProcessEvent();
    virtual void Finalize();
};