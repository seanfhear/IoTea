# Creating a Telegram Bot

### Prerequisites
- Python3
- ``npm serverless`` package
- AWS account credentials with environment variables ``AWS_ACCESS_KEY_ID`` and ``AWS_SECRET_ACCESS_KEY`` correctly configured

#### Follow the steps below to create a bot and deploy it to AWS Lambda

1. Open ``src/handler.py`` and edit the ``DEVICE_ID`` value and the ``WHITELIST`` value as appropriate. Only whitelisted accounts will be able to issue commands to the bot.
2. Open ``src/serverless.yml`` and replace the ``service`` value with ```bot-{PlantID}```

    Example:

    ```service: bot-yoshi```

3. In ``src/serverless.yml``, replace functions -> handle_message -> events -> httpApi -> path with ```/bot-{PlantID}```

    Example:

    ```
    functions: 
        handle_message:
            events:
                httpApi:
                    path: /bot-yoshi
    ```

4. In the `src` directory, install the bot's requirements into a ``vendored`` directory that can be accessed by the Lambda function.

    ```pip install -r requirements.txt -t vendored```

5. Follow the steps outlined in the [Telegram documentation](https://core.telegram.org/bots) to create a Telegram Bot and take note of the token your bot is given.

6. Create an environment variable for your token.

    ```
   export TELEGRAM_TOKEN=YOUR_BOT_TOKEN
   ```

7. Deploy the Lambda function from the ``src`` directory and take note of the URL that you are given.

    ```
   serverless deploy
   ```
   
   In the output, the URL will be under ```functions -> POST``` and should end with bot-{PlantID}
   
8. Bind your Telegram bot to your new Lambda function by registering a Webhook with Telegram. This step will require your bot's token and the URL from step 7.

    ```
   curl --request POST --url https://api.telegram.org/bot{BOT_TOKEN}/setWebhook --header 'content-type: application/json' --data '{"url": "{LAMBDA_URL}"}'
   ```
