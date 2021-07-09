#include "analysis/reconstruction/RecoAnalysis.hpp"

#include <iostream>


RecoAnalysis::RecoAnalysis(ExRootTreeReader* reader) {
    jets = reader->UseBranch("Jet");
    photons = reader->UseBranch("Photon");
    electrons = reader->UseBranch("Electron");
    muons = reader->UseBranch("Muon");
    met = reader->UseBranch("MissingET");

    reco_photon_n = new TH1D("reco_photon_n", "Reconstructed Photon multiplicity", 5, 0., 5.);
    reco_jet_n = new TH1D("reco_jet_n", "Reconstructed Jet multiplicity", 5, 0., 5.);
    reco_electron_n = new TH1D("reco_electron_n", "Reconstructed Electron multiplicity", 5, 0., 5.);
    reco_muon_n = new TH1D("reco_muon_n", "Reconstructed Muon multiplicity", 5, 0., 5.);

    reco_w_photon_pT = new TH1D("reco_w_photon_pT", "Reconstructed p_{T} (#gamma_W)", 100, 0., 100);
    reco_w_photon_eta = new TH1D("reco_w_photon_eta", "Reconstructed #eta (#gamma_W)", 25, 0., 5);
    reco_w_photon_phi = new TH1D("reco_w_photon_phi", "Reconstructed #phi (#gamma_W)", 80, -4., 4.);
    reco_w_jet_pT = new TH1D("reco_w_jet_pT", "Reconstructed p_{T} (Ds_W)", 100, 0., 100);
    reco_w_jet_eta = new TH1D("reco_w_jet_eta", "Reconstructed #eta (Ds_W)", 25, 0., 5);
    reco_w_jet_phi = new TH1D("reco_w_jet_phi", "Reconstructed #phi (Ds_W)", 80, -4., 4.);

    reco_w_mass = new TH1D("reco_w_mass", "Reconstructed W mass", 100, 50., 150.);
    reco_w_pT = new TH1D("reco_w_pT", "Reconstructed p_{T}(W)", 100, 0., 100.);
    reco_w_deltaPhi = new TH1D("reco_w_deltaPhi", "Reconstructed #Delta#phi(D_{s},#gamma)", 80, -4., 4.);
    reco_w_deltaEta = new TH1D("reco_w_deltaEta", "Reconstructed #Delta#eta(D_{s},#gamma)", 50, 0., 10.0);
    reco_w_deltaR = new TH1D("reco_w_deltaR", "Reconstructed #DeltaR(D_{s},#gamma)", 60, 0., 6.);
}

void RecoAnalysis::ProcessEvent() {

    numJets = jets->GetEntriesFast();
    numPhotons = photons->GetEntriesFast();
    numMuons = muons->GetEntriesFast();
    numElectrons = electrons->GetEntriesFast();


    v_photons.clear();
    v_jets.clear();
    //photons
    for (long long i = 0; i < numPhotons; ++i) {
        Photon *ph = (Photon*) photons->At(i);
        //premature photon selection
        if (ph->PT < 20.) continue;
        v_photons.push_back(ph->P4());
    }

    //jets
    for (long long i = 0; i < numJets; ++i) {
        Jet *jet = (Jet*) jets->At(i);
        //premature jet selection
        if (jet->PT < 20.) continue;
        v_jets.push_back(jet->P4());
    }

    //numbers of photons and jets
    reco_photon_n->Fill(v_photons.size());
    reco_jet_n->Fill(v_jets.size());
    reco_electron_n->Fill(numElectrons);
    reco_muon_n->Fill(numMuons);

    if (v_photons.size() == 0 || v_jets.size() == 0) return;
    
    //W reconstruction -> Take the leading one...
    double max_dphi = 0.;
    size_t jet, photon;
    bool found = false;

    for(size_t p = 0; p < v_photons.size(); p++)
    {
        for(size_t j = 0; j < v_jets.size(); j++)
        {
            double dphi = abs(v_photons[p].DeltaPhi(v_jets[j]));
            if (dphi > max_dphi) {
                max_dphi = dphi;
                jet = j;
                photon = p;
                found = true;
            }
        }
    }

    if (!found) return;

    TLorentzVector w = v_photons[photon] + v_jets[jet];

    reco_w_photon_pT->Fill(v_photons[photon].Pt());
    reco_w_photon_eta->Fill(v_photons[photon].Eta());
    reco_w_photon_phi->Fill(v_photons[photon].Phi());
    reco_w_jet_pT->Fill(v_jets[jet].Pt());
    reco_w_jet_eta->Fill(v_jets[jet].Eta());
    reco_w_jet_phi->Fill(v_jets[jet].Phi());

    reco_w_mass->Fill(w.M());
    reco_w_pT->Fill(w.Pt());
    reco_w_deltaPhi->Fill(v_photons[photon].DeltaPhi(v_jets[jet]));
    reco_w_deltaEta->Fill(fabs(v_photons[photon].Eta()-v_jets[jet].Eta()));
    reco_w_deltaR->Fill(v_photons[photon].DeltaR(v_jets[jet]));
}

void RecoAnalysis::Finalize() {}