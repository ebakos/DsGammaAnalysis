from .model import Model


def tabulate(model_directory, variable):
    models = Model.load_multi(model_directory)
    for model in models:
        print(model.name + '\t' + str(model.metadata[variable]))
