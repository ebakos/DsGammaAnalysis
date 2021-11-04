import os

# Need to do this so windows doesn't freak out when tenserflow loads the libs
for dir in os.environ.get("PATH").split(os.pathsep):
    try:
        os.add_dll_directory(dir)
    except:
        pass

from .__main__ import main