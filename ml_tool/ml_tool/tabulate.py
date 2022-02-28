from .model import Model


def tabulate(model_directory, variable):
    models = Model.load_multi_meta_only(model_directory)
    with open("table.csv", "w") as f:
        for model in models:
            print(model.name + ', ' + str(model.metadata[variable]), file=f)
