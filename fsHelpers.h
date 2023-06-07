
String readFile(fs::FS &fs, const char *path) {
    String fileContent = "";
    File file          = fs.open(path, "r");
    if (!file || file.isDirectory()) {
        return fileContent;
    }
    while (file.available()) {
        fileContent += String((char)file.read());
    }
    file.close();
    return fileContent;
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
    File file = fs.open(path, "w");
    if (!file) {
        return;
    }
    file.print(message);
    file.close();
}

String getFileHash(File &f) {
    if (!f) {
        return {};
    }

    if (f.seek(0, SeekSet)) {
        MD5Builder md5;
        md5.begin();
        md5.addStream(f, f.size());
        md5.calculate();
        return md5.toString();
    }
    return {};
}

void uploadFile(AsyncWebServerRequest *request, String filename, size_t index,
                uint8_t *data, size_t len, bool final) {
    if (!index) {
        request->_tempFile = SPIFFS.open("/" + filename, "w");
    }
    if (len) {
        request->_tempFile.write(data, len);
    }
    if (final) {
        request->_tempFile.close();
        request->redirect(mgrBasePath);
    }
}

String convertFileSize(const size_t bytes) {
    if (bytes < 1024) {
        return String(bytes) + " B";
    } else if (bytes < 1048576) {
        return String(bytes / 1024.0) + " kB";
    } else if (bytes < 1073741824) {
        return String(bytes / 1048576.0) + " MB";
    }
}

String formatFilePathLike(const String &s) {
    if (s.length() == 0) return "/";
    auto res = s;
    if (res.charAt(0) == '.') res = res.substring(1);
    if (res.charAt(0) != '/') res = "/" + res;
    return res;
}

std::vector<String> listFiles(fs::FS &fs, const char *dirname, uint8_t levels) {
    std::vector<String> res;
    File root = fs.open(dirname);
    if (!root) {
        return {"the library cannot be opened"};
    }
    if (!root.isDirectory()) {
        return {"this is not a library"};
    }
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            // listenFiles += file.name();
            // if (levels) {
            //     listDir(fs, file.name(), levels - 1);
            // }
        } else {
            res.push_back(file.name());
        }
        file = root.openNextFile();
    }
    return res;
}
