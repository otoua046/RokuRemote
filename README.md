# Roku Accessible Remote System

## Overview

The **Roku Accessible Remote System** provides an accessible, voice-controlled interface for Roku TVs. It integrates **Amazon Alexa**, **AWS Lambda**, **AWS IoT**, and an **ESP32 microcontroller** to execute user commands such as opening apps, navigating menus, adjusting volume, and more. This system is tailored for users with mobility and voice impairments, emphasizing accessibility and simplicity.

---

## Features

- **Voice-Controlled Commands:** Execute commands like "Amazon, ask my remote to open Netflix" or "Amazon, ask my remote to go up."
- **AWS IoT Integration:** Ensures reliable communication between AWS Lambda and the ESP32.
- **Simple Setup:** The ESP32 supports AP mode for configuration and auto-detection of Roku devices.
- **Accessibility-Focused:** Designed for users with voice impairments, featuring noise-tolerant Alexa recognition.

---

## Repository Contents

This repository contains:
- **ESP32 Code:** Handles communication between AWS IoT and the Roku TV.
- **AWS Lambda Function:** Processes Alexa commands and forwards them to the ESP32 via AWS IoT.
- **Setup Instructions:** Guides for configuring the ESP32, AWS IoT, AWS Lambda, and Alexa Skill.
- **Terms of Use & Privacy Policy:** Details on how to use the system responsibly and securely.

---

## Prerequisites

To use this system, users must set up the following:

1. **Alexa Skill**  
   Create an Alexa Skill in the Amazon Developer Console. This will allow you to invoke commands via Alexa and send requests to your AWS Lambda function. Follow the instructions in the [Alexa Skill Guide](https://developer.amazon.com/en-US/alexa/alexa-skills-kit).

2. **AWS Lambda Function**  
   Deploy a Lambda function using the code provided in this repository. This function processes Alexa requests and communicates with AWS IoT. Refer to the [AWS Lambda Documentation](https://docs.aws.amazon.com/lambda/latest/dg/getting-started.html) for setup steps.

3. **AWS IoT**  
   Set up an AWS IoT Thing to enable secure communication with the ESP32. Download the IoT device credentials (certificate, private key, and root CA) and configure the ESP32 with these credentials.

---

## Setup Instructions

### 1. Configure AWS IoT
1. Create an AWS IoT Thing in the AWS Management Console.
2. Download the **AWS IoT device credentials** (certificate, private key, and root CA).
3. Place these credentials in the `aws_credentials.h` file included in this repository.

### 2. Flash ESP32 with Provided Code
1. Download the ESP32 code from the `esp32/` folder in this repository.
2. Flash the code onto your ESP32 using a tool like [Arduino IDE](https://www.arduino.cc/en/software).
3. During initial setup, connect to the ESP32’s access point (AP mode) and configure your network credentials through the built-in web interface.

### 3. Deploy AWS Lambda Function
1. Download the Lambda code from the `lambda/` folder in this repository.
2. Deploy the function to your AWS account following the steps in the [AWS Lambda Setup Guide](https://docs.aws.amazon.com/lambda/latest/dg/getting-started.html).
3. Link the Lambda function to AWS IoT for publishing commands to the ESP32.

### 4. Configure Alexa Skill
1. Create a new Alexa Skill in the Amazon Developer Console.
2. Use the invocation name `roku remote` (or customize it to your preference).
3. Add intents and link the skill to the deployed AWS Lambda function.

---

### **Setup Demo Video**
For a detailed walkthrough of the setup process, watch the [Setup Demo Video](https://share.icloud.com/photos/001LS_O4mv5t3VmgZr-0Iz4tA).

---

## Workflow

1. **Voice Command:** The user speaks a command to Alexa, such as "Amazon, ask my remote to open Netflix."
2. **Alexa Skill:** Alexa processes the command and sends a request to AWS Lambda.
3. **AWS Lambda:** The Lambda function interprets the request and publishes a command to AWS IoT.
4. **AWS IoT:** AWS IoT securely forwards the command to the ESP32.
5. **ESP32:** The ESP32 receives the command and sends the appropriate HTTP request to the Roku TV using ECP.

---

## Supported Commands

| Command                            | Functionality                          |
|------------------------------------|----------------------------------------|
| "Amazon, ask my remote to go up"   | Moves up in the Roku menu.             |
| "Amazon, ask my remote to open Netflix" | Launches the Netflix app on Roku.      |
| "Amazon, ask my remote to turn off the TV" | Powers off the Roku TV.              |
| "Amazon, ask my remote to volume up" | Increases the volume.                 |
| "Amazon, ask my remote to volume down" | Decreases the volume.                 |

---

## File Structure

```plaintext
.
├── esp32/
│   ├── eps32.ino             # Main code for the ESP32
│   ├── aws_credentials.h     # AWS IoT credentials
├── lambda/
│   ├── lambda_function.py    # AWS Lambda function code
├── docs/
│   ├── Privacy_Policy.pdf    # Privacy policy for the Alexa Skill
│   ├── Terms_of_Use.pdf      # Terms of use for the Alexa Skill
│   └── Setup_Guide.pdf       # Comprehensive setup guide
└── README.md                 # Main documentation for the repository

