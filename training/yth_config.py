from mmengine.config import read_base
from mmengine.dataset import DefaultSampler

from mmengine.hooks import (
    IterTimerHook,
    LoggerHook,
    CheckpointHook,
    ParamSchedulerHook,
    DistSamplerSeedHook,
)

with read_base():
    from configs._base_.default_runtime import *
    from configs._base_.schedules.schedule_1x import *

from sscma.datasets import CustomDataset
from sscma.datasets.transforms import (
    LoadImageFromFile,
    PackInputs,
)
from sscma.datasets import ClsDataPreprocessor
from sscma.engine import DetVisualizationHook
from sscma.models import (
    ImageClassifier,
    MobileNetv2,
    LinearClsHead,
    GlobalAveragePooling,
    CrossEntropyLoss,
)
from sscma.visualization import UniversalVisualizer
from sscma.evaluation import Accuracy


# =========================
# CONFIG
# =========================
num_classes = 3
data_root = "./dataset/"
exp_name = "exp_name_notdefined"
work_dir = f"./run/{exp_name}"

img_size = (240, 240)
batch_size = 16
val_batch_size = 16
num_workers = 4

epochs = 50
widen_factor = 0.5  # 1.0 for max features


# =========================
# DATA PREPROCESSOR
# =========================
data_preprocessor = dict(
    type=ClsDataPreprocessor,
    mean=[0, 0, 0],
    std=[255.0, 255.0, 255.0],
    to_rgb=True,
    num_classes=num_classes,
)


# =========================
# MODEL
# =========================
model = dict(
    type=ImageClassifier,
    data_preprocessor=data_preprocessor,
    backbone=dict(
        type=MobileNetv2,
        widen_factor=widen_factor,
        rep=False,
    ),
    neck=dict(type=GlobalAveragePooling),
    head=dict(
        type=LinearClsHead,
        in_channels=int(64 * widen_factor),
        num_classes=num_classes,
        loss=dict(
            type=CrossEntropyLoss,
            loss_weight=1.0,
        ),
    ),
)
deploy = model

# =========================
# PIPELINES
# =========================
train_pipeline = [
    dict(type=LoadImageFromFile, imdecode_backend="cv2"),
    dict(type=PackInputs),
]

test_pipeline = [
    dict(type=LoadImageFromFile, imdecode_backend="cv2"),
    dict(type=PackInputs),
]


# =========================
# DATA LOADERS
# =========================
train_dataloader = dict(
    batch_size=batch_size,
    num_workers=num_workers,
    persistent_workers=True,
    dataset=dict(
        type=CustomDataset,
        data_root=data_root,
        data_prefix=dict(img_path="train"),
        pipeline=train_pipeline,
    ),
    sampler=dict(type=DefaultSampler, shuffle=True),
)

val_dataloader = dict(
    batch_size=val_batch_size,
    num_workers=num_workers,
    persistent_workers=True,
    dataset=dict(
        type=CustomDataset,
        data_root=data_root,
        data_prefix=dict(img_path="validation"),
        pipeline=test_pipeline,
    ),
    sampler=dict(type=DefaultSampler, shuffle=False),
)

test_dataloader = val_dataloader


# =========================
# EVALUATION
# =========================
val_evaluator = dict(
    type=Accuracy,
    topk=(1,),
)

test_evaluator = val_evaluator


# =========================
# TRAIN LOOP
# =========================
train_cfg = dict(
    by_epoch=True,
    max_epochs=epochs,
    val_interval=1,
)


# =========================
# LR SCHEDULE
# =========================
param_scheduler = [
    dict(
        type="LinearLR",
        begin=0,
        end=5,
        start_factor=0.001,
        by_epoch=True,
    ),
    dict(
        type="MultiStepLR",
        begin=0,
        end=epochs,
        milestones=[30, 40],
        gamma=0.3,
        by_epoch=True,
    ),
]


# =========================
# HOOKS
# =========================
default_hooks = dict(
    timer=dict(type=IterTimerHook),

    logger=dict(
        type=LoggerHook,
        interval=10,
    ),

    param_scheduler=dict(type=ParamSchedulerHook),

    checkpoint=dict(
        type=CheckpointHook,
        interval=1,
        max_keep_ckpts=5,
        save_best="accuracy/top1",
        rule="greater",
    ),

    sampler_seed=dict(type=DistSamplerSeedHook),

    visualization=dict(
        type=DetVisualizationHook,
        score_thr=0.5,
    ),
)


# =========================
# VISUALIZER + TENSORBOARD
# =========================
visualizer = dict(
    type=UniversalVisualizer,
    vis_backends=[
        dict(type='LocalVisBackend'),
        dict(type='TensorboardVisBackend'),
    ]
)
visualization = visualizer

# =========================
# LOGGING (for TensorBoard + metrics)
# =========================
log_processor = dict(
    type='LogProcessor',
    window_size=50,
    by_epoch=True
)


# =========================
# OTHER
# =========================
find_unused_parameters = True