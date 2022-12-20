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


## ML tool

A python package to do machine learning. Run `pip install ./ml_tool` to install (including dependencies). Only tested on windows (there I have a graphics card). For help with commands you can now run `ml_tool --help` and `ml_tool subcommand --help`.

### Training: 

To run type `ml_tool` with the arguments: 
```
  -h, --help            show this help message and exit
  -d DATA_DIRECTORY, --data-directory DATA_DIRECTORY
                        Where to load data files from
  -m MODEL_DIRECTORY, --model-directory MODEL_DIRECTORY
                        Where to store model files
  -j CONFIG_FILE, --config-file CONFIG_FILE
                        json file with config options
```
If no `.json` file provided, the `default_model.json` will be used.

**Contents of the `.json` config file: 

*Common run settings:*
- `model_name`: name of the models
- `batch_size`: training batch size
- `training_epochs`: training epochs
- `run_options`: "dense_only", "leave_one_out", "conv_only", "dense_conv" (combined dense and convolutional)
- `train_qq`: train only on qq sample
- `train_gg`: train only on gg sample
- `test_qq`: test only with qq sample
- `test_gg`: test only with gg sample

*Dense model settings:*
Setup for dense model, up to 3 layers.
- `layer1_nodes`: layer nodes in layer 1
- `layer1_activation`: layer 1 activation functions: `tanh`, `relu`, `softmax` etc.
- `layer1_dropout`: use droput layer after layer 1, if set to `false`, rest of the dropout layer parameters ignored
- `layer1_dropout_nodes`: dropout rate
- `layer2`: if set to `false`, no second layer is present, rest of the layer 2 are options ignored.
- `layer2_nodes`: layer nodes in layer 2
- `layer2_activation`: layer 2 activation functions: `tanh`, `relu`, `softmax` etc.
- `layer2_dropout`: use droput layer after layer 2, if set to `false`, rest of the dropout layer parameters ignored
- `layer2_dropout_nodes`: dropout rate
- `layer3`: if set to `false`, no third layer is present, rest of the layer 3 are options ignored. Note: If layer 2 is set to `false`, but layer 3 is `true`, layer 3 becomes layer 2.
- `layer3_nodes`: layer nodes in layer 3
- `layer3_activation`: layer 3 activation functions: `tanh`, `relu`, `softmax` etc.
- `layer3_dropout`: use droput layer after layer 3, if set to `false`, rest of the dropout layer parameters ignored
- `layer3_dropout_nodes`: dropout rate
- `layer_output_activation`: `simoid`,`tanh`, `relu`, `softmax` etc.

*Leave-one-out model settings:* 
With this run option, it is possible to leave one ore more parameter out of the dense model training. The dense model setup is the same as it is defined above.
- `excluded_keys`: list of parameters which will be left out from the training. Can be one or more parameter, like: `["p_core_1", "p_core_2"]`

*Convolutional model settings:*
Setup for convolutional model, up to 3 layers.
- `conv_layer1_nodes`: layer nodes in layer 3, where the first number is the number of filters, the seond and third number is the kernel size: `[16, 3, 3]`
- `conv_layer1_activation`: layer 1 activation functions: `tanh`, `relu`, `softmax` etc.
- `conv_layer1_maxpooling`: if set to `false`, no maxpooling layer is present after layer 1
- `conv_layer2`:  if set to `false`, no second layer is present, rest of the layer 2 are options ignored.
- `conv_layer2_nodes`: layer nodes in layer 2
- `conv_layer2_activation`: layer 2 activation functions: `tanh`, `relu`, `softmax` etc.
- `conv_layer2_maxpooling`: if set to `false`, no maxpooling layer is present after layer 2
- `conv_layer3`: if set to `false`, no third layer is present, rest of the layer 3 are options ignored. Note: If layer 2 is set to `false`, but layer 3 is `true`, layer 3 becomes layer 2.
- `conv_layer3_nodes`: layer nodes in layer 3
- `conv_layer3_activation`: layer 3 activation functions: `tanh`, `relu`, `softmax` etc.
- `conv_layer3_maxpooling`: if set to `false`, no maxpooling layer is present after layer 3
- `conv_dense`: if set to `true`, a dense layer is added in the end of the convolutional layers. Note: Turn this to `false` if running with combined mode!
- `conv_denselayer_nodes`: Number of nodes in the dense layer.
- `conv_denselayer_activation`: dense layer activation functions: `tanh`, `relu`, `softmax` etc.
- `conv_output_activation`: `simoid`,`tanh`, `relu`, `softmax` etc.

*Combined model settings:*
This model takes the dense model defined above and the convolutional model defined above, and combines them. The combined layer settings can be set up with the parameters below. These layers takes the output of the dense and convolutional model as the input, and combines them to a single output.
- `comb_denselayer_nodes`: dense layers nodes. This dense layer takes the output of the previous models as input.
- `comb_denselayer_activation`: dense layer activation functions: `tanh`, `relu`, `softmax` etc.
- `comb_output_activation`: `simoid`,`tanh`, `relu`, `softmax` etc.

To run multiple models at once, it is possible to define every argument as a list, for example: 
```
{
    "model_name" : "denseOnly",
    "run_options" : "dense_only",

    "layer1_nodes" : [10, 15, 20],
    "layer1_activation" : ["tanh", "relu"],
    "layer1_dropout" : [false, true],
    "layer1_dropout_nodes" : 0.2,
}
```
This config file will configure and the program will run 3x2x2 = 12  models. In case, the argument is a list (for example `excluded_keys`) one needs to define a list of a list. In this case an integer will be placed next to the model name. The model parameters, together with the list of the keys (variables) used, the accuracy, loss and the model configuration is saved into a metadata file within the model directory. 
This metadata is used for plotting.

### Plot:

This will plot the ROC curve of every model from the model directory.

To run type `ml_tool` with the arguments: 
```
  -h, --help            show this help message and exit
  -d DATA_DIRECTORY, --data-directory DATA_DIRECTORY
                        Where to load data files from
  -m MODEL_DIRECTORY, --model-directory MODEL_DIRECTORY
                        Where to load model files
  -p PLOT_DIRECTORY, --plot-directory PLOT_DIRECTORY
                        Where to store plot images
```

### Tabulate results: 
Creates a file with all the models listed in a folder, together with the variable values. 

Usage: 
```usage: ml_tool tabulate [-h] [-m MODEL_DIRECTORY] variable                                                          

positional arguments:                                       
  variable              Variable name

options:  
  -h, --help            show this help message and exit  
  -m MODEL_DIRECTORY, --model-directory MODEL_DIRECTORY                        
                        Where from load the model files
```

Example: `ml_tool tabulate accuracy`

### Compariplot 

Creates plot with all the models. 

Arguments: 
- `model-directory`: Where from load model files
- `plot-directory`: Where to store plot images
- `variable`: Variable out of metadata which to put on the Y axis, for example `accuracy`
- `range`: Y-axis range.
- `constraint`: constraints on variables. Filter out variables, for example `--constraint layer2 True` will only plot models where layer 2 is present. Can be used with any variables from the config file.
- `category`: Category for the X axis. 
- `color-category`: colour of points category. Makes it possible to plot 2 variables in the same time: one as the x-axis, the second as a color.
- `filename`: output plot filename.
- `markersize`: markersize.