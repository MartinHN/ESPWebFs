String filesDropdownOptions = "";

String listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    filesDropdownOptions = "";
    String listenFiles =
        "<table><tr><th id=\"first_td_th\">List the library: </th><th>";
    listenFiles += dirname;
    listenFiles += "</th></tr>";

    File root   = fs.open(dirname);
    String fail = "";
    if (!root) {
        fail = " the library cannot be opened";
        return fail;
    }
    if (!root.isDirectory()) {
        fail = " this is not a library";
        return fail;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            listenFiles += "<tr><td id=\"first_td_th\">Library: ";
            listenFiles += file.name();

            filesDropdownOptions += "<option value=\"";
            filesDropdownOptions += file.name();
            filesDropdownOptions += "\">";
            filesDropdownOptions += file.name();
            filesDropdownOptions += "</option>";

            listenFiles += "</td><td> - </td></tr>";

            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            listenFiles += "<tr><td id=\"first_td_th\">File: ";
            listenFiles += file.name();

            filesDropdownOptions += "<option value=\"";
            filesDropdownOptions += file.name();
            filesDropdownOptions += "\">";
            filesDropdownOptions += file.name();
            filesDropdownOptions += "</option>";

            listenFiles += " </td><td>\tSize: ";
            listenFiles += convertFileSize(file.size());
            listenFiles += "</td></tr>";
        }
        file = root.openNextFile();
    }
    listenFiles += "</table>";
    return listenFiles;
}

String processor(const String &var) {
    if (var == "ALLOWED_EXTENSIONS_EDIT") {
        return allowedExtensionsForEdit;
    }
    if (var == "SPIFFS_FREE_BYTES") {
        return convertFileSize((SPIFFS.totalBytes() - SPIFFS.usedBytes()));
    }

    if (var == "SPIFFS_USED_BYTES") {
        return convertFileSize(SPIFFS.usedBytes());
    }

    if (var == "SPIFFS_TOTAL_BYTES") {
        return convertFileSize(SPIFFS.totalBytes());
    }

    if (var == "LISTEN_FILES") {
        return listDir(SPIFFS, "/", 0);
    }
    if (var == "MGR_BASE_PATH") {
        return mgrBasePath;
    }

    if (var == "DELETE_FILES") {
        String deleteDropdown = "<select name=\"file_path\" id=\"file_path\">";
        deleteDropdown +=
            "<option value=\"choose\">Select file to delete</option>";
        deleteDropdown += filesDropdownOptions;
        deleteDropdown += "</select>";
        return deleteDropdown;
    }

    return String();
}
