from setuptools import setup


setup(
    name="ds_gamma_mltool",
    version="0.0.1",
    description="ML processes for W->DS+Gamma",
    author="Evelin Bakos",
    packages=['ml_tool'],
    package_data={'ml_tool': ["default_model.json"]},
    entry_points={
        'console_scripts': [
            'ml_tool=ml_tool:__main__.main'
        ]
    },
    install_requires=[
        "numpy",
        "matplotlib",
        "tensorflow",
        "uproot",
        "sklearn",
        "pandas",
        "seaborn"
    ],
    zip_safe=False
)