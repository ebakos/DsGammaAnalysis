#include "analysis/ntupler/NTupler.hpp"

#include <iostream>


NTupler::NTupler(bool signal, ExRootTreeReader* reader) : consistency(reader), signal(signal) {
    jets = reader->UseBranch("Jet");
    truthParticles = reader->UseBranch("Particle");

    tree = new TTree("DS", "DS tagger ML tuples");
    tree->SetBranchAddress("delta_eta", &br_delta_eta);
    tree->SetBranchAddress("delta_phi", &br_delta_phi);
    tree->SetBranchAddress("n_neutral", &br_n_neutral);
    tree->SetBranchAddress("n_charged", &br_n_charged);
    tree->SetBranchAddress("charge", &br_charge);
    tree->SetBranchAddress("invariant_mass", &br_invariant_mass);
    tree->SetBranchAddress("btag", &br_btag);
    tree->SetBranchAddress("e_had_over_e_em", &br_e_had_over_e_em);
    tree->SetBranchAddress("tau_0", &br_tau_0);
    tree->SetBranchAddress("tau_1", &br_tau_1);
    tree->SetBranchAddress("tau_2", &br_tau_2);
    tree->SetBranchAddress("abs_qj", &br_abs_qj);
    tree->SetBranchAddress("r_em", &br_r_em);
    tree->SetBranchAddress("r_track", &br_r_track);
    tree->SetBranchAddress("f_em", &br_f_em);
    tree->SetBranchAddress("p_core_1", &br_p_core_1);
    tree->SetBranchAddress("p_core_2", &br_p_core_2);
    tree->SetBranchAddress("f_core_1", &br_f_core_1);
    tree->SetBranchAddress("f_core_2", &br_f_core_2);
    tree->SetBranchAddress("f_core_3", &br_f_core_3);
    tree->SetBranchAddress("pt_d_square", &br_pt_d_square);
    tree->SetBranchAddress("les_houches_angularity", &br_les_houches_angularity);
    tree->SetBranchAddress("width", &br_width);
    tree->SetBranchAddress("mass", &br_mass);
    tree->SetBranchAddress("track_magnitude", &br_track_magnitude);
}

bool NTupler::PassCommonJetCuts(Jet* jet) {
    if (abs(jet->Eta) > 2.1) return false;
    if (jet->PT < 20.0) return false;
    if (jet->PT > 80.0) return false;
    //if (jet->NCharged < 2) return false;
    return true;
}

void NTupler::GetSignalEventJets() {
    num_selected_jets = 0;
    numJets = jets->GetEntriesFast();
    numTruth = truthParticles->GetEntriesFast();

    long long dsi = consistency->GetDS();
    if (dsi == -1) return;

    GenParticle* ds = (GenParticle*) truthParticles->At(dsi);

    double minr = 0.2;
    long long mini = -1;
    for (long long i = 0; i < numJets; ++i) {
        Jet *jet = (Jet*) jets->At(i);

        if (!PassCommonJetCuts(jet)) continue;

        double r = jet->P4().DeltaR(ds->P4());
        if (r < minr) {
            minr = r;
            mini = i;
        }
    }

    if (mini == -1) return;

    selected_jets[0] = (Jet*) jets->At(i);
    num_selected_jets = 1;
}

void NTupler::GetBackgroundEventJets() {
    num_selected_jets = 0;
    numJets = jets->GetEntriesFast();

    for (long long i = 0; i < numJets; ++i) {
        Jet *jet = (Jet*) jets->At(i);

        if (!PassCommonJetCuts(jet)) continue;

        if (jet->Flavor == 21 || (jet->Flavor > 0 && jet->Flavor < 6)) {
            selected_jets[num_selected_jets++] = jet;
        }
    }
    
}

void NTupler::ProcessEvent() {
    if (signal) {
        GetSignalEventJets();
    } else {
        GetBackgroundEventJets();
    }

    for (size_t i = 0; i < num_selected_jets; ++i) {
        Jet *jet = selected_jets[i];

        br_delta_eta = jet->DeltaEta;
        br_delta_phi = jet->DeltaPhi;
        br_n_neutral = jet->NNeutrals;
        br_n_charged = jet->NCharged;
        br_charge = abs(jet->Charge);
        br_invariant_mass = jet->Mass;
        br_btag = jet->BTag % 2;
        br_e_had_over_e_em = jet->EhadOverEem;
        br_tau_0 = jet->Tau[0];
        br_tau_1 = jet->Tau[1];
        br_tau_2 = jet->Tau[2];

        {
            double deltaR, mass, Eecone[8], Econe[8], Pcone[8];
            double pt, eta, dEta, dPhi;
            double Rem = 0.;
            double Qjet = 0., SumRtPT = 0.;
            double Rtrack = 0, SumPT = 0.;
            double z, theta, SPT=0., LHA=0., MSS=0., WDT=0.;
            TLorentzVector trackJet;

            for (j = 0; j < jet->Constituents.GetEntriesFast(); ++j)
            {
                object = jet->Constituents.At(j);

                // Check if the constituent is accessible

                if (object == nullptr) continue;

                if (object->IsA() == Track::Class()) {
                    track = (Track *) object;
                    trackJet += track->P4();
                    deltaR = jetMomentum.DeltaR(track->P4());
                    z = track->PT / jet-> PT;
                    theta = deltaR / 0.4;
                    if (track->PT > 0.4) {
                        nCharged++;
                        Qjet += track->Charge * pow(jetDir.Dot(track->P4().Vect()),0.5);
                        Rtrack += track->PT *deltaR;
                        SumPT += pow(jetDir.Dot(track->P4().Vect()),0.5);
                        SumRtPT += sqrt(track->PT);
                    }
                } else if (object->IsA() == Tower::Class()) {
                    tower = (Tower *) object;
                    deltaR = jetMomentum.DeltaR(tower->P4());
                    Rem += tower->Eem *deltaR;
                    iBin = floor(deltaR / 0.1);
                    z = tower->ET / jet-> PT;
                    theta = deltaR / 0.4;
                    if (deltaR < 0.4) {
                        for (int ii = iBin; ii < 4; ii++) { Econe[ii] += tower->ET; }
                        for (int ii = iBin; ii < 4; ii++) { 
                            Eecone[ii] += tower->Eem / TMath::CosH(tower->Eta); 
                        }
                    }
                    else if (deltaR < 0.6) {
                        Econe[iBin] += tower->ET;
                        Eecone[iBin] += tower->Eem / TMath::CosH(tower->Eta);
                    }
                } else {
                    continue;
                }

                LHA += z * sqrt(theta); 
                SPT += z * z;
                WDT += z * theta;
                MSS += z * theta * theta;
            }
            Rem /= (Eecone[3] + 0.00001);
            if (Rem > 1.5) Rem = 1.5;
            double Fcore1 = Econe[0] / (Econe[3] + 0.00001);
            double Fcore2 = Econe[1] / (Econe[3] + 0.00001);
            double Fcore3 = Econe[2] / (Econe[3] + 0.00001);
            double Fem = Eecone[3] / (Econe[3] + 0.00001);

            //
            //-- now loop over tracks and make p-cones
            //
            for (j = 0; j < branchTrack->GetEntriesFast(); ++j) {
                track = (Track *) branchTrack->At(j);
                deltaR = jetMomentum.DeltaR(track->P4());
                if (deltaR < 0.6)
                {
                    iBin = floor(deltaR / 0.1);
                    Pcone[iBin] += track->PT;
                }
            }
            //
            //-- all done, scale cones and fill the historgrams
            //
            for (int k = 0; k < 4; k++) Econe[k] /= Econe[3];
            for (int k = 1; k < 7; k++) Pcone[k] += Pcone[k - 1];
            for (int k = 0; k < 4; k++) Pcone[k] /= trackJet.Perp();

            double Pcore1 = Pcone[0] / (Pcone[3] + 0.00001);
            double Pcore2 = Pcone[1] / (Pcone[3] + 0.00001);
            Qjet /= (SumPT + 0.00001);
            Rtrack /= (SumRtPT + 0.000001);

            br_track_magnitude = trackJet.Mag();
            br_abs_qj = abs(Qjet);
            br_r_em = Rem;
            br_r_track = Rtrack;
            br_f_em = Fem;
            br_p_core_1 = Pcore1;
            br_p_core_2 = Pcore2;
            br_f_core_1 = Fcore1;
            br_f_core_2 = Fcore2;
            br_f_core_3 = Fcore3;
            br_pt_d_square = SPT;
            br_les_houches_angularity = LHA;
            br_width = WDT;
            br_mass = MSS;
        }

        tree->fill();
    }

}

void NTupler::Finalize() {
}
