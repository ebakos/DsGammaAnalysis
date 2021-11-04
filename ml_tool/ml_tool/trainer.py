from .model import Model
from .dataset import DataSet
from .designer import make_tanh_layer, create_model, create_conv_plus_dense_model
from .config import BATCH_SIZE, TRAINING_EPOCHS


def trainer(model: Model, dataset: DataSet):
    print(f"Training model {model.name}")
    train_x, train_Y = dataset.train_data()
    test_x, test_Y = dataset.test_data()

    model.model.fit(x=train_x, y=train_Y, batch_size=BATCH_SIZE, epochs=TRAINING_EPOCHS, verbose=0)
    loss, accuracy = model.model.evaluate(x=test_x, y=test_Y, batch_size=BATCH_SIZE, verbose=0)

    print(f"Finished trainig model {model.name}.")
    print(f"    Validation loss: {loss}")
    print(f"    Validation accuracy: {accuracy}")
    print()

    model.metadata['loss'] = loss
    model.metadata['accuracy'] = accuracy
    model.metadata['batch_size'] = BATCH_SIZE
    model.metadata['epochs'] = TRAINING_EPOCHS


def train(dataset: DataSet, model_directory, name, leave_one_out):
    model = create_model(name, make_tanh_layer(), len(dataset.keys()))
    trainer(model, dataset)
    model.save(model_directory)

    if leave_one_out:
        keys = dataset.keys().copy()

        for nname, rkeys in {
            'f_cores': ['f_core_1', 'f_core_2', 'f_core_3', 'f_em'],
            'p_core': ['p_core_1', 'p_core_2'],
            'tau': ['tau_0', 'tau_1', 'tau_2']
        }.items():
            new_keys = keys.copy()
            for rkey in rkeys:
                new_keys.remove(rkey)

            model = create_model(f"{name}_no_{nname}", make_tanh_layer(), len(new_keys))
            dataset.reset_keys(new_keys)
            trainer(model, dataset)
            model.save(model_directory)

        for key in keys:
            new_keys = keys.copy()
            new_keys.remove(key)

            model = create_model(f"{name}_no_{key}", make_tanh_layer(), len(new_keys))
            dataset.reset_keys(new_keys)
            trainer(model, dataset)
            model.save(model_directory)



def train_with_images(dataset: DataSet, model_directory, name):
    model = create_conv_plus_dense_model(name, dataset.image_dimensions(), len(dataset.keys()) - 1)
    trainer(model, dataset)
    model.save(model_directory)
