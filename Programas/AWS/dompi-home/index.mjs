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

var DompiHomeTurnOn = function(request, context)
{
    // get device ID passed in during discovery
    var requestMethod = request.directive.header.name;
    var responseHeader = request.directive.header;
    responseHeader.namespace = "Alexa";
    responseHeader.name = "Response";
    responseHeader.messageId = responseHeader.messageId + "-R";
    
    luz_cocina_status = 1;

    var contextResult = {
        "properties": [{
            "namespace": "Alexa.PowerController",
            "name": "powerState",
            "value": "ON",
            "timeOfSample": "0000-00-00T00:00:00.00Z", //retrieve from result.
            "uncertaintyInMilliseconds": 50
        },
        {
            "namespace": "Alexa.EndpointHealth",
            "name": "connectivity",
            "value": {
            "value": "OK"
        },
        "timeOfSample": "0000-00-00T00:00:00.00Z",
        "uncertaintyInMilliseconds": 0
        }]
    };
    var response = {
        context: contextResult,
        event: {
            header: responseHeader,
            endpoint: {
                scope: {
                    type: "BearerToken",
                    token: request.directive.payload.accessToken,
                },
                endpointId: request.directive.payload.applianceId,
            },
            payload: {}
        }
    };

    context.succeed(response);
}

var DompiHomeTurnOff = function(request, context)
{
    // get device ID passed in during discovery
    var requestMethod = request.directive.header.name;
    var responseHeader = request.directive.header;
    responseHeader.namespace = "Alexa";
    responseHeader.name = "Response";
    responseHeader.messageId = responseHeader.messageId + "-R";
    
    luz_cocina_status = 0;

    var contextResult = {
        "properties": [{
            "namespace": "Alexa.PowerController",
            "name": "powerState",
            "value": "OFF",
            "timeOfSample": "0000-00-00T00:00:00.00Z", //retrieve from result.
            "uncertaintyInMilliseconds": 50
        },
        {
            "namespace": "Alexa.EndpointHealth",
            "name": "connectivity",
            "value": {
            "value": "OK"
        },
        "timeOfSample": "0000-00-00T00:00:00.00Z",
        "uncertaintyInMilliseconds": 0
        }]
    };
    var response = {
        context: contextResult,
        event: {
            header: responseHeader,
            endpoint: {
                scope: {
                    type: "BearerToken",
                    token: request.directive.payload.accessToken,
                },
                endpointId: request.directive.payload.applianceId,
            },
            payload: {}
        }
    };

    context.succeed(response);
}

var DompiHomeStatus = function(request, context)
{
    // get device ID passed in during discovery
    var requestMethod = request.directive.header.name;
    var responseHeader = request.directive.header;
    responseHeader.namespace = "Alexa";
    responseHeader.name = "Response";
    responseHeader.messageId = responseHeader.messageId + "-R";

    var strStatus = "OFF"

    if(luz_cocina_status == 1) strStatus = "ON";

    var contextResult = {
        "properties": [{
            "namespace": "Alexa",
            "name": "powerState",
            "value": strStatus,
            "timeOfSample": "0000-00-00T00:00:00.00Z", //retrieve from result.
            "uncertaintyInMilliseconds": 50
        },
        {
            "namespace": "Alexa.EndpointHealth",
            "name": "connectivity",
            "value": {
            "value": "OK"
        },
        "timeOfSample": "0000-00-00T00:00:00.00Z",
        "uncertaintyInMilliseconds": 0
        }]
    };
    var response = {
        context: contextResult,
        event: {
            header: responseHeader,
            endpoint: {
                scope: {
                    type: "BearerToken",
                    token: request.directive.payload.accessToken,
                },
                endpointId: request.directive.payload.applianceId,
            },
            payload: {}
        }
    };

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
        // Discovery
        request.directive.header.name = "Discover.Response";

        var header = request.directive.header;
        DompiHomeDiscover(header, context);

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
    LogMessage("[dompi-home]", "context at response:",  JSON.stringify(context));
};
