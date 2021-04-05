import json
import boto3

def request_all_vitals(event, context):
    client = boto3.client('iot-data', region_name='eu-west-1')

    # Read Plant ID Registration table
    dynamodb = boto3.resource('dynamodb')
    table = dynamodb.Table('iotea_register')
    response = table.scan()

    # For each Plant ID, trigger a vitals request
    for i in response['Items']:
        plantTriggerTopic = 'IoTea/'+i['plantID']+'/triggers'
        response = client.publish(
            topic=plantTriggerTopic,
            qos=1,
            payload=json.dumps({"category":"vitals"})
        )
    return {
        'statusCode': 200,
    }

def water_threshold(event, context):
    if event['moisturePer'] < 25:
        client = boto3.client('iot-data', region_name='eu-west-1')
        plantTriggerTopic = 'IoTea/'+event['plantID']+'/triggers'
        response = client.publish(
            topic=plantTriggerTopic,
            qos=1,
            payload=json.dumps({"category":"pump", "value":5})
        )
        return {
            'statusCode': 200,
            'message': 'Watered '+event['plantID']
        }
    return {
        'statusCode': 200,
        'message': 'No need to water'
    }