#pragma once

#include "TClonesArray.h"
#include "classes/DelphesClasses.h"
#include "external/ExRootAnalysis/ExRootTreeReader.h"

#include "analysis/AnalysisTool.hpp"

#include "TLorentzVector.h"
#include "TH1.h"
#include "TH2.h"


class RecoAnalysis: AnalysisTool
{
  private:
    long long numPhotons;
    TClonesArray *photons;
    
    long long numJets;
    TClonesArray *jets;

    long long numElectrons;
    TClonesArray *electrons;

    long long numMuons;
    TClonesArray *muons;

    TClonesArray *met;

    std::vector<TLorentzVector> v_jets; 
    std::vector<TLorentzVector> v_photons; 


    TH1D* reco_photon_n;
    TH1D* reco_jet_n;
    TH1D* reco_electron_n;
    TH1D* reco_muon_n;

    TH1D* reco_w_photon_pT;
    TH1D* reco_w_photon_eta;
    TH1D* reco_w_photon_phi;
    TH1D* reco_w_jet_pT;
    TH1D* reco_w_jet_eta;
    TH1D* reco_w_jet_phi;

    TH1D* reco_w_mass;
    TH1D* reco_w_pT;
    TH1D* reco_w_deltaPhi;
    TH1D* reco_w_deltaEta;
    TH1D* reco_w_deltaR;

  public:
    RecoAnalysis(ExRootTreeReader*);
    virtual void ProcessEvent();
    virtual void Finalize();
};