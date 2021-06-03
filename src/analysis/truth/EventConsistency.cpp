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

    // Get parent particle
    GenParticle *parent = GENPARTICLE(truthParticles->At(particle->M1));

    if (parent->M1 >= 0 && parent->M1 < numTruthParticles) {
        GenParticle *grandfather = GENPARTICLE(truthParticles->At(parent->M1));
        if (grandfather->D1 == particle->M1 && grandfather->D2 == particle->M1) {
            return GetParent(particle->M1);
        }
    }

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

        std::cout << i << " " << p->PID
            << " S: " << p->Status
            << " M1: " << p->M1
            << " M2: " << p->M2
            << " D1: " << p->D1
            << " D2: " << p->D2 << std::endl;
    }
    
    std::cout << std::endl << std::endl;
}

TruthEventConsistency::TruthEventConsistency(ExRootTreeReader* reader) {
    parent_energy = new TH1D("parent_energy", "W+W- parent particle energy", 100, 0., 500.);
    wplus_energy = new TH1D("wplus_energy", "W+ particle energy", 100, 90.0, 110.);
    wminus_energy = new TH1D("wminus_energy", "W- particle energy", 100, 90.0, 110.);
    ds_energy = new TH1D("ds_energy", "Ds particle energy", 50, 0.0, 100.);
    gamma_energy = new TH1D("gamma_energy", "Gamma particle energy", 50, 0.0, 100.);

    parent_pt = new TH1D("parent_pt", "W+W- parent particle pt", 100, 0.0, 100.0);
    wplus_pt = new TH1D("wplus_pt", "W+ particle pt", 100, 0.0, 100.0);
    wminus_pt = new TH1D("wminus_pt", "W- particle pt", 100, 0.0, 100.0);
    ds_pt = new TH1D("ds_pt", "Ds particle pt", 100, 0.0, 100.0);
    gamma_pt = new TH1D("gamma_pt", "Gamma particle pt", 100, 0.0, 100.0);

    ds_charge = new TH1D("ds_charge", "Ds particle charge", 8, -2, 2);
    
    wplus_eta = new TH1D("wplus_eta", "Eta W+", 80, -10., 10.0);
    wminus_eta = new TH1D("wminus_eta", "Eta W-", 80, -10., 10.0);

    delta_phi_ds_gamma = new TH1D("delta_phi_ds_gamma", "Delta-Phi Ds-Gamma", 80, -4., 4);
    delta_eta_ds_gamma = new TH1D("delta_eta_ds_gamma", "Delta-Eta Ds-Gamma", 40, 0., 10.0);
    delta_r_ds_gamma = new TH1D("delta_r_ds_gamma", "Delta-R Ds-Gamma", 60, 0., 6.);
    delta_ds_gamma = new TH2D("delta_ds_gamma", "Delta Ds-Gamma", 40, -4., 4, 40, 0., 10.);

    delta_phi_wp_wm = new TH1D("delta_phi_wp_wm", "Delta-Phi W+W-", 80, -4., 4);
    delta_eta_wp_wm = new TH1D("delta_eta_wp_wm", "Delta-Eta W+W-", 40, 0., 10.0);
    delta_r_wp_wm = new TH1D("delta_r_wp_wm", "Delta-R W+W-", 60, 0., 6.);

    truthParticles = reader->UseBranch("Particle");
    valid_events = 0;
    invalid_events = 0;
}

void TruthEventConsistency::ProcessEvent() {
    numTruthParticles = truthParticles->GetEntriesFast();

    bool event_valid = false;
    for (long long i = 0; i < numTruthParticles; ++i) {
        if (!HasPID(i, PID_PARTICLE_DSPLUS)) continue;

        long long i_photon = GetSiblingOfType(i, PID_PARTICLE_PHOTON);
        long long i_w1 = GetParentOfType(i, PID_PARTICLE_W);
        long long i_w2 = GetSiblingOfType(i_w1, PID_PARTICLE_W);

        long long wparent = i_w1;
        
        while(i_w2 == -1) {
            wparent = GetParentOfType(wparent, PID_PARTICLE_W);
            long long radiative_g = GetDaughterOfType(wparent, PID_PARTICLE_PHOTON);

            if (wparent == -1 || radiative_g == -1) break;

            i_w2 = GetSiblingOfType(wparent, PID_PARTICLE_W);
        }

        if (i_photon >= 0 && i_w1 >= 0 && i_w2 >= 0) {
            GenParticle* ds = GENPARTICLE(truthParticles->At(i));
            GenParticle* photon = GENPARTICLE(truthParticles->At(i_photon));
            GenParticle* w1 = GENPARTICLE(truthParticles->At(i_w1));
            GenParticle* w2 = GENPARTICLE(truthParticles->At(i_w2));

            TLorentzVector parent = w1->P4() + w2->P4();

            parent_energy->Fill(parent.E());
            parent_pt->Fill(parent.Pt());

            ds_energy->Fill(ds->E);
            ds_pt->Fill(ds->PT);
            ds_charge->Fill(ds->Charge);

            gamma_energy->Fill(photon->E);
            gamma_pt->Fill(photon->PT);

            if (w1->PID == 24) {
                wplus_energy->Fill(w1->E);
                wplus_pt->Fill(w1->PT);
                wminus_energy->Fill(w2->E);
                wminus_pt->Fill(w2->PT);

                wplus_eta->Fill(w1->Eta);
                wminus_eta->Fill(w2->Eta);
            } else {
                wplus_energy->Fill(w2->E);
                wplus_pt->Fill(w2->PT);
                wminus_energy->Fill(w1->E);
                wminus_pt->Fill(w1->PT);
                
                wplus_eta->Fill(w2->Eta);
                wminus_eta->Fill(w1->Eta);
            }

            delta_phi_ds_gamma->Fill(ds->P4().DeltaPhi(photon->P4()));
            delta_eta_ds_gamma->Fill(abs(ds->Eta - photon->Eta));
            delta_r_ds_gamma->Fill(ds->P4().DeltaR(photon->P4()));
            delta_ds_gamma->Fill(ds->P4().DeltaPhi(photon->P4()), abs(ds->Eta - photon->Eta));

            delta_phi_wp_wm->Fill(w1->P4().DeltaPhi(w2->P4()));
            delta_eta_wp_wm->Fill(abs(w1->Eta - w2->Eta));
            delta_r_wp_wm->Fill(w1->P4().DeltaR(w2->P4()));
            
            event_valid = true;
            break;
        }

        if (!event_valid) {
            GenParticle* ds = GENPARTICLE(truthParticles->At(i));
            GenParticle* parent = GENPARTICLE(truthParticles->At(ds->M1));
            if (parent->PID == 24 && parent->M1 >= 0) {
                QuickTreePrint(i);
                std::cout << "i: " << i << " gamma:" << i_photon << " w1:" << i_w1 << " w2:" << i_w2 << std::endl;
            }
        }
    }

    if (event_valid) {
        valid_events++;
    }
    else {
        invalid_events++;
    }
}

void TruthEventConsistency::Finalize() {
    std::cout << "Found " << valid_events << " valid events and " << invalid_events << " invalid events." << std::endl;
}