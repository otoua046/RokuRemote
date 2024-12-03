#ifndef AWS_CREDENTIALS_H
#define AWS_CREDENTIALS_H

// MQTT Broker details
const char* mqtt_server = "a1ifmp9k0l7mdx-ats.iot.us-east-1.amazonaws.com";
const int mqtt_port = 8883;
const char* mqtt_topic = "iot/roku/control";

// Root CA certificate
const char* root_ca = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

// Device certificate
const char* device_cert = R"EOF(
-----BEGIN CERTIFICATE-----
<Your Device Certificate>
-----END CERTIFICATE-----
)EOF";

// Private key
const char* private_key = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
<Your Private Key>
-----END RSA PRIVATE KEY-----
)EOF";

#endif // AWS_CREDENTIALS_H