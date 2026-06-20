### Training

From `training/` directory:

1. Activate virtual env & install requirements
2. Set python path to bring-in ModelAssistant packages into training directory
    `export PYTHONPATH=$PWD/ModelAssistant:$PYTHONPATH`
    (or)
    `set PYTHONPATH=%CD%\ModelAssistant;%PYTHONPATH%`
3. Train
    `python -m tools.train yth_config.py --cfg-options exp_name=FirstRun`