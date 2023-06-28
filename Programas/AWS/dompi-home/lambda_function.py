import json
import logging
import http.client
import ssl

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)


def QueryExternalHost(fcn, request, context):
    external_host="witchblade.com.ar:8443"
    alexa_api_url="/cgi-bin/dompi_cloud_amazon.cgi"

    nocertverify = ssl._create_unverified_context()
    conn = http.client.HTTPSConnection(external_host, context=nocertverify)

    url = alexa_api_url + "/?funcion=" + fcn
    # Convierte a Python -> JSON y formatea en html
    post_body = format(json.dumps(request))

    logger.debug("Host Request: {}".format(post_body))

    conn.request("POST", url, body=post_body)
    response = conn.getresponse()
    response_data = response.read().decode()

    logger.debug("Host Response: {}".format(response_data))

    # Convierte de JSON -> Python y devuelve
    return json.loads(response_data)

def AlexaDiscover(request, context):
    header = request["directive"]["header"]
    dompi_response = QueryExternalHost(header["name"], request, context)
    endpoints = []
    for dompi_object in dompi_response["response"]:
        #if dompi_object["Tipo"] == "1":
            # Entradas digitales
        if dompi_object["Tipo"] == "0" or dompi_object["Tipo"] == "3" or dompi_object["Tipo"] == "5":
            # Salidas / Alarma / Pulso
            #if dompi_object["Grupo_Visual"] == "1":
                # Alarma
            if dompi_object["Grupo_Visual"] == "2":
                # Luces
                endpointId = dompi_object["Objeto"].replace(" ", "-")
                friendlyName = dompi_object["Objeto"]
                # IMPORTANTE #
                # El valor True del objeto "retrievable" debe estar sin comillas
                # El "endpointId" solo permite caracteres alfanuméricos '.' y '-'
                #    no permite '@' ni ASCII extendido (nada de 'n', 'Ñ' o acentos)
                endpoints.append( { "endpointId": endpointId,
                                    "manufacturerName": "WGP",
                                    "friendlyName": friendlyName,
                                    "description": "Objeto DomPiWeb",
                                    "displayCategories": ["LIGHT"],
                                    "capabilities": [ 
                                        { "interface": "Alexa.PowerController", "version": "3", "type": "AlexaInterface", "properties":  { "supported": [ { "name": "powerState"} ], "retrievable": True } },
                                        { "type": "AlexaInterface", "interface": "Alexa.EndpointHealth", "version": "3.2", "properties": { "supported": [ { "name": "connectivity" } ], "retrievable": True } },
                                        { "type": "AlexaInterface", "interface": "Alexa", "version": "3" } ] } )

            #if dompi_object["Grupo_Visual"] == "3":
                # Puertas
            #if dompi_object["Grupo_Visual"] == "4":
                # Climatizacion
            #if dompi_object["Grupo_Visual"] == "5":
                # Camaras
            #if dompi_object["Grupo_Visual"] == "6":
                # Riego
        #if dompi_object["Tipo"] == "2":
            # Entradas analogicas

    header["name"] = "Discover.Response"
    response = { "event": { "header": header, "payload": { "endpoints": endpoints } } }

    return response

def AlexaStatus(request, context):
    header = request["directive"]["header"]
    endpoint = request["directive"]["endpoint"]

    dompi_response = QueryExternalHost(header["name"], request, context)

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

def AlexaOn(request, context):
    response =  {}
    return response

def AlexaOff(request, context):
    response =  {}
    return response

def AlexaAuth(request, context):
    response =  {}
    return response

def lambda_handler(request, context):
    # TODO implement
    response =  {}

    logger.debug("Alexa Request: {}".format(request))

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
            else:
                if request["directive"]["header"]["namespace"] == "Alexa.Authorization" and request["directive"]["header"]["name"] == "AcceptGrant":
                    response = AlexaAuth(request, context)
    

    logger.debug("Alexa Response: {}".format(response))
    return response
