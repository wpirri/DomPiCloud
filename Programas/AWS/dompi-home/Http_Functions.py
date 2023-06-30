import json
import logging
import http.client
import ssl

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

def GetBearerTokenInfo(token):
    external_host="api.amazon.com"
    amazon_api_url="/user/profile"

    nocertverify = ssl._create_unverified_context()
    conn = http.client.HTTPSConnection(external_host, context=nocertverify)

    logger.debug("API Request: {}".format(external_host + amazon_api_url))

    autorization = "Bearer " + token
    conn.request("GET", amazon_api_url, headers={"Authorization": autorization})
    response = conn.getresponse()
    response_data = response.read().decode()

    logger.debug("API Response: {}".format(response_data))

    # Convierte de JSON -> Python y devuelve
    return json.loads(response_data)

def QueryExternalHost(fcn, request, context, user):
    external_host="witchblade.com.ar:8443"
    alexa_api_url="/cgi-bin/dompi_cloud_amazon.cgi"

    nocertverify = ssl._create_unverified_context()
    conn = http.client.HTTPSConnection(external_host, context=nocertverify)

    url = alexa_api_url + "/?funcion=" + fcn
    # Convierte a Python -> JSON y formatea en html
    post_body = format( json.dumps( { "data": request, "user": user } ) )

    logger.debug("Host Request: {}".format(post_body))

    conn.request("POST", url, body=post_body)
    response = conn.getresponse()
    response_data = response.read().decode()

    logger.debug("Host Response: {}".format(response_data))

    # Convierte de JSON -> Python y devuelve
    return json.loads(response_data)
