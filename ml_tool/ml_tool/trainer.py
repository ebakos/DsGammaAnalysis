from .model import Model
from .dataset import DataSet
from .designer import create_dense_layers, create_model, create_conv_plus_dense_model, create_conv_layers, create_conv_model


def trainer(model: Model, dataset: DataSet, config):
    print(f"Training model {model.name}")
    train_x, train_Y = dataset.train_data()
    test_x, test_Y = dataset.test_data()

    model.model.fit(x=train_x, y=train_Y, batch_size=config['batch_size'], epochs=config['training_epochs'], verbose=0)
    loss, accuracy = model.model.evaluate(x=test_x, y=test_Y, batch_size=config['batch_size'], verbose=0)

    print(f"Finished trainig model {model.name}.")
    print(f"    Validation loss: {loss}")
    print(f"    Validation accuracy: {accuracy}")
    print()

    model.metadata['loss'] = loss
    model.metadata['accuracy'] = accuracy
    model.metadata['batch_size'] = config['batch_size']
    model.metadata['epochs'] = config['training_epochs']
    model.metadata['keys'] = dataset.keys()
    model.metadata['config'] = config


def train(dataset: DataSet, model_directory, config):
    ## Dense only
    if config['run_options'] == 'dense_only': 
        layers = create_dense_layers(config)
        model = create_model(config['model_name'], layers, len(dataset.keys()))
        trainer(model, dataset, config)
        model.save(model_directory)

    ## Leave one or group out
    elif config['run_options'] == 'leave_one_out':
        keys = dataset.keys().copy()
        #create layers
        layers = create_dense_layers(config)

        new_keys = keys.copy()
        for key in config['excluded_keys']:
            new_keys.remove(key)

        dataset.reset_keys(new_keys)

        model = create_model(f"{config['model_name']}_no_{'_'.join(config['excluded_keys'])}", layers, len(new_keys))
        trainer(model, dataset, config)
        model.save(model_directory)

    ## run with convolutional only:
    elif config['run_options'] == 'conv_only':
        dataset.reset_keys(["jet_image"])

        layers = create_conv_layers(config)
        model = create_conv_model(config['model_name'], layers, dataset.image_dimensions())
        trainer(model, dataset,config)
        model.save(model_directory)

    ## run with combined model: Dense + Convoltutional
    elif config['run_options'] == 'combi':
        dense_layers = create_dense_layers(config)
        conv_layers = create_conv_layers(config)
        model = create_conv_plus_dense_model(config, len(dataset.keys()) - 1, dataset.image_dimensions(), dense_layers, conv_layers)
        trainer(model, dataset, config)
        model.save(model_directory)

    else:
        print('Wrong run options argument!')

