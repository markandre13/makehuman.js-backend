#
# since building mediapipe currently fails on newer macOS versions,
# this script uses the pre-compiled mediapipe for python and sends
# face data via udp to the makejuman.js backend
#
# HOW TO USE
#
# python3.12 -m venv python3.12
# source ./python3.12/bin/activate
# python -m pip install mediapipe opencv-python
# curl --output holistic_landmarker.task -q https://storage.googleapis.com/mediapipe-models/holistic_landmarker/holistic_landmarker/float16/1/holistic_landmarker.task
# python ./mediapipe-holistic-to-udp.py
#

import socket
import sys
import array
import mediapipe as mp
import cv2 as cv
from time import time
from itertools import chain

udp_ip = "127.0.0.1"
udp_port = 11112
model_path = './holistic_landmarker.task'

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"sending mediapipe holistic landmarkers to {udp_ip}:{udp_port}")
print("press [Q] to quit")

BaseOptions = mp.tasks.BaseOptions
HolisticLandmarker = mp.tasks.vision.HolisticLandmarker
HolisticLandmarkerOptions = mp.tasks.vision.HolisticLandmarkerOptions
HolisticLandmarkerResult = mp.tasks.vision.HolisticLandmarkerResult
VisionRunningMode = mp.tasks.vision.RunningMode

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
loop = True

# Create a face landmarker instance with the live stream mode:
def print_result(result: any, output_image: mp.Image, timestamp_ms: int):
    global sock, loop

    # @0: face (53 entries)
    if result.face_blendshapes != None and len(result.face_blendshapes) > 0:
        f = list(map(lambda x: x.score, result.face_blendshapes))
        f.append(0) # tongueOut
    else:
        f = [0] * 53

    # pose (165 entries (33*5))
    if len(result.pose_world_landmarks) > 0:
        f.extend(
            list(chain.from_iterable(
                list(map(lambda it: [it.x, it.y, it.z, it.visibility, it.presence], result.pose_world_landmarks))
            ))
        )
    else:
        f.extend([0] * 165)

    # left hand (63 entries (21*3))
    if len(result.left_hand_world_landmarks) > 0:
        f.extend(
            list(chain.from_iterable(
                list(map(lambda it: [it.x, it.y, it.z], result.left_hand_world_landmarks))
            ))
        )
    else:
        f.extend([0] * 63)

    # right hand (63 entries (21*3))
    if len(result.right_hand_world_landmarks) > 0:
        f.extend(
            list(chain.from_iterable(
                list(map(lambda it: [it.x, it.y, it.z], result.right_hand_world_landmarks))
            ))
        )
    else:
        f.extend([0] * 63)

    b = bytes(memoryview(array.array("f", f)))
    b = b + timestamp_ms.to_bytes(8, byteorder=sys.byteorder)

    sock.sendto(b, (udp_ip, udp_port))

options = HolisticLandmarkerOptions(
    base_options=BaseOptions(
        model_asset_path=model_path,
    ),
    output_face_blendshapes=True,
    running_mode=VisionRunningMode.LIVE_STREAM,
    result_callback=print_result)

landmarker = HolisticLandmarker.create_from_options(options)
 
cap = cv.VideoCapture(0)
if not cap.isOpened():
    print("Cannot open camera")
    exit()
while loop:
    ret, frame = cap.read()
 
    # if frame is read correctly ret is True
    if not ret:
        print("Can't receive frame (stream end?). Exiting ...")
        break

    mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=frame)
    landmarker.detect_async(image = mp_image, timestamp_ms = int(time() * 1000))

    # Display the resulting frame
    cv.imshow('frame', frame)
    if cv.waitKey(1) == ord('q'):
        break
 
# When everything done, release the capture
cap.release()
cv.destroyAllWindows()    
