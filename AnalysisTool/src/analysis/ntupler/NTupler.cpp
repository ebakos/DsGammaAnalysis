#include "analysis/ntupler/NTupler.hpp"
#include <iostream>
#include <assert.h>

#ifdef __MAKECINT__
#pragma link C++ class std::vector<std::vector<double>>+;
#endif


NTupler::NTupler(std::string sample_ident, ExRootTreeReader* reader) : consistency(reader) {
    jets = reader->UseBranch("Jet");
    tracks = reader->UseBranch("EFlowTrack");
    branchTower1 = reader->UseBranch("EFlowPhoton");
    branchTower2 = reader->UseBranch("EFlowNeutralHadron");
    towers = reader->UseBranch("Tower");
    printed = 0;

    for (size_t i = 0; i < JET_IMAGE_DIM; ++i) {
        br_jet_image.push_back(std::vector<double>(JET_IMAGE_DIM, 0.0));
    }    
    
    tree = new TTree("DS", "DS tagger ML tuples");
    tree->Branch("delta_eta", &br_delta_eta);
    tree->Branch("delta_phi", &br_delta_phi);
    tree->Branch("n_neutral", &br_n_neutral);
    tree->Branch("n_charged", &br_n_charged);
    tree->Branch("charge", &br_charge);
    tree->Branch("invariant_mass", &br_invariant_mass);
    tree->Branch("btag", &br_btag);
    tree->Branch("e_had_over_e_em", &br_e_had_over_e_em);
    tree->Branch("tau_0", &br_tau_0);
    tree->Branch("tau_1", &br_tau_1);
    tree->Branch("tau_2", &br_tau_2);
    tree->Branch("abs_qj", &br_abs_qj);
    tree->Branch("r_em", &br_r_em);
    tree->Branch("r_track", &br_r_track);
    tree->Branch("f_em", &br_f_em);
    tree->Branch("p_core_1", &br_p_core_1);
    tree->Branch("p_core_2", &br_p_core_2);
    tree->Branch("f_core_1", &br_f_core_1);
    tree->Branch("f_core_2", &br_f_core_2);
    tree->Branch("f_core_3", &br_f_core_3);
    tree->Branch("pt_d_square", &br_pt_d_square);
    tree->Branch("les_houches_angularity", &br_les_houches_angularity);
    tree->Branch("width", &br_width);
    tree->Branch("mass", &br_mass);
    tree->Branch("track_magnitude", &br_track_magnitude);
    tree->Branch("jet_image", &br_jet_image);

    if (sample_ident == "SignalWplus") {
        sample_type = SampleType::SignalWplus;
    } else if (sample_ident == "SignalWminus") {
        sample_type = SampleType::SignalWminus;
    } else if (sample_ident == "BackgroundGG") {
        sample_type = SampleType::BackgroundGG;
    } else if (sample_ident == "BackgroundQQ") {
        sample_type = SampleType::BackgroundQQ;
    } else {
        std::cerr << "Please specify the sampletype for ntupler ->" <<
            " SignalWplus/SignalWminus/BackgroundGG/BackgroundQQ" << std::endl;
        assert(0);
    }
}

bool NTupler::PassCommonJetCuts(Jet* jet) {
    if (abs(jet->Eta) > 2.1) return false;
    if (jet->PT < 30.0) return false;
    //if (jet->PT > 80.0) return false;
    //if (jet->NCharged < 2) return false;
    return true;
}

void NTupler::GetSignalEventJets() {
    num_selected_jets = 0;
    numJets = jets->GetEntriesFast();

    GenParticle* ds = consistency.GetDS();
    if (ds == nullptr) return;

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

    selected_jets[0] = (Jet*) jets->At(mini);
    num_selected_jets = 1;
}

void NTupler::GetBackgroundEventJets() {
    num_selected_jets = 0;
    numJets = jets->GetEntriesFast();

    for (long long i = 0; i < numJets; ++i) {
        Jet *jet = (Jet*) jets->At(i);

        if (!PassCommonJetCuts(jet)) continue;

        if (
            (sample_type == SampleType::BackgroundGG && jet->Flavor == 21) ||
            (sample_type == SampleType::BackgroundQQ && (jet->Flavor > 0 && jet->Flavor < 6))
        ){
            selected_jets[num_selected_jets++] = jet;
        }
    }
}

void NTupler::ProcessEvent() {
    numTracks = tracks->GetEntriesFast();

    if (sample_type == SampleType::SignalWplus || sample_type == SampleType::SignalWminus) {
        GetSignalEventJets();
    } else {
        GetBackgroundEventJets();
    }

    for (size_t i = 0; i < num_selected_jets; ++i) {
        Jet *jet = selected_jets[i];

        Qjet = 0., //jet charge pt weighted
        nCharged = 0;
        Rtrack = 0.; //Average dR weighted with pT
        Rem = 0.; // Average dR weighted with EM energy
        deltaR = 0.;
        SumRtPT = 0.;
        SumPT = 0.;
        z = 0.;
        theta = 0.;
        SPT = 0.;
        LHA = 0.;
        MSS = 0.;
        WDT = 0.;


        make_jet_image(towers, jet, JET_IMAGE_R_SIZE, JET_IMAGE_R_SIZE, JET_IMAGE_DIM);
        /*printed++;

        if (printed == 20) {
            char* a = (char*) malloc(1);
            for(size_t i =0; i < 1000; i++)
                *(a+i) = 'b';
        }*/

        for(int i = 0; i < JET_CONE_N; ++i)
        {
            Eecone[i] = 0.0;
            Econe[i] = 0.0;
            Pcone[i] = 0.0;
            Fcore[i] = 0.0;
            Pcore[i] = 0.0;
        }

        TLorentzVector trackJet, jetMomentum = jet->P4();
        TVector3 jetDir = jetMomentum.Vect();

        //loop through jet constituents:
        for (long long j = 0; j < jet->Constituents.GetEntriesFast(); ++j)
        {
            TObject *object = jet->Constituents.At(j);

            // Check if the constituent is accessible
            if (object == nullptr) continue;

            // It is a track
            if (object->IsA() == Track::Class()) {
                Track* track = (Track *) object;

                trackJet += track->P4();
                deltaR = jetMomentum.DeltaR(track->P4());
                z = track->PT / jet-> PT;
                theta = deltaR / JET_CONE;

                if (track->PT < 0.4)  continue;
                nCharged++;
                Qjet += track->Charge * pow(jetDir.Dot(track->P4().Vect()),0.5); //q jet pT weighted
                Rtrack += track->PT *deltaR; //deltaR pt weighted

                SumPT += pow(jetDir.Dot(track->P4().Vect()),0.5); //used for: Qjet
                SumRtPT += track->PT; //sum of the track pt
            }

            // It is a energy deposit
            else if (object->IsA() == Tower::Class()) {
                Tower* tower = (Tower *) object;

                deltaR = jetMomentum.DeltaR(tower->P4());
                z = tower->ET / jet-> PT;
                theta = deltaR / JET_CONE;
                Rem += tower->Eem *deltaR; //deltaR EM reweighted

                if (deltaR >= JET_CONE) continue;

                Econe[(int)floor(deltaR / JET_CONE_STEP)] += tower->ET; //summing transverse energy in a cone of 0.1, 0.2,  0.3 and 0.4
                Eecone[(int)floor(deltaR / JET_CONE_STEP)] += tower->Eem / TMath::CosH(tower->Eta);  //transverse em energy in cones
            }

            // We don't care about anything else...
            else {
                continue;
            }

            LHA += z * sqrt(theta);
            SPT += z * z; //ptd_square
            WDT += z * theta;
            MSS += z * theta * theta;
        }

        // Pcones
        for (long long j = 0; j < numTracks; ++j) {
            Track *track = (Track *) tracks->At(j);

            deltaR = jetMomentum.DeltaR(track->P4());
            if (deltaR >= JET_CONE) continue;
            Pcone[(int)floor(deltaR / JET_CONE_STEP)] += track->PT;
        }

        // Cumulative summing cones
        for (int k = 1; k < JET_CONE_N; k++) 
        {
            Econe[k] += Econe[k - 1];
            Eecone[k] += Eecone[k - 1];
            Pcone[k] += Pcone[k - 1];
        }

        // jet charge, pT weighted
        Qjet /= (SumPT + std::numeric_limits<double>::epsilon());

        // Average dR weighted with pT
        Rtrack /= (SumRtPT + std::numeric_limits<double>::epsilon());

        // Average dR weighted with EM energy
        Rem /= (Eecone[3] + std::numeric_limits<double>::epsilon());  //Eecone[3] energy in the cone of dR < 0.4
        if (Rem > 1.5) Rem = 1.5;

        // f_em
        double Fem = Eecone[3] / (Econe[3] + std::numeric_limits<double>::epsilon());

        // transverse pT
        for (uint k = 0; k < Pcone.size(); k++)
            Pcone[k] /= (trackJet.Perp() + std::numeric_limits<double>::epsilon());

        // fcores and pcores
        for (int k = 1; k < JET_CONE_N; k++)
        {
            Fcore[k] = Econe[k-1] / (Econe[JET_CONE_N-1] + std::numeric_limits<double>::epsilon());
            Pcore[k] = Pcone[k-1] / (Pcone[JET_CONE_N-1] + std::numeric_limits<double>::epsilon());
        }

        //filling branches:
        br_delta_eta = jet->DeltaEta; //Width of the jet in eta
        br_delta_phi = jet->DeltaPhi; //Width of the jet in phi
        br_n_neutral = jet->NNeutrals; //Netural particle multiplicity
        br_n_charged = nCharged; //Charged  particle multiplocity
        br_charge = abs(jet->Charge); //Absolute value of the total charge
        br_invariant_mass = jet->Mass; //Invariant mass of the Jet
        br_btag = jet->BTag % 2; //btagging
        br_track_magnitude = trackJet.Mag();
        br_abs_qj = abs(Qjet);
        br_r_em = Rem;
        br_r_track = Rtrack;
        br_f_em = Fem;
        br_p_core_1 = Pcore[1];
        br_p_core_2 = Pcore[2];
        br_f_core_1 = Fcore[1];
        br_f_core_2 = Fcore[2];
        br_f_core_3 = Fcore[3];
        br_pt_d_square = SPT;
        br_les_houches_angularity = LHA;
        br_width = WDT;
        br_mass = MSS;
        //extra added variables:
        br_e_had_over_e_em = jet->EhadOverEem;
        br_tau_0 = jet->Tau[0];
        br_tau_1 = jet->Tau[1];
        br_tau_2 = jet->Tau[2];

        tree->Fill();
    }

}

void NTupler::Finalize() {
}


void NTupler::make_jet_image(TClonesArray* towers, Jet *jet, double relative_eta_range, double relative_phi_range, size_t dim)
{
    for(size_t x = 0; x < dim; ++x)
        for(size_t y = 0; y < dim; ++y)
            br_jet_image[x][y] = 0.0;

    double center_eta, center_phi;
    if (jet->NSubJetsTrimmed == 0) {
        //std::cout << "There is no leading subjet" << std::endl;
        center_eta = jet->Eta;
        center_phi = jet->Phi;
    } else {
        //std::cout << "There is a leading subjet" << std::endl;
        center_eta = jet->TrimmedP4[1].Eta();
        center_phi = jet->TrimmedP4[1].Phi();
    }
    //std::cout << "eta: " << center_eta << " phi: " << center_phi << std::endl;

    for (long long j = 0; j < towers->GetEntriesFast(); ++j) {
        TObject *object = towers->At(j);

        // Check if the constituent is accessible
        if (object == nullptr) continue;
        if (object->IsA() == Tower::Class()) {
            Tower *cell = (Tower*) object;

            double delta_eta = cell->Eta - center_eta;
            double delta_phi = calc_delta_phi((double) cell->Phi, center_phi);

            double index_eta_raw = (delta_eta + relative_eta_range) / (2.0 * relative_eta_range);
            if (index_eta_raw < 0.0 || index_eta_raw >= 1.0) continue;
            double index_phi_raw = (delta_phi + relative_phi_range) / (2.0 * relative_phi_range);
            if (index_phi_raw < 0.0 || index_phi_raw >= 1.0) continue;

            //std::cout << "tower eta: " << cell->Eta << " delta_eta: " << delta_eta << " phi: " << cell->Phi << " delta_phi: " << delta_phi << std::endl;
            //std::cout << "      pix: " << (index_eta_raw*dim) << "       piy: " << (index_phi_raw*dim) << " Ev: " << cell->E / std::cosh(cell->Eta) << std::endl << std::endl;

            br_jet_image[(size_t)(index_eta_raw * dim)][(size_t)(index_phi_raw * dim)] += cell->E / std::cosh(cell->Eta) ;
        }
    }

    for (long long j = 0; j < jet->Constituents.GetEntriesFast(); ++j) {
        TObject *object = jet->Constituents.At(j);

        // Check if the constituent is accessible
        if (object == nullptr) continue;
        if (object->IsA() == Track::Class()) {
            Track *track = (Track*) object;

            double delta_eta = track->Eta - center_eta;
            double delta_phi = calc_delta_phi((double) track->Phi, center_phi);

            double index_eta_raw = (delta_eta + relative_eta_range) / (2.0 * relative_eta_range);
            if (index_eta_raw < 0.0 || index_eta_raw >= 1.0) continue;
            double index_phi_raw = (delta_phi + relative_phi_range) / (2.0 * relative_phi_range);
            if (index_phi_raw < 0.0 || index_phi_raw >= 1.0) continue;

            //std::cout << "tower eta: " << cell->Eta << " delta_eta: " << delta_eta << " phi: " << cell->Phi << " delta_phi: " << delta_phi << std::endl;
            //std::cout << "      pix: " << (index_eta_raw*dim) << "       piy: " << (index_phi_raw*dim) << " Ev: " << cell->E / std::cosh(cell->Eta) << std::endl << std::endl;

            if (index_eta_raw * dim >= 1.0)
                br_jet_image[(size_t)(index_eta_raw * dim - 1)][(size_t)(index_phi_raw * dim)] += track->PT / 4.0;
            if (index_eta_raw * dim + 1.0 < dim - 1)
                br_jet_image[(size_t)(index_eta_raw * dim + 1)][(size_t)(index_phi_raw * dim)] += track->PT / 4.0;

            if (index_phi_raw * dim >= 1.0)
                br_jet_image[(size_t)(index_eta_raw * dim)][(size_t)(index_phi_raw * dim - 1)] += track->PT / 4.0;
            if (index_phi_raw * dim + 1.0 < dim - 1)
                br_jet_image[(size_t)(index_eta_raw * dim)][(size_t)(index_phi_raw * dim + 1)] += track->PT / 4.0;
        }
    }
}
