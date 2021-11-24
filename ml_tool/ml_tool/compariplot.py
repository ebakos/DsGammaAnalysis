from .model import Model
import pandas as pd
import seaborn as sns
import numpy as np
sns.set_theme(style="whitegrid", palette="muted")


def compariplot(model_directory, plot_directory, yrange, constraints, category, variable, col_category):
    models = Model.load_multi_meta_only(model_directory)

    vari_data = np.array([model.metadata[variable] for model in models])
    cati_data = np.array([str(model.metadata["config"][category]) for model in models])

    if col_category is not None:
        ccati_data = np.array([str(model.metadata["config"][col_category]) for model in models])

    sel = None
    for constraint in constraints:
        subcati_data = np.array([str(model.metadata["config"][constraint[0]]) for model in models])
        sel = (subcati_data == constraint[1]) if sel is not None else np.logical_and(sel, subcati_data == constraint[1])

    if sel is not None:
        sel = np.where(sel)
        vari_data = vari_data[sel]
        cati_data = cati_data[sel]
        if col_category is not None:
            ccati_data = ccati_data[sel]

    if yrange is not None:
        sel = np.where(np.logical_and(vari_data > yrange[0], vari_data < yrange[1]))
        vari_data = vari_data[sel]
        cati_data = cati_data[sel]
        if col_category is not None:
            ccati_data = ccati_data[sel]

    data = {
        variable: vari_data,
        category: cati_data
    }

    if col_category is not None:
        data[col_category] = ccati_data

    ds = pd.DataFrame(data)

    if col_category is not None:
        ax = sns.swarmplot(data=ds, x=category, y=variable, hue=col_category, size=3)
    else:
        ax = sns.swarmplot(data=ds, x=category, y=variable, size=3)
    fig = ax.get_figure()
    fig.savefig(f"{plot_directory}/{variable}_{category}.png", dpi=600)
