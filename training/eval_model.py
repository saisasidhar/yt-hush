import argparse
import torch
import numpy as np

from mmengine.config.config import Config
from mmengine.runner import Runner
from sklearn.metrics import classification_report, confusion_matrix
import seaborn as sns
import matplotlib.pyplot as plt


def main():
    parser = argparse.ArgumentParser(description='Evaluate model')
    parser.add_argument('--cfg-file', required=True, help='Path to the config file')
    parser.add_argument('--ckpt-file', required=True, help='Path to the checkpoint file')
    args = parser.parse_args()

    cfg_file = args.cfg_file
    ckpt_file = args.ckpt_file

    cfg = Config.fromfile(cfg_file)
    cfg.val_dataloader.num_workers = 0
    cfg.val_dataloader.persistent_workers = False

    runner = Runner.from_cfg(cfg)
    runner.load_checkpoint(ckpt_file)

    model = runner.model
    model.eval()

    y_true, y_pred = [], []

    for batch in runner.val_dataloader:
        with torch.no_grad():
            outputs = model.test_step(batch)

        for out in outputs:
            y_true.append(out.gt_label.item())
            y_pred.append(out.pred_label.item())

    cm = confusion_matrix(y_true, y_pred)

    print("\nClassification Report:\n")
    print(classification_report(y_true, y_pred))
    print("\nConfusion Matrix:\n")
    print(cm)

    plt.figure(figsize=(6, 5))
    sns.heatmap(cm, annot=True, fmt="d", cmap="Blues")
    plt.title("Confusion Matrix")
    plt.xlabel("Predicted")
    plt.ylabel("Ground Truth")
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    main()
