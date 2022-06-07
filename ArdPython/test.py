from cvzone.SerialModule import SerialObject
from cvzone.HandTrackingModule import HandDetector
import cv2
import time
import serial



#port = serial.Serial('COM15', 9600)
pTime = 0
cTime = 0
period = 1000
lClose = [1, 0, 0, 0, 0]
lOpen = [0, 1, 1, 1, 1]

suc = True
cap = cv2.VideoCapture(0)
wCam, hCam = 1024, 768
cap.set(3, wCam)
cap.set(4, hCam)
detector = HandDetector(maxHands=2, detectionCon=0.7)
#mySerial = SerialObject("COM19", 9600, 1)

while True:
    success, img = cap.read()
    hands, img = detector.findHands(img, flipType=False)
    if suc:
        cv2.putText(img, "захват", (500, 50), cv2.FONT_HERSHEY_COMPLEX, 1, (0, 255, 0), 2)

    # hands = detector.findHands(img, draw=False) #no draw

    if hands:
        suc = False
        cv2.putText(img, "цель", (500, 50), cv2.FONT_HERSHEY_COMPLEX, 1, (100, 0, 255), 2)
        hand1 = hands[0]
        lmList1 = hand1["lmList"]  # list of 21 landmarks points
        bbox1 = hand1["bbox"]
        centerPoint1 = hand1["center"]  # center of the hand cx cy
        handType = hand1["type"]

        right = detector.fingersUp(hand1)
        cx, cy = hand1['center']
        #cy = centerPoint1[1]
        lOpen.sort()
        lClose.sort()
        right.sort()

        #print(cx, cy)





        #print(lmList1)
        # print(bbox1)
        #print(centerPoint1)
        # print(handType)

        fingers = detector.fingersUp(hand1)
        print(fingers)

        if len(hands)==2:
            hand2 = hands[1]
            lmList2 = hand2["lmList"]  # list of 21 landmarks points
            bbox2 = hand2["bbox"]
            centerPoint2 = hand2["center"]  # center of the hand cx cy
            handType2 = hand2["type"]

            left = detector.fingersUp(hand2)
            c2x, c2y = hand2['center']

            if right == [0, 1, 1, 1, 1]:
                if left == [1, 0, 0, 0, 0]:
                   # port.write('C'.encode())
                   cv2.putText(img, "захват", (10, 70), cv2.FONT_HERSHEY_COMPLEX, 1, (100, 0, 255), 2)
                if left == [0, 1, 1, 1, 1]:
                    cv2.putText(img, "разжать", (10, 70), cv2.FONT_HERSHEY_COMPLEX, 1, (100, 0, 255), 2)
                   # port.write('O'.encode())

            else:

                cv2.putText(img, "ОСТАНОВ", (480, 100), cv2.FONT_HERSHEY_COMPLEX, 1, (100, 0, 255), 2)

        #print(right)
            print(centerPoint1,centerPoint2)
            print(left,right)
    else:
        suc = True




       # mySerial.sendData(fingers)
    cTime = time.time()
    fps = 1 / (cTime - pTime)
    pTime = cTime

    cv2.rectangle(img, (200, 100), (450, 350), (0, 255, 0), 1)
    cv2.putText(img, str(int(fps)), (10, 30), cv2.FONT_HERSHEY_PLAIN, 3, (255, 0, 0), 1)
    cv2.imshow("image", img)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break


