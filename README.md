# DsGammaAnalysis

This repository contains files related to the DsGamma analysis.

## EventGeneration

Contains the Pythia card for signal generation and the ATLAS Delphes card for detector simulation.

## AnalysisTool

This program can be used for analysing Monte-Carlo samples generated with Madraph, Pythia8 and Delphes. The code is meant for analysing pp -> W events, where  the W boson decays into DsGamma. The code is divided into routines: 
- Event consisteny
- Reconstructed object analyzer
- Plotter
- Ntupler

### Event consistency:

This part of the program doing the truth event validation. We are looking for the Ds particle, checking its sibling, what is a photon, checking the parents, and the sibling of a parent (both W boson). The event consistency also making histograms filled with some relevant variables.

### Reconstructed object analyser:

This part of the program is looking for the reconstructed objects, most importantly the reconstructed W from the Ds and the photon. The progam also fills some of the relevant variables into histograms. 

### Plotter:

Plots the previoulsy made histograms, possibly overlays them. 

### Ntuples

Creates a Root TTree named `DS`, with the variables as branches used for Machine Learning. 

## Program usage: 

Source Delphes and ROOT before using. Type `make` for compiling. Running the program can be done with some of the following commands: 

```
Mode analysis, operations: event_consistency, reco
Usage: ./tool/bin/analyze analysis <in_file> <out_file> <operation1> [operation2]
Usage: ./bin/analyze analysis <in_file> <out_file> ntupler <SignalWplus,SignalWminus,BackgroundQQ,BackgroundGG>
Mode: plot
Usage #1: ./tool/bin/analyze plot <in_file1> <in_file2>
Usage #2: ./tool/bin/analyze plot <in_file1>
```

## PlotDifferences

A simple root macro for plotting variables.


# ml_tool

A python package to do machine learning. Run `pip install ./ml_tool` to install (including dependencies). Only tested on windows (there I have a graphics card). For help with commands you can now run `ml_tool --help` and `ml_tool subcommand --help`.
