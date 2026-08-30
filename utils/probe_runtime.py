import cv2
import numpy as np

net = cv2.dnn.readNetFromONNX("assets/model/yolov10n/yolov10n.onnx")
net.setInput(cv2.dnn.blobFromImage(
    np.zeros((640, 640, 3), dtype=np.uint8), 1.0 / 255.0, (640, 640),
    swapRB=True, crop=False))

out = net.forward()

# --- diagnostic ---
print("output layer(s):", net.getUnconnectedOutLayersNames())
print("output shape:", out.shape)
