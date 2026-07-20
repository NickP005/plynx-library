
#include <WiFi.h>
#include <Update.h>
#include <HTTPClient.h>

String overTheAirURL;

extern PlynxTimer edgentTimer;

PLYNX_WRITE(InternalPinOTA) {
  overTheAirURL = param.asString();

  edgentTimer.setTimeout(2000L, [](){
    // Start OTA
    Plynx.logEvent("sys_ota", "OTA started");

    // Disconnect, not to interfere with OTA process
    Plynx.disconnect();

    PlynxState::set(MODE_OTA_UPGRADE);
  });
}

void enterOTA() {
  PlynxState::set(MODE_OTA_UPGRADE);

  DEBUG_PRINT(String("Firmware update URL: ") + overTheAirURL);

  HTTPClient http;
  http.begin(overTheAirURL);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    DEBUG_PRINT("HTTP response should be 200");
    PlynxState::set(MODE_ERROR);
    return;
  }
  int contentLength = http.getSize();
  if (contentLength <= 0) {
    DEBUG_PRINT("Content-Length not defined");
    PlynxState::set(MODE_ERROR);
    return;
  }

  bool canBegin = Update.begin(contentLength);
  if (!canBegin) {
    DEBUG_PRINT("Not enough space to begin OTA");
    PlynxState::set(MODE_ERROR);
    return;
  }

  Client& client = http.getStream();
  int written = Update.writeStream(client);
  if (written != contentLength) {
    DEBUG_PRINT(String("OTA written ") + written + " / " + contentLength + " bytes");
    PlynxState::set(MODE_ERROR);
    return;
  }

  if (!Update.end()) {
    DEBUG_PRINT("Error #" + String(Update.getError()));
    PlynxState::set(MODE_ERROR);
    return;
  }

  if (!Update.isFinished()) {
    DEBUG_PRINT("Update failed.");
    PlynxState::set(MODE_ERROR);
    return;
  }

  DEBUG_PRINT("=== Update successfully completed. Rebooting.");
  restartMCU();
}

