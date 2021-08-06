#pragma once

#include "TClonesArray.h"
#include "classes/DelphesClasses.h"
#include "external/ExRootAnalysis/ExRootTreeReader.h"

#include "analysis/AnalysisTool.hpp"
#include "analysis/truth/EventConsistency.hpp"

#include "TLorentzVector.h"
#include "TFile.h"
#include "TTree.h"

#define MAX_JETS 20
#define JET_CONE 0.4
#define JET_CONE_STEP 0.1
#define JET_CONE_N 4 // 0.4/0.1


class NTupler: AnalysisTool
{
  private:
    TruthEventConsistency consistency;

    long long numJets;
    TClonesArray *jets;

    long long numTracks;
    TClonesArray *tracks;

    TClonesArray *branchTower1;
    TClonesArray *branchTower2;

    TFile* file;
    TTree* tree;

    Jet* selected_jets[MAX_JETS];
    size_t num_selected_jets;
    bool signal;

    void GetBackgroundEventJets();
    void GetSignalEventJets();
    bool PassCommonJetCuts(Jet* jet);
    
    double br_delta_eta;
    double br_delta_phi;
    double br_n_neutral;
    double br_n_charged;
    double br_charge;
    double br_invariant_mass;
    double br_btag;
    double br_e_had_over_e_em;
    double br_tau_0;
    double br_tau_1;
    double br_tau_2;
    double br_abs_qj;
    double br_r_em;
    double br_r_track;
    double br_f_em;
    double br_p_core_1;
    double br_p_core_2;
    double br_f_core_1;
    double br_f_core_2;
    double br_f_core_3;
    double br_pt_d_square;
    double br_les_houches_angularity;
    double br_width;
    double br_mass;
    double br_track_magnitude;

    //variables needed for calculation: 
    double Qjet; //jet charge pt weighted
    double nCharged;
    double Rtrack; //Average dR weighted with pT
    double Rem; // Average dR weighted with EM energy
    double deltaR;
    double SumRtPT;
    double SumPT;

    double z;
    double theta;
    double SPT;
    double LHA;
    double MSS;
    double WDT;


    std::array<double, JET_CONE_N> Econe;
    std::array<double, JET_CONE_N> Eecone;
    std::array<double, JET_CONE_N> Pcone;

    std::array<double, JET_CONE_N> Fcore;
    std::array<double, JET_CONE_N> Pcore;

  public:
    NTupler(bool signal, ExRootTreeReader*);
    virtual void ProcessEvent();
    virtual void Finalize();
};