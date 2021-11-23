import itertools
from pathlib import Path
import itertools
import json
from typing import Type


def get_configs(json_file_name):
    #this function returns the temporary configurations
    with open(Path(__file__).resolve().parent / 'default_model.json') as f_def:
        temp_config = json.load(f_def)

    if json_file_name != 'default_model.json':
        with open(json_file_name) as f:
            for k,v in json.load(f).items():
                if k not in temp_config:
                    raise ValueError(f"'{k}' as specified in your config {json_file_name} is not a valid config parameter.")
                temp_config[k] = v

    list_fields = ["excluded_keys", "conv_layer1_nodes", "conv_layer2_nodes", "conv_layer3_nodes"]
    noncombinators = {}
    combinators = {}

    for k, v in temp_config.items():
        if k in list_fields:
            if type(v[0]) == list:
                # 2D
                combinators[k] = v
            else:
                noncombinators[k] = v
        else:
            if type(v) == list:
                # 1D
                combinators[k] = v
            else:
                noncombinators[k] = v

    for i, combination in enumerate(itertools.product(*list(combinators.values()))):
        config = noncombinators.copy()
        config["model_name"] = f"{config['model_name']}_{i}"
        for key, value in zip(combinators.keys(), combination):
            config[key] = value
        yield config

