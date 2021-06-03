# DsGammaAnalysis

This program can be used for analysing Monte-Carlo samples generated with Madraph, Pythia8 and Delphes. The code is meant for analysing e+e- -> W+W- events, where one of the W decays into DsGamma. The code is divided into routines: 
- Event consisteny
- Reconstructed object analyzer
- Plotter

### Event consistency:

This part of the program doing the truth event validation. We are looking for the Ds particle, checking its sibling, what is a photon, checking the parents, and the sibling of a parent (both W boson). The event consistency also making histograms filled with some relevant variables.

### REconstructed object analyser:

This part of the program is looking for the reconstructed objects, most importantly the reconstructed W from the Ds and the photon. The progam also fills some of the relevant variables into histograms. 

### Plotter

Plots the previoulsy made histograms, possibly overlays them. 


## Program usage: 

Source Delphes and ROOT before using. Type `make` for compiling. Running the program can be done with some of the following commands: 

```
Mode analysis, operations: event_consistency, reco
Usage: ./bin/analyze analysis <in_file> <out_file> <operation1> [operation2]
Mode: plot
Usage #1: ./bin/analyze plot <in_file1> <in_file2>
Usage #2: ./bin/analyze plot <in_file1>
```
