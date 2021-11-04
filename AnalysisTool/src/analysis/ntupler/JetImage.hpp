#ifndef JET_IMAGE_H
#define JET_IMAGE_H

#include <array>
#include <cmath>
#include <iostream>
#include "classes/DelphesClasses.h"

template <typename T>
inline T calc_delta_phi(T phi1, T phi2) {
  T result = phi1 - phi2;  // same convention as reco::deltaPhi()
  constexpr T _twopi = M_PI*2.;
  result /= _twopi;
  result -= std::round(result);
  result *= _twopi;  // result in [-pi,pi]
  return result;
}


void make_jet_image(TClonesArray* towers, Jet *jet, double relative_eta_range, double relative_phi_range, double *my_image[], size_t dim)
{
    for(size_t x = 0; x < dim; ++x)
        for(size_t y = 0; y < dim; ++y)
            my_image[x * dim+y] = 0.0;

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

            my_image[(size_t)(index_eta_raw * dim) * dim][(size_t)(index_phi_raw * dim)] += cell->E / std::cosh(cell->Eta) ;
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

            my_image[(size_t)(index_eta_raw * dim - 1)][(size_t)(index_phi_raw * dim - 1)] += track->PT / 4.0;
            my_image[(size_t)(index_eta_raw * dim - 1)][(size_t)(index_phi_raw * dim + 1)] += track->PT / 4.0;
            my_image[(size_t)(index_eta_raw * dim + 1)][(size_t)(index_phi_raw * dim - 1)] += track->PT / 4.0;
            my_image[(size_t)(index_eta_raw * dim + 1)][(size_t)(index_phi_raw * dim + 1)] += track->PT / 4.0;
        }
    }
}

#endif
