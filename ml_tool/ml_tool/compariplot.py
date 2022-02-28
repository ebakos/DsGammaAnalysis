from .model import Model
import pandas as pd
import seaborn as sns
import json
import numpy as np
from matplotlib.ticker import FuncFormatter
sns.set_theme(style="whitegrid", palette="colorblind")


def compariplot(model_directory, plot_directory, yrange, constraints, category, variable, col_category, filename, markersize):
    models = Model.load_multi_meta_only(model_directory)

    if type(models[0].metadata["config"][category]) in [list, dict]:
        conv = lambda x: str(x)
    else:
        conv = lambda x: x

    vari_data = np.array([model.metadata[variable] for model in models])
    cati_data = np.array([conv(model.metadata["config"][category]) for model in models])

    if col_category is not None:
        ccati_data = np.array([str(model.metadata["config"][col_category]) for model in models])

    sel = None
    if constraints:
        for constraint in constraints:
            subcati_data = np.array([str(model.metadata["config"][constraint[0]]) for model in models])
            sel = (subcati_data == constraint[1]) if sel is None else np.logical_and(sel, subcati_data == constraint[1])

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

    if variable == "accuracy":
        variable = "accuracy (%)"
    data = {
        variable: vari_data,
        category: cati_data
    }

    try:
        vals = list(set(cati_data))
        try:
            sorter = lambda x: x[0]
            dd = [(json.loads(v), v) for v in vals]
            if dd[0][0] == list:
                sorter = lambda x: x[0][0]
            order = list(k[1] for k in sorted(dd, key=sorter))
        except:
            order = list(sorted(vals))

        if "softmax" in order:
            order.remove("softmax")
            order.append("softmax")
    except:
        order = None

    if col_category is not None:
        data[col_category] = ccati_data
        hue_order = list(sorted(set(ccati_data)))
        if "softmax" in hue_order:
            hue_order.remove("softmax")
            hue_order.append("softmax")

    ds = pd.DataFrame(data)

    if col_category is not None:
        ax = sns.swarmplot(data=ds, x=category, y=variable, hue=col_category, size=markersize, order=order, hue_order=hue_order, dodge=True)
    else:
        ax = sns.swarmplot(data=ds, x=category, y=variable, size=markersize, order=order)

    fig = ax.get_figure()
    if yrange:
        ax.set_ylim(*yrange)
    ax.yaxis.set_major_formatter(FuncFormatter(lambda y, _: '{:.1f}'.format(y*100)))
    sns.despine(top=True, right=True, left=False, bottom=True)
    fig.tight_layout()

    filename = filename or f"{variable}_{category}.png"
    fig.savefig(f"{plot_directory}/{filename}", dpi=600)
