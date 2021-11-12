from tensorflow.keras import layers
from tensorflow.keras import models
import tensorflow as tf
from tensorflow import keras
from .model import Model


def make_tanh_layer():
    return [
        layers.Dense(15, activation="tanh"),
        layers.Dense(1, activation="sigmoid")
    ]


def make_tan_and_conv_layer():
    return [
        layers.Dense(15, activation="tanh"),
        layers.Dense(1, activation="sigmoid")
    ]

def create_model(name, prepped_layers, input_size):
    all_layers = [layers.InputLayer(input_shape=(input_size,))] + prepped_layers
    model = keras.Sequential(all_layers)
    model.compile(loss="binary_crossentropy", optimizer="adam", metrics=["accuracy"])
    return Model(model, name=name, metadata={})


def create_conv_plus_dense_model(name, conv_input_shape, dense_input_shape):
    dense = keras.Sequential()
    dense.add(layers.InputLayer(input_shape=dense_input_shape))
    dense.add(layers.Dense(20, activation="tanh"))
    dense.add(layers.Dense(10, activation="tanh"))

    conv = keras.Sequential()
    conv.add(layers.InputLayer(input_shape=conv_input_shape))
    conv.add(layers.Conv2D(16, (3, 3)))
    conv.add(layers.Activation('relu'))
    conv.add(layers.Conv2D(8, (3, 3)))
    conv.add(layers.Activation('relu'))
    conv.add(layers.MaxPooling2D())
    conv.add(layers.Flatten())

    combined = layers.concatenate((dense.output, conv.output))
    x = layers.Dense(12, activation='tanh')(combined)
    x = layers.Dense(1, activation='sigmoid')(x)

    model = models.Model(inputs=[dense.input, conv.input], outputs=x)
    model.compile(loss="binary_crossentropy", optimizer="adam", metrics=["accuracy"])
    return Model(model, name=name, metadata={})

