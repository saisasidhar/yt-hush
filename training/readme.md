### Training

From `training/` directory:

1. Activate virtual env & install requirements
2. Set python path to bring-in ModelAssistant packages into training directory
    `export PYTHONPATH=$PWD/ModelAssistant:$PYTHONPATH`
    (or)
    `set PYTHONPATH=%CD%\ModelAssistant;%PYTHONPATH%`
3. Train
    `python -m tools.train yth_config.py --cfg-options exp_name=FirstRun`
4. Eval
    `python eval_model.py --cfg-file yth_config.py --ckpt-file run/0_5WidenFactor/best_accuracy_top1_epoch_31.pth`
5. Export
    `python -m tools.export yth_config.py run/0_5WidenFactor/best_accuracy_top1_epoch_31.pth --format vela --image_path dataset/validation/`
6. Deploy
    Flash the generated `model_int8_vela.tflite` to Seeed Grove Vision AI Kit v2 using the old [SenseCraft Web Toolkit](https://seeed-studio.github.io/SenseCraft-Web-Toolkit/#/setup/process) at offset `0x200000`