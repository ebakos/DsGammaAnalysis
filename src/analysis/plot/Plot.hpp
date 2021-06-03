#pragma once

#include "TCanvas.h"
#include "TROOT.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TLegend.h"
#include "TArrow.h"
#include "TFile.h"
#include "TLatex.h"
#include "TStyle.h"

#include <string>
#include <vector>


class Plotter
{
  private:
    void myText(Double_t x,Double_t y,Color_t color, const char *text);

    std::vector<std::string> input_files;
    std::vector<std::string> variables;
    std::vector<std::string> titles;
    std::vector<TFile *> files;
    std::string llable;
    std::string reg_lab;

    TH1D *myhist1;
    TH1D *myhist2;

  public:
    Plotter(std::vector<std::string>);
    void ProcessFiles();
    void Finalize();
};