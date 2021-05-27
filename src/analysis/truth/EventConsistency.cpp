#include "analysis/truth/EventConsistency.hpp"

#include <cmath>
#include <iostream>

#define GENPARTICLE(X) ((GenParticle*) X)

bool TruthEventConsistency::HasPID(long long index, int pid) {
    // Valid particle index passed?
    if (index < 0 || index > numTruthParticles) return -1;

    // Get particle
    GenParticle *particle = GENPARTICLE(truthParticles->At(index));

    return abs(particle->PID) == pid;
}

long long  TruthEventConsistency::GetParent(long long  index) {
    // Valid particle index passed?
    if (index < 0 || index > numTruthParticles) return -1;

    // Get particle
    GenParticle *particle = GENPARTICLE(truthParticles->At(index));

    // Valid parent particle?
    if (particle->M1 < 0 || particle->M1 > numTruthParticles) return -1;

    return particle->M1;
}

long long  TruthEventConsistency::GetParentOfType(long long  index, int pid) {
    // Valid particle index passed?
    if (index < 0 || index > numTruthParticles) return -1;

    // Get particle
    GenParticle *particle = GENPARTICLE(truthParticles->At(index));

    // Valid parent particle?
    if (particle->M1 < 0 || particle->M1 > numTruthParticles) return -1;

    // Get parent particle
    GenParticle *parent = GENPARTICLE(truthParticles->At(particle->M1));

    // Right PID?
    if (abs(parent->PID) != pid) return -1;

    return particle->M1;
}

long long  TruthEventConsistency::GetSibling(long long index) {
    // Valid particle index passed?
    if (index < 0 || index > numTruthParticles) return -1;

    // Get particle
    GenParticle *particle = GENPARTICLE(truthParticles->At(index));

    // Valid parent particle?
    if (particle->M1 < 0 || particle->M1 > numTruthParticles) return -1;

    // Get parent particle
    GenParticle *parent = GENPARTICLE(truthParticles->At(particle->M1));

    if (parent->D1 == index) {
        // go for D2
        if (parent->D2 < 0 || parent->D2 > numTruthParticles) return -1;

        return parent->D2;
    } else if (parent->D2 == index) {
        // go for D1
        if (parent->D1 < 0 || parent->D1 > numTruthParticles) return -1;

        return parent->D1;
    }

    // huh? marked as parent but parent does not have this daughter...
    return -1;
}

long long  TruthEventConsistency::GetSiblingOfType(long long index, int pid) {
    long long siblingIndex = GetSibling(index);
    if (siblingIndex < 0 || siblingIndex > numTruthParticles) return -1;

    GenParticle *sibling = GENPARTICLE(truthParticles->At(siblingIndex));

    if (abs(sibling->PID) != pid) return -1;
    return siblingIndex;
}


TruthEventConsistency::TruthEventConsistency(ExRootTreeReader* reader) {
    parent_momentum = new TH1D("parent_momentum", "W+W- parent particle momentum", 100, 10.0, 210.0);
    parent_pt = new TH1D("parent_pt", "W+W- parent particle pt", 100, 10.0, 210.0);
    wplus_momentum = new TH1D("wplus_momentum", "W+ particle momentum", 100, 0.0, 200.0);
    wminus_momentum = new TH1D("wminus_momentum", "W- particle momentum", 100, 0.0, 200.0);
    wplus_pt = new TH1D("wplus_pt", "W+ particle pt", 100, 0.0, 150.0);
    wminus_pt = new TH1D("wminus_pt", "W- particle pt", 100, 0.0, 150.0);
    ds_momentum = new TH1D("ds_momentum", "Ds particle momentum", 50, 0.0, 100.0);
    ds_pt = new TH1D("ds_pt", "Ds particle pt", 50, 0.0, 100.0);
    gamma_momentum = new TH1D("gamma_momentum", "Gamma particle momentum", 50, 0.0, 100.0);
    gamma_pt = new TH1D("gamma_pt", "Gamma particle pt", 50, 0.0, 100.0);

    truthParticles = reader->UseBranch("Particle");
    valid_events = 0;
    invalid_events = 0;
}

void TruthEventConsistency::ProcessEvent() {
    numTruthParticles = truthParticles->GetEntriesFast();

    bool event_valid = true;
    for (long long i = 0; i < numTruthParticles; ++i) {
        if (!HasPID(i, PID_PARTICLE_DSPLUS)) continue;

        long long i_photon = GetSiblingOfType(i, PID_PARTICLE_PHOTON);
        long long i_w1 = GetParentOfType(i, PID_PARTICLE_W);
        long long i_w2 = GetSiblingOfType(i_w1, PID_PARTICLE_W);
        long long i_parent = GetParent(i_w1);

        if (i_photon >= 0 && i_w1 >= 0 && i_w2 >= 0 && i_parent >= 0) {
            GenParticle* ds = GENPARTICLE(truthParticles->At(i));
            GenParticle* photon = GENPARTICLE(truthParticles->At(i_photon));
            GenParticle* w1 = GENPARTICLE(truthParticles->At(i_w1));
            GenParticle* w2 = GENPARTICLE(truthParticles->At(i_w2));
            GenParticle* parent = GENPARTICLE(truthParticles->At(i_parent));

            parent_momentum->Fill(parent->P);
            parent_pt->Fill(parent->PT);

            ds_momentum->Fill(ds->P);
            ds_pt->Fill(ds->PT);

            gamma_momentum->Fill(photon->P);
            gamma_pt->Fill(photon->PT);

            if (w1->PID == 24) {
                wplus_momentum->Fill(w1->P);
                wplus_pt->Fill(w1->PT);
                wminus_momentum->Fill(w2->P);
                wminus_pt->Fill(w2->PT);
            } else {
                wplus_momentum->Fill(w2->P);
                wplus_pt->Fill(w2->PT);
                wminus_momentum->Fill(w1->P);
                wminus_pt->Fill(w1->PT);
            }

            event_valid = true;
            break;
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