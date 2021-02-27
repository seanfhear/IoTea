# Image Processing with PlantCV
The Jupyter notebook consists of the example VIS workflow given by PlantCV at https://plantcv.readthedocs.io/en/stable/vis_tutorial/

# To Run
To create an anaconda environment and install the required modules run `conda create --name <env> --file requirements.txt` from the src folder.

If that doesn't work you may need to install PlantCV by adding the conda-forge channel to the conda config and then installing PlantCV. Instructions found [here](https://plantcv.readthedocs.io/en/stable/installation/).

`conda config --add channels conda-forge`
`conda install plantcv`

Run Jupyter with the command `jupyter notebook` from the src folder. 