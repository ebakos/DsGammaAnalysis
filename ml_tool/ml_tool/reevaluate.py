from .model import Model
from .dataset import DataSet


def reevaluate(model, dataset: DataSet):
    model = Model.load(model)
    batch_size = model.metadata['batch_size']
    epochs = model.metadata['epochs']

    dataset.reset_keys(model.metadata['keys'])
    train_x, train_Y = dataset.train_data()
    test_x, test_Y = dataset.test_data()

    loss, accuracy = model.model.evaluate(x=test_x, y=test_Y, batch_size=batch_size, verbose=0)
    print(f"Pre re-training {model.name}.")
    print(f"    Validation loss: {loss}")
    print(f"    Validation accuracy: {accuracy}")
    print()

    for layer in model.model.layers:
        if hasattr(layer, 'kernel_initializer') and \
                hasattr(layer, 'bias_initializer'):
            print(f"Resetting layer {layer}")
            weight_initializer = layer.kernel_initializer
            bias_initializer = layer.bias_initializer

            old_weights, old_biases = layer.get_weights()

            layer.set_weights([
                weight_initializer(shape=old_weights.shape),
                bias_initializer(shape=old_biases.shape)])
        else:
            print(f"Not resetting layer {layer}")

    model.model.fit(x=train_x, y=train_Y, batch_size=batch_size, epochs=epochs, verbose=1)
    loss, accuracy = model.model.evaluate(x=test_x, y=test_Y, batch_size=batch_size, verbose=0)

    print(f"Post re-training {model.name}.")
    print(f"    Validation loss: {loss}")
    print(f"    Validation accuracy: {accuracy}")
    print()
