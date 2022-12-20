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
    long long numGenJets;
    TClonesArray *truthParticles;
    TClonesArray *genJets;

    TH1D* w_energy;
    TH1D* w_pt;
    TH1D* w_eta;

    TH1D* ds_energy;
    TH1D* ds_pt;
    TH1D* ds_charge;
    TH1D* gamma_energy;
    TH1D* gamma_pt;

    TH1D* delta_phi_ds_gamma;
    TH1D* delta_eta_ds_gamma;
    TH1D* delta_r_ds_gamma;
    TH2D* delta_ds_gamma;

    TH1D* jet_n;
    //TH1D* jet_delta_r;
    //TH1D* jet_delta_phi;
    //TH1D* jet_delta_eta; 
    //TH1D* jet_width_phi;
    //TH1D* jet_width_eta; 

    unsigned long long valid_events;
    unsigned long long invalid_events;

    bool HasPID(long long index, int pid);
    long long GetParent(long long  index);
    long long GetParentOfType(long long index, int pid);
    std::pair<long long, long long> GetDaughters(long long index);
    long long GetSibling(long long index);
    long long GetSiblingOfType(long long index, int pid);
    long long GetDaughterOfType(long long index, int pid);
    void QuickTreePrint(long long index);

  public:
    TruthEventConsistency(ExRootTreeReader*);
    virtual void ProcessEvent();
    virtual void Finalize();

    GenParticle* GetDS();
    std::pair<GenParticle*, GenParticle*> GetBkgParticles(bool quark);
};