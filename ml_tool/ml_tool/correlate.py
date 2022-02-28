from .model import Model
from .dataset import DataSet

import numpy as np


def correlate(model_1, model_2, dataset: DataSet):
    model1 = Model.load(model_1)
    model2 = Model.load(model_2)

    # Model1 prediction
    dataset.reset_keys(model1.metadata["keys"])
    test_x1, test_y1 = dataset.test_data()
    pred_y1 = model1.model.predict(test_x1).flatten()

    # Model2 prediction
    dataset.reset_keys(model2.metadata["keys"])
    test_x2, test_y2 = dataset.test_data()
    pred_y2 = model2.model.predict(test_x2).flatten()

    assert np.all(test_y1 == test_y2)

    # true-pos1 true-pos2
    num_tp_tp = np.logical_and(test_y1 == 1, np.logical_and(pred_y1 >= 0.5, pred_y2 >= 0.5)).sum()

    # false-neg1 true-pos2
    num_fn_tp = np.logical_and(test_y1 == 1, np.logical_and(pred_y1 < 0.5, pred_y2 >= 0.5)).sum()

    # true-pos1 false-neg2
    num_tp_fn = np.logical_and(test_y1 == 1, np.logical_and(pred_y1 >= 0.5, pred_y2 < 0.5)).sum()

    # false-neg1 false-neg2
    num_fn_fn = np.logical_and(test_y1 == 1, np.logical_and(pred_y1 < 0.5, pred_y2 < 0.5)).sum()

    # true-neg1 true-neg2
    num_tn_tn = np.logical_and(test_y1 == 0, np.logical_and(pred_y1 < 0.5, pred_y2 < 0.5)).sum()

    # false-pos1 true-neg2
    num_fp_tn = np.logical_and(test_y1 == 0, np.logical_and(pred_y1 >= 0.5, pred_y2 < 0.5)).sum()

    # true-neg1 false-pos2
    num_tn_fp = np.logical_and(test_y1 == 0, np.logical_and(pred_y1 < 0.5, pred_y2 >= 0.5)).sum()

    # false-pos1 false-pos2
    num_fp_fp = np.logical_and(test_y1 == 0, np.logical_and(pred_y1 >= 0.5, pred_y2 >= 0.5)).sum()

    print("true-pos-1 true-pos-2", num_tp_tp)
    print("false-neg-1 true-pos-2", num_fn_tp)
    print("true-pos-1 false-neg-2", num_tp_fn)
    print("false-neg-1 false-neg-2", num_fn_fn)

    print("true-neg-1 true-neg-2", num_tn_tn)
    print("false-pos-1 true-neg-2", num_fp_tn)
    print("true-neg-1 false-pos-2", num_tn_fp)
    print("false-pos-1 false-pos-2", num_fp_fp)

    print("total ev", test_y1.shape[0])

    print("percentages")

    print("true-pos-1 true-pos-2", num_tp_tp / float(test_y1.shape[0]) * 100)
    print("false-neg-1 true-pos-2", num_fn_tp / float(test_y1.shape[0]) * 100)
    print("true-pos-1 false-neg-2", num_tp_fn / float(test_y1.shape[0]) * 100)
    print("false-neg-1 false-neg-2", num_fn_fn / float(test_y1.shape[0]) * 100)

    print("true-neg-1 true-neg-2", num_tn_tn / float(test_y1.shape[0]) * 100)
    print("false-pos-1 true-neg-2", num_fp_tn / float(test_y1.shape[0]) * 100)
    print("true-neg-1 false-pos-2", num_tn_fp / float(test_y1.shape[0]) * 100)
    print("false-pos-1 false-pos-2", num_fp_fp / float(test_y1.shape[0]) * 100)

    print("Models agree correctly:", (num_tp_tp + num_tn_tn) / float(test_y1.shape[0]) * 100)
    print("Models agree incorrectly:", (num_fp_fp + num_fn_fn) / float(test_y1.shape[0]) * 100)
    print("Model 1 is right, 2 is wrong:", (num_tp_fn + num_tn_fp) / float(test_y1.shape[0]) * 100)
    print("Model 2 is right, 1 is wrong:", (num_fn_tp + num_fp_tn) / float(test_y1.shape[0]) * 100)
