# Setting up Automation

Automation allows for automatic watering of the plant based on received vitals requests.

### Prerequisites
- Python3
- ``npm serverless`` package
- AWS account credentials with environment variables ``AWS_ACCESS_KEY_ID`` and ``AWS_SECRET_ACCESS_KEY`` correctly configured

#### Steps to deploy

1. Deploy the Lambda function using serverless.

    ```
   serverless deploy
   ```

2. Configure appropriate IAM permissions on the lambda via browser to access IoT core and DynamoDB.
