from .model import Model
from .dataset import DataSet

import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from sklearn.metrics import roc_curve
import numpy as np


def plot(model_directory, plot_directory, dataset: DataSet):
    fig, ax = plt.subplots()

    ax.plot([0, 1], [0, 1], 'k--')
    models = Model.load_multi(model_directory)
    for model in models:
        dataset.reset_keys(model.metadata["keys"])
        test_x, test_y = dataset.test_data()
        bsel = np.where(test_y == 0)
        ssel = np.where(test_y == 1)

        if type(test_x) == list:
            bkgdata = [test_x[0][bsel], test_x[1][bsel]]
            sigdata = [test_x[0][ssel], test_x[1][ssel]]
        else:
            bkgdata = test_x[bsel]
            sigdata = test_x[ssel]

        score = model.model.evaluate(x=test_x, y=test_y, batch_size=model.metadata['batch_size'], verbose=0)

        # ROC draw
        y_pred = model.model.predict(test_x)
        fpr, tpr, thr = roc_curve(test_y, y_pred)

        ax.plot(fpr, tpr, label=f"{model.name} {score[1]*100:.2f}% {score[0]:.3f}")

        # NNout
        fig1, ax1 = plt.subplots()
        bkgpred = model.model.predict(bkgdata)
        sigpred = model.model.predict(sigdata)
        ax1.set_xlabel('NNout')
        ax1.set_ylabel('#entries')

        r1 = Rectangle((0,0), 1, 1, fill=False, edgecolor='none', visible=False)

        ax1.hist(bkgpred, bins=20, range=(0,1), color='blue', histtype='bar', label='Background')
        ax1.hist(sigpred, bins=20, range=(0,1), color='red', histtype='bar', label='Signal')
        ax1.legend([r1], [model.name], loc=1)

        fig1.tight_layout()
        fig1.savefig(str(plot_directory / f"{model.name}_nnout.png"))
        fig1.clf()

    ax.set_xlabel('False positive rate')
    ax.set_ylabel('True positive rate')
    ax.set_ylim(0.8, 1)
    ax.set_xlim(0, 0.2)
    ax.set_title(f'ROC curves')
    ax.legend(loc='best')
    fig.savefig(str(plot_directory / "roc_curves.png"))
    fig.clf()