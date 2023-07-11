import json
import logging
import http.client
import ssl

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

from Alexa_Discover import *
from Alexa_Status import *
from Alexa_On import *
from Alexa_Off import *
from Alexa_Lock import *
from Alexa_UnLock import *

def AlexaAuth(request, context):
    response =  {}
    return response

def lambda_handler(request, context):
    # TODO implement
    response =  {}

    logger.debug("Alexa Request: {}".format(request))
    logger.debug("Alexa Context: {}".format(context))

    if request["directive"]["header"]["namespace"] == "Alexa.Discovery" and request["directive"]["header"]["name"] == "Discover":
        response = AlexaDiscover(request, context)
    else:
        if request["directive"]["header"]["namespace"] == "Alexa" and request["directive"]["header"]["name"] == "ReportState":
            response = AlexaStatus(request, context)
        else:
            if request["directive"]["header"]["namespace"] == "Alexa.PowerController":
                if request["directive"]["header"]["name"] == "TurnOn":
                    response = AlexaOn(request, context)
                if request["directive"]["header"]["name"] == "TurnOff":
                    response = AlexaOff(request, context)
            if request["directive"]["header"]["namespace"] == "Alexa.LockController":
                if request["directive"]["header"]["name"] == "Lock":
                    response = AlexaLock(request, context)
                if request["directive"]["header"]["name"] == "Unlock":
                    response = AlexaUnLock(request, context)
            if request["directive"]["header"]["namespace"] == "Alexa.Authorization" and request["directive"]["header"]["name"] == "AcceptGrant":
                response = AlexaAuth(request, context)

    logger.debug("Alexa Response: {}".format(response))

    return response
