import json
import logging
import http.client
import ssl

from Http_Functions import *

def AlexaLock(request, context):
    header = request["directive"]["header"]
    endpoint = request["directive"]["endpoint"]
    user = GetBearerTokenInfo(request["directive"]["endpoint"]["scope"]["token"])
    dompi_response = QueryExternalHost(header["name"], request, context, user)
    if dompi_response["response"]["Estado"] == "0":
        object_status = "LOCKED"
    else:
        object_status = "UNLOCKED"
        
    timeOfSample = dompi_response["response"]["Ultimo_Update"].replace(" ", "T") + ".00Z"
    properties = [ {
        "namespace": "Alexa.LockController",
        "name": "lockState",
        "value": object_status,
        "timeOfSample": timeOfSample,
        "uncertaintyInMilliseconds": 0
        } ]
    header["namespace"] = "Alexa"
    header["name"] = "Response"
    response = { "event": { "header": header, "endpoint": endpoint, "payload": { } }, "context": { "properties": properties } }
    return response
