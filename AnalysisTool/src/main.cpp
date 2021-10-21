#include "external/ExRootAnalysis/ExRootTreeReader.h"
#include "analysis/truth/EventConsistency.hpp"
#include "analysis/reconstruction/RecoAnalysis.hpp"
#include "analysis/ntupler/NTupler.hpp"
#include "analysis/plot/Plot.hpp"

#include <cstring>
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <iomanip>

#include "TFile.h"
#include "TChain.h"


int plotter(std::vector<std::string> files) {
    std::cout << "Running mode plot." << std::endl;

    if (files.size() > 2)
    {
        std::cout<<"More than two input file provided for plotting. This is not implemented (yet..)"<<std::endl;
        return 1;
    }

    Plotter theplotter(files);
    theplotter.ProcessFiles();
    theplotter.Finalize();
    return 0;
}


int analysis(std::string in_file, std::string out_file, std::vector<std::string> tool_names)
{
    std::cout << "Running mode analysis." << std::endl;

    TChain *chain = new TChain("Delphes");
    chain->Add(in_file.c_str());

    ExRootTreeReader *treeReader = new ExRootTreeReader(chain);

    TFile* out = TFile::Open(out_file.c_str(), "CREATE");

    if (out == nullptr || out->IsZombie()) {
        std::cout << "Error opening output file, does it already exist?" << std::endl;
        return 1;
    }

    std::vector<AnalysisTool*> tools;

    for (size_t i = 0; i < tool_names.size(); ++i) {
        auto operation = tool_names[i];

        std::cout << "Adding operation " << operation << "." << std::endl;
        if (operation == "event_consistency") {
            AnalysisTool* tool = (AnalysisTool*) new TruthEventConsistency(treeReader);
            tools.push_back(tool);
        } else if (operation == "reco") {
            AnalysisTool* tool = (AnalysisTool*) new RecoAnalysis(treeReader);
            tools.push_back(tool);
        } else if (operation == "ntupler") {
            if (i == tool_names.size() - 1) {
                std::cerr << "ntupler needs a sample type." << std::endl;
                return 1;
            }

            AnalysisTool* tool = (AnalysisTool*) new NTupler(tool_names[i+1], treeReader);
            i += 1;
            tools.push_back(tool);
        }else {
            std::cout << "Unknown operation '" << operation << "'." << std::endl;
            return 1;
        }
    }

    long long entries = treeReader->GetEntries();
    std::cout << "** Chain contains " << entries << " events." << std::endl;

    for (long long entry = 0; entry < entries; ++entry) {
        if (entry % 1000 == 0) 
            std::cout << "\r" << std::setfill(' ') << std::setw(12) << entry << " processed" << std::flush;
        treeReader->ReadEntry(entry);

        for (auto tool: tools)
            tool->ProcessEvent();
    }
    std::cout << std::endl;

    for (auto tool: tools)
        tool->Finalize();

    for (auto tool: tools)
        delete tool;

    out->Write();
    out->Close();
    delete out;
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Mode analysis, operations: event_consistency, reco, ntupler" << std::endl;
        std::cout << "Usage: " << argv[0] << " analysis <in_file> <out_file> <operation1> [operation2]" << std::endl;
        std::cout << "Usage: " << argv[0] << " analysis <in_file> <out_file> ntupler <SignalWplus,SignalWminus,BackgroundQQ,BackgroundGG>" << std::endl;
        std::cout << "Mode: plot" << std::endl;
        std::cout << "Usage #1: " << argv[0] << " plot <in_file1> <in_file2>" << std::endl;
        std::cout << "Usage #2: " << argv[0] << " plot <in_file1>" << std::endl;
        return 1;
    }

    std::string mode(argv[1]);

    if (mode == "plot") {
        std::vector<std::string> files;

        for (int i = 2; i < argc; ++i)
            files.emplace_back(argv[i]);

        return plotter(files);
    } else if (mode == "analysis") {
        if (argc < 4) {
            std::cout << "Need at least an in_file, out_file and a tool" << std::endl;
            return 1;
        }
        std::string in_file(argv[2]);
        std::string out_file(argv[3]);
        std::vector<std::string> tools;

        for(int i = 4; i < argc; ++i)
            tools.emplace_back(argv[i]);

        return analysis(in_file, out_file, tools);
    } else {
        std::cout << "Unknown mode " << mode << "." << std::endl;
        return 1;
    }
    return 0;
}
