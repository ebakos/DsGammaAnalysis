#include "external/ExRootAnalysis/ExRootTreeReader.h"
#include "analysis/truth/EventConsistency.hpp"


#include <cstring>
#include <iostream>
#include <string>

#include "TFile.h"
#include "TChain.h"


int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <operation> <in_file> <out_file>" << std::endl;
        std::cout << "Operations: event_consistency" << std::endl;
        return 1;
    }

    std::string operation(argv[1]);
    std::string in_file(argv[2]);
    std::string out_file(argv[3]);

    TChain *chain = new TChain("Delphes");
    chain->Add(in_file.c_str());

    ExRootTreeReader *treeReader = new ExRootTreeReader(chain);

    TFile* out = TFile::Open(out_file.c_str(), "CREATE");

    if (out == nullptr || out->IsZombie()) {
        std::cout << "Error opening output file, does it already exist?" << std::endl;
        return 1;
    }

    if (operation == "event_consistency") {
        TruthEventConsistency consistency(treeReader);

        long long entries = treeReader->GetEntries();
        std::cout << "** Chain contains " << entries << " events." << std::endl;

        for (long long entry = 0; entry < entries; ++entry) {
            treeReader->ReadEntry(entry);

            consistency.ProcessEvent();
        }

        consistency.Finalize();
    } else {
        std::cout << "Unknows operation '" << operation << "'." << std::endl;
        return 1;
    }

    out->Write();
    out->Close();
    delete out;

    return 0;
}