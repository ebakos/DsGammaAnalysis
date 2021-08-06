//Macro for generating plots for differents set of samples
//Created by ebakos


#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TLegend.h"
#include "TArrow.h"
#include "TLatex.h"

#include<string>
#include<vector>

void myText(Double_t x,Double_t y,Color_t color, const char *text) {
  Double_t tsize=0.04;
  TLatex l; //l.SetTextAlign(12);
  l.SetTextSize(tsize);
  l.SetNDC();
  l.SetTextColor(color);
  l.DrawLatex(x,y,text);
}


void PlotDifferences() {

    std::string output_folder  = "./variable_plots/";

    std::string llable = "ATLAS, #sqrt{s} = 14 TeV";
    std::string reg_lab = "W(D_{s}, #gamma)";

    std::vector<std::string> legend_names = {"W(D_{s}^{-}#gamma)", "W(D_{s}^{+}#gamma)", "gg", "qq"};

    std::vector<std::string> file = {"AnalysisTool/files/ntuple_wminus_ds_gamma.root",
                                     "AnalysisTool/files/ntuple_wplus_ds_gamma.root",
                                     "AnalysisTool/files/ntuple_gg_background.root",
                                     "AnalysisTool/files/ntuple_qq_background.root"};

    std::vector<std::string> variables = {"delta_eta", 
                                          "delta_phi",
                                          "n_neutral",
                                          "n_charged",
                                          "charge",
                                          "invariant_mass",
                                          "btag",
                                          "e_had_over_e_em",
                                          "abs_qj",
                                          "r_em",
                                          "r_track",
                                          "f_em",
                                          "p_core_1",
                                          "p_core_2",
                                          "f_core_1",
                                          "f_core_2",
                                          "f_core_3",
                                          "pt_d_square",
                                          "les_houches_angularity",
                                          "width",
                                          "mass",
                                          "track_magnitude",
                                          "tau_0",
                                          "tau_1",
                                          "tau_2"};

    std::vector<std::string> label = {"#Delta#eta", 
                                      "#Delta#phi",
                                      "n_{0}",
                                      "n_{ch}",
                                      "|Q|",
                                      "m_{j}",
                                      "b tag",
                                      "E_{had}/E_{em}",
                                      "|q_{j}|",
                                      "R_{em}",
                                      "R_{tr}",
                                      "f_{em}",
                                      "p_{core1}",
                                      "p_{core2}",
                                      "f_{core1}",
                                      "f_{core2}",
                                      "f_{core3}",
                                      "(p_{T}^{D})^{2}",
                                      "LHA",
                                      "Width",
                                      "Mass",
                                      "m_{tr}",
                                      "#tau_{0}",
                                      "#tau_{1}",
                                      "#tau_{2}"};
    std::vector<int> bin_number = {20, 20, 20, 5, 3, 30, 2, 20, 20, 20, 20, 10, 10, 10, 10, 10, 10, 10, 10, 20, 10, 20, 20, 10, 10};
    std::vector<std::pair<double,double>> bin_boundaries= { { 0, 0.4}, 
                                                            { 0, 0.4},
                                                            { 0, 20},
                                                            { 0, 5},
                                                            { 0, 3}, //charge
                                                            { 0, 15},
                                                            { 0, 2},
                                                            { 0, 4},
                                                            { 0, 1},
                                                            {0, 1}, //r_em
                                                            {0, 1}, //rTrack
                                                            {0, 1}, //f_em
                                                            {0, 1}, //pcore1
                                                            {0, 1}, //pcore2
                                                            {0, 1}, //fcore1
                                                            {0, 1}, //fcore2
                                                            {0, 1}, //fcore3
                                                            {0, 1}, //P_T_D
                                                            {0, 1}, //LHA
                                                            {0, 1},  //Width
                                                            {0, 0.5},  //Mass
                                                            {0, 10}, //m_tr
                                                            {0, 1}, //tau0
                                                            {0, 0.5}, //tau1
                                                            {0, 0.5}  //tau2
                                                            };
    
    std::vector<TH1D*> hists; 

    std::cout<<"Number of plots: "<<variables.size()<<std::endl;
    if(variables.size() != label.size()) std::cout<<"Label size is wrong: "<<label.size()<<std::endl;
    if(variables.size() != bin_number.size()) std::cout<<"Bin number is wrong: "<<bin_number.size()<<std::endl;
    if(variables.size() != bin_boundaries.size()) std::cout<<"Bin boundaries size is wrong: "<<bin_boundaries.size()<<std::endl;
    if(file.size() != legend_names.size()) std::cout<<"Input file amount and legend names are not matching!"<<std::endl;


    for (int v = 0; v < variables.size(); v++)
    {
        hists.clear();

        std::cout<<"Plotting "<<variables[v]<<std::endl;
        for (int fileNumber = 0; fileNumber < file.size(); fileNumber++)
        {

            TFile *f = new TFile(file[fileNumber].c_str(), "read");
            TTree *t = (TTree*) f->Get("DS");

            //set up variables
            double variable;
            t->SetBranchAddress(variables[v].c_str(), &variable);

            //setup histograms
            TH1D* hist = new TH1D(variables[v].c_str(), "", bin_number[v], bin_boundaries[v].first, bin_boundaries[v].second);

            //looop
            int entries = t->GetEntries();
            for (int i=0; i<entries; i++)
            {
               t->GetEntry(i);
               hist->Fill(variable);
            } 
             

            hists.push_back(hist);
        }

        //plotting 
        TLegend *legend = legend = new TLegend(0.70, 0.68, 0.85, 0.87);
        legend->SetBorderSize(0);
        legend->SetTextFont(43);
        legend->SetTextSize(20);
            
        std::stringstream ylabel;
        ylabel.str("");
        ylabel << "A.U.";

        double max = 0;
        for (int h = 0; h < hists.size(); h++)
        {
            hists[h]->Scale(1. / hists[h]->Integral());
            if (hists[h]->GetMaximum() > max)
            {
                max = hists[h]->GetMaximum();
            }
        }

        std::vector<double> colors = {632, 600, 807, 419};

        for (int h = 0; h < hists.size(); h++)
        {
            hists[h]->SetLineColor(colors[h]);
            hists[h]->SetLineWidth(2);
            hists[h]->GetYaxis()->SetTitle(ylabel.str().c_str());
            hists[h]->GetXaxis()->SetTitle(label[v].c_str());
            hists[h]->SetMinimum(0.);
            hists[h]->SetMaximum(1.5 * max);
            legend->AddEntry(hists[h], legend_names[h].c_str(), "l");

        }


        TCanvas *c = new TCanvas("plot", "plot", 800, 600);
        gStyle->SetOptStat(0);
        c->SetLeftMargin(0.15);
        hists[0]->Draw("HIST");


        for (int h =  1; h < hists.size(); h++)
        {
            hists[h]->Draw("HIST SAME");
        }
        
        myText(0.2, 0.85, 1, llable.c_str());  //0.83
        myText(0.2, 0.80, 1, reg_lab.c_str()); //0.78
        legend->Draw();

        c->Update();
        std::string pdfname = output_folder + variables[v] + ".pdf";
        //c->SaveAs(pdfname.c_str());
        std::string pngname = output_folder + variables[v] + ".png";
        c->SaveAs(pngname.c_str());
        c->Close();
    }
}
