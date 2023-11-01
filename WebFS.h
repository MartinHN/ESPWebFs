#include "FS.h"
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <ESPmDNS.h>
#include "config.h"
#include "auth.h"
#include "edit_html.h"
#include "manager_html.h"
#include "ok_html.h"
#include "failed_html.h"

#define FORMAT_SPIFFS_IF_FAILED true

namespace WebFS {
#define mgrBasePath "/m"
#define relP(c)     mgrBasePath c

const char *param_file_path = "file_path";

bool rebooting = false;

#include "fsHelpers.h"
#include "htmlHelpers.h"

bool isDoingOTA() { return Update.isRunning(); }
void setupAsyncServer(AsyncWebServer &server) {
  Serial.println("setting  up server");

  server.on(
      relP("/update"), HTTP_POST,
      [](AsyncWebServerRequest *request) {
        rebooting = !Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", rebooting ? ok_html : failed_html);

        response->addHeader("Connection", "close");
        request->send(response);
      },
      [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
          Serial.print("Updating: ");
          Serial.println(filename.c_str());

          if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
            Update.printError(Serial);
          }
        }
        if (!Update.hasError()) {
          if (Update.write(data, len) != len) {
            Update.printError(Serial);
          }
        }
        if (final) {
          if (Update.end(true)) {
            Serial.print("The update is finished: ");
            Serial.println(convertFileSize(index + len));
          } else {
            Update.printError(Serial);
          }
        }
      });

  server.on(relP("/compDate"), HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", __DATE__ " " __TIME__); });

  server.on(
      relP("/upload"), HTTP_POST, [](AsyncWebServerRequest *request) { request->send(200); }, uploadFile);

  server.on(relP("/hash"), HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!authenticate(request))
      return;
    String inputMessage = request->getParam(param_file_path)->value();
    inputMessage = formatFilePathLike(inputMessage);

    Serial.print("getting hash of ");
    Serial.println(inputMessage);
    auto file = SPIFFS.open(inputMessage);
    auto hash = getFileHash(file);
    Serial.print(" is ");
    Serial.println(hash);

    request->send(200, "text/plain", hash);
  });

  server.on(
      relP("/download"), HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!authenticate(request))
          return;

        String inputMessage = request->getParam(param_file_path)->value();
        inputMessage        = formatFilePathLike(inputMessage);

        auto file          = SPIFFS.open(inputMessage);
        String contentType = {};

        request->send(file, inputMessage, contentType /* download=false */
                      /* AwsTemplateProcessor */);
      });

  server.on(relP("/delete"), HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!authenticate(request))
      return;

    String inputMessage = request->getParam(param_file_path)->value();
    inputMessage = formatFilePathLike(inputMessage);
    Serial.print("would delete");
    Serial.println(inputMessage);
    Serial.println(param_file_path);
    if (inputMessage != "choose") {
      if (!SPIFFS.remove(inputMessage.c_str()))
        Serial.print("can't delete");
    }
    request->redirect(mgrBasePath);
  });

  server.on(relP("/ls"), HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!authenticate(request))
      return;

    String inputMessage = request->getParam(param_file_path)->value();
    inputMessage = formatFilePathLike(inputMessage);
    Serial.print("listing ");
    Serial.println(inputMessage);
    auto list = listFiles(SPIFFS, inputMessage.c_str(), 0);
    String res;
    for (const auto &l : list) {
      res += l;
      res += ",";
    }
    if (res.length() > 1)
      res = res.substring(0, res.length() - 1);
    request->send(200, "text/plain", res);
  });

  server.on(relP("/format"), HTTP_POST, [](AsyncWebServerRequest *request) {
    SPIFFS.format();
    request->send(200);
    ESP.restart();
  });

  server.on(mgrBasePath, HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!authenticate(request))
      return;
    request->send_P(200, "text/html", manager_html, processor);
  });

  server.serveStatic("/", SPIFFS, "/", "");

  // CORS Bullshit
  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404, "text/plain", "Page not found");
    }
  });

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Method", "GET, POST, PATCH, PUT, DELETE, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Header", "Origin, Content-Type, X-Auth-Token");

  server.begin();
}

void setup(AsyncWebServer &server, const String &hostName = {},
           bool announceCaps = false) {
    // pinMode(ledPin, OUTPUT);

  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS mount failed!");
    return;
  }

    setupAsyncServer(server);

    if (hostName.length()) {
    MDNS.begin(hostName.c_str());
    }
    if (announceCaps) {
        MDNS.addService("WebFs", "tcp", 80);
    }
}

void loop() {
    if (rebooting) {
        delay(100);
        ESP.restart();
    }

}  // loop end
}  // namespace WebFS
