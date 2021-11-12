from setuptools import setup


setup(
    name="ds_gamma_mltool",
    version="0.0.1",
    description="ML processes for W->DS+Gamma",
    author="Evelin Bakos",
    packages=['ml_tool'],
    entry_points={
        'console_scripts': [
            'ml_tool=ml_tool:main'
        ]
    },
    install_requires=[
        "numpy",
        "matplotlib",
        "tensorflow",
        "uproot",
        "sklearn"
    ],
    zip_safe=False
)