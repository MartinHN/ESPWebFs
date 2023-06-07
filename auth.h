bool authenticate(AsyncWebServerRequest *request) {
    if (strlen(http_username) == 0) return true;

    if (!request->authenticate(http_username, http_password)) {
        request->requestAuthentication();
        return false;
    }
    return true;
}
