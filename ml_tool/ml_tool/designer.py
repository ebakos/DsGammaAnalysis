from tensorflow.keras import layers
from tensorflow.keras import models
import tensorflow as tf
from tensorflow import keras
from .model import Model
from .config import *

## Here define models:
# dense models
def create_dense_layers(config):
    return_layers = []
    return_layers.append(layers.Dense(config['layer1_nodes'], activation=config['layer1_activation']))
    if config['layer1_dropout']:
        return_layers.append(layers.Dropout(config['layer1_dropout_nodes']))
    if config['layer2']:
        return_layers.append(layers.Dense(config['layer2_nodes'], activation=config['layer2_activation']))
    if config['layer2_dropout']:
        return_layers.append(layers.Dropout(config['layer2_dropout_nodes']))
    if config['layer3']:
        return_layers.append(layers.Dense(config['layer3_nodes'], activation=config['layer3_activation']))
    if config['layer3_dropout']:
        return_layers.append(layers.Dropout(config['layer3_dropout_nodes']))
    if config['layer_use_sigmoid']:
        return_layers.append(layers.Dense(1, activation='sigmoid'))
    return return_layers
    
#This is needed to create model from the layers above
def create_model(name, prepped_layers, input_size):

    all_layers = [layers.InputLayer(input_shape=(input_size,))] + prepped_layers
    model = keras.Sequential(all_layers)
    model.compile(loss="binary_crossentropy", optimizer="adam", metrics=["accuracy"])
    return Model(model, name=name, metadata={})

# Convolutional only
def create_conv_layers(config):

    return_layers = []
    param1 = config['conv_layer1_nodes']
    return_layers.append(layers.Conv2D(param1[0], (param1[1], param1[2]), activation = config['conv_layer1_activation']))
    if config['conv_layer1_maxpooling']:
        return_layers.append(layers.MaxPooling2D())
    if config['conv_layer2']:
        param2 = config['conv_layer2_nodes']
        return_layers.append(layers.Conv2D(param2[0], (param2[1], param2[2]), activation = config['conv_layer2_activation']))
    if config['conv_layer2_maxpooling']:
        return_layers.append(layers.MaxPooling2D())
    if config['conv_layer3']:
        param3 = config['conv_layer3_nodes']
        return_layers.append(layers.Conv2D(param3[0], (param3[1], param3[2]), activation = config['conv_layer3_activation']))
    if config['conv_layer3_maxpooling']:
        return_layers.append(layers.MaxPooling2D())
    return_layers.append(layers.Flatten())
    # Dense layers to finish the convoutional model:
    if config['conv_dense']:
        return_layers.append(layers.Dense(config['conv_denselayer_nodes'], activation=config['conv_denselayer_activation']))
    if config['conv_use_sigmoid']:
        return_layers.append(layers.Dense(1, activation='sigmoid'))
    return return_layers

#This is needed to create model from the layers above
def create_conv_model(name, prepped_layers, conv_input_shape):
    all_layers = [layers.InputLayer(input_shape=conv_input_shape)] + prepped_layers
    model = keras.Sequential(all_layers)
    model.compile(loss="binary_crossentropy", optimizer="adam", metrics=["accuracy"])
    return Model(model, name=name, metadata={})


# convolutional + dense
def create_conv_plus_dense_model(config, dense_input_shape, conv_input_shape, dense_layers, conv_layers):
    
    #dense layers
    final_dense_layers = [layers.InputLayer(input_shape=dense_input_shape)] + dense_layers
    dense = keras.Sequential(final_dense_layers)
    
    #convolutional layers
    final_conv_layers = [layers.InputLayer(input_shape=conv_input_shape)] + conv_layers
    conv = keras.Sequential(final_conv_layers)

    combined = layers.concatenate((dense.output, conv.output))
    x = layers.Dense(config['comb_denselayer_nodes'], activation=config['comb_denselayer_activation'])(combined)
    if config['comb_use_sigmoid']:
        x = layers.Dense(1, activation='sigmoid')(x)

    model = models.Model(inputs=[dense.input, conv.input], outputs=x)
    model.compile(loss="binary_crossentropy", optimizer="adam", metrics=["accuracy"])
    return Model(model, name=config['model_name'], metadata={})

