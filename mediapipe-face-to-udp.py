#
# since building mediapipe currently fails on newer macOS versions,
# this script uses the pre-compiled mediapipe for python and sends
# face data via udp to the makejuman.js backend
#
# HOW TO USE
#
# python3.12 -m venv python3.12
# . ./python3.12/bin/activate
# python -m pip install mediapipe
# python -m pip install opencv-python
# curl --output face_landmarker_v2_with_blendshapes.task -q https://storage.googleapis.com/mediapipe-models/face_landmarker/face_landmarker/float16/1/face_landmarker.task
# python ./mediapipe-face-to-udp.py
#

import socket
import sys
import array
import mediapipe as mp
import cv2 as cv
from time import time

udp_ip = "127.0.0.1"
udp_port = 11110
model_path = './face_landmarker_v2_with_blendshapes.task'

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"sending mediapipe facelandmarkers to {udp_ip}:{udp_port}")
print("press [Q] to quit")

BaseOptions = mp.tasks.BaseOptions
FaceLandmarker = mp.tasks.vision.FaceLandmarker
FaceLandmarkerOptions = mp.tasks.vision.FaceLandmarkerOptions
FaceLandmarkerResult = mp.tasks.vision.FaceLandmarkerResult
VisionRunningMode = mp.tasks.vision.RunningMode

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Create a face landmarker instance with the live stream mode:
def print_result(result: any, output_image: mp.Image, timestamp_ms: int):
    global sock

    if (len(result.face_blendshapes) == 0):
        return

    a = list(map(lambda x: x.score, result.face_blendshapes[0]))
    a.append(0) # tongueOut
    a.append(0) # spacer

    a.extend(result.facial_transformation_matrixes[0][0])
    a.extend(result.facial_transformation_matrixes[0][1])
    a.extend(result.facial_transformation_matrixes[0][2])
    a.extend(result.facial_transformation_matrixes[0][3])

    # a.append(timestamp_ms)

    b = bytes(memoryview(array.array("f", a)))
    b = b + timestamp_ms.to_bytes(8, byteorder=sys.byteorder)

    sock.sendto(b, (udp_ip, udp_port))

options = FaceLandmarkerOptions(
    base_options=BaseOptions(
        model_asset_path=model_path,
    ),
    output_face_blendshapes=True,
    output_facial_transformation_matrixes=True,
    running_mode=VisionRunningMode.LIVE_STREAM,
    result_callback=print_result)

landmarker = FaceLandmarker.create_from_options(options)
 
cap = cv.VideoCapture(0)
if not cap.isOpened():
    print("Cannot open camera")
    exit()
while True:
    # Capture frame-by-frame
    ret, frame = cap.read()
 
    # if frame is read correctly ret is True
    if not ret:
        print("Can't receive frame (stream end?). Exiting ...")
        break

    mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=frame)
    landmarker.detect_async(image = mp_image, timestamp_ms = int(time() * 1000))

    # Display the resulting frame
    # cv.imshow('frame', frame)
    if cv.waitKey(1) == ord('q'):
        break
 
# When everything done, release the capture
cap.release()
cv.destroyAllWindows()    
