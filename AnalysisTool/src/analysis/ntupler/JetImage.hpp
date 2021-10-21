#ifndef JET_IMAGE_H
#define JET_IMAGE_H

#include <array>
#include <cmath>
#include "classes/DelphesClasses.h"


template <typename T>
inline T rad_delta_phi(T phi1, T phi2) {
    T result = phi1 - phi2;
    constexpr T _twopi = M_PI*2.;
    result /= _twopi;
    result -= std::round(result);
    result *= _twopi;  // result in [-pi,pi]
    return result;
}

template<size_t size>
std::array<std::array<double, size>, size> make_jet_image(Jet *jet, double relative_eta_range, double relative_phi_range)
{
    std::array<std::array<double, size>, size> my_image = {};

    for (long long j = 0; j < jet->Constituents.GetEntriesFast(); ++j) {
        TObject *object = jet->Constituents.At(j);

        // Check if the constituent is accessible
        if (object == nullptr) continue;
        if (object->IsA() == Tower::Class()) {
            Tower *cell = (Tower*) object;

            double delta_eta = cell->Eta - jet->Eta;
            double delta_phi = rad_delta_phi<double>(jet->Phi, cell->Phi);

            double index_eta_raw = (delta_eta + relative_eta_range) / (2.0 * relative_eta_range);
            if (index_eta_raw < 0.0 || index_eta_raw >= 1.0) continue;
            double index_phi_raw = (delta_phi + relative_phi_range) / (2.0 * relative_phi_range);
            if (index_phi_raw < 0.0 || index_phi_raw >= 1.0) continue;

            my_image[(size_t)(index_eta_raw * size)][(size_t)(index_phi_raw * size)] += cell->E / std::cosh(cell->Eta) ;
        }
    }

    return my_image;
}

#endif
