#include "analysis/plot/Plot.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>

void Plotter::myText(Double_t x,Double_t y,Color_t color, const char *text) {
  Double_t tsize=0.04;
  TLatex l; //l.SetTextAlign(12);
  l.SetTextSize(tsize);
  l.SetNDC();
  l.SetTextColor(color);
  l.DrawLatex(x,y,text);
}

Plotter::Plotter(std::vector<std::string> input_files) : input_files(input_files) {

    llable = "ATLAS, #sqrt{s} = 14 TeV";
    reg_lab = "W(D_{s}, #gamma)";

    //plots to plot
    variables = {"truth_w_pt",
                "truth_gamma_pt",
                "truth_ds_pt",
                "truth_delta_phi_ds_gamma",
                "truth_delta_eta_ds_gamma",
                "truth_delta_r_ds_gamma",
                "truth_w_energy",
                "truth_ds_energy",
                "truth_gamma_energy",
                "truth_w_eta",
                "truth_ds_charge",
                "reco_w_mass",
                "reco_w_pT",
                "reco_w_deltaPhi",
                "reco_w_deltaEta",
                "reco_w_deltaR",
                "reco_photon_n",
                "reco_jet_n",
                "reco_w_photon_pT",
                "reco_w_jet_pT"
                };
    titles = {"Truth p_{T} (W) [GeV]",
                "Truth p_{T} (#gamma_{W}) [GeV]",
                "Truth p_{T} (Ds_{W}) [GeV]",
                "Truth #Delta#phi (Ds_{W},#gamma_{W})",
                "Truth #Delta#eta (Ds_{W},#gamma_{W})",
                "Truth #DeltaR (Ds_{W},#gamma_{W})",
                "Truth E (W)  [GeV]",
                "Truth E (#gamma)  [GeV]",
                "Truth E (Ds)  [GeV]",
                "Truth #eta (W)  [GeV]",
                "Truth Ds_{W} charge",
                "Reconstructed W mass (D_{s},#gamma) [GeV]",
                "Reconstructed p_{T} (W) [GeV]",
                "Reconstructed #Delta#phi (Ds_{W},#gamma_{W})",
                "Reconstructed #Delta#eta (Ds_{W},#gamma_{W})",
                "Reconstructed #DeltaR (Ds_{W},#gamma_{W})",
                "Reconstructed #gamma multiplicity",
                "Reconstructed jet multiplicity",
                "Reconstructed p_{T} (#gamma_{W}) [GeV]",
                "Reconstructed p_{T} (Ds_{W}) [GeV]",
                };
    

    //input files added in the commnd line...
    for (size_t i = 0; i < input_files.size(); i++)
    {
        TFile *f = new TFile(input_files[i].c_str(), "read");
        files.push_back(f);
    }
}

void Plotter::ProcessFiles(){
    if (variables.size() != titles.size())
    {
        std::cout<<"Plots and titles are not matching!"<<std::endl;
        return;
    }

    for (size_t i = 0; i < variables.size(); i++) {
        mkdir("./plots", 0770);


        TLegend *legend = new TLegend(0.60, 0.75, 0.84, 0.89);
        legend->SetBorderSize(0);
        legend->SetTextFont(43);
        legend->SetTextSize(20);

        //First plot
        myhist1 = (TH1D*) files[0]->Get(variables[i].c_str());
        myhist1->SetLineColor(kRed);
        myhist1->SetLineWidth(2);
        myhist1->SetTitle("");
        myhist1->GetXaxis()->SetTitle(titles[i].c_str());
        myhist1->GetYaxis()->SetTitle("Events");
        if (input_files[0] == "WminusPlots.root") legend->AddEntry(myhist1, "W^{-}(Ds, #gamma)", "l");
        else if (input_files[0] == "WplusPlots.root") legend->AddEntry(myhist1, "W^{+}(Ds, #gamma)", "l");
        else if (input_files[0] == "signal_plots.root") legend->AddEntry(myhist1, "W(Ds, #gamma)", "l");
        else if (input_files[0] == "background_plots.root") legend->AddEntry(myhist1, "Background", "l");
        else legend->AddEntry(myhist2, input_files[0].c_str(), "l");
        
        myhist1->SetMaximum(1.5*myhist1->GetMaximum());


        if (input_files.size() > 1)
        {
            myhist2 = (TH1D*) files[1]->Get(variables[i].c_str());  
            myhist2->SetLineColor(kBlue);
            myhist2->SetLineWidth(2);
            myhist2->SetTitle("");
            myhist2->GetXaxis()->SetTitle(titles[i].c_str());
            myhist2->GetYaxis()->SetTitle("Events");
            if (input_files[1] == "WminusPlots.root") legend->AddEntry(myhist2, "W^{-}(Ds, #gamma)", "l");
            else if (input_files[1] == "WplusPlots.root") legend->AddEntry(myhist2, "W^{+}(Ds, #gamma)", "l");
            else if (input_files[1] == "signal_plots.root") legend->AddEntry(myhist2, "W(Ds, #gamma)", "l");
            else if (input_files[1] == "background_plots.root") legend->AddEntry(myhist2, "Background", "l");
            else legend->AddEntry(myhist2, input_files[1].c_str(), "l");
        }

        TCanvas *c = new TCanvas("plot", "plot", 800, 600);
        gStyle->SetOptStat(0);
        c->SetLeftMargin(0.15);

        myhist1->Draw("HIST");
        if (input_files.size() > 1) myhist2->Draw("HIST SAME");
        myText(0.2, 0.85, 1, llable.c_str());  //0.83
        myText(0.2, 0.80, 1, reg_lab.c_str()); //0.78
        legend->Draw();
        c->Update();

        std::string pdfname = "./plots/" + variables[i] + ".pdf";
        c->SaveAs(pdfname.c_str());
        std::string pngname = "./plots/" + variables[i] + ".png";
        c->SaveAs(pngname.c_str());
        c->Close();
    }

}

void Plotter::Finalize() {
    std::cout << "All plots have been plotted!" << std::endl;
}