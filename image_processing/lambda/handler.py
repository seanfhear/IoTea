import os
import sys
import json
import boto3

here = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(here, "./vendored"))

import cv2
import numpy as np
from plantcv import plantcv as pcv
from PIL import Image


def is_plant_leaning(img, threshold=0.05):
    # If image is not in Portrait rotate to ensure center of mass splits the plant correctly
    if img.shape[0] < img.shape[1]:
        img = np.array(Image.fromarray(img).transpose(Image.ROTATE_270))

    # Convert RGB to HSV and extract the saturation channel
    s = pcv.rgb2gray_hsv(rgb_img=img, channel='s')

    # Threshold the saturation image
    s_thresh = pcv.threshold.binary(gray_img=s, threshold=85, max_value=255, object_type='light')

    # Median Blur
    s_mblur = pcv.median_blur(gray_img=s_thresh, ksize=5)

    # Convert RGB to LAB and extract the Blue channel
    b = pcv.rgb2gray_lab(rgb_img=img, channel='b')

    # Threshold the blue image
    b_cnt = pcv.threshold.binary(gray_img=b, threshold=160, max_value=255, object_type='light')

    # Join the thresholded saturation and blue-yellow images
    bs = pcv.logical_or(bin_img1=s_mblur, bin_img2=b_cnt)

    # Apply Mask (for VIS images, mask_color=white)
    masked = pcv.apply_mask(img=img, mask=bs, mask_color='white')

    # Convert RGB to LAB and extract the Green-Magenta and Blue-Yellow channels
    masked_a = pcv.rgb2gray_lab(rgb_img=masked, channel='a')
    masked_b = pcv.rgb2gray_lab(rgb_img=masked, channel='b')

    # Threshold the green-magenta and blue images
    maskeda_thresh = pcv.threshold.binary(gray_img=masked_a, threshold=115, max_value=255, object_type='dark')
    maskeda_thresh1 = pcv.threshold.binary(gray_img=masked_a, threshold=135, max_value=255, object_type='light')
    maskedb_thresh = pcv.threshold.binary(gray_img=masked_b, threshold=128, max_value=255, object_type='light')

    # Join the thresholded saturation and blue-yellow images (OR)
    ab1 = pcv.logical_or(bin_img1=maskeda_thresh, bin_img2=maskedb_thresh)
    ab = pcv.logical_or(bin_img1=maskeda_thresh1, bin_img2=ab1)

    # Fill small objects
    ab_fill = pcv.fill(bin_img=ab, size=200)

    # Apply mask (for VIS images, mask_color=white)
    masked2 = pcv.apply_mask(img=masked, mask=ab_fill, mask_color='white')

    # Identify objects
    id_objects, obj_hierarchy = pcv.find_objects(img=masked2, mask=ab_fill)

    # Define ROI
    roi1, roi_hierarchy = pcv.roi.rectangle(img=masked2, x=300, y=300, h=img.shape[0] - 600, w=img.shape[1] - 600)

    # Decide which objects to keep
    roi_objects, hierarchy3, _, _ = pcv.roi_objects(img=img, roi_contour=roi1,
                                                    roi_hierarchy=roi_hierarchy,
                                                    object_contour=id_objects,
                                                    obj_hierarchy=obj_hierarchy,
                                                    roi_type='partial')

    # Object combine kept objects
    obj, mask = pcv.object_composition(img=img, contours=roi_objects, hierarchy=hierarchy3)

    # ---------------- Analysis ----------------

    # Find shape properties, output shape image (optional)
    _ = pcv.analyze_object(img=img, obj=obj, mask=mask, label="default")

    x, y = pcv.outputs.observations['default']['center_of_mass']['value']
    width = pcv.outputs.observations['default']['width']['value']
    height = pcv.outputs.observations['default']['height']['value']

    x = int(x)
    y = int(y)

    top = max(int(y - (height / 2)), 0)
    bottom = min(int(y + (height / 2)), img.shape[0])

    left = max(int(x - (width / 2)), 0)
    right = min(int(x + (width / 2)), img.shape[1])

    left_half = mask[top:bottom, left:x]
    right_half = mask[top:bottom, x:right]

    left_half_count = cv2.countNonZero(left_half)
    right_half_count = cv2.countNonZero(right_half)

    left_percent = (left_half_count / (left_half_count + right_half_count))
    right_percent = 1 - left_percent

    return abs(left_percent - right_percent) > threshold


def convert_to_image(byte_array):
    # TODO Convert image sent over MQTT to actual image
    return None


def handle_image(event, context):
    if not event['image']:
        return {
            'statusCode': 400,
            'message': 'No image provided in the message'
        }

    img = convert_to_image(event['image'])

    if not img:
        return {
            'statusCode': 400,
            'message': 'Byte array provided could not be interpreted as an image'
        }

    if not is_plant_leaning(img):
        return {
            'statusCode': 200,
            'message': 'No need to turn'
        }
    
    client = boto3.client('iot-data', region_name='eu-west-1')
    response = client.publish(
        topic = f"IoTea/{event['plantID']}/triggers",
        qos=1,
        payload=json.dumps({"category":"turn", "value": 2038})
    )

    return {
        'statusCode': 200,
        'body': response
    }
