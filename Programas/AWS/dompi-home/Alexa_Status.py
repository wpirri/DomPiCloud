import json
import logging
import http.client
import ssl

from Http_Functions import *

def AlexaStatus(request, context):
    header = request["directive"]["header"]
    endpoint = request["directive"]["endpoint"]
    user = GetBearerTokenInfo(request["directive"]["endpoint"]["scope"]["token"])
    dompi_response = QueryExternalHost(header["name"], request, context, user)
    if dompi_response["response"]["Estado"] == "0":
        object_status = "OFF"
    else:
        object_status = "ON"
    timeOfSample = dompi_response["response"]["Ultimo_Update"].replace(" ", "T") + ".00Z"
    properties = [ {
        "namespace": "Alexa.PowerController",
        "name": "powerState",
        "value": object_status,
        "timeOfSample": timeOfSample,
        "uncertaintyInMilliseconds": 0
        } ]
    header["name"] = "StateReport"
    response = { "event": { "header": header, "endpoint": endpoint, "payload": { } }, "context": { "properties": properties } }
    return response
