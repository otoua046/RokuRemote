import json
import logging
import boto3
from botocore.exceptions import ClientError

logger = logging.getLogger()
logger.setLevel(logging.INFO)

# Initialize AWS IoT Data client
iot_client = boto3.client('iot-data', region_name="us-east-1")

# Entry point for the Lambda function
def lambda_handler(event, context):
    logger.info("Event received: %s", json.dumps(event))
    
    try:
        # Route based on Smart Home or Custom Interaction model
        if "directive" in event:  # Smart Home Model
            return handle_smart_home(event)
        elif "request" in event:  # Custom Interaction Model
            return handle_custom_model(event)
        else:
            raise ValueError("Unsupported event structure")
    except Exception as e:
        logger.error("Error handling request: %s", e)
        return generate_error_response("INTERNAL_ERROR", str(e))

# --------------- Smart Home Model Handlers ---------------
def handle_smart_home(event):
    namespace = event['directive']['header']['namespace']
    
    if namespace == "Alexa.Discovery":
        return handle_discovery()
    elif namespace == "Alexa.PowerController":
        return handle_power_control(event)
    elif namespace == "Alexa.StepSpeaker":
        return handle_volume_control(event)
    elif namespace == "Alexa.PlaybackController":
        return handle_playback_control(event)
    else:
        raise ValueError(f"Unsupported Smart Home namespace: {namespace}")

def handle_discovery():
    """Handle Smart Home discovery requests."""
    logger.info("Handling discovery")
    return {
        "event": {
            "header": {
                "namespace": "Alexa.Discovery",
                "name": "Discover.Response",
                "payloadVersion": "3",
                "messageId": "abc-123-def-456"
            },
            "payload": {
                "endpoints": [
                    {
                        "endpointId": "roku_tv",
                        "manufacturerName": "Roku",
                        "friendlyName": "Roku TV",
                        "description": "Smart Roku TV",
                        "displayCategories": ["TV"],
                        "capabilities": [
                            {
                                "type": "AlexaInterface",
                                "interface": "Alexa.PowerController",
                                "version": "3"
                            },
                            {
                                "type": "AlexaInterface",
                                "interface": "Alexa.StepSpeaker",
                                "version": "1"
                            },
                            {
                                "type": "AlexaInterface",
                                "interface": "Alexa.PlaybackController",
                                "version": "1"
                            }
                        ]
                    }
                ]
            }
        }
    }

def handle_power_control(event):
    """Handle Smart Home power control requests."""
    directive = event['directive']
    name = directive['header']['name']
    
    if name == "TurnOn":
        command = "PowerOn"
    elif name == "TurnOff":
        command = "PowerOff"
    else:
        raise ValueError(f"Unsupported PowerController command: {name}")
    
    publish_to_iot("iot/roku/control", command)
    return generate_smart_home_response(name)

def handle_volume_control(event):
    """Handle Smart Home volume control requests."""
    directive = event['directive']
    name = directive['header']['name']
    
    if name == "VolumeUp":
        command = "volume up"
    elif name == "VolumeDown":
        command = "volume down"
    else:
        raise ValueError(f"Unsupported StepSpeaker command: {name}")
    
    publish_to_iot("iot/roku/control", command)
    return generate_smart_home_response(name)

def handle_playback_control(event):
    """Handle Smart Home playback control requests."""
    directive = event['directive']
    name = directive['header']['name']
    
    if name == "Play":
        command = "keypress play"
    elif name == "Pause":
        command = "keypress pause"
    elif name == "Stop":
        command = "keypress stop"
    else:
        raise ValueError(f"Unsupported PlaybackController command: {name}")
    
    publish_to_iot("iot/roku/control", command)
    return generate_smart_home_response(name)

# Generate Smart Home responses
def generate_smart_home_response(name):
    return {
        "context": {},
        "event": {
            "header": {
                "namespace": "Alexa",
                "name": "Response",
                "messageId": "abc-123-def-456",
                "correlationToken": "correlation-token",
                "payloadVersion": "3"
            },
            "endpoint": {
                "endpointId": "roku_tv"
            },
            "payload": {}
        }
    }

# --------------- Custom Interaction Model Handlers ---------------
def handle_custom_model(event):
    intent_name = event['request']['intent']['name']
    if intent_name == "NavigateUpIntent":
        command = "Up"
    elif intent_name == "NavigateDownIntent":
        command = "Down"
    elif intent_name == "NavigateLeftIntent":
        command = "Left"
    elif intent_name == "NavigateRightIntent":
        command = "Right"
    elif intent_name == "SelectIntent":
        command = "Select"
    elif intent_name == "BackIntent":
        command = "Back"
    elif intent_name == "HomeIntent":
        command = "Home"
    elif intent_name == "LaunchAppIntent":
        app_name = event['request']['intent']['slots']['appName']['value']
        command = app_name
    else:
        raise ValueError(f"Unsupported intent: {intent_name}")
    
    publish_to_iot("iot/roku/control", command)
    return generate_custom_response(command)

def generate_custom_response(command):
    return {
        "version": "1.0",
        "response": {
            "outputSpeech": {
                "type": "PlainText",
                "text": f"Command '{command}' sent to your Roku TV."
            },
            "shouldEndSession": True
        }
    }

# --------------- AWS IoT Integration ---------------
def publish_to_iot(topic, command):
    """Publish the command to AWS IoT."""
    logger.info(f"Publishing to IoT topic '{topic}' with command '{command}'")
    try:
        payload = {
            "command": command
        }
        response = iot_client.publish(
            topic=topic,
            qos=1,
            payload=json.dumps(payload)
        )
        logger.info(f"IoT publish response: {response}")
    except ClientError as e:
        logger.error(f"Failed to publish to IoT: {e}")
        raise