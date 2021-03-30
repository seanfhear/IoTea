import json
import os
import sys

here = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(here, "./vendored"))

import requests

TOKEN = os.environ['TELEGRAM_TOKEN']
BASE_URL = "https://api.telegram.org/bot{}".format(TOKEN)

START_COMMAND = "/start"
STATUS_COMMAND = "/status"


def handle_message(event, context):
    try:
        data = json.loads(event["body"])
        message = str(data["message"]["text"])
        chat_id = data["message"]["chat"]["id"]
        first_name = data["message"]["chat"]["first_name"]

        if message == START_COMMAND:
            handle_start(chat_id, first_name)

        if message == STATUS_COMMAND:
            handle_status(chat_id)

    except Exception as e:
        print(e)

    return {"statusCode": 200}


def handle_start(chat_id, first_name):
    response = "Hello {}".format(first_name)
    data = {"text": response.encode("utf8"), "chat_id": chat_id}
    url = BASE_URL + "/sendMessage"
    requests.post(url, data)


def handle_status(chat_id):
    response = "Getting status..."
    data = {"text": response.encode("utf8"), "chat_id": chat_id}
    url = BASE_URL + "/sendMessage"
    requests.post(url, data)
