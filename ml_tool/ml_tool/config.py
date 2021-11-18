from pathlib import Path
from json import *

def get_config(json_file_name):
    #this function returns the temporary configurations
    with open(Path(__file__).resolve().parent / 'default_model.json') as f_def:
        temp_config = json.load(f_def)

    if json_file_name != 'default_model.json':
        #read in default configs
        with open(json_file_name) as f:
            data = json.load(f)
            for key in data:
                temp_config[key] = data[key]
    
    return temp_config

