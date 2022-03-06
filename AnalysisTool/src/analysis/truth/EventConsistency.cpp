#include "analysis/truth/EventConsistency.hpp"

#include <cmath>
#include <iostream>
#include <queue>
#include <memory>
#include <random>

#define GENPARTICLE(X) ((GenParticle*) X)

bool TruthEventConsistency::HasPID(long long index, int pid) {
    // Valid particle index passed?
    if (index < 0 || index >= numTruthParticles) return -1;

    // Get particle
    GenParticle *particle = GENPARTICLE(truthParticles->At(index));

    return abs(particle->PID) == pid;
}

long long  TruthEventConsistency::GetParent(long long  index) {
    // Valid particle index passed?
    if (index < 0 || index >= numTruthParticles) return -1;

    // Get particle
    GenParticle *particle = GENPARTICLE(truthParticles->At(index));

    // Valid parent particle?
    if (particle->M1 < 0 || particle->M1 >= numTruthParticles) return -1;

    return particle->M1;
}

long long  TruthEventConsistency::GetParentOfType(long long  index, int pid) {
    // Valid particle index passed?
    if (index < 0 || index >= numTruthParticles) return -1;

    // Get parent
    long long i_parent = GetParent(index);

    // Valid parent?
    if (i_parent < 0 || i_parent > numTruthParticles) return -1;

    // Get parent particle
    GenParticle *parent = GENPARTICLE(truthParticles->At(i_parent));

    // Right PID?
    //std::cout<<"Parent for particle index#"<<index<<"parent index#"<<i_parent<<" PID:"<<parent->PID<<std::endl;
    if (abs(parent->PID) != pid) return -1;

    return i_parent;
}

std::pair<long long, long long> TruthEventConsistency::GetDaughters(long long  index) {
    // Valid particle index passed?
    if (index < 0 || index >= numTruthParticles) return std::make_pair(-1, -1);

    // Get particle
    GenParticle *particle = GENPARTICLE(truthParticles->At(index));

    // Valid parent particle?
    if (particle->D1 < 0 || particle->D1 >= numTruthParticles) return std::make_pair(-1, -1);

    // Special case propagate
    if (particle->D1 == particle->D2) {
        GenParticle *daughter = GENPARTICLE(truthParticles->At(particle->D1));
        if (daughter->PID == particle->PID) {
            // special case where only status changed
            return GetDaughters(particle->D1);
        }
    }

    if (particle->D2 < 0 || particle->D2 > numTruthParticles || particle->D1 == particle->D2) 
        return std::make_pair(particle->D1, -1);

    return std::make_pair(particle->D1, particle->D2);
}

long long TruthEventConsistency::GetSibling(long long index) {
    // Valid particle index passed?
    if (index < 0 || index >= numTruthParticles) return -1;

    // Get parent
    long long i_parent = GetParent(index);

    // Valid parent?
    if (i_parent < 0 || i_parent > numTruthParticles) return -1;

    // Get daughters
    std::pair<long long, long long> i_daughters = GetDaughters(i_parent);

    if (i_daughters.first == index && i_daughters.second != index) {
        return i_daughters.second;
    } else if (i_daughters.second == index && i_daughters.first != index) {
        return i_daughters.first;
    }
    
    // huh?
    return -1;
}

long long  TruthEventConsistency::GetSiblingOfType(long long index, int pid) {
    long long siblingIndex = GetSibling(index);
    if (siblingIndex < 0 || siblingIndex >= numTruthParticles) return -1;

    GenParticle *sibling = GENPARTICLE(truthParticles->At(siblingIndex));
    //std::cout<<"Sibling index#"<<index<<"parent index#"<<siblingIndex<<" PIDL "<<abs(sibling->PID)<<std::endl;
    if (abs(sibling->PID) != pid) return -1;
    return siblingIndex;
}

long long TruthEventConsistency::GetDaughterOfType(long long index, int pid) {
    // Valid particle index passed?
    if (index < 0 || index >= numTruthParticles) return -1;

    // Get daughters
    std::pair<long long, long long> i_daughters = GetDaughters(index);

    if (i_daughters.first == -1) return -1;
    if (HasPID(i_daughters.first, pid)) return i_daughters.first;
    if (i_daughters.second == -1) return -1;
    if (HasPID(i_daughters.second, pid)) return i_daughters.second;
    return -1;
}

void TruthEventConsistency::QuickTreePrint(long long index) {
    if (index > 35) return;

    for(long long i = 0; i < index+3; ++i) {
        GenParticle *p = GENPARTICLE(truthParticles->At(i));

        std::cout << i << "PID: " << p->PID
            << " Status: " << p->Status
            << " Mother1: " << p->M1
            << " Mother2: " << p->M2
            << " Daughter1: " << p->D1
            << " Daughter2: " << p->D2 << std::endl;
    }
    
    std::cout << std::endl << std::endl;
}

TruthEventConsistency::TruthEventConsistency(ExRootTreeReader* reader) {
    w_energy = new TH1D("truth_w_energy", "W energy", 100, 0., 500.);
    w_pt = new TH1D("truth_w_pt", "W pt", 100, 0.0, 100.0);
    w_eta = new TH1D("truth_w_eta", "W eta", 80, -10., 10.0);
    
    ds_energy = new TH1D("truth_ds_energy", "Ds particle energy", 50, 0.0, 100.);
    gamma_energy = new TH1D("truth_gamma_energy", "Gamma particle energy", 50, 0.0, 100.);
    ds_pt = new TH1D("truth_ds_pt", "Ds particle pt", 100, 0.0, 100.0);
    gamma_pt = new TH1D("truth_gamma_pt", "Gamma particle pt", 100, 0.0, 100.0);
    ds_charge = new TH1D("truth_ds_charge", "Ds particle charge", 8, -2, 2);
    
    delta_phi_ds_gamma = new TH1D("truth_delta_phi_ds_gamma", "Delta-Phi Ds-Gamma", 80, -4., 4);
    delta_eta_ds_gamma = new TH1D("truth_delta_eta_ds_gamma", "Delta-Eta Ds-Gamma", 40, 0., 10.0);
    delta_r_ds_gamma = new TH1D("truth_delta_r_ds_gamma", "Delta-R Ds-Gamma", 60, 0., 6.);
    delta_ds_gamma = new TH2D("truth_delta_ds_gamma", "Delta Ds-Gamma", 40, -4., 4, 40, 0., 10.);

    //jet_n = new TH1D("truth_gluon_jet_n", "Truth gluon jet multiplicity", 10, 0., 10.);
    //jet_delta_r = new TH1D("truth_gluon_jet_delta_r", "Delta-R truth jets", 60, 0., 6.);
    //jet_delta_phi = new TH1D("truth_jet_delta_phi", "Delta-Phi truth jets", 80, -4., 4);
    //jet_delta_eta = new TH1D("truth_jet_delta_eta", "Delta-Eta truth jets", 40, 0., 10.0);
    //jet_width_phi = new TH1D("truth_jet_width_phi", "Truth jets width in phi", 25, 0., 0.5);
    //jet_width_eta = new TH1D("truth_jet_wdth_eta", "Truth jets width in eta", 25, 0., 0.5);

    truthParticles = reader->UseBranch("Particle");
    //genJets = reader->UseBranch("GenJet");
    valid_events = 0;
    invalid_events = 0;
}



GenParticle* TruthEventConsistency::GetDS() {
    numTruthParticles = truthParticles->GetEntriesFast();

    for (long long i = 0; i < numTruthParticles; ++i) {
        if (!HasPID(i, PID_PARTICLE_DSPLUS)) continue;

        long long i_photon = GetSiblingOfType(i, PID_PARTICLE_PHOTON);
        long long i_w = GetParentOfType(i, PID_PARTICLE_W);

        if (i_photon >= 0 && i_w >= 0) {
            return (GenParticle*) truthParticles->At(i);
        }
    }

    return nullptr;
}


std::pair<GenParticle*, GenParticle*> TruthEventConsistency::GetBkgParticles(bool quark) {
    numTruthParticles = truthParticles->GetEntriesFast();
    GenParticle *one = nullptr, *two = nullptr;

    for (long long i = 0; i < std::min(numTruthParticles, 50ll); ++i) {
        GenParticle* me = GENPARTICLE(truthParticles->At(i));
        int pid = abs(me->PID);
        if (quark && (pid < 1 || pid > 6)) continue;
        if (!quark && pid != 21) continue;

        // Documentation says status==23 is outgoing of hardest subprocess
        // https://pythia.org/latest-manual/ParticleProperties.html
        if (me->Status != 23) continue;

        if (one != nullptr) {
            two = me;
            break;
        } else {
            one = me;
        }

    }
    
    if (two == nullptr)
        return std::make_pair(nullptr, nullptr);
    else 
        return std::make_pair(one, two);
}


void TruthEventConsistency::ProcessEvent() {
    numTruthParticles = truthParticles->GetEntriesFast();

    bool event_valid = false;
    for (long long i = 0; i < numTruthParticles; ++i) {

        if (!HasPID(i, PID_PARTICLE_DSPLUS)) continue;

        long long i_photon = GetSiblingOfType(i, PID_PARTICLE_PHOTON);
        long long i_w = GetParentOfType(i, PID_PARTICLE_W);

        if (i_photon >= 0 && i_w >= 0 ) {
            GenParticle* ds = GENPARTICLE(truthParticles->At(i));
            GenParticle* photon = GENPARTICLE(truthParticles->At(i_photon));
            GenParticle* w = GENPARTICLE(truthParticles->At(i_w));

            TLorentzVector parent = w->P4();

            w_energy->Fill(parent.E());
            w_pt->Fill(parent.Pt());
            w_eta->Fill(parent.Eta());

            ds_energy->Fill(ds->E);
            ds_pt->Fill(ds->PT);
            ds_charge->Fill(ds->Charge);

            gamma_energy->Fill(photon->E);
            gamma_pt->Fill(photon->PT);

            delta_phi_ds_gamma->Fill(ds->P4().DeltaPhi(photon->P4()));
            delta_eta_ds_gamma->Fill(abs(ds->Eta - photon->Eta));
            delta_r_ds_gamma->Fill(ds->P4().DeltaR(photon->P4()));
            delta_ds_gamma->Fill(ds->P4().DeltaPhi(photon->P4()), abs(ds->Eta - photon->Eta));

            event_valid = true;
            break;
        }

        if (!event_valid) {
            GenParticle* ds = GENPARTICLE(truthParticles->At(i));
            GenParticle* parent = GENPARTICLE(truthParticles->At(ds->M1));
            if (parent->PID == 24 && parent->M1 >= 0) {
                QuickTreePrint(i);
                std::cout << "i: " << i << " gamma:" << i_photon << " w:" << i_w << std::endl;
            }
        }
    }

    if (event_valid) {
        valid_events++;
    }
    else {
        invalid_events++;
    }

    //check truth jets:
    /*numGenJets = genJets->GetEntriesFast(); 
    std::vector<Jet*> truth_jet;
    for (long long i = 0; i < numGenJets; ++i) 
    {
        Jet *jet = (Jet*) genJets->At(i);
        
        truth_jet.push_back(jet);
        jet_width_phi->Fill(jet->DeltaPhi);
        jet_width_eta->Fill(jet->DeltaEta);
    }

    //plot DR between the 
    jet_n -> Fill (truth_jet.size());
    for(unsigned long k = 0; k <truth_jet.size(); k++)
    {   
        if (truth_jet.size() == 0) continue;
        for(unsigned long l = k + 1; l < truth_jet.size(); l++)
        {
            Jet* jet_k = truth_jet[k];
            Jet* jet_l = truth_jet[l];

            jet_delta_r -> Fill(jet_k->P4().DeltaR(jet_l->P4()));
            jet_delta_phi -> Fill(jet_k->P4().DeltaPhi(jet_l->P4()));
            jet_delta_eta -> Fill(abs(jet_k->Eta - jet_l->Eta));
        }
    }*/
}

void TruthEventConsistency::Finalize() {
    std::cout << "Found " << valid_events << " valid events and " << invalid_events << " invalid events." << std::endl;
}