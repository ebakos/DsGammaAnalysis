#include "external/ExRootAnalysis/ExRootTreeReader.h"
#include "analysis/truth/EventConsistency.hpp"


#include <cstring>
#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include "TFile.h"
#include "TChain.h"


int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << "<in_file> <out_file> <operations...>" << std::endl;
        std::cout << "Operations: event_consistency" << std::endl;
        return 1;
    }

    std::string in_file(argv[1]);
    std::string out_file(argv[2]);

    TChain *chain = new TChain("Delphes");
    chain->Add(in_file.c_str());

    ExRootTreeReader *treeReader = new ExRootTreeReader(chain);

    TFile* out = TFile::Open(out_file.c_str(), "CREATE");

    if (out == nullptr || out->IsZombie()) {
        std::cout << "Error opening output file, does it already exist?" << std::endl;
        return 1;
    }

    std::vector<AnalysisTool*> tools;

    for (int i = 3; i < argc; ++i) {
        std::string operation(argv[i]);

        if (operation == "event_consistency") {
            std::cout << "Running operation " << operation << "." << std::endl;
            AnalysisTool* tool = (AnalysisTool*) new TruthEventConsistency(treeReader);
        } else {
            std::cout << "Unknow operation '" << operation << "'." << std::endl;
            return 1;
        }
    }

    long long entries = treeReader->GetEntries();
    std::cout << "** Chain contains " << entries << " events." << std::endl;

    for (long long entry = 0; entry < entries; ++entry) {
        treeReader->ReadEntry(entry);

        for (auto tool: tools)
            tool->ProcessEvent();
    }

    for (auto tool: tools)
        tool->Finalize();

    for (auto tool: tools)
        delete tool;

    out->Write();
    out->Close();
    delete out;

    return 0;
}