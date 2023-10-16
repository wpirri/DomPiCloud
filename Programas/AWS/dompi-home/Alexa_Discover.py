import json
import logging
import http.client
import ssl

from Http_Functions import *

def AlexaDiscover(request, context):
    header = request["directive"]["header"]
    user = GetBearerTokenInfo(request["directive"]["payload"]["scope"]["token"])
    dompi_response = QueryExternalHost(header["name"], request, context, user)
    endpoints = []
    for dompi_object in dompi_response["response"]:
        # IMPORTANTE #
        # El valor True del objeto "retrievable" debe estar sin comillas
        # El "endpointId" solo permite caracteres alfanuméricos '.' y '-'
        #    no permite '@' ni ASCII extendido (nada de 'n', 'Ñ' o acentos)
        endpointId = dompi_object["Objeto"].replace(" ", "-")
        friendlyName = dompi_object["Objeto"]
        objectIcon = dompi_object["Icono_Apagado"]
        #if dompi_object["Tipo"] == "1":
            # Entradas digitales
        if dompi_object["Tipo"] == "0" or dompi_object["Tipo"] == "3" or dompi_object["Tipo"] == "5":
            # Salidas / Alarma / Pulso
            #if dompi_object["Grupo_Visual"] == "1":
                # Alarma
            if dompi_object["Grupo_Visual"] == "2":
                # Luces - Alexa.PowerController (Luz)
                endpoints.append( { "endpointId": endpointId,
                                    "manufacturerName": "WGP",
                                    "friendlyName": friendlyName,
                                    "description": "Iluminacion",
                                    "displayCategories": ["LIGHT"],
                                    "capabilities": [ 
                                        { "interface": "Alexa.PowerController", "version": "3", "type": "AlexaInterface", "properties":  { "supported": [ { "name": "powerState"} ], "proactivelyReported": True, "retrievable": True } },
                                        { "type": "AlexaInterface", "interface": "Alexa.EndpointHealth", "version": "3.2", "properties": { "supported": [ { "name": "connectivity" } ], "proactivelyReported": True, "retrievable": True } },
                                        { "type": "AlexaInterface", "interface": "Alexa", "version": "3" } ] } )

            if dompi_object["Grupo_Visual"] == "3":
                # Puertas - Alexa.LockController
                endpoints.append( { "endpointId": endpointId,
                                    "manufacturerName": "WGP",
                                    "friendlyName": friendlyName,
                                    "description": "Puerta",
                                    "displayCategories": ["SMARTLOCK"],
                                    "capabilities": [ 
                                        { "interface": "Alexa.LockController", "version": "3", "type": "AlexaInterface", "properties":  { "supported": [ { "name": "lockState"} ], "proactivelyReported": True, "retrievable": True } },
                                        { "type": "AlexaInterface", "interface": "Alexa.EndpointHealth", "version": "3.2", "properties": { "supported": [ { "name": "connectivity" } ], "proactivelyReported": True, "retrievable": True } },
                                        { "type": "AlexaInterface", "interface": "Alexa", "version": "3" } ] } )

            if dompi_object["Grupo_Visual"] == "4":
                # Climatizacion - Alexa.PowerController (Aire acondicionado, Calefactor,Ventilador)
                if objectIcon.startswith('aire'):
                    categoryName = "AIR_CONDITIONER"
                else:
                    if objectIcon.startswith('calef'):
                        categoryName = "THERMOSTAT"
                    else:
                        categoryName = "FAN"

                endpoints.append( { "endpointId": endpointId,
                                    "manufacturerName": "WGP",
                                    "friendlyName": friendlyName,
                                    "description": "Climatizacion",
                                    "displayCategories": [categoryName],
                                    "capabilities": [ 
                                        { "interface": "Alexa.PowerController", "version": "3", "type": "AlexaInterface", "properties":  { "supported": [ { "name": "powerState"} ], "proactivelyReported": True, "retrievable": True } },
                                        { "type": "AlexaInterface", "interface": "Alexa.EndpointHealth", "version": "3.2", "properties": { "supported": [ { "name": "connectivity" } ], "proactivelyReported": True, "retrievable": True } },
                                        { "type": "AlexaInterface", "interface": "Alexa", "version": "3" } ] } )

            #if dompi_object["Grupo_Visual"] == "5":
                # Camaras
            #if dompi_object["Grupo_Visual"] == "6":
                # Riego
        #if dompi_object["Tipo"] == "2":
            # Entradas analogicas
    header["name"] = "Discover.Response"
    response = { "event": { "header": header, "payload": { "endpoints": endpoints } } }
    return response
