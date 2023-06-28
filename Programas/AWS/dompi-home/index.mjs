var luz_cocina_status;


function LogMessage(message, message1, message2) { console.log(message + message1 + message2); }

var DompiHomeDiscover = function(header, context)
{
    var json_data = {
        "endpoints":
        [
            {
                "endpointId": "luz-cocina",
                "manufacturerName": "WGP",
                "friendlyName": "luz cocina",
                "description": "Virtual smart light bulb",
                "displayCategories": ["LIGHT"],
                "additionalAttributes":  {
                    "manufacturer" : "WGP",
                    "model" : "DomPiWeb",
                    "serialNumber": "DPW1",
                    "firmwareVersion" : "1.00",
                    "softwareVersion": "1.00",
                    "customIdentifier": "DPW-1.00"
                },
                "cookie": {
                    "key1": "-",
                    "key2": "-",
                    "key3": "-",
                    "key4": "-"
                },
                "capabilities":
                [
                    {
                        "interface": "Alexa.PowerController",
                        "version": "3",
                        "type": "AlexaInterface",
                        "properties": {
                            "supported": [{
                                "name": "powerState"
                            }],
                             "retrievable": true
                        }
                    },
                    {
                    "type": "AlexaInterface",
                    "interface": "Alexa.EndpointHealth",
                    "version": "3.2",
                    "properties": {
                        "supported": [{
                            "name": "connectivity"
                        }],
                        "retrievable": true
                    }
                },
                {
                    "type": "AlexaInterface",
                    "interface": "Alexa",
                    "version": "3"
                }
                ]
            }
        ]
    };
    
    context.succeed({ event: { header: header, payload: json_data } });    
}

var DompiHomeStatus = function(request, context)
{
    var strStatus = "OFF"

    var response = {
          "event": {
            "header": {
              "namespace": request.directive.header.namespace,
              "name": "StateReport",
              "messageId": request.directive.header.messageId,
              "correlationToken": request.directive.header.correlationToken,
              "payloadVersion": request.directive.header.payloadVersion
            },
            "endpoint": {
              "scope": {
                "type": request.directive.endpoint.scope.type,
                "token": request.directive.endpoint.scope.token
              },
              "endpointId": request.directive.endpointId
            },
            "payload": {}
          },
          "context": {
            "properties": [
              {
                "namespace": "Alexa.PowerController",
                "name": "powerState",
                "value": strStatus,
                "timeOfSample": "2023-06-22T00:00:00.000",
                "uncertaintyInMilliseconds": 0
              }
            ]
          }
        };

    LogMessage("[dompi-home]", "response: ",  JSON.stringify(response));

    context.succeed(response);
}

var DompiHomeTurnOn = function(request, context)
{
    var response = {
          event: {
            header: {
              namespace: "Alexa",
              name: "Response",
              messageId: request.directive.header.messageId,
              correlationToken: request.directive.header.correlationToken,
              payloadVersion: request.directive.header.payloadVersion
            },
            endpoint: {
              scope: {
                type: request.directive.endpoint.scope.type,
                token: request.directive.endpoint.scope.token
              },
              endpointId: request.directive.endpointId
            },
            payload: {}
          },
          context: {
            properties: [
              {
                namespace: "Alexa.PowerController",
                name: "powerState",
                value: "ON",
                timeOfSample: "2023-06-22T00:00:00.000",
                uncertaintyInMilliseconds: 500
              }
            ]
          }
        };

    LogMessage("[dompi-home]", "response: ",  JSON.stringify(response));

    context.succeed(response);
}

var DompiHomeTurnOff = function(request, context)
{
    var response = {
          event: {
            header: {
              namespace: "Alexa",
              name: "Response",
              messageId: request.directive.header.messageId,
              correlationToken: request.directive.header.correlationToken,
              payloadVersion: request.directive.header.payloadVersion
            },
            endpoint: {
              scope: {
                type: request.directive.endpoint.scope.type,
                token: request.directive.endpoint.scope.token
              },
              endpointId: request.directive.endpointId
            },
            payload: {}
          },
          context: {
            properties: [
              {
                namespace: "Alexa.PowerController",
                name: "powerState",
                value: "OFF",
                timeOfSample: "2023-06-22T00:00:00.000",
                uncertaintyInMilliseconds: 500
              }
            ]
          }
        };

    LogMessage("[dompi-home]", "response: ",  JSON.stringify(response));

    context.succeed(response);
}

var DompiHomeAuth = function(request, context)
{
    var payload = {};
    var header = request.directive.header;
    header.name = "AcceptGrant.Response";
    context.succeed({ event: { header: header, payload: payload } });
}

// ============================================================================
// Entry point
// ============================================================================

export const handler = async(request, context) => {
    LogMessage("[dompi-home]", "request: ",  JSON.stringify(request));
    LogMessage("[dompi-home]", "context: ",  JSON.stringify(context));
    
    if(request.directive.header.namespace === 'Alexa.Discovery' && request.directive.header.name === 'Discover')
    {
        DompiHomeDiscover(request, context);
    }
    else if(request.directive.header.namespace === 'Alexa.PowerController')
    {
        if(request.directive.header.name === 'TurnOn')
        {
            DompiHomeTurnOn(request, context);
        }
        else if(request.directive.header.name === 'TurnOff')
        {
            DompiHomeTurnOff(request, context);
        }
    }
    else if(request.directive.header.namespace === 'Alexa' && request.directive.header.name === 'ReportState')
    {
        DompiHomeStatus(request, context);
    }
    else if(request.directive.header.namespace === 'Alexa.Authorization' && request.directive.header.name === 'AcceptGrant')
    {
        DompiHomeAuth(request, context);
    }
};
